#include "Primary.h"
#include "utils.h"

#include <fcntl.h>
#include <inttypes.h>
#include <linux/can.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wait.h>

#define MAX_PATH_LENGTH    100
#define MAX_DATA_BYTE      8
#define MAX_ID_BYTE        3

//path exec
int pid = -1;
char interface_name[50];  //ttyACMn

int main(int argc, char **argv) {
    char log[MAX_PATH_LENGTH];  //gps log path
    char gps[MAX_PATH_LENGTH];  //gps script path
    
    if (argc == 3) {
        strcpy(gps, argv[1]);
        strcpy(log, argv[2]);
    } else {
        fprintf(stderr, "Usage: ./can_listener </path/to/gps_logger> </path/to/gps/log/folder>\n");
        return 1;
    }

    int count = 0;

    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (s < 0) {
        perror("Cannot connect to CAN");
        return 2;
    } else {
        struct ifreq ifr;
        strcpy(ifr.ifr_name, "can2");
        ioctl(s, SIOCGIFINDEX, &ifr);

        struct sockaddr_can addr;
        memset(&addr, 0, sizeof(addr));
        addr.can_family  = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;
        if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("Cannot make binding");
            return 3;
        }

        int nbytes;
        struct can_frame frame;
        bool already=false;

        while (1) {
            if ((nbytes = read(s, &frame, sizeof(struct can_frame))) > 0) {
                if (nbytes > 0) {
                    char buffer[21];
                    Primary_msgname_from_id(frame.can_id,buffer);

                    if(strcmp(buffer,"TLM_STATUS")==0){
                        uint8_t* buffer_primary_tlm_status = (uint8_t*)malloc(sizeof(Primary_TLM_STATUS));
                        Primary_TLM_STATUS* primary_tlm_status_d = (Primary_TLM_STATUS*)malloc(sizeof(Primary_TLM_STATUS));
                        deserialize_Primary_TLM_STATUS(buffer_primary_tlm_status, primary_tlm_status_d);

                        if(primary_tlm_status_d->tlm_status==Primary_Tlm_Status_ON && !already){

                            already=true;
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
                                    }
                                }

                                if (!found) {
                                    perror("Cannot find GPS");
                                    return 4;
                                    //shutdown telemetry
                                } else {
                                    char *args[] = {gps, interface_name, log, NULL};
                                    printf("%s\n%s\n%s\n", gps, interface_name,log);
                                    execv(gps, args);
                                }

                            } else {
                                pid = f;
                            }
                        } else if(primary_tlm_status_d->tlm_status==Primary_Tlm_Status_ON && already){
                            already=false;
                            kill(pid, SIGUSR1);
                            while (wait(NULL) > 0)
                                ;
                        }

                        free(buffer_primary_tlm_status);
                        free(primary_tlm_status_d);

                    }
                }
            }
        }
    }

    return 0;

}
