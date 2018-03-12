/*
 * Reseaux.c
 *
 *  Created on: 28 Février 2018
 *      Author: Damien
 */

#include "Reseaux.h"

#include "app_uart.h"
#include <stdio.h>

#include "stdbool.h"
#include "string.h"

#include "nrf_delay.h"

uint8_t tableConfigurationReseaux[NUMBER_MAX_PLOT][4];		// 1 octet pour l'ID group, 3 octets poru le numéro de série
															// Le numéro de plot sera le premier index

bool bFlagNewConfiguration = false;

uint8_t u08valueLastPlotNumberConfiguration = 0;			// Valeur du dernier Plot configuré
uint32_t u32valueLastSerialNumber = 0xFF;					// Numéro de série du dernier plot configuré


/**@brief Function to manage the Reseaux
 */
void Init_Reseaux(void)
{
	uint8_t u08IndexPremier = 0;
	uint8_t u08IndexSecond = 0;

	// Initialisation du tableau de configuration réseaux
	for (u08IndexPremier = 0; u08IndexPremier < NUMBER_MAX_PLOT; u08IndexPremier++)
	{
		for (u08IndexSecond = 0; u08IndexSecond < 4; u08IndexSecond++)
		{
			tableConfigurationReseaux[u08IndexPremier][u08IndexSecond] = 0;
		}
	}
}


/**@brief Function to save the configuration table for all the network
 */
void saveConfigurationReseaux(uint8_t *u08DataConfigReseaux)
{
	uint8_t u08Index = 0;

	if ((u08DataConfigReseaux[4] <= NUMBER_MAX_PLOT) && (u08DataConfigReseaux[4] > 0))
	{	// Test si le numéro de plot est bien inférieur au maximum de plot possible !!!
		// On sauvegarde les infos dans le tableau de configuration
		for (u08Index = 0; u08Index < 4; u08Index++)
		{
			tableConfigurationReseaux[u08DataConfigReseaux[4] - 1][u08Index] = u08DataConfigReseaux[u08Index];
		}

		u08valueLastPlotNumberConfiguration = u08DataConfigReseaux[4];

		bFlagNewConfiguration = true;
	}
	else
	{	// Le nuémro de plot est supérieur au maximum
		// On indique une erreur du numéro
		u08valueLastPlotNumberConfiguration = 0xFF;
	}
}

uint8_t returnLastPlotNumberConfiguration(void)
{
	return u08valueLastPlotNumberConfiguration;
}

bool checkIfNewConfiguration(void)
{
	return bFlagNewConfiguration;
}

void resetFlagNewConfiguration(void)
{
	bFlagNewConfiguration = false;
}

bool readTableConfigurationReseaux(uint8_t u08NumberPlot, uint8_t *table)
{
	uint8_t u08Index = 0;

	if ((u08NumberPlot <= NUMBER_MAX_PLOT) && (u08NumberPlot > 0))
	{
		for (u08Index = 0; u08Index < 4; u08Index++)
		{
			table[u08Index] = tableConfigurationReseaux[u08NumberPlot - 1][u08Index];
		}

		u32valueLastSerialNumber = (table[1] << 16) + (table[2] << 8) + table[3];

		return true;
	}
	else
	{
		u32valueLastSerialNumber = 0xFF;

		return false;
	}
}

uint32_t returnLastSerialNumberConfiguration(void)
{
	return u32valueLastSerialNumber;
}
