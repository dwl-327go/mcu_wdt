//
#include "c8051f310.h"


sfr16 TMR2RL   = 0xca;                 // Timer2 reload value
sfr16 TMR2     = 0xcc;                 // Timer2 counter


typedef unsigned int u16;	  //对数据类型进行声明定义
typedef unsigned char u8;


#define SYSCLK      24500000           // SYSCLK frequency in Hz
#define BAUDRATE     115200           // Baud rate of UART in bps
#define TIMER2_RATE     10000         // Timer 2 overflow rate in Hz
#define N 20

//#define SYSCLK_TIMER 	24500000/8
//#define TIMER_PRESCALER 12
//#define LED_TOGGLE_RATE 50
//
//
//#define TIMER_TICKS_PER_MS   SYSCLK_TIMER/TIMER_PRESCALER/1000
//
//#define AUX1 TIMER_TICKS_PER_MS*LED_TOGGLE_RATE
//#define AUX2 -AUX1
//#define TIMER2_RELOAD AUX2

u8 table1[N]; 
unsigned char temp=0x35;
u8 flag35=1;
u8 flagstart=1;
	u8 flag_bg=1;
	
bit ReceiveBit;

sbit gpio_on=P1^3;
//sbit gpio_try=P1^2;

//void SYSCLK_Init(void);
void PORT_Init (void);
void UART0_Send_Longdata(unsigned char *str, unsigned int Stringlen);
void UART0_Init (void) ;
void delay(unsigned int m) ;
void UART0_Send_data(unsigned char Send_data) ;
void Timer2_Init (int counts);
void Timer3_Init (int counts);
unsigned char ReceiveByte(void);
//void delay(void)
//{
//	int i=0,j=0;
//	for(i=0;i<500;i++)
//		for(j=0;j<1100;j++);
//}
void main()
{
	u8 abc;

	PCA0MD &= ~0x40;				//turn down wdt
	OSCICN |= 0x03;
//	SYSCLK_Init();
	PORT_Init();
 	UART0_Init();
	Timer2_Init(0);
	Timer3_Init(SYSCLK/TIMER2_RATE);
	gpio_on = 1;
//	gpio_try=1;
	
	while(1)
	{
		 if(flag_bg)						   {
			 abc=	ReceiveByte();
			UART0_Send_data(abc);
				UART0_Send_Longdata("hello",5);
				flag_bg=0;
				ET2 = 1;	
				}
//	  //if(!RI0)
//	  temp=0x01;
	}    
//	EA=1;
//   	ES0=1;
//   while(1)
//   {
//     while(ReceiveBit==1)
//     {
//      ES0=0;
//      EA=0;
//      ReceiveBit=0;

//      SBUF0=temp;
//      while(TI0==0);
//      TI0=0;
//      delay(60000);
//      delay(60000);
//      delay(60000);
//      ES0=1;
//      EA=1;
//      }
//    }
}

void PORT_Init (void)
{
	P1MDOUT = 0xff; 

    P0MDOUT |= 0x10;                    // enable  UTX pullup output
   	XBR0    = 0x01;                     // enable UART on P0.4(TX) and P0.5


	XBR1=0x40  ;
}

//
//void SYSCLK_Init(void)
//{
//	int i; // 延时计数器
//	P0MDOUT=0xff;	XBR1=0x40  ;
//	P0=0x00;
//	P0MDIN    = 0xcf;
//	OSCXCN = 0x67; // 起动外部振荡器晶体为 22.1184MHz
//	for (i=0; i < 50000; i++) ; // 等待振荡器启动(>1ms)
//	while (!(OSCXCN & 0x80)) ; // 等待晶体振荡器稳定
//
//	OSCICN = 0x88; // 选择外部振荡器作为系统时钟源并使能丢失时钟检测器
//
//	CLKSEL |= 0x01;     //switch to external osc
//
//
//}

//void SYSCLK_Init (void) 
//{ 
//	OSCXCN = 0x67; 			//开启外部振荡器12MHz晶体 
//	while(!(OSCXCN&0x80)) ; //等待晶体振荡器稳定 
//	CLKSEL = 0x01;			//使用外部振荡器
//}


void UART0_Init (void)
{
	EA=0;
   SCON0 = 0x10;                       // SCON0: 8-bit variable bit rate
                                       //        level of STOP bit is ignored
                                      //        RX enabled
                                       //        ninth bits are zeros
                                       //        clear RI0 and TI0 bits
   if (SYSCLK/BAUDRATE/2/256 < 1) {
      TH1 = -(SYSCLK/BAUDRATE/2);
      CKCON &= ~0x0B;                  // T1M = 1; SCA1:0 = xx
      CKCON |=  0x08;
   } else if (SYSCLK/BAUDRATE/2/256 < 4) {
      TH1 = -(SYSCLK/BAUDRATE/2/4);
      CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 01                  
      CKCON |=  0x09;
   } else if (SYSCLK/BAUDRATE/2/256 < 12) {
      TH1 = -(SYSCLK/BAUDRATE/2/12);
      CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 00
   } else {
      TH1 = -(SYSCLK/BAUDRATE/2/48);
      CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 10
      CKCON |=  0x02;
   }

   TL1 = TH1;                          // init Timer1
   TMOD &= ~0xf0;                      // TMOD: timer 1 in 8-bit autoreload
   TMOD |=  0x20;  
//   ES0=1;
	ES0=0;
//   EA=1;                     
   TR1 = 1;                            // START Timer1


   TI0 = 0;                            // Indicate TX0 ready
//
 RI0 = 0; 
  ES0=0;
   EA=1;

}
//-----------------------------------------------------------------------------
// Timer2_Init SYSCLK no Interrupt
//-----------------------------------------------------------------------------
//
// Configure Timer2 to auto-reload at interval specified by <counts> (no 
// interrupt generated) using SYSCLK as its time base.
#if 1
void Timer2_Init (int counts)
{
   TMR2CN = 0x00;                      // STOP Timer2; Clear TF2H and TF2L;
                                       // disable low-byte interrupt; disable
                                       // split mode; select internal timebase
   CKCON |= 0x10;                      // Timer2 uses SYSCLK as its timebase

//   TMR2RL  = -counts;       //           // Init reload values
   TMR2RL  = 0x4097	;
//   TMR2RLH  = 0x60
   TMR2    = TMR2RLL;       //            // Init Timer2 with reload value
   ET2 = 0;                            // disable Timer2 interrupts
   TR2 = 1;                            // start Timer2
}
void Timer3_Init (int counts)
{
   TMR3CN = 0x00;                      // STOP Timer2; Clear TF2H and TF2L;
                                       // disable low-byte interrupt; disable
                                       // split mode; select internal timebase
   CKCON |= 0x10;                      // Timer2 uses SYSCLK as its timebase

   TMR3RLL  = -counts;                  // Init reload values
   TMR3L    = TMR3RLL;                   // Init Timer2 with reload value
  // ET2 = 1;                            // disable Timer2 interrupts
//   TR2 = 1;                            // start Timer2
   EIE1   &= ~0x80;//
//	EIE1   |= 0x80;//ENABLE TIMER3
	TMR3CN |= 0x04;//RUN TIMER3
}
#endif
#if 0
void Timer2_Init (int counts)
{
  CKCON &= ~0x60;                     // Timer2 uses SYSCLK/12
   TMR2CN &= ~0x01;

   TMR2RL = TIMER2_RELOAD;             // Reload value to be used in Timer2
   TMR2 = TMR2RL;                      // Init the Timer2 register

   TMR2CN = 0x04;                      // Enable Timer2 in auto-reload mode
   ET2 = 1;                            // Timer2 interrupt enabled                          // start Timer2
}
//void Timer2_Init (int counts)
//{
//   TMR2CN = 0x00;                      // STOP Timer2; Clear TF2H and TF2L;
//                                       // disable low-byte interrupt; disable
//                                       // split mode; select internal timebase
//   CKCON |= 0x10;                      // Timer2 uses SYSCLK as its timebase
//
//   TMR2RLL  = 0x7d;                  // Init reload values
//   TMR2RLH  = 0x60;
//   TMR2L    = TMR2RLL;                   // Init Timer2 with reload value
//   TMR2H    = TMR2RLH;
//   TMR2CN = 0x04;
//   ET2 = 1;                            // disable Timer2 interrupts
//   TR2 = 1;                            // start Timer2
//}
void Timer3_Init (int counts)
{
   TMR3CN = 0x00;                      // STOP Timer2; Clear TF2H and TF2L;
                                       // disable low-byte interrupt; disable
                                       // split mode; select internal timebase
   CKCON |= 0x10;                      // Timer2 uses SYSCLK as its timebase

   TMR3RLL  = 0x3e;                  // Init reload values
   TMR3RLH  = 0xb0;
   TMR3L    = TMR3RLL;                   // Init Timer2 with reload value
   TMR3H    = TMR3RLH;

    EIE1   &= ~0x80;//
//	EIE1   |= 0x80;//ENABLE TIMER3
	TMR3CN |= 0x04;//RUN TIMER3
}
#endif
//------------------------------------------------------------------------------
void delay(unsigned int m)
{
   unsigned int n;
   n=0;
   while(n<m)
   {n++;}
   return;
}
//===============================================================================
//void UATR0_ISR(void)  interrupt 4
//{
//if(!TI0)
//{
//RI0=0;
//temp=SBUF0;
//ReceiveBit=1;
//}
//TI0=0;
//}

#if 1
//void Uart0_Receive_interrupt()interrupt 4
//{
////u8 flag=1;
//if(RI0)
//{
//RI0=0;
//
//temp=SBUF0;
//UART0_Send_data(temp);
//
//}
//}



void Timer2_ISR (void) interrupt 5
{
   static int m,n=0;
unsigned char rbyte;
//   u8 temp1;
//   temp1=ReceiveByte(); UART0_Send_data(temp1);
   if(!RI0){//(0x35!=temp)){
   m++;
   if(m>=20000){
   m=0;
   n++;
	if(n>=300)
	{  
	n=0;
//	  if((!RI0)||(0x35!=ReceiveByte())){
	  flag35=1;temp=0x35;
	  flagstart=0;
	  gpio_on = 0;
	  EIE1   |= 0x80;
	  ET2=0;
// 				}
	}
}
}else
{
	RI0=0;
	rbyte=SBUF0;
	if(rbyte==0x35)
	{
	m=0;n=0	 ;
	}
	else
	{
	    m++;
   if(m>=20000){
   m=0;
   n++;
	if(n>=300)
	{  
	  n=0;
//	  if((!RI0)||(0x35!=ReceiveByte())){
	  flag35=1;temp=0x35;
	  flagstart=0;
	  gpio_on = 0;
	  EIE1   |= 0x80;
	  ET2=0;
// 				}
	}
	
	}

}
//m=0;n=0;
//if(temp!=0x35)
//{
//   m++;
//   if(m>=20000){
//   m=0;
//   n++;
//	if(n>=900)
//	{  
//	n=0;
////	  if((!RI0)||(0x35!=ReceiveByte())){
//	  flag35=1;temp=0x35;
//	  flagstart=0;
//	  gpio_on = 0;
//	  EIE1   |= 0x80;
//	  ET2=0;
//
//}
//  }





}
}



void Timer3_ISR (void) interrupt 14
{
   static int l,p=0;
   l++;
   if(l>=20000){
   l=0;
   p++;
	if(p>=130)
	{  
	p=0;
	  if(!flagstart){
	  flagstart=1;temp=0x35;
	  gpio_on = 1;
	  EIE1   &= ~0x80;//
	  //ET2=1;
	  flag_bg=1;
 				  }
	}
}
}

#endif

#if 0
void Timer2_ISR (void) interrupt 5
{
	  static int pp=0;
	  pp++;
	  if(pp>=10)
	  {
	  	pp=0;
		gpio_on = 0;			//~gpio_try;
		//ET2=0;//
		TF2H=0;
	  }
}
#endif

void UART0_Send_data(unsigned char Send_data)
{
SBUF0=Send_data; 
while(!TI0);
TI0=0; 
}


unsigned char ReceiveByte(void)
{
	unsigned char rbyte;
	while(!RI0);
	RI0=0;
	rbyte=SBUF0;
	return rbyte;

}


void UART0_Send_Longdata(unsigned char *str, unsigned int Stringlen)
{
unsigned int i; 
do { 
UART0_Send_data(*(str+i)); 
i++; 
} 
while (i<Stringlen); 
}