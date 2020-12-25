//
// Created by Ari Reen on 25/12/2020.
//

#ifndef TTGO_T_HIGROW_READ_SENSORS_H
#define TTGO_T_HIGROW_READ_SENSORS_H

uint32_t readSalt();
uint16_t readSoil();
float readBattery(Config config);

#endif //TTGO_T_HIGROW_READ_SENSORS_H
