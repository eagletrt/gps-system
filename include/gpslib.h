#ifndef TELEMETRY_GPS_LIBRARY
#define TELEMETRY_GPS_LIBRARY

#include <stdint.h>
#include <stdbool.h>

//defines

#define MAX_LENGTH 100

//types

typedef struct {
	double latitude; //2
	double longitude; //4
} gps_gga_t;

typedef struct {
	uint16_t speed; //5 and 7
} gps_vtg_t;

typedef struct {
	double latitude; //3
	double longitude; //5
	uint16_t speed; //7
} gps_rmc_t;

typedef struct {
	double latitude; //1
	double longitude; //3
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
void pos2can(double latitude, double longitude);
void speed2can(uint16_t speed);
void createLogFiles();
void connect_GPS();
void setupGPS();
void finalizeGPS();
void readGPS();
static void parseLine(char *line);
static short searchType(char *current);
static bool parseGGA(short index, char *current, bool *stop);
static bool parseVTG(int index, char *current, bool *stop);
static bool parseRMC(int index, char* current, bool* stop);
static bool parseGLL(short index, char* current, bool* stop);
static void printGPS(char* backup);
static double convertDegrees(double angle);
static uint16_t convertSpeed(double decimal);
static void csv_log(double latitude, double longitude, uint16_t speed);

#endif