/*
    Copyright 2018 James Vaughn

    This software useses the wiringPi GPIO library
    More information at http://wiringpi.com

    pifancontrol FanControl.cpp
    FanControll.cpp contains the method definitions for the FanController class
    The class constructor accepts the GPIO pin number as an int and three temps
    (low, mid and high) as doubles to define the temperature range. The fan
    speed will vary from MIN_SPEED to MAX_SPEED depending on the current CPU
    temp and the provided range. 

    MIN_SPEED is defined at 750 becuase that is the lowest value that would allow
    the fan blades on my fan to reliably spin. MAX_SPEED is the top PWM value as 
    defined at http://wiringpi.com/reference/core-functions/

    Pin numbers are defined at https://projects.drogon.net/wiringpi-pin-numbering/

    Licensed under the MIT License.
*/

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

#define MAX_SPEED 1024
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

//parses the temperature value from the string that was returned
//from the vcgencmd command and converts it to a double
double FanController::processRawTemp(std::string rawTemp) {
    std::string stemp;
    std::size_t equalspos, tickposition;
    double temp;
    
    //find the position of the two characters before and after the temp
    equalspos = rawTemp.find("=", 0);
    tickposition = rawTemp.find("'", equalspos);

    //return the substring between those characters
    stemp = rawTemp.substr(equalspos + 1, tickposition - equalspos);

    //try to convert the parsed string into a double
    //return error code -1 if there is a failure
    try {
        temp = std::stod(stemp);
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Invalid argument in temp: " << ia.what() << std::endl;
        temp = -1;
    }

    return temp;
}

//executes the vcgencmd command to measure the CPU temperature
//and returns the output as a string
std::string FanController::readTemp() {
	char buffer[256];
	FILE *fp;
    std::string output = "";
	
	std::string command = "/opt/vc/bin/vcgencmd measure_temp";
    
    //open a pipe to the command so that we can read the outpu
	FILE *pipe = popen(command.c_str(), "r");

    //if the pipe was successfully opened then read it and 
    //then close the pipe; otherwise log the error and 
    //return and empty string
    if(pipe != NULL) {
        while(fgets(buffer, sizeof(buffer), pipe) != NULL) {
            output += buffer;
        }

        pclose(pipe);
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
    
    //attempt to start the fan if it is stopped and the temp is too high
    //then wait for the fan to spin up before adjusting the speed
    if(temp >= this->lowTemp && !this->fanRunning) {
        startFan();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    fanCurve(temp);

    return temp;
}

//adjust the fan speed based on temperature.  The speeds are MIN_SPEED,
//MAX_SPEED and median(MIN_SPEED, MAX_SPEED)
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

//sets up the wiringPi library and sets the pinMode
//both are required per wiringPi documentation
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

//You can guess what this does, but also it does not do anything
//if the passed speed value is not between MIN_SPEED and MAX_SPEED
//or if it's the same speed that the fan is already going
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
