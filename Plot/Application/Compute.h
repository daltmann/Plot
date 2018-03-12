/*
 * Compute.h
 *
 *  Created on: 02 novembre 2017
 *      Author: Damien
 */

#ifndef APPLICATION_COMPUTE_H_
#define APPLICATION_COMPUTE_H_

#include <stdbool.h>

unsigned int initCapacityBattery();
unsigned int initCapacityBatteryPlot();

void initGeneratedEnergy(void);

//void saveGeneratedEnergy(unsigned long u32ValueEnergy);

void saveLEDCurrent(unsigned int u16ValueEnergy);

void ComputeAverageLedEnergy(void);

unsigned int FindLEDmALevel(unsigned char BatPercentCharge);

unsigned int getPduValue(void);

unsigned int getTimeLedValue(void);

unsigned int getTimeToLightOnValue(void);

#endif /* APPLICATION_COMPUTE_H_ */
