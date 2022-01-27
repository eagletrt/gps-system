#include "Primary.h"
#include "gpslib.h"
#include "ids.h"

#include <linux/can.h>
#include <math.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

struct timeval tval_before;
char data_ora[MAX_LENGTH];
char dati_in_ingresso[MAX_LENGTH];
char dati_csv[MAX_LENGTH];
FILE *fs;
gps_t *gps;
char port[MAX_LENGTH];
char logpath[MAX_LENGTH];
int s = -1;
struct can_frame frame;
int co = 0;

void setupCan() {
    if (s == -1) {
        struct ifreq ifr;
        struct sockaddr_can addr;

        s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (s < 0) {
            perror("Error while opening socket");
            exit(2);
        }

        strcpy(ifr.ifr_name, "can2");
        ioctl(s, SIOCGIFINDEX, &ifr);

        addr.can_family  = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("Error in socket bind");
            exit(3);
        }

        frame.can_dlc = 8;
    }
}

void pos2can(double latitude, double langitude) {

    printf("%lf %lf\n", latitude, langitude);

    frame.can_id = ID_GPS_COORDS;

    uint8_t *buffer_primary_gps_coords      = (uint8_t *)malloc(sizeof(Primary_GPS_COORDS));
    Primary_GPS_COORDS primary_gps_coords_s = {(float)latitude, (float)langitude};
    serialize_Primary_GPS_COORDS(
        buffer_primary_gps_coords, primary_gps_coords_s.latitude, primary_gps_coords_s.longitude);

    frame.data[0] = buffer_primary_gps_coords[0];
    frame.data[1] = buffer_primary_gps_coords[1];
    frame.data[2] = buffer_primary_gps_coords[2];
    frame.data[3] = buffer_primary_gps_coords[3];
    frame.data[4] = buffer_primary_gps_coords[4];
    frame.data[5] = buffer_primary_gps_coords[5];
    frame.data[6] = buffer_primary_gps_coords[6];
    frame.data[7] = buffer_primary_gps_coords[7];

    // o *frame.data=buffer_primary_gps_coords;

    free(buffer_primary_gps_coords);

    int bytes;
    if ((bytes = write(s, &frame, sizeof(struct can_frame))) != sizeof(struct can_frame)) {
        perror("Write");
        exit(4);
    }
}

void speed2can(uint16_t speed) {
    frame.can_id = ID_GPS_SPEED;

    uint8_t *buffer_primary_gps_speed     = (uint8_t *)malloc(sizeof(Primary_GPS_SPEED));
    Primary_GPS_SPEED primary_gps_speed_s = {speed};
    serialize_Primary_GPS_SPEED(buffer_primary_gps_speed, primary_gps_speed_s.speed);

    frame.data[0] = buffer_primary_gps_speed[0];
    frame.data[1] = buffer_primary_gps_speed[1];
    frame.data[2] = buffer_primary_gps_speed[2];
    frame.data[3] = buffer_primary_gps_speed[3];
    frame.data[4] = buffer_primary_gps_speed[4];
    frame.data[5] = buffer_primary_gps_speed[5];
    frame.data[6] = buffer_primary_gps_speed[6];
    frame.data[7] = buffer_primary_gps_speed[7];

    // o *frame.data=buffer_primary_gps_speed;

    free(buffer_primary_gps_speed);

    int bytes;
    if ((bytes = write(s, &frame, sizeof(struct can_frame))) != sizeof(struct can_frame)) {
        perror("Write");
        exit(5);
    }
}

//terminate program when receiving SIGUSR1
void exitHandler(int sig) {
    finalizeGPS();
    exit(0);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: ./gps_logger </path/to/port> </path/to/log/folder>\n");
        return 1;
    } else {
        strcpy(port, argv[1]);
        strcpy(logpath, argv[2]);
    }

    printf("Executing GPS Logger with port %s and log path %s\n", port, logpath);

    signal(SIGUSR1, exitHandler);
    gettimeofday(&tval_before, NULL);

    createLogFiles();
    connect_GPS();
    setupGPS();
    setupCan();
    while (1) {
        readGPS();
    }
    finalizeGPS();

    return 0;
}