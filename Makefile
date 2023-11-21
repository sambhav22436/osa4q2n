CC = gcc
CFLAGS = -pthread

all: ride

ride: ride.c
	$(CC) $(CFLAGS) ride.c -o ride

clean:
	rm -f ride
