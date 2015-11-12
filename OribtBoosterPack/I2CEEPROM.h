/************************************************************************/
/*																		*/
/*	I2CEEPROM.h --	MX4cK on-board EEPROM control header file			*/
/*																		*/
/************************************************************************/
/*	Author: 	Jordan Retz												*/
/*	Copyright 2013, Digilent Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*		This module allows the user access to the on-board EEPROM on	*/
/*	the Cerebot MX4cK. It initializes I2C2 and allows both read and 	*/
/*	write cycles. The EEPROM has 32768 possible memory locations. This	*/
/*	module will initialize a temporary R/W software buffer, and thus	*/
/*	all transaction will require a requested buffer size. The default	*/
/*	buffer size is 64 bytes (EEPROM is capable of 64-byte page writes).	*/
/*																		*/
/*																		*/
/*																		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	02/20/2013(JordanR): Created										*/
/*	04/05/2013(JordanR): Ported for Stellaris Launchpad					*/
/*																		*/
/************************************************************************/


/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*				General Type Definitions						*/
/* ------------------------------------------------------------ */
#define MAX_PAGE_SIZE		64		// Bytes
#define MAX_MEM_LOC			32768	// Bytes
#define NUM_READ_CNTRL		2		// Bytes
#define NUM_WRITE_CNTRL		2		// Bytes

/* Addresses
*/
#define START_ADDR		0x0000
#define END_ADDR		0x7FFF
/* ------------------------------------------------------------ */
/*				Local Type Definitions							*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*				Local Variables									*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*				Procedure Declarations							*/
/* ------------------------------------------------------------ */	

void I2CEEPROMInit();
void I2CEEPROMTerm();
void I2CEEPROMRead(char * pbData, int bAddr, int cSize);
void I2CEEPROMWrite(char * pbData, int bAddr, int cSize);
bool I2CEEPROMIsEmpty();

/****************************************************************/
