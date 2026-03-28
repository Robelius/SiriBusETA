#pragma once
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// Returns minutes until next departure, or -1 on error
int getNextDepartureMinutes() {
    HTTPClient http;
    String url = "https://external.transitapp.com/v3/public/stop_departures?global_stop_id=";
    url += BUS_STOP_ID;

    http.begin(url);
    http.addHeader("apiKey", TRANSITAPP_KEY);

    int httpCode = http.GET();
    if (httpCode != 200) {
        http.end();
        return -1;
    }

    String payload = http.getString();
    http.end();

    // Parse JSON
    DynamicJsonDocument doc(8192);
    deserializeJson(doc, payload);

    // TransitApp returns route_departures[] → itineraries[] → departure_time (unix timestamp)
    long nextDeparture = doc["route_departures"][0]["itineraries"][0]["schedule_items"][0]["departure_time"];
    long nowSeconds = time(nullptr);
    int minutesAway = (nextDeparture - nowSeconds) / 60;

    return max(0, minutesAway);
}