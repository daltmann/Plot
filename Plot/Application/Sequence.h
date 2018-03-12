/*
 * Sequence.h
 *
 *  Created on: 25 Février 2018
 *      Author: Damien
 */

#ifndef APPLICATION_SEQUENCE_H_
#define APPLICATION_SEQUENCE_H_

#include <stdbool.h>
#include <stdint.h>

#define NUMBER_BYTE_BY_SEQUENCE		6

#define NUMBER_MAX_SEQUENCE			120

typedef struct
{
	uint8_t u08Mode;
	uint8_t u08Hour;
	uint8_t u08Minute;
	uint8_t u08Seconde;
}strucDemoMode;

typedef enum
{
	MODE_ARRET		= 0,
	MODE_DEMO		= 1,
	MODE_JOUR		= 2,
	MODE_NUIT		= 3,
	MODE_UPDATE		= 4,
	MODE_DEFAULT	= 5,
}enumModeFonctionnement;

typedef struct
{
	uint8_t u08Hour;
	uint8_t u08Minute;
	uint8_t u08Second;
	uint16_t u16MilliSecond;
}structHour;

void Init_Sequence(void);
void Init_Timer_Sequence(void);
void Init_DemoMode(void);
void Init_Hour(void);

void saveUniversNumber(const uint8_t *u08DataUniversNumber);
void saveUniversNumberUnit(uint8_t u08IdGroupValue, uint8_t u08SeqNbValue, uint8_t u08RepNuitValue);

uint8_t returnLastIDGroup(void);
uint8_t returnLastSequenceNumberGroup(void);
uint8_t returnLastRepartitionNuitGroup(void);
bool checkIfNewUniversNumberConfiguration(void);
void resetFlagNewUniversNumberConfiguration(void);
bool readUniversNumber(uint8_t *u08IdGroupValue, uint8_t *u08SeqNbValue, uint8_t *u08RepNuitValue);

void saveSequence(const uint8_t *u08DataSequence);
uint8_t returnLastSequenceNumber(void);
bool checkIfNewSequence(void);
void resetFlagNewSequence(void);
bool checkIfNewSequenceToSendRadio(void);
void resetFlagNewSequenceToSendRadio(void);
bool readTableSequence(uint8_t u08SequenceNumber, uint8_t *table);

void saveSequenceC4(const uint8_t *u08DataSequence);
uint8_t returnLastSequenceNumberC4(void);
bool checkIfNewSequenceC4(void);
void resetFlagNewSequenceC4(void);
void saveNumberSequenceC4(unsigned char u08DataValue);

bool checkIfEndSendModeScenario(void);
void resetFlagEndSendModeScenario(void);
void saveDemoMode(const uint8_t *u08DataDemoMode);

bool checkIfNewDemoMode(void);
void resetFlagNewDemoMode(void);
void initNewSequenceC5(void);
void initNewSequenceC4(unsigned char u08FonctionnementValue);

void incrementHour(void);
void saveHour(unsigned char u08Hour, unsigned char u08Minutes, unsigned char u08Seconds, unsigned int u16MilliSeconds);
structHour readHour(void);

void manageScenario(void);

#endif /* APPLICATION_SEQUENCE_H_ */
