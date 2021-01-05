#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include <DHT.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <ESP.h>
#include <Button2.h>
#include <ctime>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>

#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>

#include "main.h"
#include "file-management.h"
#include "go-to-deep-sleep.h"
#include "module-parameter-management.h"
#include "read-sensors.h"
#include "connect-to-network.h"
#include "save-configuration.h"
#include "time-management.h"

#include "user-variables.h"

// Logfile on SPIFFS
#include "SPIFFS.h"

//           rel = "2.0;    // Implemented MAC id as unique identifier for the device, at same time device_name is frozen to Tgrow_HIGrow.
//           rel = "2.0.1"; // Implemented "_" + name index for sensor icon. Corrected missing leading zero in HEX address.
//           rel = "2.0.2"; // Implemented automatic search for feaseable WIFI SSID, and connect to this.
//           rel = "3.0.0"; // Implemented Home-Assistant MQTT Autodiscover.
//           rel = "3.0.1"; // Implemented Home-Assistant MQTT Autodiscover, Salt calibration and advice included.
//           rel = "3.0.2"; // DST switch over now works
//           rel = "3.0.3"; // Small error corrections
//           rel = "3.0.4"; // Adapting to HACS frontend card: Battery State Card
//           rel = "3.0.5"; // Implemented name of plant saved to SPIFFS
//           rel = "3.0.6"; // DST calculation was way wrong, corrected now.
//           rel = "4.0.0"; // Changed from Arduino EDI to VS Code - PlatformIO
//           rel = "4.0.1"; // Error correction in connect network
//           rel = "4.0.2"; // Organising subroutines, and functional code snippets.
//           rel = "4.0.3"; // Adding battery charged date, and days since last charge
//           rel = "4.0.4"; // Adding battery charged date, and days since last charge, added to SPIFFS so that data do not dissapear at reboot.
const String rel = "4.0.5"; // Merged change from @reenari, and corrected counter days since last change
// mqtt constants
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Reboot counters
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int sleep5no = 0;
RTC_DATA_ATTR String battchargeDate = "";
RTC_DATA_ATTR int battchargeDateCnt = 0;

Config config;

const int led = 13;

BH1750 lightMeter(0x23); //0x23

DHT dht(DHT_PIN, DHT_TYPE);


// Start Subroutines
//#include <file-management.cpp>
//#include <go-to-deep-sleep.c>
// #include <get-string-value.h>
// #include <read-sensors.cpp>
//#include <save-configuration.cpp>
//#include <connect-to-network.cpp>

Button2 wakeButton;

void sleepHandler(Button2 &b)
{
    Serial.println("Enter Deepsleep ...");
    goToDeepSleep();
}

TimeManagement *timeManagement;

AsyncWebServer webServer(80);
ESPDash dashboard(&webServer);
Card temperature(&dashboard, TEMPERATURE_CARD, "Temperature", "°C");
Card humidity(&dashboard, HUMIDITY_CARD, "Humidity", "%");
Card lux(&dashboard, GENERIC_CARD, "Light", "Lux");
Card soil(&dashboard, HUMIDITY_CARD, "Soil", "%");
Card salt(&dashboard, GENERIC_CARD, "Salt", "%");
Card rrsi(&dashboard, GENERIC_CARD, "RSSI", "dBm");

void setup()
{
  Serial.begin(115200);
  Serial.println("Void Setup");

    esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
    Serial.println(wakeupCause);
    switch (wakeupCause) {
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            Serial.println("not from deep sleep");
            break;
      case ESP_SLEEP_WAKEUP_TIMER:
          Serial.println("wake_timer");
          break;
        case ESP_SLEEP_WAKEUP_EXT1:
            Serial.println("button wake");
            break;
  }

    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    time(&now);
    setenv("TZ", "EET", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    Serial.print("rtos time: ");
    Serial.println(strftime_buf);

    read_configs();
//  #include <module-parameter-management.c>

    // Start WiFi and update time
  connectToNetwork();
  Serial.println(" ");
  Serial.println("Connected to network");
  if (logging)
  {
    writeFile(SPIFFS, "/error.log", "Connected to network \n");
  }

  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());
  //  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  //  timeClient.setTimeOffset(7200);

    timeManagement = new TimeManagement(0);
  // time_management(config);
//#include <time-management.cpp>

    dht.begin();
  if (logging)
  {
    writeFile(SPIFFS, "/error.log", "DHT12 Begin OK! \n");
  }

  //! Sensor power control pin , use deteced must set high
  pinMode(POWER_CTRL, OUTPUT);
  digitalWrite(POWER_CTRL, 1);
  delay(1000);

  bool wireOk = Wire.begin(I2C_SDA, I2C_SCL); // wire can not be initialized at beginng, the bus is busy
  if (wireOk)
  {
    Serial.println(F("Wire ok"));
    if (logging)
    {
      writeFile(SPIFFS, "/error.log", "Wire Begin OK! \n");
    }
  }
  else
  {
    Serial.println(F("Wire NOK"));
  }

   if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE))
  {
    Serial.println(F("BH1750 Advanced begin"));
  }
  else
  {
    Serial.println(F("Error initialising BH1750"));
  }

  float luxRead = lightMeter.readLightLevel(); // 1st read seems to return 0 always
  Serial.print("lux ");
  Serial.println(luxRead);
  delay(2000);
  float t12 = dht.readTemperature(); // Read temperature as Fahrenheit then dht.readTemperature(true)
  config.temp = t12;
  delay(2000);
  float h12 = dht.readHumidity();
  config.humid = h12;
  uint16_t soil = readSoil();
  config.soil = soil;
  uint32_t salt = readSalt();
  config.salt = salt;
  String advice;
  if (salt < 201)
  {
    advice = "needed";
  }
  else if (salt < 251)
  {
    advice = "low";
  }
  else if (salt < 351)
  {
    advice = "optimal";
  }
  else if (salt > 350)
  {
    advice = "too high";
  }
  Serial.println(advice);
  config.saltadvice = advice;

    config.date = timeManagement->getCurrentDate();
    Serial.println(config.date);
  float bat = readBattery(config);
  config.bat = bat;
  config.batcharge = "";
  if (bat > 130)
  {
    config.batcharge = "charging";
    battchargeDate = config.date;
    battchargeDateCnt = 0;
    // Save the data
    SPIFFS.remove("/batinfo.conf");
    String batinfo = String(battchargeDate) + ":" + String(battchargeDateCnt);
    const char* batinfo_write = batinfo.c_str();
    writeFile(SPIFFS, "/batinfo.conf", batinfo_write);
  }
  config.batchargeDate = battchargeDate;

  if (battchargeDate != config.date)
  {
      struct tm *timem = timeManagement->local();
    if (timem->tm_hour == 12 && timem->tm_min == 0)
    {
      battchargeDateCnt += 1;
      // Save the data
      SPIFFS.remove("/batinfo.conf");
      String batinfo = String(battchargeDate) + ":" + String(battchargeDateCnt);
      const char *batinfo_write = batinfo.c_str();
      writeFile(SPIFFS, "/batinfo.conf", batinfo_write);
    }
  }
  config.batchargeDateCnt = battchargeDateCnt;

  if (bat > 100)
  {
    config.bat = 100;
  }

  config.bootno = bootCount;

  luxRead = lightMeter.readLightLevel();
  Serial.print("lux ");
  Serial.println(luxRead);
  config.lux = luxRead;
  config.rel = rel;

  // Create JSON file
  Serial.println(F("Creating JSON document..."));
  if (logging)
  {
    writeFile(SPIFFS, "/error.log", "Creating JSON document...! \n");
  }
  saveConfiguration(config);

  // Go to sleep
  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Go to sleep now
  if (wakeupCause != ESP_SLEEP_WAKEUP_EXT1) {
      delay(1000);
      goToDeepSleep();
  }
  Serial.println("setting button");
  wakeButton.begin(WAKE_BUTTON);
    wakeButton.setLongClickHandler(sleepHandler);
    webServer.begin();
}


void loop()
{
    static const uint64_t interval = 2000;
    static uint64_t timestamp;
    static bool readTemp = true;
    wakeButton.loop();
  //  timeManagement->update();

    if (millis() - timestamp > interval ) {
        timestamp = millis();
        if(readTemp) {
            float t = dht.readTemperature();
            temperature.update(t);
        } else {
            float h = dht.readHumidity();
            humidity.update(h);
        }
        readTemp=!readTemp;
        lux.update(lightMeter.readLightLevel());
        soil.update(readSoil());
        int saltValue = readSalt();
        salt.update(saltValue);

        rrsi.update(WiFi.RSSI());
        dashboard.sendUpdates();
    }


}
