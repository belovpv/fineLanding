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
        static void threadBody(Position &);
        bool init(const char*);
        void stop();
        void dispose();
        bool read();
        bool land();

    private:
        bool readBytes(char* buffer);
        ushort bytesToInt(char* buf);
        int descriptor;
        bool isWorking;
        ushort distance;
        ushort directAngle;
    };

} // namespace fineLanding