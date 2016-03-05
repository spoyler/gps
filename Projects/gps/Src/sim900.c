/*
 * sim900.cpp
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
 

#include "sim900.h"
#include <stdlib.h>

UART_HandleTypeDef * uart;
extern UART_HandleTypeDef LUart;				// debug uart

//SoftwareSerial *serialSIM900 = NULL;

int millis()
{
	return HAL_GetTick();
}

void sim900_init(UART_HandleTypeDef * uart_device)
{
    uart = (UART_HandleTypeDef*)uart_device;
		//begin(baud);
}

int sim900_check_readable()
{
  return (uart->Instance->ISR & UART_FLAG_RXNE) ? 1 : 0;
}

int sim900_wait_readable (int wait_time)
{
    unsigned long timerStart;
    int dataLen = 0;
    timerStart = millis();
    while((unsigned long) (millis() - timerStart) > wait_time * 1000UL) {
        HAL_Delay(500);
        dataLen = sim900_check_readable();
        if(dataLen > 0){
            break;
        }
    }
    return dataLen;
}

void sim900_flush_serial()
{
	char c = 0;
	if (uart->Instance->ISR & UART_FLAG_ORE)
	{
		uart->Instance->ICR = (uart->Instance->ICR | USART_ISR_ORE);
	}
  while(sim900_check_readable())
	{
			char c = uart->Instance->RDR; 			//LUart.Instance->TDR = c;	
  }
}

void sim900_read_buffer(char *buffer, int count, unsigned int timeout/*, unsigned int chartimeout*/)
{
		const int chartimeout = 100;
    int i = 0;
    unsigned long timerStart, prevChar;
		char c = 0;
    timerStart = millis();
    prevChar = 0;
    while(1) {
        while (sim900_check_readable()) {
            c = uart->Instance->RDR;	//LUart.Instance->TDR = c;							
            prevChar = millis();
            buffer[i++] = c;
            if(i >= count)
							break;
        }
        if(i >= count)break;
       if ((unsigned long) (millis() - timerStart) > timeout * 1000UL) {
            break;
        }
        //If interchar Timeout => return FALSE. So we can return sooner from this function. Not DO it if we dont recieve at least one char (prevChar <> 0)
//        if (((unsigned long) (millis() - prevChar) > chartimeout) && (prevChar != 0)) {
//            break;
//        }				
    }
}

void sim900_clean_buffer(char *buffer, int count)
{
    for(int i=0; i < count; i++) {
        buffer[i] = '\0';
    }
}

//HACERR quitar esta funcion ?
void sim900_send_byte(uint8_t data)
{
	while(!(uart->Instance->ISR & UART_FLAG_TXE));
	uart->Instance->TDR = data;
}

void sim900_send_char(const char c)
{
	while(!(uart->Instance->ISR & UART_FLAG_TXE));
	uart->Instance->TDR = c;
}

void sim900_send_cmd(const char* cmd, int size)
{
  for(int i=0; /*i<size ||*/ cmd[i] != 0; i++)
    {
        sim900_send_byte(cmd[i]);
    }
}

void sim900_send_cmd_P(const char* cmd)
{
 // while (pgm_read_byte(cmd) != 0x00)
   // sim900_send_byte(pgm_read_byte(cmd++));  
}

void sim900_send_AT(void)
{
    sim900_check_with_cmd(("AT\r\n"),"OK",CMD);
}

void sim900_send_End_Mark(void)
{
    sim900_send_byte((char)26);
}

boolean sim900_wait_for_resp(const char* resp, DataType type)
{
	char str[64] = {0};
	unsigned int timeout = 2000;
	unsigned int chartimeout = 500;    
	int sum = 0;
	unsigned long timerStart, prevChar;    //prevChar is the time when the previous Char has been read.
	timerStart = millis();
	int resp_size = strlen(resp);
	while(1) {
			if(sim900_check_readable()) {
				str[sum] = uart->Instance->RDR; 			//LUart.Instance->TDR = str[sum];	
				if (str[sum] == resp[sum])
					sum++;
//				else
//					sum = 0;
				
				if (sum == resp_size)
					return true;
				
			}
			if ((unsigned long) (millis() - timerStart) > timeout) {
					return false;
			}
			//If interchar Timeout => return FALSE. So we can return sooner from this function.
//			if (((unsigned long) (millis() - prevChar) > chartimeout) && (prevChar != 0)) {
//					return false;
//			}
			
	}
	//If is a CMD, we will finish to read buffer.
	if(type == CMD)
		sim900_flush_serial();
	
	return false;   
}


boolean sim900_check_with_cmd(const char* cmd, const char *resp, DataType type)
{
    sim900_send_cmd(cmd, 0);
    return sim900_wait_for_resp(resp,type);
}
/*
//HACERR que tambien la respuesta pueda ser FLASH STRING
boolean sim900_check_with_cmd(const __FlashStringHelper* cmd, const char *resp, DataType type, unsigned int timeout, unsigned int chartimeout)
{
    sim900_send_cmd(cmd);
    return sim900_wait_for_resp(resp,type,timeout,chartimeout);
}
*/
