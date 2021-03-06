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
#ifdef USE_CSV
#include <stdio.h>
#include "weather.h"

/**
 * The name of the CSV file to append to.
 */
#define CSV_FILE "weather.csv"
/**
 * The CSV output format.
 */
#define CSV_FORMAT "%s,%s,%f,%f,%i,%f,%i\n"

/**
 * This function records samples from the weather station to a CSV file.
 * @param startStr The sample start time in YYYY-MM-DD HH:MM:SS format.
 * @param endStr The sample end time in YYYY-MM-DD HH:MM:SS format.
 * @param temp The temperature to record.
 * @param wind The wind speed to record.
 * @param spins The number of ticks generated by the wind sensor.
 * @param rain The rain depth to record.
 * @param tips The number of tips of the rain meter.
 */
void record_samples_to_csv(char* startStr, char* endStr, float temp, float wind, int spins, float rain, int tips) {
	FILE* fp = fopen(CSV_FILE, "a");
	fprintf(fp, CSV_FORMAT, startStr, endStr, temp, wind, spins, rain, tips);
	fclose(fp);
}

/**
 * Register ourselves with the plugin system.
 */
RECORDER_INIT(CSV) {
    add_recorder("csv", &record_samples_to_csv);
}
#endif