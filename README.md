# Siri Bus ETA

An ESP32-based HomeKit accessory that tells you when the next bus is arriving — just say **"Hey Siri, Bus ETA"** and Siri reads it back through your iPhone or HomePod.

## How It Works

The ESP32 runs [HomeSpan](https://github.com/HomeSpan/HomeSpan), which makes it appear as a HomeKit accessory on your iPhone. It exposes two HomeKit services:

- **A Switch** — Used as a trigger. When the Siri Shortcut turns it on, the ESP32 calls the TransitApp API to fetch the next departure time. This is to stay under the TransApp free tier rate limit of 1,000 calls/month.
- **Temperature Sensor** — Stores the result (minutes until next bus) as a numeric value that the Shortcut can read and speak.



The flow once everything is setup

* Say "Hey Siri, Bus ETA"

* Siri Shortcut turns on HomeKit Switch

* ESP32 calls TransitApp API once
  
  * Switch turns off after x seconds

* Stores minutes until next bus in Temperature Sensor as a temperature

* Shortcut reads the value

* Siri speaks the value
  
  ---

## Hardware

- [ESP32S ESP32 ESP-WROOM-32 Development Board](https://www.amazon.com/dp/B0D47K5PRB) with 4mb of memory

---

## Libraries Required

Install all of these via **Arduino IDE → Tools → Manage Libraries**:

| Library                                          | Purpose                          |
| ------------------------------------------------ | -------------------------------- |
| [HomeSpan](https://github.com/HomeSpan/HomeSpan) | HomeKit integration for ESP32    |
| [ArduinoJson](https://arduinojson.org)           | Parsing TransitApp API responses |

ESP32 board support also required — add this URL under **File → Preferences → Additional Boards Manager URLs**:

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

Then install **esp32 by Espressif** via **Tools → Board → Boards Manager**.

---

## Project Structure

```
SiriBusETA/
├── SiriBusETA.ino      ← Main sketch (HomeSpan setup + WiFi)
├── TransitService.h    ← TransitApp API call + JSON parsing
├── BusSensor.h         ← HomeKit accessory definition (Switch + Sensor interaction)
├── secrets.h           ← WiFi credentials, API key, Stop ID (NOT committed here)
└── README.md
```

---

## Setup

### 1. Find Your Stop ID

Use the TransitApp API to find the `global_stop_id` for your nearest bus stop:

```bash
curl -H "apiKey: YOUR_API_KEY" \
  "https://external.transitapp.com/v3/public/nearby_stops?lat=YOUR_LAT&lon=YOUR_LON&max_distance=300" \
  | python3 -m json.tool > nearby_stops.json
```

Look for the `global_stop_id` field next to your stop name (e.g. `"MUNI:12345"`), not the `stop_code`

### 2. Create `secrets.h`

Create a `secrets.h` file in the project folder — this is excluded from git:

```cpp
#pragma once

#define WIFI_SSID       "YourWiFiName"
#define WIFI_PASSWORD   "YourWiFiPassword"
#define TRANSITAPP_KEY  "your_api_key_here"
#define BUS_STOP_ID     "MUNI:XXXXX"   // global_stop_id from step 1
```

### 3. Arduino IDE Settings

You may not need to do this step, but the default parition scheme only allowed for 1.2MB which was slightly too little to upload the required libraries. 

Before uploading, change the partition scheme to fit the sketch:

**Tools → Partition Scheme → Huge APP (3MB No OTA/1MB SPIFFS)**

Then select your board: **Tools → Board → ESP32 Arduino → ESP32 Dev Module**

### 4. Flash the ESP32

Upload the sketch via Arduino IDE.

### 5. Pair with HomeKit

1. Open the **Home app** on your iPhone
2. Tap **+** → **Add Accessory** → **More Options**
3. Select **Bus ETA Sensor**
4. Enter the HomeSpan pairing code: **466-37-726**
5. Complete the pairing wizard

### 6. Set Up the Siri Shortcut

Open the **Shortcuts app** and create a new shortcut with these 5 steps:

| Step | Action                                | Settings                                            |
| ---- | ------------------------------------- | --------------------------------------------------- |
| 1    | **Control [Home accessory]**          | Bus ETA Sensor → Switch → Turn On                   |
| 2    | **Wait**                              | 2 seconds                                           |
| 3    | **Get the state of [Home accessory]** | Bus ETA Sensor → Current Temperature                |
| 4    | **Get Numbers from Input**            | Input: result from step 3                           |
| 5    | **Speak**                             | `"The next bus arrives in [step 4 result] minutes"` |

> Step 4 ("Get Numbers from Input") is in the **Scripting** section. It strips the "°C/°F" unit that HomeKit appends to temperature values, so Siri speaks just the number.

Name the shortcut **`Bus ETA`** (or whatever you want the envoking phrase to be), then tap **Add to Siri** and record the phrase **"Bus ETA"**.

---

## HomePod Support

The shortcut works on HomePod with no extra setup — it syncs automatically via iCloud. To enable it:

1. **Settings → [Your Name] → iCloud** → make sure Shortcuts is toggled on
2. In the **Home app**, tap your HomePod → enable **Personal Requests**

Then just say **"Hey Siri, Bus ETA"** to your HomePod.

---

## How the Code Works

### `TransitService.h`

Makes a GET request to the TransitApp `stop_departures` endpoint, parses the JSON response, and returns the minutes until the next departure as an integer.

### `BusSensor.h`

Defines two HomeKit services:

- `ETATrigger` — a HomeKit Switch. When turned on, its `update()` method triggers, calls `getNextDepartureMinutes()`, stores the result, and resets the switch to off.
- `BusETA` — a HomeKit Temperature Sensor whose `CurrentTemperature` characteristic holds the minutes value.

### `SiriBusETA.ino`

Connects to WiFi, syncs time via NTP (required for comparing Unix timestamps from the API), initialises HomeSpan, and registers both services under a single HomeKit accessory.

---

## API Reference

- [TransitApp API Docs](http://api-doc.transitapp.com/)
- [HomeSpan Getting Started](https://github.com/HomeSpan/HomeSpan/blob/master/docs/GettingStarted.md)
- [ArduinoJson Docs](https://arduinojson.org/v6/doc/)
