/*
 * Scenario.h
 *
 *  Created on: 21 novembre 2017
 *      Author: Damien
 */

#ifndef APPLICATION_SCENARIO_H_
#define APPLICATION_SCENARIO_H_

#include <stdbool.h>

void Init_Scenario(void);

void setScenarioNumber(unsigned char u08Value);
unsigned char getScenarioNumber(void);

void Manage_PresenceSensor(void);

void Manage_Scenario(bool bDetectJour, bool *bSendNewScenario, bool *bSwitchOffLed, unsigned char *u08StateOfScenario);

void computeScenarioValue(void);

unsigned int getTimeScenario2Off(void);

unsigned int getTimeScenario2On(void);

void setHourLever(void);
unsigned int getHourLever(void);

void setHourCoucher(void);
unsigned int getHourCoucher(void);

extern unsigned int u16TimeScenario2Off;
extern unsigned int u16TimeScenario2On;

#endif /* APPLICATION_SCENARIO_H_ */
