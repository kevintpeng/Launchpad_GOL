
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

#define L_BITMAP 8
#define LENGTH 128 // maybe set to xchOledMax?
#define HEIGHT 32
char bitmap[LENGTH*HEIGHT/8];
char aliveNow[LENGTH][HEIGHT];
char aliveNext[LENGTH][HEIGHT];
int count = 0;
int current_count = 0;
int total_iter = 0;
int framerate_check[10] = {100,100,100,100,100,100,100,100,100,100};

void setup(){
  
  DeviceInit();
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
}

void loop(){
  int q, i,j, greenness=0, delay_time=0;
	populate();
	for(q=0;q<2;){
		fillNextArray();
		updateCurrentArray(); 
		convert(); // converts the bool array to bitmap

		if(checkStability()) {
                  if(greenness<250)greenness+=25;
                   analogWrite(GREEN_LED, greenness);
                  analogWrite(RED_LED, 255-greenness);
		}
                else {
                  if(greenness>0) greenness-=25;
                  analogWrite(GREEN_LED, greenness);
                  analogWrite(RED_LED, 255-greenness);
                }
		OrbitOledMoveTo(0,0);
		OrbitOledPutBmp(LENGTH,HEIGHT,bitmap);
                OrbitOledUpdate();
                
                delay_time = msBettweenFrame()/50;
                
                delay(delay_time);
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
   if ( 0 <= potValue && potValue < 406){
   	return fastestMs * 10;
   }
   if (  406 <= potValue && potValue < 812){
   	return fastestMs * 9;
   }
   if (  812 <= potValue && potValue < 1218){
   	return fastestMs * 8;
   }
   if (  1218 <= potValue && potValue < 1624){
   	return fastestMs * 7;
   }
   if (  1624 <= potValue && potValue < 2030){
   	return fastestMs * 6;
   }
   if (  2030 <= potValue && potValue < 2436){
   	return fastestMs * 5;
   }
   if (  2436 <= potValue && potValue < 2842){
   	return fastestMs * 4;
   }
   if (  2842 <= potValue && potValue < 3248){
   	return fastestMs * 3;
   }
   if (  3248 <= potValue && potValue < 3654){
   	return fastestMs * 2;
   }
   if (  3654 <= potValue && potValue <= 495){
   	return fastestMs;
   }
 

}
