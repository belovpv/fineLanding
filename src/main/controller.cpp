#include "controller.h"
#include "config.h"
#include "math.h"
#include "appLog.h"

#include <vector>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace fineLanding
{
    void Controller::threadBody(Controller &controller, Position &position)
    {
        // read config file;
        Config config;
        double latHome = config.getDouble("application.lat", 0);
        double lonHome = config.getDouble("application.lon", 0);
        std::string ip = config.getString("application.dji_ip", "");
        int port = config.get<int>("application.dji_port", 8080);
        using namespace std::chrono_literals;
        // waiting for craft connected
        controller.isWorking = true;
        bool success;
        do
        {
            std::cout << "connecting to craft..." << std::endl;
            success = controller.connect(ip.c_str(), port);
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
                std::cerr << "error response from craft " << inpmsg->error << std::endl;
                return;
            }
        }
        // cycle of moving craft to defined point
        while (controller.isWorking)
        {
            std::this_thread::sleep_for(1000ms);
            // getting craft control command
            OutMessage *pcmd = controller.getCommand(position, latHome, lonHome);
            if (pcmd == NULL)
            {
                controller.isWorking = false;
            }
            else
            {
                controller.sendCommandSync(pcmd);
                InMessage *pMsgIn = controller.readResponse(1000);
                // in place, go out of cycle
                if (dynamic_cast<LandOutMessage *>(pcmd) != nullptr)
                {
                    controller.isWorking = false;
                }
                delete pcmd;
            }
        }
        std::cout << "program stopped" << std::endl;
    }

    void Controller::stop()
    {
        isWorking = false;
        close(_sock);
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
                std::cout << "got craft position: " << lat << " " << lon << std::endl;
                std::cout << "home position: " << latHome << " " << lonHome << std::endl;
                // calculating craft yaw
                double yaw = Math::getDirectionAngle(lat, lon, latHome, lonHome);
                std::cout << "direction angle: " << yaw << std::endl;
                if (yaw > 180)
                {
                    yaw -= 360;
                }
                yaw -= 90;
                double dist = Math::getDistance(lat, lon, latHome, lonHome);
                std::cout << "distance: " << dist << std::endl;
                if (dist <= 1)
                {
                    // in place, land
                    pRet = new LandOutMessage(true);
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
        std::cout << "sending message " << pcmd->name << std::endl;
        std::string sBytes = pcmd->getBytes();
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
            std::cerr << "error creating socket" << std::endl;
            return false;
        }

        // Указываем параметры сервера
        addr.sin_family = AF_INET;   // домены Internet
        addr.sin_port = htons(port); // или любой другой порт...
        addr.sin_addr.s_addr = inet_addr(ip);
        inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);

        if (::connect(_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            std::cerr << "connection failed" << std::endl;
            return false;
        }
        else
        {
            std::cerr << "connection success" << std::endl;
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
        //waiting for data arrived
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(_sock, &rfds);
        struct timeval tv;
        tv.tv_usec = timeout / 1000;
        int rv = select(_sock + 1, &rfds, NULL, NULL, &tv);
        //data arrived or timeout
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