#include "gpslib.h"

#include <fcntl.h>
#include <inttypes.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 1000

extern struct timeval tval_before;
extern char data_ora[MAX_LENGTH];
extern char dati_in_ingresso[MAX_LENGTH];
extern char dati_csv[MAX_LENGTH];
extern FILE *fs;
extern gps_t *gps;
extern char port[MAX_LENGTH];
extern char logpath[MAX_LENGTH];

// create log files
void createLogFiles()
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(
        data_ora,
        "%d-%02d-%02d_%02d-%02d-%02d",
        tm.tm_year + 1900,
        tm.tm_mon + 1,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec);

    strcpy(dati_in_ingresso, logpath);
    strcat(dati_in_ingresso, data_ora);
    strcat(dati_in_ingresso, ".log");

    // strcat(dati_in_uscita,data_ora);
    // strcat(dati_in_uscita,"_PARSED.log");

    strcpy(dati_csv, logpath);
    strcat(dati_csv, data_ora);
    strcat(dati_csv, ".csv");

    /*FILE* tmp=fopen(dati_in_ingresso, "a+");
    fclose(tmp);
    tmp=fopen(dati_csv, "a+");
    fclose(tmp);*/
}

// Setup GPS connection using real GPS
void connect_GPS()
{
    // mkfifo(path, 0666);
    fs = fopen(port, "r");

    if (fs == NULL)
    {
        perror("Cannot connect to GPS");
        exit(6);
    }
}

// Allocate the union which contains the GPS data to be send via can
void setupGPS()
{
    gps = (gps_t *)malloc(sizeof(gps_t));
    if (gps == NULL)
    {
        perror("Cannot create GPS struct");
        exit(7);
    }
    gps->message = (gps_message_t *)malloc(sizeof(gps_message_t));
    if (gps->message == NULL)
    {
        perror("Cannot create GPS message struct");
        exit(8);
    }
}

// free memory and close file stream
void finalizeGPS()
{
    free(gps->message);
    free(gps);
    fclose(fs);
}

// Read GPS messages line per line
void readGPS()
{
    unsigned char line[MAX_LENGTH];
    while (fgets(line, sizeof(line), fs) != NULL)
    {
        FILE *olog = fopen(dati_in_ingresso, "a+");
        fprintf(olog, "%s", line);
        fclose(olog);
        parseLine(line);
        memset(line, 0, sizeof(line));
    }
    fclose(fs);
    fs = fopen(port, "r");
}

// Search messages in the line
static void parseLine(char *line)
{
    char *current;
    short index = 0;
    short type = -1;
    bool valid = true;
    bool stop = false;
    bool *stop_p = &stop;
    while ((current = strsep(&line, ",")) != NULL)
    {
        if (index != 0 &&
            strchr(current, '$') != NULL)
        { // check if there is a dollar in a word, and if it is, reset the index to 0
            index = 0;
        }

        if (index ==
            0)
        { // if the index is 0, look for the type of the word, if is different from -1 assign it to the struct and reset the message to valid
            type = searchType(current);
            if (type != -1)
            {
                gps->gps_type = type;
                valid = true;
                stop = false;
            }
        }

        if (valid)
        {
            switch (type)
            { // switch between the 4 different types of messages
            case 1:
                valid = valid && parseGGA(index, current, stop_p);
                break;
            case 2:
                valid = valid && parseVTG(index, current, stop_p);
                break;
            case 3:
                valid = valid && parseRMC(index, current, stop_p);
                break;
            case 4:
                valid = valid && parseGLL(index, current, stop_p);
                break;
            }
        }

        if (stop && valid)
        { // if I have all the data and the message is valid, print it

            switch (type)
            {
            case 1:
                csv_log(gps->message->gga.latitude, gps->message->gga.longitude, 0);
                pos2can(gps->message->gga.latitude, gps->message->gga.longitude);
                break;
            case 2:
                csv_log(0, 0, gps->message->vtg.speed);
                speed2can(gps->message->vtg.speed);
                break;
            case 3:
                csv_log(gps->message->rmc.latitude, gps->message->rmc.longitude, gps->message->rmc.speed);
                pos2can(gps->message->rmc.latitude, gps->message->rmc.longitude);
                speed2can(gps->message->rmc.speed);
                break;
            case 4:
                csv_log(gps->message->gll.latitude, gps->message->gll.longitude, 0);
                pos2can(gps->message->gll.latitude, gps->message->gll.longitude);
                break;
            }
            stop = false;
        }

        index++;
    }
}

// find what message we are parsing
static short searchType(char *current)
{
    int length = strlen(current);
    short type = -1;
    if (length != 0)
    {
        if (current[length - 1 - 2] == 'G' && current[length - 1 - 1] == 'G' && current[length - 1] == 'A')
            type = 1;
        else if (current[length - 1 - 2] == 'V' && current[length - 1 - 1] == 'T' && current[length - 1] == 'G')
            type = 2;
        else if (current[length - 1 - 2] == 'R' && current[length - 1 - 1] == 'M' && current[length - 1] == 'C')
            type = 3;
        else if (current[length - 1 - 2] == 'G' && current[length - 1 - 1] == 'L' && current[length - 1] == 'L')
            type = 4;
    }
    // if(length==0) type=0;

    return type;
}

// save GGA data to union
static bool parseGGA(short index, char *current, bool *stop)
{
    bool valid = true;
    switch (index)
    {
    case 2:
        if ((gps->message->gga.latitude = convertDegrees(atof(current))) == 0 || current[0] == '\0')
            valid = false;
        break;
    case 3:
        if (current[0] != 'S' && current[0] != 'N')
            valid = false;
        else if (current[0] == 'S')
            gps->message->gga.latitude *= -1;
        break;
    case 4:
        if ((gps->message->gga.longitude = convertDegrees(atof(current))) == 0 || current[0] == '\0')
            valid = false;
        break;
    case 5:
        if (current[0] != 'W' && current[0] != 'E')
            valid = false;
        else if (current[0] == 'W')
            gps->message->gga.longitude *= -1;
        *stop = true;
        break;
    }
    return valid;
}

// save VTG data to union
static bool parseVTG(int index, char *current, bool *stop)
{
    bool valid = true;
    switch (index)
    {
    case 5:
        if ((gps->message->vtg.speed = convertSpeed(atof(current))) == 0 || current[0] == '\0')
            valid = false;
        *stop = true;
        break;
    }
    return valid;
}

// save RMC data to union
static bool parseRMC(int index, char *current, bool *stop)
{
    bool valid = true;
    switch (index)
    {
    case 3:
        if ((gps->message->rmc.latitude = convertDegrees(atof(current))) == 0 || current[0] == '\0')
            valid = false;
        break;
    case 4:
        if (current[0] != 'S' && current[0] != 'N')
            valid = false;
        else if (current[0] == 'S')
            gps->message->rmc.latitude *= -1;
        break;
    case 5:
        if ((gps->message->rmc.longitude = convertDegrees(atof(current))) == 0 || current[0] == '\0')
            valid = false;
        break;
    case 6:
        if (current[0] != 'W' && current[0] != 'E')
            valid = false;
        else if (current[0] == 'W')
            gps->message->rmc.longitude *= -1;
        break;
    case 7:
        if ((gps->message->rmc.speed = convertSpeed(atof(current))) == 0 || current[0] == '\0')
            valid = false;
        *stop = true;
        break;
    }
    return valid;
}

// save GLL data to union
static bool parseGLL(short index, char *current, bool *stop)
{
    bool valid = true;
    switch (index)
    {
    case 1:
        if ((gps->message->gll.latitude = convertDegrees(atof(current))) == 0 || current[0] == '\0')
            valid = false;
        break;
    case 2:
        if (current[0] != 'S' && current[0] != 'N')
            valid = false;
        else if (current[0] == 'S')
            gps->message->gll.latitude *= -1;
        break;
    case 3:
        if ((gps->message->gll.longitude = convertDegrees(atof(current))) == 0 || current[0] == '\0')
            valid = false;
        break;
    case 4:
        if (current[0] != 'W' && current[0] != 'E')
            valid = false;
        else if (current[0] == 'W')
            gps->message->gll.longitude *= -1;
        *stop = true;
        break;
    }
    return valid;
}

// log to csv file
static void csv_log(float latitude, float longitude, uint16_t speed)
{
    struct timeval unix_time;
    gettimeofday(&unix_time, NULL);

    char dati[200];
    sprintf(
        dati,
        "%ld.%06ld,%lf,%lf,%" PRIu32,
        (long int)unix_time.tv_sec,
        (long int)unix_time.tv_usec,
        latitude,
        longitude,
        speed);
    FILE *csv = fopen(dati_csv, "a+");
    fprintf(csv, "%s\n", dati);
    fclose(csv);
}

// convert degress minutes to degrees
static float convertDegrees(float angle)
{
    float degrees = floor(angle / 100.0);
    float minutes = (angle - degrees * 100.0) / 60.0;
    return degrees + minutes;
}

// convert knots to km/h*100 to get an integer
static uint16_t convertSpeed(float decimal)
{
    decimal *= 1.852;
    decimal *= 100;
    decimal = round(decimal);
    decimal = abs(decimal);
    uint16_t speed = decimal;
    return speed;
}