#include <chrono>
#include <cstdint>
#include <iostream>
#include <future>
#include <memory>
#include <thread>

#include "position.h"
#include "message.h"

namespace fineLanding
{
    enum Status
    {
        rts = 1,
        optMove = 2,
        optLand = 3,
        landing = 4
    };

    class Controller
    {
    public:
        static void threadBody(Controller &, Position &);
        Controller(const double, const double, const char*, const int);
        void stop();
        void dispose();
        OutMessage *getCommand(const Position &, double latHome, double lonHome);
        void sendCommandSync(OutMessage *);

    private:
        bool connect(const char *, int);
        InMessage *readResponse(int timeout);
        bool isWorking;
        int _sock;
        Status enStatus;
        const double latHome;
        const double lonHome;
        const char* djiIp;
        const int djiPort;
    };
} // namespace fineLanding