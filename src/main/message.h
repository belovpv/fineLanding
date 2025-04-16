#include <cstdint>
#include <iostream>
#include <future>
#include <memory>
#include <thread>

namespace fineLanding
{
    class OutMessage
    {
    public:
        static int timestamp();
        OutMessage(const char *name);
        virtual ~OutMessage();
        const char *name;
        virtual std::string toString() = 0;
    };

    class SetProgramControlOutMessage : public OutMessage
    {
    public:
        SetProgramControlOutMessage(bool enable);
        bool enable;
        virtual std::string toString();
    };

    class LandOutMessage : public OutMessage
    {
    public:
        LandOutMessage(bool precision);
        bool precision;
        virtual std::string toString();
    };

    class MoveOutMessage : public OutMessage
    {
    public:
        MoveOutMessage(float yaw, float pitch, float roll, float throttle);
        float yawDegrees;
        float pitchVelocity;
        float rollVelocity;
        float throttleVelocity;
        virtual std::string toString();
    };

    class RequestPositionOutMessage : public OutMessage
    {
    public:
        RequestPositionOutMessage();
        virtual std::string toString();
    };

    class InMessage
    {
    public:
        static InMessage *fromString(std::string &);
        InMessage();
        InMessage(std::string &);
        std::string ToString();
        std::string name;
        bool success;
        std::string error;
    };

    class RequestPositionInMessage : public InMessage
    {
    public:
        RequestPositionInMessage(std::string &);
        double lat;
        double lon;
        double alt;
    };

}; // namespace fineLanding