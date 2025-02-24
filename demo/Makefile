CC=gcc
CFLAGS=-O2 -g -Wall -fopenmp
OPTFLAGS=-O3 -g -Wall -fopenmp
OBJS_COMMON=kernel.o

all:	check calibrate measure

check:	$(OBJS_COMMON) driver_check.o
	$(CC) $(CFLAGS) -o $@ $^
calibrate: $(OBJS_COMMON) driver_calib.o
	$(CC) $(CFLAGS) -o $@ $^
measure: $(OBJS_COMMON) driver.o
	$(CC) $(CFLAGS) -o $@ $^

driver_check.o: driver_check.c
	$(CC) $(CFLAGS) -D CHECK -c $< -o $@
driver_calib.o: driver_calib.c
	$(CC) $(CFLAGS) -D CALIB -c $< -o $@
driver.o: driver.c
	$(CC) $(CFLAGS) -c $<

kernel.o: kernel.c
	$(CC) $(OPTFLAGS) -D $(OPT) -c $< -o $@

clean:
	rm -rf $(OBJS_COMMON) driver_check.o driver_calib.o driver.o check calibrate measure
