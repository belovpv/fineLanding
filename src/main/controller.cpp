#include "controller.h"
#include "math.h"
#include "appLog.h"

#include <vector>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

double M_PER_DEGREE = 40075000 / 360;

namespace fineLanding
{
    void Controller::threadBody(Controller &controller, Position &position)
    {
        using namespace std::chrono_literals;
        // waiting for craft connected
        controller.isWorking = true;
        bool success;
        do
        {
            std::cout << "соединение с БВС..." << std::endl;
            success = controller.connect(controller.djiIp, controller.djiPort);
            if (!success)
            {
                std::this_thread::sleep_for(5000ms);
            }
        } while (!success && controller.isWorking);

        if (controller.isWorking)
        {
            // set craft program control
            SetProgramControlOutMessage msg(true);
            controller.sendCommandSync(&msg);
            InMessage *inpmsg = controller.readResponse(1000);
            if (inpmsg == NULL)
            {
                return;
            }
            else if (!inpmsg->success)
            {
                std::cerr << "Ошибка ответа БВС " << inpmsg->error << std::endl;
                return;
            }
        }
        controller.enStatus = Status::rts;
        // cycle of moving craft to defined point
        while (controller.isWorking)
        {
            std::this_thread::sleep_for(1000ms);
            // getting craft control command
            OutMessage *pcmd = controller.getCommand(position, controller.latHome, controller.lonHome);
            if (pcmd == NULL)
            {
                controller.isWorking = false;
            }
            else
            {
                controller.sendCommandSync(pcmd);
                if (dynamic_cast<LandOutMessage *>(pcmd) != nullptr)
                {
                    // waiting for landing for 10 sec and go out of cycle
                    InMessage *pMsgIn = controller.readResponse(10000);
                    controller.isWorking = false;
                }
                else
                {
                    InMessage *pMsgIn = controller.readResponse(1000);
                }
                delete pcmd;
            }
        }
        controller.dispose();

        std::cout << "Поток управления БВС завершен" << std::endl;
    }

    Controller::Controller(const double latHome, const double lonHome, const char *djiIp, const int djiPort) : latHome(latHome),
                                                                                                               lonHome(lonHome),
                                                                                                               djiIp(djiIp),
                                                                                                               djiPort(djiPort)
    {
    }

    void Controller::dispose()
    {
        if (_sock > 0)
        {
            close(_sock);
        }
    }

    void Controller::stop()
    {
        isWorking = false;
    }

    OutMessage *Controller::getCommand(const Position &position, double latHome, double lonHome)
    {
        OutMessage *pRet = NULL;
        // position.getLocation();
        // temporarily get position from craft
        RequestPositionOutMessage cmd;
        sendCommandSync((OutMessage *)&cmd);
        InMessage *pResponse = readResponse(1000);
        if (pResponse != NULL)
        {
            if (pResponse->success)
            {
                double lat = ((RequestPositionInMessage *)pResponse)->lat;
                double lon = ((RequestPositionInMessage *)pResponse)->lon;

                double alt = ((RequestPositionInMessage *)pResponse)->alt;
                // calculating craft yaw
                double yaw = Math::getDirectionAngle(lat, lon, latHome, lonHome);
                // Угол направления для БВС
                yaw -= 90;
                if (yaw > 180)
                {
                    yaw -= 360;
                }
                double dist = Math::getDistance(lat, lon, latHome, lonHome);
                if (dist < 50 && dist > 2)
                {
                    if (enStatus == Status::rts)
                    {
                        std::cout << "Подтверждение захвата оптической системой" << std::endl;
                        enStatus = Status::optMove;
                    }
                }
                else if (dist < 2)
                {
                    if (enStatus == Status::optMove)
                    {
                        std::cout << "БВС находится в точке посадки, готов к посадке" << std::endl;
                        enStatus = Status::optLand;
                    }
                }
                if (alt < 2)
                {
                    if (enStatus == Status::optLand)
                    {
                        std::cout << "Приземление БВС" << std::endl;
                        enStatus = Status::landing;
                    }
                }
                if (enStatus == Status::rts)
                {
                    std::cout << "Получены данные РТС: дирекционный угол" << yaw << " расстояние " << dist << std::endl;
                }
                else if (enStatus == Status::optMove)
                {
                    std::cout << "Получены данные оптической системы: дирекционный угол" << yaw << " расстояние " << dist << std::endl;
                }
                else if (enStatus == Status::optLand)
                {
                    std::cout << "Получены данные оптической системы: отклонение х" << (lon - lonHome) * M_PER_DEGREE << " отклонение у " << (lat - latHome) * M_PER_DEGREE << std::endl;
                }
                std::cout << "Определено положение БВС: " << lat << " " << lon << std::endl;
                std::cout << "Высота: " << alt << std::endl;
                std::cout << "Точка посадки БВС: " << latHome << " " << lonHome << std::endl;
                if (enStatus == Status::optLand)
                {
                    // in place, land
                    pRet = new MoveOutMessage(0, 0, 0, -0.5);
                }
                else if (enStatus == Status::landing)
                {
                    pRet = new LandOutMessage(false);
                }
                else
                {
                    float pitch = dist > 10 ? 0.5 : 0.1;
                    pRet = new MoveOutMessage(yaw, pitch, 0, 0);
                }
            }
            delete pResponse;
        }
        return pRet;
    }

    /**
     * send command to crart with sync waiting of response
     */
    void Controller::sendCommandSync(OutMessage *pcmd)
    {
        std::string sBytes = pcmd->toString();
        std::cout << "Отправка команды " << sBytes << std::endl;
        std::vector<char> bytes(sBytes.begin(), sBytes.end());
        bytes.push_back('\n');
        // bytes.push_back('\0');
        char *c = &bytes[0];
        int n = write(_sock, &bytes[0], bytes.size());
        if (n < 0)
        {
            std::cerr << "error writing to socket" << std::endl;
        }
    }

    bool Controller::connect(const char *ip, int port)
    {
        struct sockaddr_in addr;
        struct hostent *hostinfo;
        _sock = socket(AF_INET, SOCK_STREAM, 0);
        // setting read and write timeout
        struct timeval t;
        t.tv_sec = 1;
        t.tv_usec = 0;
        setsockopt(
            _sock,       // Socket descriptor
            SOL_SOCKET,  // To manipulate options at the sockets API level
            SO_RCVTIMEO, // Specify the receiving or sending timeouts
            &t,          // option values
            sizeof(t));

        if (_sock < 0)
        {
            std::cerr << "ошибка создания сокета" << std::endl;
            return false;
        }

        // Указываем параметры сервера
        addr.sin_family = AF_INET;   // домены Internet
        addr.sin_port = htons(port); // или любой другой порт...
        addr.sin_addr.s_addr = inet_addr(ip);
        inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);

        if (::connect(_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            std::cerr << "ошибка соединения" << std::endl;
            return false;
        }
        else
        {
            std::cerr << "соединение установлено" << std::endl;
            return true;
        }
    }

    InMessage *Controller::readResponse(int timeout)
    {
        const int buffer_size = 1024;
        char buffer[buffer_size + 1];
        bzero(buffer, buffer_size + 1);
        int n = 0;
        std::string sResponse;
        // waiting for data arrived
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(_sock, &rfds);
        struct timeval tv;
        tv.tv_usec = timeout / 1000;
        int rv = select(_sock + 1, &rfds, NULL, NULL, &tv);
        // data arrived or timeout
        n = recv(_sock, buffer, buffer_size, 0);
        while (n > 0)
        {
            sResponse += std::string(buffer);
            if (n >= buffer_size)
            {
                bzero(buffer, buffer_size + 1);
                n = read(_sock, buffer, buffer_size);
            }
            else
            {
                n = 0;
            }
        }
        if (sResponse.length() == 0)
        {
            std::cerr << "No response from server" << std::endl;
            return NULL;
        }
        else
        {
            std::cout << sResponse;
            InMessage *pResponse = InMessage::fromString(sResponse);
            if (pResponse != NULL)
            {
                std::cout << "got message " << pResponse->name << std::endl;
            }
            return pResponse;
        }
    }
}