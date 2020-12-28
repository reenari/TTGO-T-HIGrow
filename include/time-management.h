//
// Created by Ari Reen on 25/12/2020.
//

#ifndef TTGO_T_HIGROW_TIME_MANAGEMENT_H
#define TTGO_T_HIGROW_TIME_MANAGEMENT_H

class TimeManagement {
public:
    TimeManagement(int offset);

    struct tm *local();
    String getCurrentDate();
    bool update();


private:
    int thisHour;
    int thisMinute;
    String dayStamp;
    String time;

    NTPClient *timeClient;

};
#endif //TTGO_T_HIGROW_TIME_MANAGEMENT_H
