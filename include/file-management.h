//
// Created by Ari Reen on 25/12/2020.
//

#ifndef TTGO_T_HIGROW_FILE_MANAGEMENT_H
#define TTGO_T_HIGROW_FILE_MANAGEMENT_H

void writeFile(fs::FS & fs, const char * path, const char * message);
String readFile(fs::FS & fs, const char * path);
void listDir(fs::FS & fs, const char * dirname, uint8_t levels);


#endif //TTGO_T_HIGROW_FILE_MANAGEMENT_H
