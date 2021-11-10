#include <fcntl.h>
#include <linux/can.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wait.h>

#define MAX_PATH_LENGTH    100
#define MAX_DATA_BYTE      8
#define MAX_ID_BYTE        3
#define GPS_ID_MESSAGE     "0A0"
#define GPS_ENABLE_MESSAGE "66010000"
#define GPS_IDLE_MESSAGE   "66000000"

//path exec

typedef enum { false, true } bool;

int pid = -1;
char interface_name[50]; //ttyACMn

int main(int argc, char **argv) {
    char log[MAX_PATH_LENGTH]; //gps log path
    char gps[MAX_PATH_LENGTH]; //gps script path
    if (argc == 2) {
        //search gps_logger on the same folder
        getcwd(gps, sizeof(gps));
        if (gps == NULL) {
            perror("Cannot find current working directory");
            return 10;
        }
        strcat(gps, "/gps_logger");

        //log

        strcpy(log, argv[1]);

    } else if (argc == 3) {
        strcpy(gps, argv[1]);
        strcpy(log, argv[2]);
    } else {
        fprintf(stderr, "Usage: ./can_listener [/path/to/gps_logger] </path/to/gps/log/folder>\n");
    }

    int count = 0;

    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (s < 0) {
        perror("Cannot connect to CAN");
        return 1;
    } else {
        struct ifreq ifr;
        strcpy(ifr.ifr_name, "can0");
        ioctl(s, SIOCGIFINDEX, &ifr);

        struct sockaddr_can addr;
        memset(&addr, 0, sizeof(addr));
        addr.can_family  = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;
        if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("Cannot make binding");
            return 2;
        }

        int nbytes;
        struct can_frame frame;

        while (1) {
            if ((nbytes = read(s, &frame, sizeof(struct can_frame))) > 0) {
                if (nbytes > 0) {
                    char id[MAX_ID_BYTE + 1];
                    sprintf(id, "%03X", frame.can_id);
                    id[3] = 0;

                    char message[MAX_DATA_BYTE * 2 + 1];
                    for (int i = 0; i < frame.can_dlc; i++) {
                        sprintf(message + i * 2, "%02X", frame.data[i]);
                    }

                    message[frame.can_dlc * 2 + 1] = 0;  //string terminator

                    if (strcmp(id, GPS_ID_MESSAGE) == 0 && strcmp(message, GPS_IDLE_MESSAGE) == 0) {
                        if (count == 1 && pid != -1) {
                            kill(pid, SIGUSR1);
                            while (wait(NULL) > 0)
                                ;
                            count = 0;
                        }

                    } else if (strcmp(id, GPS_ID_MESSAGE) == 0 && strcmp(message, GPS_ENABLE_MESSAGE) == 0) {
                        if (count == 0) {
                            count++;
                            int f = fork();

                            if (f == 0) {
                                bool found = false;

                                for (int i = 0; i < 10 && !found; i++) {
                                    char interface[50];
                                    sprintf(interface, "/dev/ttyACM%d", i);
                                    int s = access(interface, F_OK | R_OK);
                                    if (s >= 0) {
                                        found = true;
                                        strcpy(interface_name, interface);
                                    } else {
                                        perror("Interface not found");
                                    }
                                }

                                if (!found) {
                                    perror("Cannot find GPS");
                                } else {
                                    char *args[] = {gps, interface_name, log, NULL};
                                    execv(gps, args);
                                }

                            } else {
                                pid = f;
                            }
                        }
                    }
                }
            }
            sleep(1);
        }
    }
}
