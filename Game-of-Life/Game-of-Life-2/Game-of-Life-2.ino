#include <stdio.h>
#include <math.h>
//#include <limit.h> 
//#include <stdint.h> 

// ----------------------------------
//         DEMO IMPORTS
// ----------------------------------
extern "C" {
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
/*				Local Variables			*/
//* ------------------------------------------------------------ */
char	chSwtCur;
char	chSwtPrev;
bool	fClearOled;

/* ------------------------------------------------------------ */
/*				Global Variables		*/
/* ------------------------------------------------------------ */
extern int xchOledMax; // defined in OrbitOled.c
extern int ychOledMax; // defined in OrbitOled.c

#define L_BITMAP 16
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
  
}

void loop(){
  
  main();
  
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

  /*
   * Initialize the OLED
   */
  OrbitOledInit();

  /*
   * Reset flags
   */
  chSwtCur = 0;
  chSwtPrev = 0;
  fClearOled = true;

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
	int i,j,n=8, ret=0, index =0;
	for(j=0;j<HEIGHT;j++){
		for(i=0;i<LENGTH;i++){
			if(n==0){
				bitmap[index++] = (char)ret;
				n=8;
				ret=0;
			}
			ret+= aliveNow[i][j] * power(2,--n);
		}
	}
}


int main(){
	int q, i,j;
	populate();
	for(q=0;q<2;){
		fillNextArray();
		updateCurrentArray(); 
		convert(); // converts the bool array to bitmap

		// HEX PRINT
		/*for(j=0;j<HEIGHT;j++){
			for(i=0;i<L_BITMAP;i++){
				printf("0x%02hhX ",bitmap[j*L_BITMAP+i]);
			}
			printf("\n");
		}*/

		// CONSOLE PRINT
		/*for(j=0;j<HEIGHT;j++){
			for(i=0;i<LENGTH;i++){
				//printf("%d", aliveNext[i][j]);
				if(aliveNow[i][j])printf("██");
				else printf("  ");
			}
			printf("     %d\n",j);
		}*/

		if(checkStability()){
			printf("STABLE  ");
		}
		else printf("UNSTABLE");
		printf("   %d\n", ++total_iter);

		OrbitOledMoveTo(0,0);
		OrbitOledPutBmp(LENGTH,HEIGHT,bitmap);

		//usleep(100000);
	}
}
