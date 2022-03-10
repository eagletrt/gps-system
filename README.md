# EagleTRT GPS System for Fenice

GPS parser which read raw GPS messages, selects only the valid ones and sends them to CAN bus

## Compiling
In order to compile it you can just use `cmake` suite

Firstly, execute `cmake .` in the root of this project and then you can call `make` to compile everything with the generated `Makefile`

If you want to delete the binaries you can just use the `make clean` command

## Executing

### GPS Logger

To execute it you should also provide the port where the GPS is connected and the designated CAN port where the messages will be sent

Usally, the GPS path is `/dev/ttyACM0`, but after some plugs and unplugs, the number could change since it is a serial port

Talking about the CAN port, it is provided by the telemetry system

If you want to test the GPS parser locally, you can pass an old log file instead of the GPS port and the `vcan0` port, the virtual one, as the CAN port

Example:
```./gps_reader /dev/ttyACM0 vcan0``` 

## TODO
- [ ] Print informations on the screen
