/*
 * Copyright 2018 - Michael Powers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <time.h>

/**
 * Base location for the 1-wire device node used for the temperature sensor.
 */
#define W1_BASE "/sys/bus/w1/devices"
/**
 * The device node for the 1-wire temperature sensor.
 */
#define W1_DEVNODE "w1_slave"
/**
 * The number of seconds between temperature samples (also controls the main loop).
 */
#define TEMP_SAMPLE_RATE 5
/**
 * The number of seconds between samples.
 */
#define SAMPLE_RATE 60
/**
 * This macro abuses the GCC constructor attribute to provide a simple registration
 * mechanism for plugins. These get called before main and allow us to register our
 * plugins before everything gets started. This might not work in anything other than
 * GCC (e.g. LLVM).
 */
#define RECORDER_INIT(TYPE) void recorder_init_##TYPE (void) __attribute__((constructor)); void recorder_init_##TYPE (void)

/**
 * This function describes a function that accepts weather data for a specific time
 * period and records it to some persistant data store. This can be used to create
 * new data recording plugins.
 * @param startStr The sample start time in YYYY-MM-DD HH:MM:SS format.
 * @param endStr The sample end time in YYYY-MM-DD HH:MM:SS format.
 * @param temp The temperature to record.
 * @param wind The wind speed to record.
 * @param spins The number of ticks generated by the wind sensor.
 * @param rain The rain depth to record.
 * @param tips The number of tips of the rain meter.
 */
typedef void (*weather_recorder)(char* startStr, char* endStr, float temp, float wind, int spins, float rain, int tips);

/**
 * This is a forward only linked list that will hold our recorder list.
 */
typedef struct recorder_list_item recorder_list_item;
typedef struct recorder_list_item {
	weather_recorder recorder;
	recorder_list_item* next;
	char name[50];
} recorder_list_item;


#ifdef USE_IMPERIAL
	#define C_TO_F(c) (c * 9.0 / 5.0 + 32.0)
	#define KPH_TO_MPH(kph) (kph *  0.62137119)
	#define MM_TO_INCHES(mm) (mm * 0.03937007874)
	#define CALC_TEMP(temp) C_TO_F(temp)
	#define CALC_WIND(wind) KPH_TO_MPH(WIND_SPEED(WIND_DIST(windSpins), duration))
	#define CALC_RAIN(rain) MM_TO_INCHES(rainTips * BUCKET_SIZE)
	#define DISPLAY_FORMAT "\x1b[2J\x1b[HWeather Measurements:\n     Start: %s\n       End: %s\n  Duration: %u seconds\n      Temp: %f f\n      Wind: %f mph (%i ticks)\n      Rain: %f inches (%i tips)\n\n"
#else
	#define CALC_TEMP(temp) (temp)
	#define CALC_WIND(wind) (WIND_SPEED(WIND_DIST(windSpins), duration))
	#define CALC_RAIN(rain) (rainTips * BUCKET_SIZE)
	#define DISPLAY_FORMAT "\x1b[2J\x1b[HWeather Measurements:\n     Start: %s\n       End: %s\n  Duration: %u seconds\n      Temp: %f c\n      Wind: %f kph (%i ticks)\n      Rain: %f mm (%i tips)\n\n"
#endif

/**
 * The digital pin (in WiringPi format) the wind sensor is attached to.
 */
#define WIND_SENSOR_PIN 0
/**
 * The digital pin (in WiringPi format) the rain sensor is attached to.
 */
#define RAIN_SENSOR_PIN 2
/**
 * The number of seconds in an hour.
 */
#define SECONDS_IN_HOUR 3600
/**
 * The number of centimeters in a kilometer.
 */
#define CM_IN_KM 100000.0
/**
 * Adjustment factor to multiply the wind speed by.
 */
#define ADJUSTMENT 1.18
/**
 * The bucket size of the rain meter in millimeters.
 */
#define BUCKET_SIZE 0.2794
/**
 * Constant for PI.
 */
#define MATH_PI 3.14159
/**
 * The radius of the wind sensor in centimeters.
 */
#define WIND_RADIUS_IN_CM 9.0
/**
 * The circumference of the wind sensor in centimeters.
 */
#define CIRCUMFERENCE_CM ((2 * MATH_PI) * WIND_RADIUS_IN_CM)
/**
 * Conversion between ticks and distance in kilometers.
 */
#define WIND_DIST(ticks) ((ticks / 2) * CIRCUMFERENCE_CM / CM_IN_KM)
/**
 * Conversion between kilometers and kilometers per hour.
 */
#define WIND_SPEED(dist, time) ((dist / time) * SECONDS_IN_HOUR * ADJUSTMENT) 
/**
 * This function attempts to find the path to the temperature sensor.
 * @param path Character pointer which receives the path to the temp sensor.
 * @return 1 on success, 0 on failure.
 */
unsigned char find_temp_sensor_path(char* path);
/**
 * This function attempts to read from the temperature sensor.
 * @param tempSensorPath Path to the temperature sensor.
 * @param temp Pointer to a float to fill with the current temperature.
 * @return 1 on success, 0 on failure.
 */
unsigned char read_temperature(char* tempSensorPath, float* temp);
/**
 * Callback which gets called by wiring pi when an edge gets detected from the
 * rain sensor.
 */
void rain_sensor_tip();
/**
 * Callback which gets called by wiring pi when an edge gets detected from the
 * wind sensor.
 */
void wind_sensor_spin();
/**
 * This function gets called when it's time to push a sample.
 * @param start The start time when samples began.
 * @param end The end time when samples completed.
 */
void sample_readings(time_t start, time_t end);
/**
 * Adds a recorder function pointer to the list of registered recorders.
 */
void add_recorder(char* name, weather_recorder recorder);