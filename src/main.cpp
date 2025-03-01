#include <Arduino.h>

#include "ClimateSensor.h"
#include "SmartFan.h"

SpanCharacteristic *temperature;
SpanCharacteristic *humidity;
SmartFan *fan;
int lastHumidity = -1;
int hysteresis = 1;
string modelPrefix = "X97_";
string hostNamePrefix = "SmartFan-";

void setup() {
    Serial.begin(115200);
    ClimateSensor::init();
    string model = modelPrefix + DEVICE_NAME;
    string hostName = hostNamePrefix + model;
    WiFi.setHostname(hostName.c_str());
    homeSpan.begin(Category::Fans, "Smart Fan", "SmartFan", model.c_str());
    homeSpan.enableWebLog();
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

    fan = new SmartFan(FAN_PIN, 50);
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
            if (ClimateSensor::data.humidity >= (90 + hysteresis)) {
                autoFanSpeed = 100;
            } else if (ClimateSensor::data.humidity >= (80 + hysteresis)) {
                autoFanSpeed = 90;
            } else if (ClimateSensor::data.humidity >= (70 + hysteresis)) {
                autoFanSpeed = 80;
            } else if (ClimateSensor::data.humidity >= (60 + hysteresis)) {
                autoFanSpeed = 70;
            } else if (ClimateSensor::data.humidity >= (50 + hysteresis)) {
                autoFanSpeed = 60;
            }
        } else {
            if (ClimateSensor::data.humidity <= (50 - hysteresis)) {
                autoFanSpeed = 50;
            } else if (ClimateSensor::data.humidity <= (60 - hysteresis)) {
                autoFanSpeed = 60;
            } else if (ClimateSensor::data.humidity <= (70 - hysteresis)) {
                autoFanSpeed = 70;
            } else if (ClimateSensor::data.humidity <= (80 - hysteresis)) {
                autoFanSpeed = 80;
            } else if (ClimateSensor::data.humidity <= (90 - hysteresis)) {
                autoFanSpeed = 90;
            } else if (ClimateSensor::data.humidity <= (100 - hysteresis)) {
                autoFanSpeed = 100;
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
