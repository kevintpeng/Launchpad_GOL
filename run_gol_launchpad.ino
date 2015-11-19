//#include <limit.h> 
//#include <stdint.h> 

// ----------------------------------
//         DEMO IMPORTS
// ----------------------------------
extern "C" {
#include <stdio.h>
#include <math.h>
#include <delay.h>
#include <FillPat.h>
#include <I2CEEPROM.h>
#include <LaunchPad.h>
#include <OrbitBoosterPackDefs.h>
#include <OrbitOled.h>
#include <OrbitOledChar.h>
#include <OrbitOledGrph.h>
}

/* ------------------------------------------------------------ */
/*				Global Variables		*/
/* ------------------------------------------------------------ */
extern int xchOledMax; // defined in OrbitOled.c
extern int ychOledMax; // defined in OrbitOled.c

#define BUFFER 5
#define L_BITMAP 8
#define LENGTH 128 // maybe set to xchOledMax?
#define HEIGHT 32
char bitmap[LENGTH*HEIGHT/8];
char aliveNow[LENGTH][HEIGHT];
char aliveNext[LENGTH][HEIGHT];
int count = 0;
int current_count = 0;
int total_iter = 0;

void setup(){
  
  DeviceInit();
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
}

void loop(){
  srand((unsigned int)(msBettweenFrame()*accelerometer_seed()));
        
  int q, i,j, greenness=0, delay_time=0, current_iter=0;
  int framerate_buffer[BUFFER] = {0};
  //int accel_buffer[4] = {0};
	populate();
	for(q=0;q<2;){
		fillNextArray();
		updateCurrentArray(); 
		convert(); // converts the bool array to bitmap

		if(checkStability()) {
                  if(greenness<250)greenness+=5;
                   analogWrite(GREEN_LED, greenness);
                   analogWrite(RED_LED, 255-greenness);
		}
                else {
                  if(greenness>0) greenness-=5;
                  analogWrite(GREEN_LED, greenness);
                  analogWrite(RED_LED, 255-greenness);
                }
		OrbitOledMoveTo(0,0);
		OrbitOledPutBmp(LENGTH,HEIGHT,bitmap);
                OrbitOledUpdate();
                int sum=0;                
                delay_time = msBettweenFrame()/40;

                for(i=0;i<BUFFER;i++){
                  if(i==current_iter) {
                    framerate_buffer[i]=delay_time;
                  }
                  sum+=framerate_buffer[i];
                }
                if(++current_iter>BUFFER)current_iter=0;
                
                if(accelerometer()) repopulate();
                
                delay(sum/BUFFER);
	}
}

/* ------------------------------------------------------------ */
/***	DeviceInit
 **
 **	Parameters:
 **		none
 **
 **	Return Value:
 **		none
 **
 **	Errors:
 **		none
 **
 **	Description:
 **		Initialize I2C Communication, and GPIO
 */
void DeviceInit()
{
  /*
   * First, Set Up the Clock.
   * Main OSC		  -> SYSCTL_OSC_MAIN
   * Runs off 16MHz clock -> SYSCTL_XTAL_16MHZ
   * Use PLL		  -> SYSCTL_USE_PLL
   * Divide by 4	  -> SYSCTL_SYSDIV_4
   */
  SysCtlClockSet(SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_USE_PLL | SYSCTL_SYSDIV_4);

  /*
   * Enable and Power On All GPIO Ports
   */
  //SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOA | SYSCTL_PERIPH_GPIOB | SYSCTL_PERIPH_GPIOC |
  //						SYSCTL_PERIPH_GPIOD | SYSCTL_PERIPH_GPIOE | SYSCTL_PERIPH_GPIOF);

  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOA );
  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOB );
  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOC );
  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOD );
  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOE );
  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOF );
  /*
   * Pad Configure.. Setting as per the Button Pullups on
   * the Launch pad (active low).. changing to pulldowns for Orbit
   */
  GPIOPadConfigSet(SWTPort, SWT1 | SWT2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

  GPIOPadConfigSet(BTN1Port, BTN1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
  GPIOPadConfigSet(BTN2Port, BTN2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

  GPIOPadConfigSet(LED1Port, LED1, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPadConfigSet(LED2Port, LED2, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPadConfigSet(LED3Port, LED3, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPadConfigSet(LED4Port, LED4, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);

  /*
   * Initialize Switches as Input
   */
  GPIOPinTypeGPIOInput(SWTPort, SWT1 | SWT2);

  /*
   * Initialize Buttons as Input
   */
  GPIOPinTypeGPIOInput(BTN1Port, BTN1);
  GPIOPinTypeGPIOInput(BTN2Port, BTN2);

  /*
   * Initialize LEDs as Output
   */
  GPIOPinTypeGPIOOutput(LED1Port, LED1);
  GPIOPinTypeGPIOOutput(LED2Port, LED2);
  GPIOPinTypeGPIOOutput(LED3Port, LED3);
  GPIOPinTypeGPIOOutput(LED4Port, LED4);

  /*
   * Enable ADC Periph
   */
  SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

  GPIOPinTypeADC(AINPort, AIN);

  /*
   * Enable ADC with this Sequence
   * 1. ADCSequenceConfigure()
   * 2. ADCSequenceStepConfigure()
   * 3. ADCSequenceEnable()
   * 4. ADCProcessorTrigger();
   * 5. Wait for sample sequence ADCIntStatus();
   * 6. Read From ADC
   */
  ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
  ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0);
  ADCSequenceEnable(ADC0_BASE, 0);
     ADCProcessorTrigger(ADC0_BASE, 0);

  /*
   * Initialize the OLED
   */
  OrbitOledInit();

  /*
   * Reset flags
   */

}

int power(int base, int exponent) {
    //assert (exponent>=0);
    int i, result = 1;
    for (i = 0; i < exponent; i++)
        result *= base;
    return result;
 }

int countLivingNeighbours(int i, int j){
	int surroundings = 0, k, l;
	for(k=i-1;k<=i+1;k++){
		for(l=j-1;l<=j+1;l++){
			if (0<=k && k< LENGTH && 0<=l && l< HEIGHT && !(k==i && l==j) && aliveNow[k][l])
                    surroundings++;
		}
	}
	return surroundings;
}

void fillNextArray(){
	int i,j;
	for(i=0;i<LENGTH;i++){
		for(j=0;j<HEIGHT;j++){
			aliveNext[i][j] = countLivingNeighbours(i,j);
		}
	}
}

void updateCurrentArray(){
	int i,j;
	for(i=0;i<LENGTH;i++){
		for(j=0;j<HEIGHT;j++){
			if(aliveNow[i][j]){
				if(aliveNext[i][j]<2) aliveNow[i][j]=0;
				else if(aliveNext[i][j]>3) aliveNow[i][j]=0;
				else current_count++;
			}
			else{
				if(aliveNext[i][j]==3) {
					aliveNow[i][j]=1;
					current_count++;
				}
			}
		}
	}
}

void populate(){
        
        int i,j;
	for(i=0;i<LENGTH;i++){
		for(j=0;j<HEIGHT;j++){
			aliveNow[i][j] = rand()%2;
		}
	}
}

void repopulate(){
        populate();
}

int checkStability(){
	int ret = 0;
	if(current_count==count) ret = 1;
	count = current_count;
	current_count = 0;
	return ret;
}



void convert(){
  int l=0, w=0, cell=0, ret=0, index =0;
  for(w=0;w<HEIGHT;w+=8){
  for(l=0;l<LENGTH;l++){
    
      ret = 0;
      for(cell=0;cell<8;cell++){
        ret |= aliveNow[l][w+cell] << cell;
      }
      bitmap[index++]=(char)ret;
    }
  }
}


int msBettweenFrame(){ // Returns the time between frames in ms according to the value given by the potentiometer 


	uint32_t	ulAIN0;
	char		szAIN[6] = {0};
	char		cMSB = 0x00;
	char		cMIDB = 0x00;
	char		cLSB = 0x00;
	int 		potValue = 0;
	double 		maxFPS = 10.0;
	int 		fastestMs = (int) (1000.0 / (maxFPS)); // msPerFrame = 1000 / FPS
	int 		mult =1; 

  /*
   * Initiate ADC Conversion
   */
        ADCProcessorTrigger(ADC0_BASE, 0);

   ADCSequenceDataGet(ADC0_BASE, 0, &ulAIN0);

  /*
   * Process data
   */
   cMSB = (0xF00 & ulAIN0) >> 8;
   cMIDB = (0x0F0 & ulAIN0) >> 4;
   cLSB = (0x00F & ulAIN0);

   szAIN[0] = '0';
   szAIN[1] = 'x';
   szAIN[2] = (cMSB > 9) ? 'A' + (cMSB - 10) : '0' + cMSB;
   szAIN[3] = (cMIDB > 9) ? 'A' + (cMIDB - 10) : '0' + cMIDB;
   szAIN[4] = (cLSB > 9) ? 'A' + (cLSB - 10) : '0' + cLSB;
   szAIN[5] = '\0';
	// szAIN[2-4] contains the hex value from the potentiometer 

   // Take the chars & use a switch to convert it to the right numerical value
 
   for (int i = 4; i >=2; i--){

   	// Gets me what I need to multiply by to convert
   	if (i == 4){
   		mult = 256;
   	} else if (i == 3){
   		mult = 16;
   	} else {
   		mult = 1; 
   	}

   	switch (szAIN[i]){

   	case '0':
   	potValue += 0;
   	break;
   	case '1':
   	potValue += 1 * mult;
   	break;
   	case '2':
   	potValue += 2 * mult;
   	break;
   	case '3':
   	potValue += 3 * mult;
   	break;
   	case '4':
   	potValue += 4 * mult;
   	break;
   	case '5':
   	potValue += 5 * mult;
   	break;
   	case '6' :
   	potValue += 6 * mult;
   	break;
   	case '7':
   	potValue += 7 * mult;
   	break;
   	case '8':
   	potValue += 8 * mult;
   	break;
   	case '9':
   	potValue += 9 * mult;
   	break;
   	case 'A':
   	potValue += 10 * mult;
   	break;
   	case 'B':
   	potValue += 11 * mult;
   	break;
   	case  'C':
   	potValue += 12 * mult;
   	break;
   	case 'D':
   	potValue += 13 * mult;
   	break;
   	case 'E':
   	potValue += 14 * mult;
   	break;
   	case 'F':
   	potValue += 15 * mult;
   	break;
   }

   } // for

  return potValue;
}

int accelerometer(){  //int accel_buffer[]){
  
  short	dataX;
  short dataY;
  short dataZ;
  int refresh;
  
  char  chPwrCtlReg = 0x2D;
  char 	chX0Addr = 0x32;
  char  chY0Addr = 0x34;
  char  chZ0Addr = 0x36;
  
  char 	rgchReadAccl[] = {
    0, 0, 0            };
  char 	rgchWriteAccl[] = {
    0, 0            };
  char  rgchReadAccl2[] = {
    0, 0, 0            };  
  char  rgchReadAccl3[] = {
    0, 0, 0            };
    
    /*
     * Enable I2C Peripheral
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    /*
     * Set I2C GPIO pins
     */
    GPIOPinTypeI2C(I2CSDAPort, I2CSDA_PIN);
    GPIOPinTypeI2CSCL(I2CSCLPort, I2CSCL_PIN);
    GPIOPinConfigure(I2CSCL);
    GPIOPinConfigure(I2CSDA);

    /*
     * Setup I2C
     */
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);
    
   /* Initialize the Accelerometer
     *
     */
    GPIOPinTypeGPIOInput(ACCL_INT2Port, ACCL_INT2);

    rgchWriteAccl[0] = chPwrCtlReg;
    rgchWriteAccl[1] = 1 << 3;		// sets Accl in measurement mode
    I2CGenTransmit(rgchWriteAccl, 1, WRITE, ACCLADDR);
    
    rgchReadAccl[0] = chX0Addr;
    rgchReadAccl2[0] = chY0Addr;
    rgchReadAccl3[0] = chZ0Addr;
    
    I2CGenTransmit(rgchReadAccl, 2, READ, ACCLADDR);
    I2CGenTransmit(rgchReadAccl2, 2, READ, ACCLADDR);
    I2CGenTransmit(rgchReadAccl3, 2, READ, ACCLADDR);
    
    dataX = (rgchReadAccl[2] << 8) | rgchReadAccl[1];
    dataY = (rgchReadAccl2[2] << 8) | rgchReadAccl2[1];
    dataZ = (rgchReadAccl3[2] << 8) | rgchReadAccl2[1];
    
    if(dataX > 150 || dataX < -150 || dataY > 150 || dataY < -150)
       refresh = 1;
     else refresh = 0;

     return refresh;
}


unsigned int accelerometer_seed(){  //int accel_buffer[]){
  
  short	dataX;
  short dataY;
  short dataZ;
  int refresh;
  
  char  chPwrCtlReg = 0x2D;
  char 	chX0Addr = 0x32;
  char  chY0Addr = 0x34;
  char  chZ0Addr = 0x36;
  
  char 	rgchReadAccl[] = {
    0, 0, 0            };
  char 	rgchWriteAccl[] = {
    0, 0            };
  char  rgchReadAccl2[] = {
    0, 0, 0            };  
  char  rgchReadAccl3[] = {
    0, 0, 0            };
    
    /*
     * Enable I2C Peripheral
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    /*
     * Set I2C GPIO pins
     */
    GPIOPinTypeI2C(I2CSDAPort, I2CSDA_PIN);
    GPIOPinTypeI2CSCL(I2CSCLPort, I2CSCL_PIN);
    GPIOPinConfigure(I2CSCL);
    GPIOPinConfigure(I2CSDA);

    /*
     * Setup I2C
     */
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);
    
   /* Initialize the Accelerometer
     *
     */
    GPIOPinTypeGPIOInput(ACCL_INT2Port, ACCL_INT2);

    rgchWriteAccl[0] = chPwrCtlReg;
    rgchWriteAccl[1] = 1 << 3;		// sets Accl in measurement mode
    I2CGenTransmit(rgchWriteAccl, 1, WRITE, ACCLADDR);
    
    rgchReadAccl[0] = chX0Addr;
    rgchReadAccl2[0] = chY0Addr;
    rgchReadAccl3[0] = chZ0Addr;
    
    I2CGenTransmit(rgchReadAccl, 2, READ, ACCLADDR);
    I2CGenTransmit(rgchReadAccl2, 2, READ, ACCLADDR);
    I2CGenTransmit(rgchReadAccl3, 2, READ, ACCLADDR);
    
    dataX = (rgchReadAccl[2] << 8) | rgchReadAccl[1];
    dataY = (rgchReadAccl2[2] << 8) | rgchReadAccl2[1];
    dataZ = (rgchReadAccl3[2] << 8) | rgchReadAccl2[1];
    
    return (1+dataX)*(1+dataY)*(1+dataZ);
}




char I2CGenTransmit(char * pbData, int cSize, bool fRW, char bAddr) {

  int 	i;
  char * pbTemp;

  pbTemp = pbData;

  /*Start*/

  /*Send Address High Byte*/
  /* Send Write Block Cmd*/
  I2CMasterSlaveAddrSet(I2C0_BASE, bAddr, WRITE);
  I2CMasterDataPut(I2C0_BASE, *pbTemp);

  I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);

  DelayMs(1);

  /* Idle wait*/
  while(I2CGenIsNotIdle());

  /* Increment data pointer*/
  pbTemp++;

  /*Execute Read or Write*/

  if(fRW == READ) {

    /* Resend Start condition
	** Then send new control byte
	** then begin reading
	*/
    I2CMasterSlaveAddrSet(I2C0_BASE, bAddr, READ);

    while(I2CMasterBusy(I2C0_BASE));

    /* Begin Reading*/
    for(i = 0; i < cSize; i++) {

      if(cSize == i + 1 && cSize == 1) {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

        DelayMs(1);

        while(I2CMasterBusy(I2C0_BASE));
      }
      else if(cSize == i + 1 && cSize > 1) {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

        DelayMs(1);

        while(I2CMasterBusy(I2C0_BASE));
      }
      else if(i == 0) {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

        DelayMs(1);

        while(I2CMasterBusy(I2C0_BASE));

        /* Idle wait*/
        while(I2CGenIsNotIdle());
      }
      else {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);

        DelayMs(1);

        while(I2CMasterBusy(I2C0_BASE));

        /* Idle wait */
        while(I2CGenIsNotIdle());
      }

      while(I2CMasterBusy(I2C0_BASE));

      /* Read Data */
      *pbTemp = (char)I2CMasterDataGet(I2C0_BASE);

      pbTemp++;

    }

  }
  else if(fRW == WRITE) {

    /*Loop data bytes */
    for(i = 0; i < cSize; i++) {
      /* Send Data */
      I2CMasterDataPut(I2C0_BASE, *pbTemp);

      while(I2CMasterBusy(I2C0_BASE));

      if(i == cSize - 1) {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

        DelayMs(1);

        while(I2CMasterBusy(I2C0_BASE));
      }
      else {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

        DelayMs(1);

        while(I2CMasterBusy(I2C0_BASE));

        /* Idle wait */
        while(I2CGenIsNotIdle());
      }

      pbTemp++;
    }

  }

  /*Stop*/

  return 0x00;
}

/* ------------------------------------------------------------ */
/***	I2CGenIsNotIdle()
 **
 **	Parameters:
 **		pbData	-	Pointer to transmit buffer (read or write)
 **		cSize	-	Number of byte transactions to take place
 **
 **	Return Value:
 **		TRUE is bus is not idle, FALSE if bus is idle
 **
 **	Errors:
 **		none
 **
 **	Description:
 **		Returns TRUE if the bus is not idle
 **
 */
bool I2CGenIsNotIdle() {

  return !I2CMasterBusBusy(I2C0_BASE);

}
