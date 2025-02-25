#include <Arduino.h>

#include "ClimateSensor.h"
#include "SmartFan.h"

SpanCharacteristic *temperature;
SpanCharacteristic *humidity;
SmartFan *fan;
int lastHumidity = -1;

void setup() {
    Serial.begin(115200);
    ClimateSensor::init();
    WiFi.setHostname("SmartFan-X97");
    homeSpan.begin(Category::Fans, "Smart Fan", "SmartFan", "X97");
    new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new Characteristic::Name("Smart Fan");
    new Characteristic::Manufacturer("Madtek");
    new Characteristic::Model("X97");

    new Service::HumiditySensor();
    humidity = new Characteristic::CurrentRelativeHumidity(10);
    new Characteristic::StatusActive(1);

    new Service::TemperatureSensor();
    temperature = new Characteristic::CurrentTemperature(10);
    new Characteristic::StatusActive(1);

    fan = new SmartFan(23, 50);
    homeSpan.autoPoll();
}

void loop() {
    if (ClimateSensor::data.valid) {
        if (temperature->getVal() != ClimateSensor::data.temperature) {
            temperature->setVal(ClimateSensor::data.temperature);
        }
        if (humidity->getVal() != ClimateSensor::data.humidity) {
            humidity->setVal(ClimateSensor::data.humidity);
        }

        bool humidityIncreasing = false;
        if (ClimateSensor::data.humidity > lastHumidity) {
            humidityIncreasing = true;
        }
        lastHumidity = ClimateSensor::data.humidity;

        int autoFanSpeed = 50;
        if (humidityIncreasing) {
            if (ClimateSensor::data.humidity > 92) {
                autoFanSpeed = 100;
            } else if (ClimateSensor::data.humidity > 82) {
                autoFanSpeed = 90;
            } else if (ClimateSensor::data.humidity > 72) {
                autoFanSpeed = 80;
            } else if (ClimateSensor::data.humidity > 62) {
                autoFanSpeed = 70;
            } else if (ClimateSensor::data.humidity > 52) {
                autoFanSpeed = 60;
            }
        } else {
            if (ClimateSensor::data.humidity < 48) {
                autoFanSpeed = 50;
            } else if (ClimateSensor::data.humidity < 58) {
                autoFanSpeed = 60;
            } else if (ClimateSensor::data.humidity < 68) {
                autoFanSpeed = 70;
            } else if (ClimateSensor::data.humidity < 78) {
                autoFanSpeed = 80;
            } else if (ClimateSensor::data.humidity < 88) {
                autoFanSpeed = 90;
            }
        }

        fan->requestSpeedUpdate(autoFanSpeed);

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
