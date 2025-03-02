#include "SmartFan.h"
void notifyAutoAdjustment(void *);

SmartFan::SmartFan(int pin, int startSpeed, int frequency) : Service::Fan() {
    pinMode(pin, OUTPUT);
    fan = new LedPin(pin, startSpeed, frequency, true);
    active = new Characteristic::Active(1);
    speed = new Characteristic::RotationSpeed(50);
    state = new Characteristic::CurrentFanState(2);
    xTaskCreate(notifyAutoAdjustment, "notifyAutoAdjustment", 2048, this, 1,
                NULL);
}

void SmartFan::requestSpeedUpdate(int speed) {
    unsigned int sessionTimeLeft = 0;
    if (manualAdjustment) {
        unsigned int timeInManualMode = millis() - lastManualAdjustment;
        bool safetyOverride = speed > 80 && this->speed->getVal() < 80;
        if (timeInManualMode > sessionLength || safetyOverride) {
            if (safetyOverride) {
                WEBLOG("Overriding manual adjustment for safety reasons");
            }
            manualAdjustment = false;
        } else {
            sessionTimeLeft = sessionLength - timeInManualMode;
        }
    }
    if (!manualAdjustment) {
        this->_handleSpeedUpdates(speed, true);
    } else {
        WEBLOG(
            "Manual adjustment detected. Skipping auto speed for this "
            "session. Session time left: %d",
            sessionTimeLeft);
    }
}

boolean SmartFan::update() {
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

void SmartFan::_handleSpeedUpdates(int newSpeed, bool autoAjusted) {
    int currentSpeed = speed->getVal();
    bool adjusted = false;
    if (newSpeed < 15 && newSpeed > 5) {
        newSpeed = 15;
        adjusted = true;
    } else if (newSpeed <= 5) {
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