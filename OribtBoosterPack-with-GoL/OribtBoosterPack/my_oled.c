#include <stdint.h>
#include <stdbool.h>

#include	"LaunchPad.h"
#include	"OrbitBoosterPackDefs.h"

#include	"OrbitOled.h"
#include	"OrbitOledGrph.h"
#include	"OrbitOledChar.h"
#include	"FillPat.h"

#include	"delay.h"

void init_LEDS(){
	SysCtlClockSet(SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_USE_PLL | SYSCTL_SYSDIV_4);

	/*
	 * Enable and Power On All GPIO Ports
	 */
	SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOF);


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
}
void init_OLED() {

	/*
	 * First, Set Up the Clock.
	 * Main OSC				-> SYSCTL_OSC_MAIN
	 * Runs off 16MHz clock -> SYSCTL_XTAL_16MHZ
	 * Use PLL				-> SYSCTL_USE_PLL
	 * Divide by 4			-> SYSCTL_SYSDIV_4
	 */
	SysCtlClockSet(SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_USE_PLL | SYSCTL_SYSDIV_4);

	/*
	 * Enable and Power On All GPIO Ports
	 */
	SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOF);


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

	OrbitSetOled();

}

void setup_OLED()
{
	char szAnalog[] = {'A', 'n', 'a', 'l', 'o', 'g', ':', ' ', '\0'};
	char szDemo1[]	= {'O', 'r', 'b', 'i', 't', ' ', 'D', 'e', 'm', 'o', '!', '\0'};
	char szDemo2[]	= {'B', 'y', ' ', 'D', 'i', 'g', 'i', 'l', 'e', 'n', 't', '\0'};

	OrbitOledSetCursor(0, 0);
	OrbitOledPutString(szDemo1);

	OrbitOledSetCursor(0, 1);
	OrbitOledPutString(szDemo2);

	OrbitOledMoveTo(0,19);
	OrbitOledLineTo(127, 19);

	OrbitOledSetCursor(0, 4);
	OrbitOledPutString(szAnalog);

}
