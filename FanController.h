#include <string>

class FanController {
    private:
        int pin, currentPwmValue;
        std::string mode;
        bool fanRunning;
        double lowTemp, midTemp, highTemp;

    public:
        FanController(int, double, double, double);
        int fanCurve(double);
        double processRawTemp(std::string);
        std::string readTemp();
        int stopFan();
        int startFan();
        double controlFan();
        int setFanSpeed(int);
        void setupGpio(int);   
        int speed();
};
