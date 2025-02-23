#include "HomeSpan.h"

void notifyAutoAdjustment(void *);

struct SmartFan : Service::Fan {
    LedPin *fan;
    SpanCharacteristic *active;
    SpanCharacteristic *speed;
    SpanCharacteristic *state;

    bool manualAdjustment = false;
    unsigned int lastManualAdjustment = 0;
    unsigned int sessionLength = 60 * 60 * 1000;

    int speedAdjustment = -1;
    int stateAdjustment = -1;
    int activeAdjustment = -1;

    SmartFan(int pin, int startSpeed = 50, int frequency = 25000);
    void requestSpeedUpdate(int speed);
    boolean update();
    void _handleSpeedUpdates(int newSpeed, bool autoAjusted = false);
};