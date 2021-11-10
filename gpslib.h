#ifndef TELEMETRY_GPS_LIBRARY
#define TELEMETRY_GPS_LIBRARY

//defines

#define MAX_SIZE 1100

//includes

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

//types

typedef enum {false,true} bool;

typedef struct {
	float latitude; //2
	float longitude; //4
} gps_gga_t;

typedef struct {
	uint16_t speed; //5 and 7
} gps_vtg_t;

typedef struct {
	float latitude; //3
	float longitude; //5
	uint16_t speed; //7
} gps_rmc_t;

typedef struct {
	float latitude; //1
	float longitude; //3
} gps_gll_t;

typedef union {
	gps_gga_t gga;
	gps_vtg_t vtg;
	gps_rmc_t rmc;
	gps_gll_t gll;
    
} gps_message_t;

typedef struct {
    gps_message_t* message;

    // 1 for GGA
	// 2 for VTG
	// 3 for RMC
	// 4 for GLL
    short gps_type; 

} gps_t;

//function declaration
void connect_simulated_GPS();
void connect_real_GPS();
void setupGPS();
void finalizeGPS();
void readGPS();
void parseLine(char *line);
short searchType(char *current);
bool parseGGA(short index, char *current, bool *stop);
bool parseVTG(int index, char *current, bool *stop);
bool parseRMC(int index, char* current, bool* stop);
bool parseGLL(short index, char* current, bool* stop);
void printGPS(char* backup);
float convertDegrees(float angle);
int convertSpeed(float decimal);
void createLogFiles();
void csv_log(float latitude, float longitude, uint16_t speed);
void exitHandler(int sig);

#endif