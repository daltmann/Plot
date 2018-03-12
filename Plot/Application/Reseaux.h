/*
 * Reseaux.h
 *
 *  Created on: 28 Février 2018
 *      Author: Damien
 */

#ifndef APPLICATION_RESEAUX_H_
#define APPLICATION_RESEAUX_H_

#include <stdint.h>
#include <stdbool.h>


#define NUMBER_MAX_PLOT		112

void Init_Reseaux(void);

void saveConfigurationReseaux(uint8_t *u08DataConfigReseaux);

uint8_t returnLastPlotNumberConfiguration(void);

bool checkIfNewConfiguration(void);

void resetFlagNewConfiguration(void);

bool readTableConfigurationReseaux(uint8_t u08NumberPlot, uint8_t *table);

uint32_t returnLastSerialNumberConfiguration(void);

#endif /* APPLICATION_RESEAUX_H_ */
