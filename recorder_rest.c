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
#ifdef USE_REST
#include "weather.h"
#include <curl/curl.h>

#define REST_ENDPOINT "http://localhost/test.php"

/**
 * This function records samples from the weather station to a REST endpoint.
 * @param start The sample start time in YYYY-MM-DD HH:MM:SS format.
 * @param end The sample end time in YYYY-MM-DD HH:MM:SS format.
 * @param temp The temperature to record.
 * @param wind The wind speed to record.
 * @param spins The number of ticks generated by the wind sensor.
 * @param rain The rain depth to record.
 * @param tips The number of tips of the rain meter.
 */
void record_samples_to_rest(char* start, char* end, float temp, float wind, int spins, float rain, int tips) {
    char json[4096];
    snprintf(json, sizeof(json), "{\"start\":\"%s\",\"end\":\"%s\",\"temp\":%f,\"wind\":%f,\"spins\":%u,\"rain\":%f,\"tips\":%u}",
        start, end, temp, wind, spins, rain, tips);

	CURL* curl = curl_easy_init();
    if(curl == NULL) { return; }

    curl_easy_setopt(curl, CURLOPT_URL, REST_ENDPOINT);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
    CURLcode err = curl_easy_perform(curl);
    if(err != CURLE_OK) {
        printf("Problem POSTing to %s: code %u - %s\n", REST_ENDPOINT, err, curl_easy_strerror(err));
    }
    curl_easy_cleanup(curl);
}

/**
 * Register ourselves with the plugin system.
 */
RECORDER_INIT(REST) {
    curl_global_init(CURL_GLOBAL_ALL);
    add_recorder("rest", &record_samples_to_rest);
}
#endif