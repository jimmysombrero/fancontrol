#include<string>
#include<iostream>
#include "FanController.h"
#include<chrono>
#include<thread>

#define PIN 1
#define SLEEP_TIME 1

int main(int argc, char* argv[]) {
    double mintemp = 33.0, midtemp = 45.0, maxtemp = 75.0;
    double temp;

    if(argc >= 4) {
        mintemp = std::stod(argv[1]);
        midtemp = std::stod(argv[2]);
        maxtemp = std::stod(argv[3]);
    }

    FanController* fanController = new FanController(PIN, mintemp, midtemp, maxtemp);

    //temp lower than 0 means an error
    //sleep for 1 second then check the temp again
    while((temp = fanController->controlFan()) > 0) {
        std::cout << "Current TEMP: " << temp << std::endl;
        std::cout << "Current Fan Speed: " << fanController->speed() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
    }

    delete(fanController);

    return -1;
}

