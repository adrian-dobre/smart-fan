#include <Arduino.h>

#include "ClimateSensor.h"
#include "SmartFan.h"

SpanCharacteristic *temperature;
SpanCharacteristic *humidity;
SmartFan *fan;
int hysteresis = 3;
string modelPrefix = "X97_";
string hostNamePrefix = "SmartFan-";
int defaultFanSpeed = 50;

void setup() {
    // Serial.begin(115200);
    ClimateSensor::init();
    string model = modelPrefix + DEVICE_NAME;
    string hostName = hostNamePrefix + model;
    WiFi.setHostname(hostName.c_str());
    homeSpan.begin(Category::Fans, "Smart Fan", "SmartFan", model.c_str());
    homeSpan.enableWebLog(30);
    new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new Characteristic::Name("Smart Fan");
    new Characteristic::Manufacturer("Madtek");
    new Characteristic::Model(model.c_str());

    new Service::HumiditySensor();
    humidity = new Characteristic::CurrentRelativeHumidity(10);
    new Characteristic::StatusActive(1);

    new Service::TemperatureSensor();
    temperature = new Characteristic::CurrentTemperature(10);
    new Characteristic::StatusActive(1);

    fan = new SmartFan(FAN_PIN, defaultFanSpeed);
    homeSpan.autoPoll();
}

int getFanSpeedForHumidityLevel(int humidity) {
    int currentFanSpeed = fan->speed->getVal();
    if (!fan->active->getVal()) {
        currentFanSpeed = 0;
    }
    if (humidity == 0) {
        if (currentFanSpeed == 0) {
            WEBLOG("Fan OFF, returning default speed");
            return defaultFanSpeed;
        }
        return currentFanSpeed;
    }
    int speedForHumidity = (humidity / 10 + 1) * 10;
    if (speedForHumidity > currentFanSpeed) {
        WEBLOG("Humidity %d, current fan speed %d, target speed %d \n",
               humidity, currentFanSpeed, speedForHumidity);
        return speedForHumidity;
    }
    if (speedForHumidity < currentFanSpeed) {
        if (currentFanSpeed - 10 - humidity > hysteresis) {
            WEBLOG("Humidity %d, current fan speed %d, target speed %d \n",
                   humidity, currentFanSpeed, speedForHumidity);
            return speedForHumidity;
        }
    }
    return currentFanSpeed;
}

void loop() {
    if (ClimateSensor::data.valid) {
        if (temperature->getVal() != ClimateSensor::data.temperature) {
            temperature->setVal(ClimateSensor::data.temperature);
        }
        if (humidity->getVal() != ClimateSensor::data.humidity) {
            humidity->setVal(ClimateSensor::data.humidity);
        }

        fan->requestSpeedUpdate(
            getFanSpeedForHumidityLevel(ClimateSensor::data.humidity));

    } else {
        if (temperature->getVal()) {
            temperature->setVal(0);
        }
        if (humidity->getVal()) {
            humidity->setVal(0);
        }
    }
    delay(10000);
}
