#include "main/controller.h"
#include "main/config.h"

using namespace fineLanding;

int main(int argc, char **argv)
{
    // read config file;
    Config config;
    double latHome = config.getDouble("application.lat", 0);
    double lonHome = config.getDouble("application.lon", 0);
    std::string djiIp = config.getString("application.dji_ip", "");
    int djiPort = config.get<int>("application.dji_port", 8080);
    // Init rt system position reading
    std::string rtsPort = config.getString("application.rts_port", "");

    // Position data reading thread
    Position position;
    position.init(rtsPort.c_str());
    std::thread threadPosition(Position::threadBody, std::ref(position));

    // Craft controlling thread
    Controller controller(latHome, lonHome, djiIp.c_str(), djiPort);
    std::thread threadCraftController(Controller::threadBody, std::ref(controller), std::ref(position));
    std::cout << "press any key to exit..." << std::endl;
    std::cin.get();
    position.stop();
    controller.stop();
    threadPosition.join();
    threadCraftController.join();
    std::cout << "программа остановлена" << std::endl;
    return 0;
}
