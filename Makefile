all: gps_logger can_listener

gps_logger:gps_logger.c
	@gcc gps_logger.c gpslib.c Primary.c -O3 -lm -o gps_logger
can_listener:can_listener.c
	@gcc can_listener.c Primary.c -O3 -o can_listener