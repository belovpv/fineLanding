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
    class Controller
    {
    public:
        static void threadBody(Controller &, Position &);
        void stop();
        OutMessage* getCommand(const Position &, double latHome, double lonHome);
        void sendCommandSync(OutMessage*);

    private:
        bool connect(const char*, int);
        InMessage* readResponse(int timeout);
        bool isWorking;
        int _sock;
    };
} // namespace fineLanding