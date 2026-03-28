#include "HomeSpan.h"
#include "secrets.h"
#include "TransitService.h"
#include "BusSensor.h"

void setup() {
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());

    // Sync time (required for comparing departure timestamps)
    configTime(0, 0, "pool.ntp.org");

    // HomeSpan setup
    homeSpan.begin(Category::Sensors, "Bus ETA Sensor");

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name("Bus ETA");
            new Characteristic::Manufacturer("DIY");
            new Characteristic::SerialNumber("BUS-001");
            new Characteristic::Model("ESP32");
            new Characteristic::FirmwareRevision("1.0");
            new Characteristic::Identify();
        BusETA *sensor = new BusETA();
        new ETATrigger(sensor);
}

void loop() {
    homeSpan.poll();
}