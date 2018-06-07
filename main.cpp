#include<string>
#include<iostream>
#include "FanController.h"
#include<chrono>
#include<thread>
#include<signal.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

#define PIN 1
#define SLEEP_TIME 1

FanController* fanController;
bool die = false;

void signal_handler(int signum) {
    if(signum == SIGINT) {
        die = true;
    }
}

void set_signal_handler() {
    struct sigaction action;
    
    action.sa_handler = signal_handler;
    
    sigemptyset(&action.sa_mask);
    
    action.sa_flags = 0;
    
    sigaction(SIGINT, &action, NULL);
}

void get_out(FanController* controller) {
    std::cout << controller->stopFan();
    delete(controller);
    exit(0);
}
    

int main(int argc, char* argv[]) {
    double mintemp = 33.0, midtemp = 45.0, maxtemp = 75.0;
    double temp;

    if(argc >= 4) {
        mintemp = std::stod(argv[1]);
        midtemp = std::stod(argv[2]);
        maxtemp = std::stod(argv[3]);
    }

    fanController = new FanController(PIN, mintemp, midtemp, maxtemp);

    set_signal_handler();

    //temp lower than 0 means an error
    //sleep for 1 second then check the temp again
    while((temp = fanController->controlFan()) > 0) {
        if(die) {
            get_out(fanController);
        }

        std::cout << "Current TEMP: " << temp << std::endl;
        std::cout << "Current Fan Speed: " << fanController->speed() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
    }

    delete(fanController);

    return -1;
}

