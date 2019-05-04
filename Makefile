CC=gcc

LIBS=-lwiringPi -lcrypt -lrt -lm
CFLAGS=
OBJ=weather.o

ifeq ($(USE_IMPERIAL), 1)
	CFLAGS += -DUSE_IMPERIAL=1
endif

ifeq ($(USE_PGSQL), 1)
	LIBS += -lpq
	CFLAGS += -DUSE_PGSQL=1
	OBJ += recorder_pgsql.o
endif

ifeq ($(USE_CSV), 1)
	CFLAGS += -DUSE_CSV=1
	OBJ += recorder_csv.o
endif

ifeq ($(USE_REST), 1)
	LIBS += -lcurl
	CFLAGS += -DUSE_REST=1
	OBJ += recorder_rest.o
endif


%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

weather: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm *.o weather