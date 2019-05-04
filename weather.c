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
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include "weather.h"

#ifndef PATH_MAX
	#include <linux/limits.h>
#endif

/**
 * Average temperature for the most recent sample set.
 */
float avgTemp = -10000;
/**
 * The number of wind sensor ticks in the most recent sample set.
 */
int windSpins = 0;
/**
 * The number of rain sensor ticks int he most recent sample set.
 */
int rainTips = 0;
/**
 * The list of recorders we're configured to use.
 */
recorder_list_item* recorderList = NULL;

int main() {
	//Gets a path to the temperature sensor.
	char tempSensorPath[PATH_MAX];
	//Time samples.
	time_t nextSample, lastSample, now;

	//Initialize sensors.
	if(!find_temp_sensor_path((char*)&tempSensorPath)) {
		printf("Unable to find temperature sensor\n");
		return -4;
	} else if(wiringPiSetup() < 0) {
		printf("Wiring pi setup failure.\n");
		return -1;
	} else if(wiringPiISR(WIND_SENSOR_PIN, INT_EDGE_FALLING, &wind_sensor_spin) < 0) {
		printf("Failed to set up wind sensor interrupt.\n");
		return -2;
	} else if(wiringPiISR(RAIN_SENSOR_PIN, INT_EDGE_FALLING, &rain_sensor_tip) < 0) {
		printf("Failed to set up rain sensor interrupt.\n");
		return -3;
	}

	time(&lastSample);
	nextSample = lastSample + SAMPLE_RATE;

	while(1) {
		float temp;
		read_temperature((char*)&tempSensorPath, &temp);
		avgTemp = (avgTemp > -1000) ? ((avgTemp + temp) / 2) : temp;
		sleep(TEMP_SAMPLE_RATE);
		
		time(&now);
		if(now >= nextSample) {
			sample_readings(lastSample, now);
			lastSample = now;
			nextSample += SAMPLE_RATE;
		}
	}

	return 0;
}

void sample_readings(time_t prev, time_t now) {
	int duration = now - prev;
	char startStr[50], endStr[50];
	strftime((char*)&startStr, sizeof(startStr), "%Y-%m-%d %H:%M:%S", localtime(&prev));
	strftime((char*)&endStr, sizeof(endStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
	float temp = CALC_TEMP(avgTemp);
	float wind = CALC_WIND(windSpins);
	float rain = CALC_RAIN(rainTips);

	printf(DISPLAY_FORMAT, startStr, endStr, duration, temp, wind, windSpins, rain, rainTips);

	//Run through the list of configured recorders.
	recorder_list_item* itr = recorderList;
	while(itr != NULL) {
		printf("Recording to: %s\n", itr->name);
		(itr->recorder)(startStr, endStr, temp, wind, windSpins, rain, rainTips);
		itr = itr->next;
	}

	avgTemp = -10000;
	windSpins = 0;
	rainTips = 0;
}

unsigned char find_temp_sensor_path(char* path) {
	DIR* dir = opendir(W1_BASE);
	if(dir == NULL) { return 0; }
	struct dirent* ent;
	while((ent = readdir(dir)) != NULL) {
		if(strstr(ent->d_name, "28-") != NULL) {
			//Add the lengths of the base path, device node, directory name,
			//two slashes and a null terminator.
			int pathLen = strlen(W1_BASE) + strlen(W1_DEVNODE) + strlen(ent->d_name) + 3;
			sprintf(path, "%s/%s/%s", W1_BASE, ent->d_name, W1_DEVNODE);
			closedir(dir);
			return 1;
		}
	}
	closedir(dir);
	return 0;
}

unsigned char read_temperature(char* tempSensorPath, float* temp) {
	char line[256];
	FILE* device = fopen(tempSensorPath, "r");
	unsigned char crcValid = 0;
	char* offset;
	while(fgets(line, sizeof(line), device) != NULL) {
		if(strstr(line, " YES") != NULL) {
			crcValid = 1;
		} else if((offset = strstr(line, "t=")) != NULL) {
			*temp = atoi(offset  + 2) / 1000.0;
		}
	}
	fclose(device);
	return crcValid;
}

void wind_sensor_spin() {
	windSpins++;
}

void rain_sensor_tip() {
	rainTips++;
}

void add_recorder(char* name, weather_recorder recorder) {
	recorder_list_item* item = malloc(sizeof(recorder_list_item));
	item->recorder = recorder;
	item->next = NULL;
	strncpy(item->name, name, sizeof(item->name));

	//Nothing in the list, add our pointer.
	if(recorderList == NULL) {
		recorderList = item;
		return;
	}

	//Add ourselves to the end.
	recorder_list_item* itr = recorderList;
	while(itr->next != NULL) {
		itr = itr->next;
	}
	itr->next = item;
}