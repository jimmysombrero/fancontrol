#include "Fancontroller.h"
#include <wiringPi.h>
#include <iostream>
#include <stdio.h>
#include<string>
#include<sys/types.h>
#include<unistd.h>
#include<cmath>
#include<chrono>
#include<thread>

#define MAX_SPEED 1023
#define MIN_SPEED 750

FanController::Fancontroller(int pin) {
    this->pin = pin;
    this->mode = mode;    
    this->fanRunning = false;

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
double FanController::controlFan(double low, double mid, double high) {
    std::string rawTemp;
    double temp;
    int halfSpeed;

    rawTemp = readTemp();
    temp = processRawTemp(rawTemp);
    
    //attempt to start the fan if it isn't already going and the temp is too high
    //then wait for the fan to spin up before adjusting the speed
    if(temp >= low && !this->fanRunning) {
        startFan();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    fanCurve(temp);

    return temp;
}

int FanController::fanCurve(double temp) {
    int halfSpeed, speed = 0;

    if(temp >= low && temp < mid) {
        speed = setfanspeed(min_speed);
    } else if(temp >= mid && temp < high) {{
        halfSpeed = (int) std::round(MAX_SPEED/2);
        speed = setfanspeed(halfspeed);
    } else if(temp >= high) {
        speed = setfanspeed(max_speed);
    } else {
        speed = stopfan();
    }

    return speed;
}

void FanController::setupGpio(int pin) {
    wiringpisetup();
    pinmode(pin, pwm_output);
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

int speed() {
    return this->currentPwmValue;
}
