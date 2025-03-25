#include "position.h"


namespace fineLanding
{
    void Position::threadBody(Position& position)
    {
        using namespace std::chrono_literals;
        position.isWorking = true;
        while(position.isWorking){
            position.read();
            std::this_thread::sleep_for(1000ms);
        }
    }

    void Position::stop() {
        isWorking = false;
    }

    bool Position::read()
    {
        //2 bytes - distance, meters
        //2 bytes - azimuth * 100, degrees
        return false;
    }

    bool Position::land()
    {
        return false;
    }
}