#pragma once
#include "HomeSpan.h"
#include "TransitService.h"

// Forward declaration
struct BusETA;

// The trigger: Shortcut turns this on → fires the API call
struct ETATrigger : Service::Switch {
    Characteristic::On *triggerOn;
    BusETA *sensor;  // pointer to the sensor to update

    ETATrigger(BusETA *s) : Service::Switch() {
        triggerOn = new Characteristic::On(false);
        sensor = s;
    }

    boolean update() override;  // defined below after BusETA
};

// The result: holds minutes until next bus as a temperature value
struct BusETA : Service::TemperatureSensor {
    Characteristic::CurrentTemperature *etaMinutes;

    BusETA() : Service::TemperatureSensor() {
        etaMinutes = new Characteristic::CurrentTemperature(0);
    }
};

// Fetch API, update sensor, turn switch back off
boolean ETATrigger::update() {
    if (triggerOn->getNewVal()) {
        int mins = getNextDepartureMinutes();
        if (mins >= 0) {
            sensor->etaMinutes->setVal(mins);
            Serial.printf("Next bus in %d minutes\n", mins);
        }
        triggerOn->setVal(false);  // auto-reset switch
    }
    return true;
}