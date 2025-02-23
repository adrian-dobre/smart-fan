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

    SmartFan(int pin) : Service::Fan() {
        pinMode(pin, OUTPUT);
        fan = new LedPin(pin, 50, 64);
        active = new Characteristic::Active(1);
        speed = new Characteristic::RotationSpeed(50);
        state = new Characteristic::CurrentFanState(2);
        xTaskCreate(notifyAutoAdjustment, "notifyAutoAdjustment", 2048, this, 1,
                    NULL);
    }

    void requestSpeedUpdate(int speed) {
        unsigned int sessionTimeLeft = 0;
        if (manualAdjustment) {
            unsigned int timeInManualMode = millis() - lastManualAdjustment;
            if (timeInManualMode > sessionLength) {
                manualAdjustment = false;
            } else {
                sessionTimeLeft = sessionLength - timeInManualMode;
            }
        }
        if (!manualAdjustment) {
            _handleSpeedUpdates(speed, true);
        } else {
            Serial.print(
                "Manual adjustment detected. Skipping auto speed for this "
                "session. Session time left: ");
            Serial.println(sessionTimeLeft);
        }
    }

    boolean update() {
        if (active->updated()) {
            manualAdjustment = true;
            lastManualAdjustment = millis();
            int currentActiveState = active->getVal();
            int newActiveState = active->getNewVal();
            if (!newActiveState && currentActiveState) {
                fan->set(0);
            } else if (newActiveState && !currentActiveState) {
                fan->set(speed->getVal());
            }
        } else if (speed->updated()) {
            manualAdjustment = true;
            lastManualAdjustment = millis();
            _handleSpeedUpdates(speed->getNewVal());
        }
        return true;
    }

    void _handleSpeedUpdates(int newSpeed, bool autoAjusted = false) {
        int currentSpeed = speed->getVal();
        bool adjusted = false;
        if (newSpeed < 30 && newSpeed > 10) {
            newSpeed = 30;
            adjusted = true;
        } else if (newSpeed <= 10) {
            newSpeed = 0;
            adjusted = true;
        }

        if (adjusted || autoAjusted) {
            speedAdjustment = newSpeed;
        }

        if (newSpeed == 0 && (currentSpeed != 0 || active->getVal())) {
            stateAdjustment = 1;
            activeAdjustment = 0;
        } else if (newSpeed != 0 && (currentSpeed == 0 || !active->getVal())) {
            stateAdjustment = 2;
            activeAdjustment = 1;
        }

        fan->set(newSpeed);
    }
};

void notifyAutoAdjustment(void *_smartFan) {
    while (true) {
        vTaskDelay(1000);
        SmartFan *fan = (SmartFan *)_smartFan;
        if (fan->speedAdjustment != -1) {
            if (fan->speed->getVal() != fan->speedAdjustment) {
                fan->speed->setVal(fan->speedAdjustment);
                vTaskDelay(10);
            }
            fan->speedAdjustment = -1;
        }
        if (fan->activeAdjustment != -1) {
            if (fan->active->getVal() != fan->activeAdjustment) {
                fan->active->setVal(fan->activeAdjustment);
                vTaskDelay(10);
            }
            fan->activeAdjustment = -1;
        }
        if (fan->stateAdjustment != -1) {
            if (fan->state->getVal() != fan->stateAdjustment) {
                fan->state->setVal(fan->stateAdjustment);
                vTaskDelay(10);
            }
            fan->stateAdjustment = -1;
        }
    }
}