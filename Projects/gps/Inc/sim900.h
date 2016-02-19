/*
 * sim900.h 
 * A library for SeeedStudio seeeduino GPRS shield 
 *  
 * Copyright (c) 2015 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : lawliet zou
 * Create Time: April 2015
 * Change Log :
 *
 * The MIT License (MIT)
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

#ifndef __SIM900_H__
#define __SIM900_H__

#include <stdint.h>
#include "stm32l0xx_nucleo_ihm04a1.h"

#define DEFAULT_TIMEOUT     		 	1   			//seconds
#define DEFAULT_INTERCHAR_TIMEOUT 100   //miliseconds

typedef int boolean;

typedef enum {
    CMD     = 0,
    DATA    = 1,
} DataType;

enum 
{
	false = 0,
	true = 1
};

void  sim900_init(UART_HandleTypeDef * uart_device);
int   sim900_check_readable(void);
int   sim900_wait_readable(int wait_time);
void  sim900_flush_serial(void);
//void  sim900_read_buffer(char* buffer,int count,  unsigned int timeout = DEFAULT_TIMEOUT, unsigned int chartimeout/* = DEFAULT_INTERCHAR_TIMEOUT*/);
void  sim900_read_buffer(char* buffer,int count,  unsigned int timeout/*, unsigned int chartimeout*/);
void  sim900_clean_buffer(char* buffer, int count);
void  sim900_send_byte(uint8_t data);
void  sim900_send_char(const char c);
//void  sim900_send_cmd(const char* cmd);
void sim900_send_cmd(const char* cmd, int size);
//void  sim900_send_cmd(const __FlashStringHelper* cmd);
void  sim900_send_cmd_P(const char* cmd);   
void  sim900_send_AT(void);
void  sim900_send_End_Mark(void);
boolean  sim900_wait_for_resp(const char* resp, DataType type);
boolean  sim900_check_with_cmd(const char* cmd, const char *resp, DataType type);
//boolean  sim900_check_with_cmd(const __FlashStringHelper* cmd, const char *resp, DataType type, unsigned int timeout = DEFAULT_TIMEOUT, unsigned int chartimeout = DEFAULT_INTERCHAR_TIMEOUT);

#endif
