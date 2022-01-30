# EagleTRT GPS System for Fenice

GPS parser which read raw GPS messages, selects only the valid ones and sends them to CAN bus

##### :warning: Remeber to provide absolute paths as arguments

## Compiling
In order to compile it you can just use `cmake` suite

Firstly, execute `cmake .` in the root of this project and then you can call `make` to compile everything with the generated `Makefile`

If you want to delete the binaries you can just use the `make clean` command

## Executing

### GPS Logger

To execute it you should also provide the port where the GPS is connected and the path to the folder where you want to save the logs

Usally, the GPS path is `/dev/ttyACM0`, but after some plugs and unplugs the number could change since it is a serial port

Example:
```./gps_reader /dev/ttyACM0 /home/ubuntu/gps-system/logs/``` 

### CAN Listener

When you execute this script you should provide the path of the GPS Logger and also the folder where the logs would be saved.

Example:
```./can_listener /home/ubuntu/gps-system/gps_reader /home/ubuntu/gps-system/logs/``` 

## Systemd service for CAN Listener

You can also use a service to keep the CAN Listener executing, searching for the activation or idle message, just move it to `/etc/systemd/system` folder, then start it and make it executes on startup if you want

## TODO
- [ ] Check actual service on telemetry Pi 
- [ ] Upload changes on telemetry Pi 
- [ ] Print informations on the screen