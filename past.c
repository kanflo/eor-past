/* 
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Johan Kanflo (github.com/kanflo)
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

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <espressif/spi_flash.h>
#include <past.h>

/*
 * past - parameter storage
 * past enables parameter storage in the internal SPI flash of the ESP8266.
 * Parameters are stored by integer id
 */

// TODO:
//  - Describe format
//  - Add flag field following the magic
//  - Garbage collection


#define PAST_MAGIC "past"

#define PAST_UNITID_INVALID (0L)
#define PAST_UNITID_END     (0xffffffff)

#define PAST_SECTOR (0x3c)
#define PAST_SIZE SPI_FLASH_SEC_SIZE

#ifdef PAST_DEBUG
 #define PASTLOG(x) x
#else // PAST_DEBUG
 #define PASTLOG(x)
#endif // PAST_DEBUG

static uint32_t past_end_addr;

static uint32_t past_find_unit(past_t id);

bool past_valid(void)
{
	uint8_t magic[5];
	past_end_addr = past_find_unit(PAST_UNITID_END);
	(void) sdk_spi_flash_read(PAST_SECTOR*SPI_FLASH_SEC_SIZE, (void*) magic, strlen(PAST_MAGIC));
	magic[4] = 0;
	return (strcmp(PAST_MAGIC, (char*) magic) == 0) && (past_end_addr != 0);
}

bool past_format(void)
{
	bool success = false;
	past_end_addr = 0;
	do {
		if (SPI_FLASH_RESULT_OK != sdk_spi_flash_erase_sector(PAST_SECTOR))
			break;
		if (SPI_FLASH_RESULT_OK != sdk_spi_flash_write(PAST_SECTOR*SPI_FLASH_SEC_SIZE, (void*) PAST_MAGIC, 4))
			break;
		past_end_addr = past_find_unit(PAST_UNITID_END);
		if (past_end_addr)
			success = true;
	} while(0);
	return success;
}

uint32_t past_size(void)
{
	if (!past_end_addr) {
		past_end_addr = past_find_unit(PAST_UNITID_END);
	}
	if (past_end_addr)
		return past_end_addr - PAST_SECTOR*SPI_FLASH_SEC_SIZE;
	else
		return 0;
}

bool past_read_unit(past_t id, void *data, uint32_t length)
{
	uint32_t addr = past_find_unit(id);
	bool success = false;
	if (addr) {
		do {
			uint32_t len;
			if (SPI_FLASH_RESULT_OK != sdk_spi_flash_read(addr+4, (void*) &len, 4))
				break;
			if (len < length) {
				length = len;
			}
			if (SPI_FLASH_RESULT_OK != sdk_spi_flash_read(addr+8, (void*) &len, 4))
				break;
			if (SPI_FLASH_RESULT_OK != sdk_spi_flash_read(addr+8, data, length))
				break;
			success = true;
		} while(0);
	}
	return success;
}

bool past_write_unit(past_t id, void *data, uint32_t length)
{
	bool success = false;
	if (!past_end_addr)
		past_end_addr = past_find_unit(PAST_UNITID_END);
	if (past_end_addr) {
		do {
			uint32_t old_addr = past_find_unit(id);
			PASTLOG(printf("Writing new unit at 0x%08x\n", (unsigned int) past_end_addr));
			if (SPI_FLASH_RESULT_OK != sdk_spi_flash_write(past_end_addr, (void*) &id, 4))
				break;
			if (SPI_FLASH_RESULT_OK != sdk_spi_flash_write(past_end_addr+4, (void*) &length, 4))
				break;
			if (SPI_FLASH_RESULT_OK != sdk_spi_flash_write(past_end_addr+8, data, length))
				break;
			past_end_addr += 8 + length;
			if (past_end_addr%4) past_end_addr += 4 - (past_end_addr%4); // Word align
			if (old_addr != 0) {
				uint32_t zero = 0;
				PASTLOG(printf("Erasing old unit at 0x%08x\n", (unsigned int) old_addr));
				if (SPI_FLASH_RESULT_OK != sdk_spi_flash_write(old_addr, (void*) &zero, 4))
					break;
			}
			success = true;
		} while(0);

	}
	return success;
}

bool past_erase_unit(past_t id)
{
	uint32_t old_addr = past_find_unit(id);
	bool success = false;
	if (old_addr) {
		uint32_t zero = 0;
		if (SPI_FLASH_RESULT_OK == sdk_spi_flash_write(old_addr, (void*) &zero, 4))
			success = true;
	}
	return success;
}

void past_dump(void)
{
	printf("Past dump:\n");
	for (int i = 0; i < PAST_SIZE; i++) {
		uint32_t data;
		if (i && i%16==0) printf("\n");
		(void) sdk_spi_flash_read(PAST_SECTOR*SPI_FLASH_SEC_SIZE+i, (void*) &data, 1);
		printf(" %02x", (uint8_t) (data & 0xff));
	}
	printf("\n");
}

static uint32_t past_find_unit(past_t id)
{
	bool found = false;
	uint32_t cur_id, len;
	uint32_t addr = PAST_SECTOR*SPI_FLASH_SEC_SIZE + 4;

	PASTLOG(printf("Finding unit 0x%08x\n at 0x%08x", (unsigned int) id, (unsigned int) addr));
	while(1) {
		(void) sdk_spi_flash_read(addr, (void*) &cur_id, 4);
		(void) sdk_spi_flash_read(addr+4, (void*) &len, 4);
		PASTLOG(printf(" > 0x%08x\n", (unsigned int) cur_id));
		if (cur_id == id) {
			found = true;
			break;
		} else if (cur_id == PAST_UNITID_END) {
			break;
		} else if (cur_id & 0xff000000) { // We have run astray
			break;
		}
		addr += 8 + len;
		if (addr%4) addr += 4 - (addr%4); // Word align
	}
	PASTLOG(if (found) printf("  Found at 0x%08x\n", (unsigned int) addr); else printf("  Not found\n"));
	return found ? addr : 0;
}
