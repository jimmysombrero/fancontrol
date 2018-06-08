/*    
    Copyright 2018 James Vaughn

    This software useses the wiringPi GPIO library
    More information at http://wiringpi.com

    pifancontrol main.cpp
    The purpose of main.cpp is to check the fan controller
    every SLEEP_TIME seconds.  A signal handler is set up
    to catch SIGINT so that there is a way to kill the 
    program that will stop the fan and clean up memory.
*/

#include<string>
#include<iostream>
#include "FanController.h"
#include<chrono>
#include<thread>
#include<signal.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

//GPIO pin 1 according to https://projects.drogon.net/wiringpi-pin-numbering/
#define PIN 1
#define SLEEP_TIME 1

FanController* fanController;
bool die = false;

//If the signal is SIGINT(ctrl+c) the set the die flag
void signalHandler(int signum) {
    if(signum == SIGINT) {
        die = true;
    }
}

//set up the signal handler the catch SIGINT
//uses sigaction instead of signal()
void setSignalHandler() {
    struct sigaction action;
    
    action.sa_handler = signalHandler;
    
    sigemptyset(&action.sa_mask);
    
    action.sa_flags = 0;
    
    sigaction(SIGINT, &action, NULL);
}

void getOut(FanController* controller) {
    controller->stopFan();
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

    setSignalHandler();

    //temp lower than 0 means an error, so exit the loop
    //clean up memory and return non zero exit
    //sleep for 1 second then check the temp again
    while((temp = fanController->controlFan()) >= 0) {
        if(die) {
            getOut(fanController);
        }

        std::cout << "Current TEMP: " << temp << std::endl;
        std::cout << "Current Fan Speed: " << fanController->speed() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
    }

    delete(fanController);

    return -1;
}

