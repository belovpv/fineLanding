#include "message.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace fineLanding
{
    OutMessage::OutMessage(const char *name1)
    {
        name = name1;
    }

    OutMessage::~OutMessage() {}

    int OutMessage::timestamp()
    {
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        return ms.count();
    }

    SetProgramControlOutMessage::SetProgramControlOutMessage(bool enable1) : OutMessage("SET_PROGRAM_CONTROL")
    {
        enable = enable1;
    }

    std::string SetProgramControlOutMessage::getBytes()
    {
        json js;
        js["name"] = name;
        js["timestamp"] = OutMessage::timestamp();
        js["data"]["enable"] = enable;
        return js.dump();
    }

    LandOutMessage::LandOutMessage(bool precision1) : OutMessage("LAND")
    {
        precision = precision1;
    }

    std::string LandOutMessage::getBytes()
    {
        json js;
        js["name"] = name;
        js["timestamp"] = OutMessage::timestamp();
        js["data"]["precision"] = precision;
        return js.dump();
    }

    MoveOutMessage::MoveOutMessage(float yaw1, float pitch1, float roll1, float throttle1) : OutMessage("MOVE")
    {
        yawDegrees = yaw1;
        pitchVelocity = pitch1;
        rollVelocity = roll1;
        throttleVelocity = throttle1;
    }

    std::string MoveOutMessage::getBytes()
    {
        json js;
        js["name"] = name;
        js["timestamp"] = OutMessage::timestamp();
        js["data"]["yaw"] = yawDegrees;
        js["data"]["pitch"] = pitchVelocity;
        js["data"]["roll"] = rollVelocity;
        js["data"]["throttle"] = throttleVelocity;
        return js.dump();
    }

    RequestStateOutMessage::RequestStateOutMessage() : OutMessage("REQUEST_STATE")
    {
    }

    std::string RequestStateOutMessage::getBytes()
    {
        json js;
        js["name"] = name;
        js["timestamp"] = OutMessage::timestamp();
        return js.dump();
    }

    InMessage::InMessage() {}

    InMessage::InMessage(std::string &sMessage)
    {
        json js = json::parse(sMessage);
        name = js["name"];
        bool success = js["data"]["success"];
        if (!success)
        {
            error = js["data"]["error"];
        }
    }

    RequestStateInMessage::RequestStateInMessage(std::string &sMessage)
    {
        json js = json::parse(sMessage);
        name = js["name"];
        lat = js["data"]["lat"];
        lon = js["data"]["lon"];
    }

    InMessage *InMessage::fromString(std::string &sMessage)
    {
        InMessage *pMessage = NULL;
        try
        {
            json js = json::parse(sMessage);
            std::string sName(js["name"]);
            bool success = js["data"]["success"];
            if (!success)
            {
                pMessage = new InMessage(sMessage);
                return pMessage;
            }

            if (sName == "REQUEST_STATE")
            {
                pMessage = new RequestStateInMessage(sMessage);
            }
            else
            {
                // responses without parameters
                pMessage = new InMessage(sMessage);
            }
        }
        catch (const std::exception &exc)
        {
            std::cerr << "Error parsing message" << exc.what();
            return NULL;
        }
        return pMessage;
    }

}