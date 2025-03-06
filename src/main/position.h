#include <chrono>
#include <cstdint>
#include <iostream>
#include <future>
#include <memory>
#include <thread>

namespace fineLanding
{
    class Position
    {
    public:
        static void threadBody(Position&);
        void stop();
        bool read();
        bool land();

    private:
        bool isWorking;
    };

} // namespace fineLanding