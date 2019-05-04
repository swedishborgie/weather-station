# Weather Station
An application for recording weather from a MakerLife Raspberry Pi Zero W
Weather Station kit and saving it to the console, a PostgreSQL database, or a
CSV file.

![MakerLife Raspberry Pi Zero W Weather Station](weatherstation.jpg)


The weather station kit includes the following sensors to be attached to a
Raspberry Pi Zero W:

 * A [DS18S20](https://www.maximintegrated.com/en/products/sensors/DS18S20.html) temperature sensor.
 * A rainfall gauge.
 * A wind meter.

Both the rainfall gauge and wind meter are simple switches that activate after
a set amount of wind and/or rain.

## Why?

The example project provided by MakerLife was only in Python and only displayed
output to the terminal (and only in metric units). I wanted something that could
start a little faster and would record to a PostgreSQL database so I could use
the historical data later.

I also wanted an application that could record to a remote database so I could
mount the SD card as read only to prevent wearing out the SD card.

## Configuring

The application is split out into several different "recorders" that can be used to send data to different endpoints. You can enable all recorders by doing the following:

    make USE_PGSQL=1 USE_CSV=1 USE_REST=1 USE_IMPERIAL=1

If `USE_PGSQL` is 1 the application will be configured to connect to a PostgreSQL
database (see below) to insert samples at fixed intervals into a table. This
requires libpq-dev.

If `USE_IMPERIAL` is 1 the units will be set to imperial units instead of metric.

If `USE_CSV` is 1 the application will output a CSV file.

In the `recorder_pgsql.c` file the following configuration options are available:

    #define PGSQL_CONNECT_STRING "host=localhost dbname=weather user=weather password=password"
    #define CSV_FILE "weather.csv"
    #define TEMP_SAMPLE_RATE 5
    #define SAMPLE_RATE 60

Change the `PGSQL_CONNECT_STRING` to connect to your database. The syntax for this
string [can be found here](https://www.postgresql.org/docs/current/static/libpq-connect.html#LIBPQ-PARAMKEYWORDS).

An example table for PostgreSQL can be found in weather.sql.

Change `CSV_FILE` in `recorder_csv.c` to change the name/path of the generated CSV file.

Change `TEMP_SAMPLE_RATE` to change the rate in seconds at which the temperature
sensor is polled.

Change `SAMPLE_RATE` to change the rate in seconds at which the samples are sent
to the terminal and/or the database and/or the CSV file.

If `USE_REST` is 1 then the application will attempt to send a JSON encoded REST POST to the endpoint `REST_ENDPOINT` in `recorder_rest.c`.

## Compiling

To compile this project you'll need the following packages (Raspbian):

    sudo apt install build-essential wiringpi libpq-dev

Note: libpq-dev is only required if you want to save to a PostgreSQL database
(see above).

In order for the temperature sensor to work you'll also need to make sure you
[enable the 1-wire kernel module](https://www.raspberrypi-spy.co.uk/2018/02/enable-1-wire-interface-raspberry-pi/).

To make the application:

    make

To run the application:

    ./weather

The application should now send samples to the terminal and any other configured
destinations at fixed intervals:

	Weather Measurements:
	     Start: 2018-10-23 21:54:16
	       End: 2018-10-23 21:55:15
	  Duration: 59 seconds
	      Temp: 42.180801 f
	      Wind: 1.037264 mph (83 ticks)
	      Rain: 0.000000 inches (0 tips)

## Writing your own Recorder

If you want to write your own recorder to send weather data to a custom endpoint you can do that by creating a new file (for example `recorder_example.c`) like this:

    #include "weather.h"

    void record_example(char* start, char* end, float temp, float wind, int spins, float rain, int tips) {
        //Do your custom recording here.
    }

    /**
    * Register ourselves with the plugin system.
    */
    RECORDER_INIT(EXAMPLE) {
        add_recorder("example", &record_example);
    }

Then add the following to `Makefile`:

    ifeq ($(USE_EXAMPLE), 1)
        OBJ += recorder_example.o
    endif

Then make by doing the following:

    make USE_EXAMPLE=1