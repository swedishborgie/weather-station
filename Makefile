CC=gcc
LIBS=-lpq -lwiringPi
CFLAGS=-DUSE_PGSQL=1 -DUSE_IMPERIAL=1
OBJ=weather.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

weather: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm *.o weather