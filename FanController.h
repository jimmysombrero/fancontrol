#include <string>

class FanController {
private:
    int pin, currentPwmValue;
    std::string mode;
    bool fanRunning;

public:
    FanController(int, double, double, double);
    double fanCurve(double);
    double processRawTemp(std::string);
    std::string readTemp();
    int stopFan();
    int startFan();
    int controlFan(double, double, double);
    int setFanSpeed(int);
    void setupGpio(int);   
};
