#ifndef CLIMATE_SENSOR_H
#define CLIMATE_SENSOR_H
#include <SensirionI2cSht4x.h>
#include <Wire.h>

struct ClimateData {
    float temperature;
    float humidity;
    boolean valid;
};

class ClimateSensor {
   private:
    static SensirionI2cSht4x sensor;
    static unsigned long int lastUpdate;

   public:
    static ClimateData data;
    static void init();
};
#endif