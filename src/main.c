/**
 *****************************************************************************
 **
 **  File        : main.c
 **
 **  Abstract    : main function.
 **
 **  Functions   : main
 **
 **  Environment : Eclipse with Atollic TrueSTUDIO(R) Engine
 **                STMicroelectronics STM32F4xx Standard Peripherals Library
 **
 **
 **
 *****************************************************************************
 */

/* Includes */
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "delay.h"

/* Private macro */
/* Private variables */
int addresssend[3];
int busycheckflag[3]={1,1,1};
int transmitter[3]={0,0,0};
int16_t cmps=0;

/* Private function prototypes */
//void		I2C_Configuration();
void		i2c_start();
void		i2c_stop();
int8_t		i2c_writebyte(uint8_t dat);
uint8_t		i2c_readbyte(char noack);

uint8_t 	newinit_cmps();
int16_t 	newcmps_get();

void 		uart_putchar(const char c);
void 		uart_putstr(const char* str);
void 		uart_putdec(uint16_t i);

/* Private functions */
void i2c_start()
{
	if(busycheckflag[0]){
		while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	    busycheckflag[0] = 0;
	}
	I2C_GenerateSTART(I2C1, ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	addresssend[0] = 1;
}

void i2c_stop()
{
	 if(transmitter[0])I2C_GenerateSTOP(I2C2, ENABLE);
	 else I2C_AcknowledgeConfig(I2C2, ENABLE);
	 addresssend[1] = 0;
	 busycheckflag[1] = 1;
}

int8_t i2c_writebyte(uint8_t dat)
{
	//アドレスを送信
	if(addresssend[1]){
		if(dat%2 == 0){
			I2C_Send7bitAddress(I2C1, dat, I2C_Direction_Transmitter);
	        transmitter[1]=1;
	        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	    }else{
	        I2C_Send7bitAddress(I2C1, dat&0xfe, I2C_Direction_Receiver);
	        transmitter[1]=0;
	        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	    }
	    addresssend[1]=0;
	}
	//データを送信
	else{
		I2C_SendData(I2C1, (uint8_t)(dat));
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}
	return 0;
}

uint8_t i2c_readbyte(char noack)
{
	uint8_t data;
	if(noack){
		I2C_AcknowledgeConfig(I2C1, DISABLE);
	    I2C_GenerateSTOP(I2C1, ENABLE);
	}
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)){}
	data = I2C_ReceiveData(I2C1);

	return data;
}

uint8_t newinit_cmps(){ //1成功

       // #define i2c_start i2c_start_i2c1
       // #define i2c_writebyte i2c_writebyte_i2c1
       // #define i2c_readbyte i2c_readbyte_i2c1
       // #define i2c_stop i2c_stop_i2c1

        delay_ms(300);
        uint8_t flag = 0;
        //CMPS Setup
        i2c_start();
        if(i2c_writebyte(0x42 + 0)==0){ //反応あり
                i2c_writebyte('G');
                i2c_writebyte(0x74);
                i2c_writebyte(0x70); //0x72
                flag = 1;
        }
        i2c_stop();
        return flag;
}



int16_t newcmps_get(){
       // #define i2c_start i2c_start_i2c1
       // #define i2c_writebyte i2c_writebyte_i2c1
       // #define i2c_readbyte i2c_readbyte_i2c1
       // #define i2c_stop i2c_stop_i2c1

        int16_t cmps=0;
        i2c_start();
        if(i2c_writebyte(0x42 + 0)==0){ //反応あり
                i2c_writebyte('A');
        }else{
                cmps=-1;
        }
        i2c_stop();
        if(cmps>=0){
                delay_ms(1);
                //delay_ms(6);
                i2c_start();
                if(i2c_writebyte(0x42 + 1)==0){ //反応あり
                        cmps = (i2c_readbyte(0) << 8);
                        cmps += i2c_readbyte(1);
                }else{
                        cmps=-2;
                }
                i2c_stop();
        }


        return cmps;
}

void uart_putchar(const char c)
{
	/*Wait while there is the data to send*/
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET){}

	/* Output the character data */
	USART_SendData(USART2,c);

	/* Wait while it have not finished sending */
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC) == RESET){}
}

void uart_putstr(const char* str)
{
	char c;	//for output string

	/* Output the string data by one character */
	while(1)
	{
		c = *str++;
		if(c == 0)break;
		uart_putchar(c);
	}
}

void uart_putdec(uint16_t i)
{
	uart_putchar(i/1000 + '0');
	i %= 1000;
	uart_putchar(i/100 + '0');
	i %= 100;
	uart_putchar(i/10 + '0');
	i %= 10;
	uart_putchar(i + '0');
}

/**
 **===========================================================================
 **
 **  Abstract: main program
 **
 **===========================================================================
 */
int main(void)
{
	delay_init();

	/* initialize structure variable */
	GPIO_InitTypeDef	GPIO_InitStructure;
	I2C_InitTypeDef		I2C_InitStructure;
	USART_InitTypeDef	USART_InitStructure;

	/* supply clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,  ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

	/* initialize GPIOB (for I2C1) */
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_6 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* initialize I2C1 */
	I2C_InitStructure.I2C_Mode					= I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle				= I2C_DutyCycle_16_9;
	I2C_InitStructure.I2C_Ack					= I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress	= I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed			= 50000;
	I2C_Init(I2C1, &I2C_InitStructure);
	I2C_Cmd(I2C1, ENABLE);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);

	//GPIO initialize for USART2
	/* Configure USART2 Tx as alternate function push_pull */
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);


	/* Configure USART2 Rx as inport floating */
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//USART initialize
	USART_InitStructure.USART_BaudRate				= 9600;
	USART_InitStructure.USART_WordLength			= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits				= USART_StopBits_1;
	USART_InitStructure.USART_Parity				= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode					= USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2,ENABLE);

	uart_putstr("preparing to initialize...\n\r");

	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	I2C_GenerateSTART(I2C1, ENABLE);
	uart_putstr("I2C started!\n\r");
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	uart_putstr("checked event(master mode select)\n\r");

	delay_ms(10);

	I2C_Send7bitAddress(I2C1, 0x42, I2C_Direction_Transmitter);
	uart_putstr("sent 7bit address\n\r");
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	uart_putstr("checked event(master transmitter mode selected)\n\r");
	I2C_SendData(I2C1, 'G');
	uart_putstr("selected address G\n\r");
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	uart_putstr("checked event(master byte transmitted)\n\r");
	I2C_SendData(I2C1, 0x74);
	uart_putstr("sent data");
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	uart_putstr("checked event(master byte transmitted)\n\r");
	I2C_SendData(I2C1, 0x70);
	uart_putstr("sent data");
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	uart_putstr("checked event(master byte transmitted)\n\r");
	I2C_GenerateSTOP(I2C1, ENABLE);
	uart_putstr("I2C stooped!\n\r");

	__IO uint16_t SR1_Tmp= 0;
	__IO uint32_t polling_count = 0;

	uart_putstr("start polling\n\r");
	do{
		I2C_GenerateSTART(I2C1, ENABLE);
		SR1_Tmp = I2C_ReadRegister(I2C1, I2C_Register_SR1);
		I2C_Send7bitAddress(I2C1, 0x42, I2C_Direction_Transmitter);
		polling_count++;
	}while(!I2C_ReadRegister(I2C1, I2C_Register_SR1 & 0x0002));
	I2C_ClearFlag(I2C1, I2C_FLAG_AF);
	I2C_GenerateSTOP(I2C1, ENABLE);
	uart_putstr("ended polling\n\r");
	while(1)
	{
		I2C_AcknowledgeConfig(I2C1, ENABLE);
		while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
		I2C_GenerateSTART(I2C1, ENABLE);
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
		I2C_Send7bitAddress(I2C1, 0x42, I2C_Direction_Transmitter);
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
		I2C_SendData(I2C1, (uint8_t)('A'));
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
		I2C_GenerateSTART(I2C1, ENABLE);
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
		I2C_Send7bitAddress(I2C1, 0x42, I2C_Direction_Receiver);
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
		cmps = I2C_ReceiveData(I2C1)<<8;
		I2C_AcknowledgeConfig(I2C1, DISABLE);
		I2C_GenerateSTOP(I2C1, ENABLE);
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
		cmps += I2C_ReceiveData(I2C1);
		uart_putdec(cmps);
		uart_putstr("\n\r");
	}
	return 0;
}


void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size){
	/* TODO, implement your code here */
	return;
}

/*
 * Callback used by stm324xg_eval_audio_codec.c.
 * Refer to stm324xg_eval_audio_codec.h for more info.
 */
uint16_t EVAL_AUDIO_GetSampleCallBack(void){
	/* TODO, implement your code here */
	return -1;
}

