#include "ClimateSensor.h"

ClimateData ClimateSensor::data;
SensirionI2cSht4x ClimateSensor::sensor;

void readClimateSensor(void *sensor) {
    while (true) {
        int result = ((SensirionI2cSht4x *)sensor)
                         ->measureHighPrecision(ClimateSensor::data.temperature,
                                                ClimateSensor::data.humidity);
        ClimateSensor::data.valid = result == 0;
        vTaskDelay(10000);
    }
}

void ClimateSensor::init() {
    Wire.begin();
    sensor.begin(Wire, SHT41_I2C_ADDR_44);
    sensor.softReset();
    delay(10);
    xTaskCreate(readClimateSensor, "Read Climate Sensor", 2048, &sensor, 1,
                NULL);
}