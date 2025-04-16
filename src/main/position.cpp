#include "position.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sstream>

namespace fineLanding
{
    void Position::threadBody(Position &position)
    {
        using namespace std::chrono_literals;
        position.isWorking = true;
        while (position.isWorking)
        {
            position.read();
            // std::this_thread::sleep_for(1000ms);
        }
        std::cout << "Поток опроса РТС завершен" << std::endl;
        position.dispose();
    }

    bool Position::init(const char *port)
    {
        std::cout << "Открываем порт РТ системы " << port << std::endl;
        descriptor = ::open(port, O_RDONLY);
        if (descriptor > 0)
        {
            std::cout << "Успешно открыт порт РТ системы" << std::endl;
            struct termios tty;
            memset(&tty, 0, sizeof tty);

            /* Error Handling */
            if (tcgetattr(descriptor, &tty) != 0)
            {
                std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
            }

            /* Set Baud Rate */
            cfsetospeed(&tty, B9600);
            cfsetispeed(&tty, B9600);

            /* Setting other Port Stuff */
            // tty.c_cflag &= ~PARENB; // Make 8n1
            // tty.c_cflag &= ~CSTOPB;
            tty.c_cflag &= ~CSIZE;
            tty.c_cflag |= CS8;
            tty.c_cflag &= ~CRTSCTS; // no flow control
            tty.c_lflag = 0;         // no signaling chars, no echo, no canonical processing
            tty.c_oflag = 0;         // no remapping, no delays
            tty.c_cc[VMIN] = 1;      // read doesn't block
            // tty.c_cc[VTIME] = 5;     // 0.5 seconds read timeout

            tty.c_cflag |= CREAD | CLOCAL;                  // turn on READ & ignore ctrl lines
            tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // turn off s/w flow ctrl
            tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
            tty.c_oflag &= ~OPOST;                          // make raw

            /* Flush Port, then applies attributes */
            tcflush(descriptor, TCIFLUSH);

            if (tcsetattr(descriptor, TCSANOW, &tty) != 0)
            {
                std::cout << "Error " << errno << " from tcsetattr" << std::endl;
            }
        }
        else
        {
            std::cerr << "Ошибка открытия порта РТ системы: " << descriptor << std::endl;
        }
        return descriptor > 0;
    }

    void Position::dispose()
    {
        if (descriptor > 0)
        {
            ::close(descriptor);
        }
    }

    void Position::stop()
    {
        isWorking = false;
    }

    bool Position::read()
    {
        // 2 bytes - distance, meters
        // 2 bytes - azimuth * 100, degrees
        bool ret = true;
        const int buffer_size = 1024;
        const int data_len = 4;
        char buffer[buffer_size];
        bzero(buffer, buffer_size);
        // reading 4 bytes
        if (readBytes(buffer))
        {
            // Last bytes
            distance = bytesToInt(buffer);
            directAngle = bytesToInt(&buffer[2]);
            std::cout << "Расстояние " << distance << " Угол " << directAngle << std::endl;
        }
        else
        {
            ret = false;
        }

        return ret;
    }

    bool Position::readBytes(char *buffer)
    {
        char const hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        long lastTime = 0;
        std::cout << "Данные РТС: ";
        for (int idx = 0; idx < 4; idx++)
        {
            std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            lastTime = ms.count();
            int count = ::read(descriptor, &buffer[idx], 1);
            if (count <= 0)
            {
                std::cerr << "РТC: ошибка чтения байта " << count << std::endl;
                return false;
            }
            ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            if (ms.count() - lastTime > 300)
            {
                // it is first byte be read with delay
                if (idx != 0)
                {
                    buffer[0] = buffer[idx];
                    idx = 0;
                }
            }
            std::cout << (hex_chars[(buffer[idx] & 0xF0) >> 4]) << (hex_chars[(buffer[idx] & 0x0F) >> 0]);
        }
        std::cout << std::endl;
        return true;
    }

    bool Position::land()
    {
        return false;
    }

    ushort Position::bytesToInt(char *buf)
    {
        return ushort((unsigned char)(buf[1]) << 8 |
                      (unsigned char)(buf[0]));
    }
}