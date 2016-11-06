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
#include "watchdog.h"


extern SSystemInfo system_info;
extern const char uin[];
UART_HandleTypeDef UartGSM;			// gsm uart


uint32_t str_to_ip(const char* str);
uint32_t _ip;
char ip_string[16]; //XXX.YYY.ZZZ.WWW + \0
const char resp_ok[] = "OK\r\n";


const uint16_t tmp_data_size = 512;
uint8_t tmp_data[tmp_data_size];

uint8_t is_not_first_data_send = 0;
uint8_t server_connection_state = 0;
uint8_t count_try_to_connect = 0;
extern char uin_string[32];
extern uint8_t uin_string_size;
extern const char eof[];
extern const char event[];

const char traker_id[] = "$traker_id=";
const char server_addr[] = "$server_addr=";
const char read_messages_intervel = 10;	// read sms message everea 10 seconds
const char connecting_timeout = 30; 		// 30 seconds wait befor reconnecting
uint32_t last_read_messages_time = 0;
uint32_t connecting_start_time = 0;
void Add_Error_InConnection();


void delay(int time)
{
	HAL_Delay(time);
}

void itoa(int n, char * s)
 {
		sprintf(s, "%d", n);
 }

 bool SetBoudrate(rate)
 {
	 bool result = false;
	 const char * rate_cmd = "AT+IPR=";
	 char command[16] = {0};
	 sprintf(command, "%s%d\r\n", rate_cmd, rate);
	 	 
	 result = sim900_check_with_cmd(command, resp_ok, CMD);
	 
	 result &=sim900_check_with_cmd("AT&W\r\n", resp_ok, CMD);
	 
	 return result;
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
		
	DEBUG_PRINTF("GSM baudrate adjusting...");
	
	sim900_flush_serial();
	sim900_flush_serial();
	sim900_flush_serial();
	
	if (!sim900_check_with_cmd("AT\r\n",resp_ok,CMD))
	{
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
	}
	DEBUG_PRINTF("Ok\r\n");
	
	sim900_flush_serial();
	sim900_flush_serial();
	sim900_flush_serial();
	sim900_flush_serial();

	
	sim900_flush_serial();
	sim900_flush_serial();
	sim900_flush_serial();
	
	DEBUG_PRINTF("Sending AT...");
	while(!sim900_check_with_cmd("AT\r\n",resp_ok,CMD));
	DEBUG_PRINTF("Ok\r\n");
	    	  
	DEBUG_PRINTF("Check SIM status...");  
	while(!checkSIMStatus());
	DEBUG_PRINTF("Ok\r\n");
	
	DEBUG_PRINTF("Get signal level...");
	GetSignalLevel();
	DEBUG_PRINTF("\r\n");
	
	const char text_mode_sms[] = "AT+CMGF=1\r\n";
	sim900_check_with_cmd(text_mode_sms, "OK\r\n", CMD);
	
	DEBUG_PRINTF("Set GSM full functional...");
	while(!sim900_check_with_cmd("AT+CFUN=1\r\n",resp_ok,CMD));
	DEBUG_PRINTF("Ok\r\n");
	
	const char manual_data_cmd[] = "AT+CIPRXGET=1\r\n";	
	sim900_check_with_cmd(manual_data_cmd, "OK\r\n", CMD);
	
	SetBoudrate(9600);
		
	//debug_simm800(&UartGSM);
	SetServerConnectionState(NOT_CONNECTED);
	count_try_to_connect = 0;
}
void GSM_Debug()
{
	DEBUG_PRINTF("Get signal level...");
	GetSignalLevel();
	DEBUG_PRINTF("\r\n");
  
	DEBUG_PRINTF("Check SIM status...");  
	while(!checkSIMStatus());
	DEBUG_PRINTF("Ok\r\n");
}

void GSM_Task()
{
	char * ptr_gps_msg = 0;
	bool data_not_send = false;
	uint8_t tcp_state =  IP_INITIAL;
	bool send_eof = false;

	//if (Get_Accelero_State())
	
	if ((HAL_GetTick() - last_read_messages_time) > read_messages_intervel*1000)
	{
		Read_Messages();
		last_read_messages_time = HAL_GetTick();
	}
	
	if (GetServerConnectionState() == NEED_TO_REBOOT)
	{
		DEBUG_PRINTF("Reboot GSM modul\r\n");
		Set_GSM_Sleep_Mode();
		GSM_Init();
	}
	
	
		tcp_state = is_connected();
	
		if (tcp_state == TCP_CLOSED 
			&& GetServerConnectionState() == CONNECTING)
		{
			close();
			Add_Error_InConnection();
		}
	
		if (tcp_state == TCP_CLOSED)
		{			
			is_not_first_data_send = 0;
			DEBUG_PRINTF("Connecting to %s:%d... ", system_info.host_name, system_info.host_port);
			SetServerConnectionState(CONNECTING);
			// start connection to server
			if (connect(TCP,system_info.host_name, system_info.host_port, 1, 50))
			{
				DEBUG_PRINTF("Ok\r\n");
				is_not_first_data_send = 0;
				SetServerConnectionState(CONNECTED);
			}
		}

		tcp_state = is_connected();				
		bool timeout = ((HAL_GetTick() - connecting_start_time) > connecting_timeout*1000);
		//
		if (((tcp_state != TCP_CONNECTING)	&&(tcp_state != TCP_CONNECT_OK))
			||((tcp_state != TCP_CONNECT_OK)	&&(timeout)))
		{
			close();	
			Add_Error_InConnection();
		}

		
		//else 
		if (is_connected() == TCP_CONNECT_OK)		
		{			
			if (GetServerConnectionState() == CONNECTING)
			{
				SetServerConnectionState(CONNECTED);
				DEBUG_PRINTF("Ok\r\n");
			}
			//while(1) 
			{
				// send the data
				count_try_to_connect = 0;
				uint8_t flags = 0;
				uint8_t * data = (uint8_t*)NULL;
				uint32_t data_size = ReadBuffer(&data, &flags);
				memset(tmp_data, 0, tmp_data_size);
				uint16_t tmp_data_pos = 0;
				
			
				if ((data_size > 0) && data != NULL)
				{
					HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);
					if (!is_not_first_data_send)
					{
						// send uin
						tmp_data_pos = sprintf((char *)tmp_data, "%s", uin_string);
						is_not_first_data_send = 1;
					}
								
					tmp_data_pos += sprintf((char *)&tmp_data[tmp_data_pos], "%s", data);
					data[data_size] = 0;

					
					// set event state-------------------------------------------------------
					for (int i = EVENT_FREE_FALL; i < MAX_EVENTS; i++)
					{
						if (GetEventState(i) == EVENT_ACTIVE)
						{
							tmp_data_pos += sprintf((char *)&tmp_data[tmp_data_pos], "%s,%d,0\r\n", event, i);
							
							SetEventState(i, EVENT_PUSHED_TO_BUFFER);
							// we must make sure that the event is delivered
							// flags will be resets, when data will be delivered
							SetBufferFlags(i);
							//
							if (i == EVENT_SLEEP)
							{
								//flush message buffer and sed eof
								while(ReadBuffer(&data, &flags));
							}
						}			
					}					
					
					if (!IsDataToSend())
					{
						// if we have not more data, send "eof"
						tmp_data_pos += sprintf((char *)&tmp_data[tmp_data_pos], "%s", eof);
						send_eof  = true;
					}
						
					DEBUG_PRINTF("%s", tmp_data);
					DEBUG_PRINTF("Data send %d...", tmp_data_pos);
					if(send((const char*)tmp_data, tmp_data_pos) == tmp_data_pos)
					{
						DEBUG_PRINTF("Ok\r\n");
						// check message on events
						for (int i = EVENT_FREE_FALL; i < MAX_EVENTS; i++)
						{
							if (GetEventState(i) == EVENT_PUSHED_TO_BUFFER)
							{
							if ((i == EVENT_SLEEP) || (i == EVENT_WAKEUP))
								SetEventState(i, EVENT_SEND);
							else
								SetEventState(i, EVENT_NONE);
							}
						}
					}
					else
					{
						for (int i = EVENT_FREE_FALL; i < MAX_EVENTS; i++)
						{
							if (GetEventState(i) == EVENT_PUSHED_TO_BUFFER)
								SetEventState(i, EVENT_ACTIVE);
						}
						close();
						data_not_send = true;
						DEBUG_PRINTF("Error\r\n");
					}
				}
				HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);
				
				if (data_not_send)
				{
					return;
				}
			
				//recive data
				if (send_eof)
				{
					uint32_t start_tick = HAL_GetTick();
					while((data_size =  recv((char *)tmp_data, 256)) == -1)
					{
						if ((HAL_GetTick() - start_tick ) > 2000)
							break;
					}

					DEBUG_PRINTF("RX_DATA = %d\r\n", data_size);
					
					uint8_t * answer = (uint8_t *) nullptr;
					
					data_size = ReadBufferAnsw(&answer);
					
					if (data_size > 0)
					{
						DEBUG_PRINTF("%s", answer);
						DEBUG_PRINTF("Send answer...");
						if(send(answer, data_size) == data_size)
						{
							DEBUG_PRINTF("Ok\r\n");		
							DEBUG_PRINTF("Connection close\r\n");
							close();					
						}
						else
						{
							close();
							is_not_first_data_send = 0;
							DEBUG_PRINTF("Error\r\n");
						}
					}
				}				
			}								
		}			

	if (is_connected() == TCP_CLOSED && (GetEventState(EVENT_SLEEP) == EVENT_SEND))
	{
		close();
	}
}

void Add_Error_InConnection()
{
	DEBUG_PRINTF("Error\r\n");
	count_try_to_connect++;
	if (count_try_to_connect < MAX_TRY_TO_CONNECT)
	{
		// try to connect again 
		SetServerConnectionState(NOT_CONNECTED);
	}
	else
	{
		if ((count_try_to_connect >= MAX_TRY_TO_CONNECT) &&
			 (count_try_to_connect < MAX_TRY_NEED_TO_REBOOT))
		{
			// we cannot connect to server, enter in the sleeping mode without send at "sleep event"
			SetServerConnectionState(ERROR_IN_CONNECTION);
		}
		else
		{
			// too many times to connect, something wrong, reboot gsm module
			SetServerConnectionState(NEED_TO_REBOOT);
		}
	}
}

bool checkPowerUp(void)
{
  return sim900_check_with_cmd("AT\r\n",resp_ok,CMD);
}

void powerUpDown()
{
	Set_GSM_Sleep_Mode();
  // power on pulse
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);// digitalWrite(pin,LOW);
  delay(1000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);//  digitalWrite(pin,HIGH);
  delay(1000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);//digitalWrite(pin,LOW);
  delay(1000);
}

void GetSignalLevel()
{
		const char cmd[] = "AT+CSQ\r\n";
    char gprsBuffer[32] = {0};
    char * tmp_pos = 0;
		char * tmp_pos_last = 0; 
		char * pos_del = 0;
		int rssi = 0;
		int ber = 0;
		sim900_send_cmd(cmd, strlen(cmd));
		sim900_read_buffer(gprsBuffer,32,DEFAULT_TIMEOUT);

		tmp_pos = gprsBuffer;
		while(1)
		{
			tmp_pos = (char*)strstr(tmp_pos, "+CSQ: ");
			if (tmp_pos != nullptr)
			{
				tmp_pos_last = tmp_pos;
				tmp_pos += sizeof("+CSQ: ");
			}
			else
				break;
		}
		tmp_pos = tmp_pos_last;
		if(tmp_pos != nullptr)
		{
			tmp_pos += sizeof("+CSQ: ") - 1;
			pos_del = (char*)strchr(tmp_pos, ',');
			if (pos_del != nullptr)
			{
				char number[8] = {0};		
				int size = (uint32_t)pos_del - (uint32_t)(tmp_pos);
				size = size < 8 ? size : 8;
				strncpy ( number, tmp_pos, size );
				rssi = atoi(number) * 2 - 113;
				
				if (pos_del != nullptr)
				{
					pos_del += 1;
					tmp_pos = nullptr;
					tmp_pos = (char*)strchr(pos_del, '\r');
					
					if (tmp_pos != nullptr)
					{
						memset(number, 0, 8);
						size = (uint32_t)tmp_pos - (uint32_t)(pos_del);
						size = size < 8 ? size : 8;
						strncpy ( number, pos_del, size );
						ber = atoi(number);
					}
				}
			}
			DEBUG_PRINTF("\r\nRSSI = %ddBm, BER = %d",rssi, ber);
		}
		else
			DEBUG_PRINTF("Error");
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


bool getDateTime(char *buffer)
{
	//AT+CCLK? --> 8 + CR = 9
	//+CCLK: "14/11/13,21:14:41+04"	--> CRLF + 29+ CRLF = 33
	//								
	//OK --> CRLF + 2 + CRLF =  6

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
		
	// save time when we try to connect
	connecting_start_time = HAL_GetTick();
		
    return false;
}


char is_connected(void)
{
    char resp[96] = {0};
    sim900_send_cmd("AT+CIPSTATUS\r\n", strlen("AT+CIPSTATUS\r\n"));
    sim900_read_buffer(resp,sizeof(resp),DEFAULT_TIMEOUT);
    if(NULL != strstr(resp,"STATE: CONNECT OK")) {
        //+STATE: CONNECT OK
        return TCP_CONNECT_OK;
		}
		else
		{				
			if (NULL != strstr(resp,"STATE: TCP CONNECTING")
				//||NULL != strstr(resp,"STATE: IP INITIAL")
				||NULL != strstr(resp,"STATE: IP START")
				||NULL != strstr(resp,"STATE: IP CONFIG") 
				||NULL != strstr(resp,"STATE: IP GPRSACT")
				||NULL != strstr(resp,"STATE: IP STATUS"))
			{
				return TCP_CONNECTING;
			}
			else 
			{
			//+CIPSTATUS: 1,0,"TCP","216.52.233.120","80","CLOSED"
			//+CIPSTATUS: 0,,"","","","INITIAL"
				return TCP_CLOSED;
			}				
		}
}

bool close()
{
		SetServerConnectionState(NOT_CONNECTED);
    // if not connected, return
    if (is_connected() == TCP_CLOSED) {
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
	const uint32_t wait_data_ok = 10000;
  char cmd[256] = {0};
		
	//DEBUG_PRINTF("%s", str);
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
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);//  digitalWrite(pin,HIGH);
  if (strstr(resp,"NORMAL POWER DOWN") != NULL)
		return true;
	else
    return false;
}

void SetServerConnectionState(uint8_t new_state)
{
	if (new_state == CONNECTED)
	{
		count_try_to_connect = 0;
	}
	server_connection_state = new_state;
}

uint8_t GetServerConnectionState(void)
{
	return server_connection_state;
}

void Delete_Message(uint32_t message_index, char * message_type)
{
	char cmd[16] = {0};
	
	snprintf(cmd, 16, "AT+CMGD=%d,\"%s\"\r\n", message_index, message_type);
	sim900_check_with_cmd(cmd,"OK\r\n",CMD);
}

void Read_Messages()
{
	const uint16_t tmp_string_size = 64;
	char string[tmp_string_size] = {0};
	char size = 0;
	const char * answer = "+CMGL: ";	
	const char * message_type = "REC UNREAD";
	uint8_t mode = 1;
	const char timeout = 1;
	uint8_t num_messages = 0;
	uint16_t indexes[tmp_string_size] = {0};
	
	
	
	// get list of all unread messages
	size = sprintf(string, "AT+CMGL=\"%s\",%d\r\n", message_type, mode);
	sim900_send_cmd(string, size);
	
	// parse sms list
	// save index of sms, samples of answer
	// +CMGL: xxx,"REC UNREAD","+7915...","16/10/20,22:24;20+12"
	// Text message
	//
	//
	// Ok
	
	const int chartimeout = 100;
	int i = 0;
	unsigned long timerStart, prevChar;
	char c = 0;
	timerStart = HAL_GetTick();
	prevChar = 0;
	size = strlen(answer) - 1;
	bool is_index_read = false;
	bool is_message = false;
	
	// read indexes of unread messages
	while(1) 
	{
		if (sim900_check_readable()) 
		{
			c = UartGSM.Instance->RDR;	//LUart.Instance->TDR = c;							
			prevChar = HAL_GetTick();
			if (!is_index_read && (i < tmp_data_size))
			{
				tmp_data[i++] = c;
			}
			else
			{
				if ((i < tmp_string_size) && (c != ','))
				{
					string[i++] = c;
				}
				else
				{
					if (num_messages < tmp_data_size)
					{
						indexes[num_messages++] = atoi(string);
						is_index_read = false;
					}
					else
					{
						break;
					}
				}
			}
			
			if(i >= size)
			{
				// find the key word "+CMGL: "
				if (!memcmp(&tmp_data[i - size - 1],answer, size))
				{
					i = 0;
					is_index_read = true;
					memset(string, 0, tmp_string_size);
				}
			}
		}
		if (i >= tmp_data_size)
			break;
		
		if ((unsigned long) (HAL_GetTick() - timerStart) > timeout * 1000UL) 
			break;
	}
	
	//TODO 1 Only for debug
	mode = 1;// 0; // make message read 
	for (int i = 0; i < num_messages; i++)
	{
		memset(string, 0, tmp_string_size);
		memset(tmp_data, 0, tmp_data_size);
		
		size = snprintf(string, tmp_string_size, "AT+CMGR=%d,%d\r\n",indexes[i], mode);
		sim900_send_cmd(string, size);
		
		// read message
		sim900_read_buffer((char *)tmp_data, tmp_data_size, 1);
		// message end of "\r\n\r\nOk"
		
		// find end for header
		uint16_t symbol_index = 0;
		while(((symbol_index + 1) < tmp_data_size) &&
					(tmp_data[symbol_index] == 0) &&
					(tmp_data[symbol_index] == '\r') &&
					(tmp_data[symbol_index + 1] == '\n'))
		{
			symbol_index++;
		}
		
		// check key word
		//const char traker_id[] = "$TRAKER_ID";
		
		char* key_word_index = (char*)nullptr;
				
		key_word_index = (char *)strstr((char *)&tmp_data[symbol_index], traker_id);
		
		if (key_word_index != nullptr)
		{
			uint8_t index = 0;
			memset(string, 0, tmp_string_size);
			uint8_t k = 0;
			key_word_index += strlen(traker_id);
			while((index < tmp_string_size) 
				&& (key_word_index[index] != ';') )
			{
				string[k++] = key_word_index[index++];
			}
			uint16_t new_traker_id = atoi(string);
			
			if (new_traker_id != 0)
			{
				DEBUG_PRINTF("new_traker_id = %d\r\n", new_traker_id);
				system_info.tracker_id = new_traker_id;
				sprintf(uin_string, "%s,%d,%d,%d\r\n", uin, system_info.tracker_id, system_info.sw_version, system_info.hw_version);
			}
		}
		
		//const char server_addr[] = "$SERVER_ADDR";
		key_word_index = (char *)strstr((char *)&tmp_data[symbol_index], server_addr);
		
		if (key_word_index != nullptr)
		{
			uint8_t index = 0;
			memset(string, 0, tmp_string_size);
			uint8_t k = 0;
			key_word_index += strlen(server_addr);
			while((index < tmp_string_size) && 
						(key_word_index[index] != ':'))
			{
				string[k++] = key_word_index[index++];
			}
			
			char port_string[8];
			k = 0; index++;
			
			while((k < 8) && 
				((key_word_index[index] != ';') && (key_word_index[index] != '\r')))
			{
				port_string[k++] = key_word_index[index++];
			}
			uint16_t new_server_port = atoi(port_string);
			if (new_server_port != 0)
			{
				snprintf(system_info.host_name, 32, "%s", (const char *)string);
				system_info.host_port = new_server_port;
				DEBUG_PRINTF("new_server_address = %s:%d\r\n", string, new_server_port);
			}
		}
		
		mode = 0;
		memset(string, 0, tmp_string_size);
		size = snprintf(string, tmp_string_size, "AT+CMGD=%d,%d\r\n",indexes[i], mode);
		sim900_send_cmd(string, size);
	}

}


void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartGSM);
}
