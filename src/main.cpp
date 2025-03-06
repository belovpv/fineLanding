#include "main/controller.h"

using namespace fineLanding;

int main(int argc, char **argv)
{
    // Position data reading thread
    Position position;
    //std::thread threadPosition(Position::threadBody, std::ref(position));
    // Craft controlling thread
    Controller controller;
    std::thread threadCraftController(Controller::threadBody, std::ref(controller), std::ref(position));
    std::cout << "press any key to exit..." << std::endl;
    std::cin.get();
    //position.stop();
    controller.stop();
    //threadPosition.join();
    threadCraftController.join();
    return 0;
}
