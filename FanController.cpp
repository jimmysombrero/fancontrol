#include "FanController.h"
#include<wiringPi.h>
#include<iostream>
#include<stdio.h>
#include<string>
#include<sys/types.h>
#include<unistd.h>
#include<cmath>
#include<chrono>
#include<thread>

#define MAX_SPEED 1023
#define MIN_SPEED 750

FanController::FanController(int pin, double low, double mid, double high) {
    this->pin = pin;
    this->mode = mode;    
    this->fanRunning = false;
    this->lowTemp = low;
    this->midTemp = mid;
    this->highTemp = high;

    setupGpio(pin);
}

double FanController::processRawTemp(std::string rawTemp) {
    std::string stemp;
    std::size_t equalspos, tickposition;
    double temp;
    
    equalspos = rawTemp.find("=", 0);
    tickposition = rawTemp.find("'", equalspos);

    stemp = rawTemp.substr(equalspos + 1, tickposition - equalspos);

    try {
        temp = std::stod(stemp);
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Invalid argument in temp: " << ia.what() << std::endl;
        temp = -1;
    }

    return temp;
}

std::string FanController::readTemp() {
	char buffer[256];
	FILE *fp;
    std::string output = "";
	
	std::string command = "/opt/vc/bin/vcgencmd measure_temp";

	FILE *pipe = popen(command.c_str(), "r");

    if(pipe != NULL) {
        while(fgets(buffer, sizeof(buffer), pipe) != NULL) {
            output += buffer;
        }
    } else {
        std::cerr << "There was an error opening the pipe " << errno << std::endl;
    }

    return output;
}

//recommend that low <= ambient that way the fan isn't constantly starting and stopping
double FanController::controlFan() {
    double temp;
    std::string rawTemp;

    rawTemp = readTemp();
    temp = processRawTemp(rawTemp);
    
    //attempt to start the fan if it isn't already going and the temp is too high
    //then wait for the fan to spin up before adjusting the speed
    if(temp >= this->lowTemp && !this->fanRunning) {
        startFan();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    fanCurve(temp);

    return temp;
}

int FanController::fanCurve(double temp) {
    int halfSpeed, speed = 0;

    if(temp < this->lowTemp) {
        stopFan();
    } else if(temp >= this->lowTemp && temp < this->midTemp) {
        speed = setFanSpeed(MIN_SPEED);
    } else if(temp >= this->midTemp && temp < this->highTemp) {
        halfSpeed = (int) std::round((MAX_SPEED -  MIN_SPEED)/2 + MIN_SPEED);
        speed = setFanSpeed(halfSpeed);
    } else if(temp >= this->highTemp) {
        speed = setFanSpeed(MAX_SPEED);
    } else {
        speed = stopFan();
    }

    return speed;
}

void FanController::setupGpio(int pin) {
    wiringPiSetup();
    pinMode(pin, PWM_OUTPUT);
}

int FanController::stopFan() {
    if(this->currentPwmValue > 0) {
        pwmWrite(this->pin, 0);
        this->currentPwmValue = 0;
        this->fanRunning = false;
    }

    return 0;
}

int FanController::setFanSpeed(int speed) {
    if(speed >= MIN_SPEED && speed <= MAX_SPEED && speed != currentPwmValue) {
        pwmWrite(this->pin, speed);
        this->currentPwmValue = speed;
    } 

    return speed;
}

//Start the fan with full power because it's a cheap fan.
//also don't try do anything if the fan is already going
int FanController::startFan() {
    if(this->currentPwmValue == 0) {
        pwmWrite(this->pin, MAX_SPEED);
        this->currentPwmValue = MAX_SPEED;
        this->fanRunning = true;
    }

    return MAX_SPEED; 
}

int FanController::speed() {
    return this->currentPwmValue;
}
