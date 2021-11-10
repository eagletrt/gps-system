
all: gps_logger can_listener

gps_logger:gps_logger.c
	@gcc gps_logger.c -O3 -lm -D REAL -o gps_logger
can_listener:can_listener.c
	@gcc can_listener.c -O3 -o can_listener