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

uint32_t str_to_ip(const char* str);
// SoftwareSerial gprsSerial;
//static GPRS* inst;
uint32_t _ip;
char ip_string[16]; //XXX.YYY.ZZZ.WWW + \0
		
extern UART_HandleTypeDef * uart;		

const char resp_ok[] = "OK\r\n";



void delay(int time)
{
	HAL_Delay(time);
}

void itoa(int n, char * s)
 {
		sprintf(s, "%d", n);
 }



void gsm(UART_HandleTypeDef * uart_handle)//:gprsSerial(tx,rx)
{

	// UART Init
	__USART2_CLK_ENABLE();
	uart_handle->Instance = USART2;
	uart_handle->Init.BaudRate   = 9600;
	uart_handle->Init.WordLength = UART_WORDLENGTH_8B;
	uart_handle->Init.StopBits   = UART_STOPBITS_1;
	uart_handle->Init.Parity     = UART_PARITY_NONE;
	uart_handle->Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	uart_handle->Init.Mode       = UART_MODE_TX_RX;
	uart_handle->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	HAL_UART_DeInit(uart_handle);  
  HAL_UART_Init(uart_handle);
 /* Enable the UART Parity Error Interrupt */
	__HAL_UART_ENABLE_IT(uart_handle, UART_IT_ORE);
	
	/* Process Unlocked */
	__HAL_UNLOCK(uart_handle); 
	HAL_NVIC_EnableIRQ(USART2_IRQn);

	//
	sim900_init(uart_handle);
	
	DEBUG_PRINTF("GSM power up...");
	powerUpDown();
	DEBUG_PRINTF("Ok\r\n");

	gsm_init();
}

bool gsm_init()
{
	char c = 0;
	char cc = 0;
	
	DEBUG_PRINTF("UART GSM baudrate adjusting...");
	while(1)
	{		
		while(!(uart->Instance->ISR & UART_FLAG_TXE));
		uart->Instance->TDR = 'A';		
		HAL_Delay(10);		
		while (uart->Instance->ISR & UART_FLAG_RXNE)
		{
			c = uart->Instance->RDR;
		}		
		HAL_Delay(500);
		while(!(uart->Instance->ISR & UART_FLAG_TXE));
		uart->Instance->TDR = 'T';			
		
		HAL_Delay(10);
		
		while (uart->Instance->ISR & UART_FLAG_RXNE)
		{
			cc = uart->Instance->RDR;
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
  
	DEBUG_PRINTF("Check SIM status...");  
	while(!checkSIMStatus());
	DEBUG_PRINTF("Ok\r\n");

	return true;
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
  
bool checkSIMStatus(void)
{
    char gprsBuffer[32];
    int count = 0;
    sim900_clean_buffer(gprsBuffer,32);
    while(count < 3) {
        sim900_send_cmd("AT+CGREG?\r\n", sizeof("AT+CGREG?\r\n"));
        sim900_read_buffer(gprsBuffer,32,DEFAULT_TIMEOUT);
        if((NULL != strstr(gprsBuffer,"+CGREG: 0,1"))
				|| (NULL != strstr(gprsBuffer,"+CGREG: 0,5")))
					
					{
            break;
        }
        count++;
        delay(300);
    }
    if(count == 3) {
        return false;
    }
    return true;
}

bool sendSMS(char *number, char *data)
{
    //char cmd[32];
    if(!sim900_check_with_cmd("AT+CMGF=1\r\n", "OK\r\n", CMD)) { // Set message mode to ASCII
        return false;
    }
    delay(500);
	sim900_flush_serial();
	sim900_send_cmd("AT+CMGS=\"", sizeof("AT+CMGS=\""));
	sim900_send_cmd(number, sizeof(number));
  //sprintf(cmd,"AT+CMGS=\"%s\"\r\n", number);
	//snprintf(cmd, sizeof(cmd),"AT+CMGS=\"%s\"\r\n", number);
	//if(!sim900_check_with_cmd(cmd,">",CMD)) {
    if(!sim900_check_with_cmd("\"\r\n",">",CMD)) {
        return false;
    }
    delay(1000);
    sim900_send_cmd(data, sizeof(data));
    delay(500);
    sim900_send_End_Mark();
    return sim900_wait_for_resp("OK\r\n", CMD);
}

char isSMSunread()
{
    char gprsBuffer[48];  //48 is enough to see +CMGL:
    char *s;
    

    //List of all UNREAD SMS and DON'T change the SMS UNREAD STATUS
    sim900_send_cmd("AT+CMGL=\"REC UNREAD\",1\r\n", 0);
    /*If you want to change SMS status to READ you will need to send:
          AT+CMGL=\"REC UNREAD\"\r\n
      This command will list all UNREAD SMS and change all of them to READ
      
     If there is not SMS, response is (30 chars)
         AT+CMGL="REC UNREAD",1  --> 22 + 2
                                 --> 2
         OK                      --> 2 + 2

     If there is SMS, response is like (>64 chars)
         AT+CMGL="REC UNREAD",1
         +CMGL: 9,"REC UNREAD","XXXXXXXXX","","14/10/16,21:40:08+08"
         Here SMS text.
         OK  
         
         or

         AT+CMGL="REC UNREAD",1
         +CMGL: 9,"REC UNREAD","XXXXXXXXX","","14/10/16,21:40:08+08"
         Here SMS text.
         +CMGL: 10,"REC UNREAD","YYYYYYYYY","","14/10/16,21:40:08+08"
         Here second SMS        
         OK           
    */

    sim900_clean_buffer(gprsBuffer,31); 
    sim900_read_buffer(gprsBuffer,30,DEFAULT_TIMEOUT); 
    //Serial.print("Buffer isSMSunread: ");Serial.println(gprsBuffer);

    if(NULL != ( s = strstr(gprsBuffer,"OK"))) {
        //In 30 bytes "doesn't" fit whole +CMGL: response, if recieve only "OK"
        //    means you don't have any UNREAD SMS
        delay(50);
        return 0;
    } else {
        //More buffer to read
        //We are going to flush serial data until OK is recieved
        sim900_wait_for_resp("OK\r\n", CMD);        
        //sim900_flush_serial();
        //We have to call command again
        sim900_send_cmd("AT+CMGL=\"REC UNREAD\",1\r\n", sizeof("AT+CMGL=\"REC UNREAD\",1\r\n"));
        sim900_clean_buffer(gprsBuffer,48); 
        sim900_read_buffer(gprsBuffer,47,DEFAULT_TIMEOUT);
		//Serial.print("Buffer isSMSunread 2: ");Serial.println(gprsBuffer);       
        if(NULL != ( s = strstr(gprsBuffer,"+CMGL:"))) {
            //There is at least one UNREAD SMS, get index/position
            s = strstr(gprsBuffer,":");
            if (s != NULL) {
                //We are going to flush serial data until OK is recieved
                sim900_wait_for_resp("OK\r\n", CMD);
                return atoi(s+1);
            }
        } else {
            return -1; 

        }
    } 
    return -1;
}

bool readSMS(int messageIndex, char *message, int length, char *phone, char *datetime)  
{
  /* Response is like:
  AT+CMGR=2
  
  +CMGR: "REC READ","XXXXXXXXXXX","","14/10/09,17:30:17+08"
  SMS text here
  
  So we need (more or lees), 80 chars plus expected message length in buffer. CAUTION FREE MEMORY
  */

    int i = 0;
    char gprsBuffer[80 + length];
    //char cmd[16];
	char num[4];
    char *p,*p2,*s;
    
    sim900_check_with_cmd("AT+CMGF=1\r\n","OK\r\n",CMD);
    delay(1000);
	//sprintf(cmd,"AT+CMGR=%d\r\n",messageIndex);
    //sim900_send_cmd(cmd);
	sim900_send_cmd("AT+CMGR=", sizeof("AT+CMGR="));
	itoa(messageIndex, num);
	sim900_send_cmd(num, 0);
	sim900_send_cmd("\r\n", sizeof("\r\n"));
    sim900_clean_buffer(gprsBuffer,sizeof(gprsBuffer));
    sim900_read_buffer(gprsBuffer,sizeof(gprsBuffer), 0);
      
    if(NULL != ( s = strstr(gprsBuffer,"+CMGR:"))){
        // Extract phone number string
        p = strstr(s,",");
        p2 = p + 2; //We are in the first phone number character
        p = strstr((char *)(p2), "\"");
        if (NULL != p) {
            i = 0;
            while (p2 < p) {
                phone[i++] = *(p2++);
            }
            phone[i] = '\0';            
        }
        // Extract date time string
        p = strstr((char *)(p2),",");
        p2 = p + 1; 
        p = strstr((char *)(p2), ","); 
        p2 = p + 2; //We are in the first date time character
        p = strstr((char *)(p2), "\"");
        if (NULL != p) {
            i = 0;
            while (p2 < p) {
                datetime[i++] = *(p2++);
            }
            datetime[i] = '\0';
        }        
        if(NULL != ( s = strstr(s,"\r\n"))){
            i = 0;
            p = s + 2;
            while((*p != '\r')&&(i < length-1)) {
                message[i++] = *(p++);
            }
            message[i] = '\0';
        }
        return true;
    }
    return false;    
}


bool deleteSMS(int index)
{
    //char cmd[16];
	char num[4];
    //sprintf(cmd,"AT+CMGD=%d\r\n",index);
    sim900_send_cmd("AT+CMGD=", sizeof("AT+CMGD="));
	itoa(index, num);
	sim900_send_cmd(num, 0);
	//snprintf(cmd,sizeof(cmd),"AT+CMGD=%d\r\n",index);
    //sim900_send_cmd(cmd);
    //return 0;
    // We have to wait OK response
	//return sim900_check_with_cmd(cmd,"OK\r\n",CMD);
	return sim900_check_with_cmd("\r","OK\r\n",CMD);	
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
	sim900_send_cmd(number, 0);
	sim900_send_cmd(";\r\n", sizeof(";\r\n"));
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
    sim900_send_cmd("AT+CIPSHUT\r\n", sizeof("AT+CIPSHUT\r\n"));
}

bool connect(Protocol ptl,const char * host, int port, int timeout, int chartimeout)
{
    char cmd[128];
    char resp[128];

    //sim900_clean_buffer(cmd,64);
    if(ptl == TCP) {
      sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",host, port);
    } 
		else 
			if(ptl == UDP) {
        sprintf(cmd, "AT+CIPSTART=\"UDP\",\"%s\",%d\r\n",host, port);
    } else {
        return false;
    }
    

    sim900_send_cmd(cmd, 128);
    sim900_read_buffer(resp, 128, timeout);
    if((strstr(resp,"CONNECT OK") != NULL) ||
			 (strstr(resp, "ALREADY CONNECT") != NULL))	
        return true;
		
    return false;
}


bool is_connected(void)
{
    char resp[96];
    sim900_send_cmd("AT+CIPSTATUS\r\n", sizeof("AT+CIPSTATUS\r\n"));
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
	const uint32_t wait_data_ok = 5000;
  char cmd[64] = 0;
	char num[16];
    if(len > 0){
    //snprintf(cmd,sizeof(cmd),"AT+CIPSEND=%d\r\n",len);
		sprintf(cmd,"AT+CIPSEND=%d\r\n",len);
		//sim900_send_cmd("AT+CIPSEND=", sizeof("AT+CIPSEND="));
		//itoa(len, num);
		//sim900_send_cmd(cmd, 0);
		if(sim900_check_with_cmd(cmd,">",CMD)) {
        //sim900_send_cmd(">", 0);
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
    sim900_clean_buffer(buf,len);
    sim900_read_buffer(buf,len,DEFAULT_TIMEOUT);   //Ya he llamado a la funcion con la longitud del buffer - 1 y luego le estoy añadiendo el 0
    return strlen(buf);
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


