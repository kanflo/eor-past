/* 
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Johan Kanflo (github.com/kanflo)
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __PAST_H__
#define __PAST_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	past_wifi_ssid = 10,
	past_wifi_pw,

	past_mqtt_server = 20,
	past_mqtt_port,
	past_mqtt_user,
	past_mqtt_pw,

	// Any id above 256 is free for the taking
	past_user_defined = 256
} past_t;

// Check if past if valid and return true if so
bool past_valid(void);

// Format past and return true if successful
bool past_format(void);

// Return size of past
uint32_t past_size(void);

// Read unit from past
// id - unit id
// data - pointer where unit data goes
// length - length of data buffer
// Returns true if unit was found and could be read
bool past_read_unit(past_t id, void *data, uint32_t length);

// Write unit to past
// id - unit id
// data - pointer where unit data goes
// length - length of data buffer
// Returns true if unit was found and could be read
bool past_write_unit(past_t id, void *data, uint32_t length);

// Erase unit from past
// id - unit id
// Returns true if unit was found and could be erased
bool past_erase_unit(past_t id);

// Debug: dump contents of past
void past_dump(void);

#endif // __PAST_H__
