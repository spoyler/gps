/*
 * GPRS_Shield_Arduino.cpp
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

#include <stdio.h>
#include <string.h>
#include "GPRS_Shield_Arduino.h"
#include "debug.h"
#include "gps.h"
#include "accelero.h"
#include "adc.h"
#include "command.h"

UART_HandleTypeDef UartGSM;			// gsm uart


uint32_t str_to_ip(const char* str);
uint32_t _ip;
char ip_string[16]; //XXX.YYY.ZZZ.WWW + \0
const char resp_ok[] = "OK\r\n";

const char host_name[] = "paulfertser.info";
const int host_port = 10123;

uint8_t tmp_data[512];

uint8_t is_not_first_data_send = 0;
uint8_t server_connection_state = 0;
uint8_t count_try_to_connect = 0;
extern char uin_string[32];
extern uint8_t uin_string_size;
extern const char eof[];



void delay(int time)
{
	HAL_Delay(time);
}

void itoa(int n, char * s)
 {
		sprintf(s, "%d", n);
 }



void GSM_Init(void)//:gprsSerial(tx,rx)
{

	// UART Init
	__USART2_CLK_ENABLE();
	UartGSM.Instance = USART2;
	UartGSM.Init.BaudRate   = 9600;
	UartGSM.Init.WordLength = UART_WORDLENGTH_8B;
	UartGSM.Init.StopBits   = UART_STOPBITS_1;
	UartGSM.Init.Parity     = UART_PARITY_NONE;
	UartGSM.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	UartGSM.Init.Mode       = UART_MODE_TX_RX;
	UartGSM.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	HAL_UART_DeInit(&UartGSM);  
  HAL_UART_Init(&UartGSM);
 /* Enable the UART Parity Error Interrupt */
	__HAL_UART_ENABLE_IT(&UartGSM, UART_IT_ORE);
	
	/* Process Unlocked */
	__HAL_UNLOCK(&UartGSM); 
	HAL_NVIC_EnableIRQ(USART2_IRQn);

	//
	sim900_init(&UartGSM);
	
	DEBUG_PRINTF("GSM power up...");
	powerUpDown();
	DEBUG_PRINTF("Ok\r\n");


	char c = 0;
	char cc = 0;
	
	DEBUG_PRINTF("UART GSM baudrate adjusting...");
	while(1)
	{		
		while(!(UartGSM.Instance->ISR & UART_FLAG_TXE));
		UartGSM.Instance->TDR = 'A';		
		HAL_Delay(10);		
		while (UartGSM.Instance->ISR & UART_FLAG_RXNE)
		{
			c = UartGSM.Instance->RDR;
		}		
		HAL_Delay(500);
		while(!(UartGSM.Instance->ISR & UART_FLAG_TXE));
		UartGSM.Instance->TDR = 'T';			
		
		HAL_Delay(10);
		
		while (UartGSM.Instance->ISR & UART_FLAG_RXNE)
		{
			cc = UartGSM.Instance->RDR;
		}
		
		if ((cc == 0x54) && (c == 0x41))
				break;
		
		HAL_Delay(500);
	}
	DEBUG_PRINTF("Ok\r\n");
	
	DEBUG_PRINTF("Sending AT...");
	while(!sim900_check_with_cmd("AT\r\n",resp_ok,CMD));
	DEBUG_PRINTF("Ok\r\n");
	    
	DEBUG_PRINTF("Set GSM full functional...");
	while(!sim900_check_with_cmd("AT+CFUN=1\r\n",resp_ok,CMD));
	DEBUG_PRINTF("Ok\r\n");
	
	DEBUG_PRINTF("Get signal level...");
	GetSignalLevel();
	DEBUG_PRINTF("\r\n");
  
	DEBUG_PRINTF("Check SIM status...");  
	while(!checkSIMStatus());
	DEBUG_PRINTF("Ok\r\n");
	
	const char manual_data_cmd[] = "AT+CIPRXGET=1\r\n";	
	sim900_check_with_cmd(manual_data_cmd, "OK\r\n", CMD);
		
	debug_simm800(&UartGSM);
	SetServerConnectionState(NOT_CONNECTED);
	count_try_to_connect = 0;	
}

void GSM_Task()
{
	char * ptr_gps_msg = 0;

	//if (Get_Accelero_State())
	{
		if (!is_connected())
		{			
			DEBUG_PRINTF("Connecting to %s:%d... ", host_name, host_port);
			if (connect(TCP,host_name, host_port, 3, 100))
			{
				DEBUG_PRINTF("Ok\r\n");
				is_not_first_data_send = 0;
				SetServerConnectionState(CONNECTED);
				count_try_to_connect = 0;
			}
			else
			{
				DEBUG_PRINTF("Error\r\n");
				
				count_try_to_connect++;
				if (count_try_to_connect < MAX_TRY_TO_CONNECT)
					SetServerConnectionState(NOT_CONNECTED);
				else
					SetServerConnectionState(ERROR_IN_CONNECTION);
			}
		}
		else //if (GetServerConnectionState() == CONNECTED)		
		{			
			while(1) 
			{
				// send the data
				uint8_t flags = 0;
				uint8_t * data = (uint8_t*)NULL;
				uint32_t data_size = ReadBuffer(&data, &flags);
				
			
				if ((data_size > 0) && data != NULL)
				{
					if (!is_not_first_data_send)
					{
						// send uin
						if(send(uin_string, uin_string_size) == uin_string_size)
							is_not_first_data_send = 1;
					}
								
					data[data_size] = 0;
					DEBUG_PRINTF("Data send %d...", data_size);
					if(send(data, data_size) == data_size)
					{
						DEBUG_PRINTF("Ok\r\n");
						
						if (!IsDataToSend())
						{
							// if we have not more data, send "eof"
							data_size = strlen(eof);
							if (send(eof, data_size) == data_size)
							{
								DEBUG_PRINTF("End of data, send \"EOF\"\r\n");
							}
							else
							{
								is_not_first_data_send = 0;
							}
						}
						// check message on events
						for (int i = EVENT_FREE_FALL; i < MAX_EVENTS; i++)
						{
							if ((i == EVENT_SLEEP) || (i == EVENT_WAKEUP))
								SetEventState(i, EVENT_SEND);
							else
								SetEventState(i, EVENT_NONE);
						}
					}
					else
					{
						DEBUG_PRINTF("Error\r\n");
						is_not_first_data_send = 0;
					}
				}
				else
					break;

			
				//recive data
				uint32_t start_tick = HAL_GetTick();
				while((data_size =  recv(tmp_data, 256)) == -1)
				{
					if ((HAL_GetTick() - start_tick ) > 2000)
						break;
				}

				DEBUG_PRINTF("RX_DATA = %d\r\n", data_size);
				
				uint8_t * answer = (uint8_t *) nullptr;
				
				data_size = ReadBufferAnsw(&answer);
				
				if (data_size > 0)
				{
					DEBUG_PRINTF("Send answer...");
					if(send(answer, data_size) == data_size)
					{
						DEBUG_PRINTF("Ok\r\n");		
						DEBUG_PRINTF("Connection close\r\n");
						close();					
						SetServerConnectionState(NOT_CONNECTED);
					}
					else
					{
						is_not_first_data_send = 0;
						DEBUG_PRINTF("Error\r\n");
					}
				}			
			}								
		}			
	}
	if (is_connected() && (GetEventState(EVENT_SLEEP) == EVENT_SEND))
	{
		close();
		SetServerConnectionState(NOT_CONNECTED);
	}
}

bool checkPowerUp(void)
{
  return sim900_check_with_cmd("AT\r\n",resp_ok,CMD);
}

void powerUpDown()
{
  // power on pulse
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);// digitalWrite(pin,LOW);
  delay(1000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);//  digitalWrite(pin,HIGH);
  delay(1000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);//digitalWrite(pin,LOW);
  delay(1000);
}

void GetSignalLevel()
{
		const char cmd[] = "AT+CSQ\r\n";
    char gprsBuffer[32] = {0};
    int count = 0;    		
		sim900_send_cmd(cmd, strlen(cmd));
		sim900_read_buffer(gprsBuffer,32,DEFAULT_TIMEOUT);
		DEBUG_PRINTF("%s",gprsBuffer );
}
  
bool checkSIMStatus(void)
{	
		const char cmd[] = "AT+CGREG?\r\n";
    char gprsBuffer[32];
    int count = 0;
    sim900_clean_buffer(gprsBuffer,32);
    while(count < 3) {
        sim900_send_cmd(cmd, strlen(cmd));
        sim900_read_buffer(gprsBuffer,32,DEFAULT_TIMEOUT);
        if((NULL != strstr(gprsBuffer,"+CGREG: 0,1"))
				|| (NULL != strstr(gprsBuffer,"+CGREG: 0,5")))
					
					{
            return true;
        }
        count++;
        delay(300);
    }

		return false;
}

bool callUp(char *number)
{
    //char cmd[24];
    if(!sim900_check_with_cmd("AT+COLP=1\r\n","OK\r\n",CMD)) {
        return false;
    }
    delay(1000);
	//HACERR quitar SPRINTF para ahorar memoria ???
  //sprintf(cmd,"ATD%s;\r\n", number);
  //sim900_send_cmd(cmd);
	sim900_send_cmd("ATD", sizeof("ATD"));
	sim900_send_cmd(number, strlen(number));
	sim900_send_cmd(";\r\n", 3);
    return true;
}

void answer(void)
{
    sim900_send_cmd("ATA\r\n", sizeof("ATA\r\n"));  //TO CHECK: ATA doesnt return "OK" ????
}

bool hangup(void)
{
    return sim900_check_with_cmd("ATH\r\n","OK\r\n",CMD);
}

bool disableCLIPring(void)
{
    return sim900_check_with_cmd("AT+CLIP=0\r\n","OK\r\n",CMD);
}

bool getSubscriberNumber(char *number)
{
	//AT+CNUM								--> 7 + CR = 8
	//+CNUM: "","+628157933874",145,7,4		--> CRLF + 45 + CRLF = 49
	//										-->
	//OK									--> CRLF + 2 + CRLF = 6

    int i = 0;
    char gprsBuffer[65];
    char *p,*s;
	sim900_flush_serial();
    sim900_send_cmd("AT+CNUM\r\n", sizeof("AT+CNUM\r\n"));
    sim900_clean_buffer(gprsBuffer,65);
    sim900_read_buffer(gprsBuffer,65,DEFAULT_TIMEOUT);
	//Serial.print(gprsBuffer);
    if(NULL != ( s = strstr(gprsBuffer,"+CNUM:"))) {
        s = strstr((char *)(s),",");
        s = s + 2;  //We are in the first phone number character 
        p = strstr((char *)(s),"\""); //p is last character """
        if (NULL != s) {
            i = 0;
            while (s < p) {
              number[i++] = *(s++);
            }
            number[i] = '\0';
        }
        return true;
    }  
    return false;
}

bool isCallActive(char *number)
{
    char gprsBuffer[46];  //46 is enough to see +CPAS: and CLCC:
    char *p, *s;
    int i = 0;

    sim900_send_cmd("AT+CPAS\r\n", sizeof("AT+CPAS\r\n"));
    /*Result code:
        0: ready
        2: unknown
        3: ringing
        4: call in progress
    
      AT+CPAS   --> 7 + 2 = 9 chars
                --> 2 char              
      +CPAS: 3  --> 8 + 2 = 10 chars
                --> 2 char
      OK        --> 2 + 2 = 4 chars
    
      AT+CPAS
      
      +CPAS: 0
      
      OK
    */

    sim900_clean_buffer(gprsBuffer,29);
    sim900_read_buffer(gprsBuffer,27, 0);
    //HACERR cuando haga lo de esperar a OK no me haría falta esto
    //We are going to flush serial data until OK is recieved
    sim900_wait_for_resp("OK\r\n", CMD);    
    //Serial.print("Buffer isCallActive 1: ");Serial.println(gprsBuffer);
    if(NULL != ( s = strstr(gprsBuffer,"+CPAS:"))) {
      s = s + 7;
      if (*s != '0') {
         //There is something "running" (but number 2 that is unknow)
         if (*s != '2') {
           //3 or 4, let's go to check for the number
           sim900_send_cmd("AT+CLCC\r\n", sizeof("AT+CLCC\r\n"));
           /*
           AT+CLCC --> 9
           
           +CLCC: 1,1,4,0,0,"656783741",161,""
           
           OK  

           Without ringing:
           AT+CLCC
           OK              
           */

           sim900_clean_buffer(gprsBuffer,46);
           sim900_read_buffer(gprsBuffer,45, 0);
			//Serial.print("Buffer isCallActive 2: ");Serial.println(gprsBuffer);
           if(NULL != ( s = strstr(gprsBuffer,"+CLCC:"))) {
             //There is at least one CALL ACTIVE, get number
             s = strstr((char *)(s),"\"");
             s = s + 1;  //We are in the first phone number character            
             p = strstr((char *)(s),"\""); //p is last character """
             if (NULL != s) {
                i = 0;
                while (s < p) {
                    number[i++] = *(s++);
                }
                number[i] = '\0';            
             }
             //I need to read more buffer
             //We are going to flush serial data until OK is recieved
             return sim900_wait_for_resp("OK\r\n", CMD); 
           }
         }
      }        
    } 
    return false;
}

bool getDateTime(char *buffer)
{
	//AT+CCLK?						--> 8 + CR = 9
	//+CCLK: "14/11/13,21:14:41+04"	--> CRLF + 29+ CRLF = 33
	//								
	//OK							--> CRLF + 2 + CRLF =  6

    uint8_t i = 0;
    char gprsBuffer[50];
    char *p,*s;
	sim900_flush_serial();
    sim900_send_cmd("AT+CCLK?\r", sizeof("AT+CCLK?\r"));
    sim900_clean_buffer(gprsBuffer,50);
    sim900_read_buffer(gprsBuffer,50,DEFAULT_TIMEOUT);
    if(NULL != ( s = strstr(gprsBuffer,"+CCLK:"))) {
        s = strstr((char *)(s),"\"");
        s = s + 1;  //We are in the first phone number character 
        p = strstr((char *)(s),"\""); //p is last character """
        if (NULL != s) {
            i = 0;
            while (s < p) {
              buffer[i++] = *(s++);
            }
            buffer[i] = '\0';            
        }
        return true;
    }  
    return false;
}

bool getSignalStrength(int *buffer)
{
	//AT+CSQ						--> 6 + CR = 10
	//+CSQ: <rssi>,<ber>			--> CRLF + 5 + CRLF = 9						
	//OK							--> CRLF + 2 + CRLF =  6

	uint8_t i = 0;
	char gprsBuffer[26];
	char *p, *s;
	char buffers[4];
	sim900_flush_serial();
	sim900_send_cmd("AT+CSQ\r", sizeof("AT+CSQ\r"));
	sim900_clean_buffer(gprsBuffer, 26);
	sim900_read_buffer(gprsBuffer, 26, DEFAULT_TIMEOUT);
	if (NULL != (s = strstr(gprsBuffer, "+CSQ:"))) {
		s = strstr((char *)(s), " ");
		s = s + 1;  //We are in the first phone number character 
		p = strstr((char *)(s), ","); //p is last character """
		if (NULL != s) {
			i = 0;
			while (s < p) {
				buffers[i++] = *(s++);
			}
			buffers[i] = '\0';
		}
		*buffer = atoi(buffers);
		return true;
	}
	return false;
}

bool sendUSSDSynchronous(char *ussdCommand, char *resultcode, char *response)
{
	//AT+CUSD=1,"{command}"
	//OK
	//
	//+CUSD:1,"{response}",{int}

		uint8_t i = 0;
    char gprsBuffer[200];
    char *p,*s;
    sim900_clean_buffer(response, sizeof(response));
	
	sim900_flush_serial();
    sim900_send_cmd("AT+CUSD=1,\"", sizeof("AT+CUSD=1,\""));
    sim900_send_cmd(ussdCommand, 0);
    sim900_send_cmd("\"\r", sizeof("\"\r"));
	if(!sim900_wait_for_resp("OK\r\n", CMD))
		return false;
    sim900_clean_buffer(gprsBuffer,200);
    sim900_read_buffer(gprsBuffer,200,DEFAULT_TIMEOUT);
    if(NULL != ( s = strstr(gprsBuffer,"+CUSD: "))) {
        *resultcode = *(s+7);
		resultcode[1] = '\0';
		if(!('0' <= *resultcode && *resultcode <= '2'))
			return false;
		s = strstr(s,"\"");
        s = s + 1;  //We are in the first phone number character
        p = strstr(s,"\""); //p is last character """
        if (NULL != s) {
            i = 0;
            while (s < p) {
              response[i++] = *(s++);
            }
            response[i] = '\0';            
        }
		return true;
	}
	return false;
}

bool cancelUSSDSession(void)
{
    return sim900_check_with_cmd("AT+CUSD=2\r\n","OK\r\n",CMD);
}

void disconnect()
{
	const char cmd[] = "AT+CIPSHUT\r\n";
  sim900_send_cmd(cmd, strlen(cmd));
}

bool connect(Protocol ptl,const char * host, int port, int timeout, int chartimeout)
{
    char cmd[128] = {0};
    char resp[128] = {0};
	uint32_t size = 0;


    //sim900_clean_buffer(cmd,64);
    if(ptl == TCP) {
      size = sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",host, port);
    } 
		else 
			if(ptl == UDP) {
        size = sprintf(cmd, "AT+CIPSTART=\"UDP\",\"%s\",%d\r\n",host, port);
    } else {
        return false;
    }
    

    sim900_send_cmd(cmd, size);
    sim900_read_buffer(resp, 128, timeout);
    if((strstr(resp,"CONNECT OK") != NULL) ||
			 (strstr(resp, "ALREADY CONNECT") != NULL))	
        return true;
		
    return false;
}


bool is_connected(void)
{
    char resp[96] = {0};
    sim900_send_cmd("AT+CIPSTATUS\r\n", strlen("AT+CIPSTATUS\r\n"));
    sim900_read_buffer(resp,sizeof(resp),DEFAULT_TIMEOUT);
    if(NULL != strstr(resp,"STATE: CONNECT OK")) {
        //+STATE: CONNECT OK
        return true;
    } else {
        //+CIPSTATUS: 1,0,"TCP","216.52.233.120","80","CLOSED"
        //+CIPSTATUS: 0,,"","","","INITIAL"
        return false;
    }
}

bool close()
{
    // if not connected, return
    if (!is_connected()) {
        return true;
    }
    return sim900_check_with_cmd("AT+CIPCLOSE\r\n", "CLOSE OK\r\n", CMD);
}

int readable(void)
{
    return sim900_check_readable();
}

int wait_readable(int wait_time)
{
    return sim900_wait_readable(wait_time);
}

int wait_writeable(int req_size)
{
    return req_size+1;
}

int send(const char * str, int len)
{
	const uint32_t wait_data_ok = 2000;
  char cmd[256] = {0};
	
	DEBUG_PRINTF("%s", str);
	if(len > 0)
	{
		sprintf(cmd,"AT+CIPSEND=%d\r\n",len);

	if(sim900_check_with_cmd(cmd,">",CMD)) 
	{
			sim900_flush_serial();
			sim900_send_cmd(str, len);
			uint32_t tick = HAL_GetTick();
			while(!sim900_wait_for_resp("SEND OK\r\n", DATA))  /*, DEFAULT_TIMEOUT * 10*, *DEFAULT_INTERCHAR_TIMEOUT * 10)*/
			{
				if ((HAL_GetTick() - tick ) > wait_data_ok)
				{
					return 0;
				}
			}        
    }
		else
		{
			return 0;
		}
		return len;
	}
	return 0;
}
    

int recv(char* buf, int len)
{
	
	/*
		AT+CIPRXGET=2,1460
	+CIPRXGET:2,11,0
	HELLOW WORLD
	
	OK
	+CIPRXGET:1
	*/
	char cmd[32] = {0};
	int overheads = 64;	
			
	sim900_clean_buffer(buf,len);	
	int size = sprintf(cmd, "AT+CIPRXGET=2,%d\r\n", len);
	sim900_send_cmd(cmd, size);
	sim900_read_buffer(buf, len + overheads,DEFAULT_TIMEOUT);
	
	char * ptr = nullptr;
	
	ptr = strstr(buf, "+CIPRXGET: 2,");
	
	if (ptr != nullptr)
	{
		ptr += strlen("+CIPRXGET: 2,");
		
		char * end_ptr =  (char *)strchr(ptr, ',');
		
		if (end_ptr != nullptr)
		{
			char number[8] = {0};		
			int size = (uint32_t)end_ptr - (uint32_t)(ptr);
			size = size < 8 ? size : 8;
			strncpy ( number, ptr, size );
			
			char * data = strstr(end_ptr, "\r\n");
			
			if (data != nullptr)
			{
				data += strlen("\r\n");
				
				int num_recv_byte = atoi(number);
				
				//num_recv_byte = sprintf(data, "$CMD,W,0,1\r\n");
				
				if ((num_recv_byte > 0) && (num_recv_byte <= len))
				{
					memcpy(buf, data, num_recv_byte);
					Parse_Command(buf, num_recv_byte);
					return num_recv_byte;				
				}
			}
		}		
	}

	return -1;
}

void listen(void)
{
//	gprsSerial.listen();
}

bool isListening(void)
{
//	return gprsSerial.isListening();
	return 0;
}

uint32_t str_to_ip(const char* str)
{
    uint32_t ip = 0;
    char* p = (char*)str;
    for(int i = 0; i < 4; i++) {
        ip |= atoi(p);
        p = strchr(p, '.');
        if (p == NULL) {
            break;
        }
        ip <<= 8;
        p++;
    }
    return ip;
}

char* getIPAddress()
{
    //I have already a buffer with ip_string: snprintf(ip_string, sizeof(ip_string), "%d.%d.%d.%d", (_ip>>24)&0xff,(_ip>>16)&0xff,(_ip>>8)&0xff,_ip&0xff); 
    return ip_string;
}

unsigned long getIPnumber()
{
    return _ip;
}

uint8_t Set_GSM_Sleep_Mode()
{
	const char cmd_sleep[] = "AT+CPOWD=1\r\n";
	char resp[64] = {0};

  sim900_send_cmd(cmd_sleep, strlen(cmd_sleep));
  sim900_read_buffer(resp, 64, 1);
  if (strstr(resp,"NORMAL POWER DOWN") != NULL)
		return true;
	else
    return false;
}

void SetServerConnectionState(uint8_t new_state)
{
	server_connection_state = new_state;
}

uint8_t GetServerConnectionState(void)
{
	return server_connection_state;
}


void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartGSM);
}
