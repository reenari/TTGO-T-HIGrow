#include <Arduino.h>
#include <ctime>
#include <NTPClient.h>
#include <main.h>
#include <WiFi.h>
#include <time-management.h>


extern NTPClient timeClient;
String formattedDate;
String dayStamp;
String timeStamp1;

#if 0
void time_management(Config &config) {
    timeClient.setTimeOffset(gmtOffset_sec);
    while (!timeClient.update())
    {
        timeClient.forceUpdate();
    }

    // The formattedDate comes with the following format:
    // 2018-05-28T16:00:13Z
    // We need to extract date and time
    formattedDate = timeClient.getFormattedDate();
    // Extract date
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    dayStamp = dayStamp.substring(5);
    String dateMonth = dayStamp.substring(0, 2);
    String dateDay = dayStamp.substring(3, 5);
    dayStamp = dateDay + "-" + dateMonth;
    config.date = dayStamp;
    // Extract time
    timeStamp1 = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
    config.time = timeStamp1.substring(0, 5);
    // variables needed for DST test
    thisHour = timeClient.getHours();
    thisMinute = timeClient.getMinutes();
    int thisDay = dateDay.toInt();
    int thisMonth = dateMonth.toInt();
    int thisWeekday = timeClient.getDay();
    bool dst = false;

    // Test for DST active
    if (thisMonth == 10 && thisDay < 25 && thisWeekday < 7) {
        dst = true;
    }

    if (thisMonth == 10 && thisDay > 24 && thisWeekday == 7 && thisHour < 2) {
        dst = true;
    }

    if (thisMonth < 10 && thisMonth > 3) {
        dst = true;
    }

    if (thisMonth == 3) {
        dst = true;
        if (thisDay < 25) {
            dst = false;
        } else
            // thisDay > 25
        {
            if (thisWeekday == 7 && thisHour < 2) {
                dst = false;
            } else {
                if (thisWeekday == 7) {
                    dst = true;
                } else {
                    if (thisWeekday < 7) {
                        int checkSum = thisDay - thisWeekday + 7;
                        if (checkSum > 31) {
                            dst = true;
                        } else {
                            dst = false;
                        }
                    }
                }
            }
        }
    }

    if (dst) {
        Serial.println("IN SOMMERTIME");
        timeClient.setTimeOffset(gmtOffset_sec + 3600);
        while (!timeClient.update()) {
            timeClient.forceUpdate();
        }
        // The formattedDate comes with the following format:
        // 2018-05-28T16:00:13Z
        // We need to extract date and time
        formattedDate = timeClient.getFormattedDate();
        // Extract date
        int splitT = formattedDate.indexOf("T");
        dayStamp = formattedDate.substring(0, splitT);
        dayStamp = dayStamp.substring(5);
        String dateMonth = dayStamp.substring(0, 2);
        String dateDay = dayStamp.substring(3, 5);
        dayStamp = dateDay + "-" + dateMonth;
        timeStamp1 = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
    } else {
        Serial.println("IN VINTERTIME");
    }
}
#endif

TimeManagement::TimeManagement(int offset) {

    auto *ntpUDP = new WiFiUDP;
    this->timeClient = new NTPClient(*ntpUDP, offset);
    this->timeClient->forceUpdate();
    setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);
    tzset();

}

bool TimeManagement::update() {
    return this->timeClient->update();
}

struct tm *TimeManagement::local() {
    timeClient->update();
    const long epochTime = timeClient->getEpochTime();
    return localtime(&epochTime);

}

String TimeManagement::getCurrentDate() {
    struct tm *timem= this->local();
    time_t timer = mktime(timem);
    Serial.println(ctime(&timer));
    char buffer [6];
    strftime(buffer, sizeof buffer, "%d-%m", timem);
    Serial.println(buffer);

    return String(buffer);
}
