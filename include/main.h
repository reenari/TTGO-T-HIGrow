//
// Created by Ari Reen on 25/12/2020.
//

#ifndef TTGO_T_HIGROW_MAIN_H
#define TTGO_T_HIGROW_MAIN_H

extern const bool logging;
extern const bool  readLogfile;
extern const bool  deleteLogfile;
extern int soil_min;
extern int soil_max;
extern bool calibrate_soil;
extern const bool update_plant_name;
extern String plant_name ;
extern const String device_name;

extern String ssidArr[];
extern int ssidArrNo;

extern const char* ssid; // no need to fill in
extern const char* password;
extern const char* ntpServer;

// Off-sets for time, and summertime. each hour is 3.600 seconds.
extern const long  gmtOffset_sec;

extern const char broker[] ;
extern int        port  ;
extern const char mqttuser[] ; //add eventual mqtt username
extern const char mqttpass[] ; //add eventual mqtt password

extern const uint64_t uS_TO_S_FACTOR;
extern const uint64_t TIME_TO_SLEEP;

extern RTC_DATA_ATTR int sleep5no;
extern RTC_DATA_ATTR String battchargeDate;
extern RTC_DATA_ATTR int battchargeDateCnt;

extern const int led;

#define I2C_SDA 25
#define I2C_SCL 26
#define DHT_PIN 16
#define BAT_ADC 33
#define SALT_PIN 34
#define SOIL_PIN 32
#define BOOT_PIN 0
#define POWER_CTRL 4
#define WAKE_BUTTON 35

//json construct setup
struct Config
{
    String date;
    String time;
    int bootno;
    int sleep5no;
    float lux;
    float temp;
    float humid;
    float soil;
    float salt;
    String saltadvice;
    float bat;
    String batcharge;
    String batchargeDate;
    int batchargeDateCnt;
    float batvolt;
    float batvoltage;
    String rel;
};

#endif //TTGO_T_HIGROW_MAIN_H
