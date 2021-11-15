# EagleTRT GPS test System for Chimera Evoluzione

GPS parser which read raw GPS messages, selects only the valid ones and sends them to CAN bus

## Compiling
### GPS Logger

`gps_logger` works both with a real GPS connected via USB and with GPS simulator of [Eagle-CLI](https://github.com/eagletrt/eagle-cli) 

In order to compile it you can just do

```gcc gps_logger.c gpslib.c Primary.c -O3 -lm -o gps_logger``` 

Keep `gpslib.h` in the same folder of the `gps_logger`

### CAN Listener

In order to start and stop the GPS logger when needed, there is `can_listener` which looks for the starting/ending CAN messages sent to the telemetry, so it is easier to find the desired data later, based on timestamp of the execution

In order to compile it you can just do

```gcc can_listener.c Primary.c -O3 -o can_listener```

### Makefile

You can also just use `make` command to compile the source files

## Executing

### GPS Logger

To execute it, you should also provide the port where it is connected and the path to the folder where you want to save the logs

If you are using the real GPS the port should be `/dev/ttyACM0`, but after some plugs and unplugs the number could change since it is a serial port

```./gps_logger /dev/ttyACM0 /path/to/gps/log/folder``` 

If you are simulating the GPS the port should be `/dev/pts/n`, where `n` is a number

```./gps_logger /dev/pts/n /path/to/gps/log/folder```

### CAN Listener

When you execute this script you can also provide a different path of the GPS Logger, otherwise it supposed to be in the same folder and it searches for it there

Moreover you need to define the folder where the GPS Logger would save its data

```./can_listener /path/to/gps/log/folder```

```./can_listener /path/to/gps_logger /path/to/gps/log/folder```

For test purposes it also shows GPS data sent to CAN with the GPS Logger

## Systemd service for CAN Listener

You can also use a service to keep the CAN Listener executing, searching for the activation/idle service. Remember to edit the `ExecStart` option providing `can_listener` path (it may not work on the real car since it has to be tested)

## TODO
- [x] Format and upload the code
- [x] Pass the log paths
- [ ] Print informations on the screen
