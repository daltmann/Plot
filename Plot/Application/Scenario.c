/*
 * Scenario.c
 *
 *  Created on: 21 novembre 2017
 *      Author: Damien
 */

#include "Scenario.h"

#include "app_uart.h"
#include <stdio.h>
#include "nrf_drv_uart.h"
#include "nrf_gpio.h"

#include "stdbool.h"
#include "string.h"

#include "Epever_Serie.h"
#include "ble_Scenario.h"
#include "Compute.h"

#include "nrf_delay.h"

unsigned char u08ScenarioNumber = 0;

unsigned char u08TableOfScenarioState[4];		// Tableau avec les différents state de chaque scénario

unsigned int u16TimeScenario2Off = 0x0400;			// Temps d'extinction du scénario 2 après heure de coucher : par défaut à 4h00 après le coucher

unsigned int u16TimeScenario2On = 0x0200;			// Temps d'allumage du scénario 2 avant heure de lever : par défaut à 2h00 avant le lever

unsigned int u16HourCoucher = 0x1200;					// L'heure du coucher du soleil, par défaut à 18h00

unsigned int u16HourLever = 0x0800;					// L'heure de lever du soleil, par défaut à 8h00

bool bDetectionPresenceActivation = false;
unsigned int u16CounterToStartPirDetection = 0;
unsigned char u08CounterToStopLedAfterPirDetection = 0;
unsigned int u16CounterToStopPirDetection = 0;
bool bDectionPirSensor = false;
bool bLastDectionPirSensor = false;

#define MAXIMUM_COUNTER_TO_START_PIR_DETECTION	65000		// Temps pour la détection du capteur PIR

#define MAXIMUM_COUNTER_TO_STOP_PIR_DETECTION	65000		// Temps pour la non détection du capteur PIR

/**@brief Function to manage the scenario
 */
void Init_Scenario(void)
{
	uint8_t scenarioNumber_char[4];
	uint8_t u08ScenarioNumberValue = 0;

	// Lecture du numéro de scénario
	memcpy(scenarioNumber_char, (uint32_t *)0x10001088, 4);
	u08ScenarioNumberValue = ((uint8_t)scenarioNumber_char[0]);

	// Scénario : par exemple, scénario 2
	// nrfjprog.exe -f nrf52 --memwr 0x10001088 --val 0xFFFFFF02

	if (u08ScenarioNumberValue == 0xFF)
	{	// Ici, pas de scénario enregitré en mémoire
		u08ScenarioNumber = 0;
		printf("Scenario vide donc = 0\r\n");
	}
	else
	{
		u08ScenarioNumber = u08ScenarioNumberValue;
		printf("Scenario = %d\r\n", u08ScenarioNumberValue);
	}
	nrf_delay_ms(50);	// Attente pour printf

	u08TableOfScenarioState[0] = 0;
	u08TableOfScenarioState[1] = 0;
	u08TableOfScenarioState[2] = 0;
	u08TableOfScenarioState[3] = 0;
}

void setScenarioNumber(unsigned char u08Value)
{
	u08ScenarioNumber = u08Value;
}

unsigned char getScenarioNumber(void)
{
	return u08ScenarioNumber;
}

/**@brief Function to manage the presence sensor
 */
void Manage_PresenceSensor(void)
{
	if (bDetectionPresenceActivation == true)
	{
		if (nrf_gpio_pin_read(27) == 1)
		{	// Capteur PIR renvoi 1, détection de présence
			u16CounterToStartPirDetection++;
			if (u16CounterToStartPirDetection >= MAXIMUM_COUNTER_TO_START_PIR_DETECTION)
			{
				activateReceive_rs485();
				printf("PIR detecte\r\n");
				nrf_delay_ms(500);

				u08CounterToStopLedAfterPirDetection = 40;	// Pour 2 minutes d'allumage pendant la phase de détection présence

				bDectionPirSensor = true;

				u16CounterToStartPirDetection = 0;
				u16CounterToStopPirDetection = 0;
			}
		}
		else
		{
			if (bDectionPirSensor == true)
			{
				u16CounterToStopPirDetection++;
				if (u16CounterToStopPirDetection >= MAXIMUM_COUNTER_TO_STOP_PIR_DETECTION)
				{
					activateReceive_rs485();
					printf("PIR not detecte\r\n");
					nrf_delay_ms(500);

					bDectionPirSensor = false;

					u16CounterToStopPirDetection = 0;
					u16CounterToStartPirDetection = 0;
				}
			}
			else
			{
				u16CounterToStopPirDetection = 0;
				u16CounterToStartPirDetection = 0;
			}
		}
	}
}

/**@brief Function to manage the scenario
 */
void Manage_Scenario(bool bDetectJour, bool *bSendNewScenario, bool *bSwitchOffLed, unsigned char *u08StateOfScenario)
{
	//unsigned int u16ValueTemp = 0;

	*bSendNewScenario = false;
	*bSwitchOffLed = false;
	//*u08StateOfScenario = 0;

	// TODO : en cas de changement de scénario, le u08TableOfScenarioState de l'ancien scénario revient à 0 !!!

	if (bDetectJour == true)
	{	// Ici, nous sommes dans la journée
		if (u08TableOfScenarioState[u08ScenarioNumber] != 0)
		{	// On test si le scénario était déjà en cours
			// Si oui, on éteint les leds
			*bSwitchOffLed = true;

			// On enverra la commande à l'EPEVER
			*bSendNewScenario = true;

			// L'état du scénario en cours est aussi remis à 0
			*u08StateOfScenario = 0;

			// On indique que le scénario est remis à 0
			u08TableOfScenarioState[0] = 0;
			u08TableOfScenarioState[1] = 0;
			u08TableOfScenarioState[2] = 0;
			u08TableOfScenarioState[3] = 0;

			activateReceive_rs485();
			printf("Scen : led eteinte\r\n");
			nrf_delay_ms(500);
		}
	}
	else
	{	// Ici, nous sommes la nuit
		switch(u08ScenarioNumber)
		{
			case 0:
				if (u08TableOfScenarioState[0] == 0)
				{
					// Eteindre les leds
					*bSwitchOffLed = true;

					// Indiquer un état autre car on est passé par là
					u08TableOfScenarioState[0] = 1;

					// L'état du scénario 0 est mis à 1
					*u08StateOfScenario = 1;

					// On demande à envoyer la commande vers l'EPEVER
					*bSendNewScenario = true;

					activateReceive_rs485();
					printf("Scen 0 : led eteinte\r\n");
					nrf_delay_ms(50);
				}
				break;

			case 1:
				switch(u08TableOfScenarioState[1])
				{
					case 0:
						// Le scénario 1 n'est pas démarré, donc, on va envoyer la première commande
						break;
				}
				break;

			case 2:
				switch(u08TableOfScenarioState[2])
				{
					case 0:
						// Le scénario 1 n'est pas démarré, donc, on va envoyer la première commande
						u16HourCoucher = getRealTimeHourValueInteger();

						// On demande à envoyer la commande vers l'EPEVER
						*bSendNewScenario = true;

						// Etape 1 pour le scénario 2
						*u08StateOfScenario = 1;

						u08TableOfScenarioState[2] = 1;

						activateReceive_rs485();
						printf("Scen 2 : etape 0\r\n");
						nrf_delay_ms(50);
						break;

					case 1:
						break;

					case 2:
						break;
				}
				break;

			case 3:
				switch(u08TableOfScenarioState[3])
				{
					case 0:
						// Le scénario 1 n'est pas démarré, donc, on va envoyer la première commande
						u16HourCoucher = getRealTimeHourValueInteger();

						// On demande à envoyer la commande vers l'EPEVER
						*bSendNewScenario = true;

						// Etape 1 pour le scénario 3
						*u08StateOfScenario = 1;

						u08TableOfScenarioState[3] = 1;

						activateReceive_rs485();
						printf("Scen 3 : etape 0\r\n");
						nrf_delay_ms(50);
						break;

					case 1:
						// Ici, on test si l'heure n'a pas dépassée l'heure d'extinction de la première séquence
						if (getRealTimeHourValueInteger() == getTimeScenario2Off())
						{	// Si oui, on démarre la détection de présence
							activateReceive_rs485();
							printf("Scen 3 : etape 1\r\n");
							nrf_delay_ms(50);

							u08TableOfScenarioState[3] = 2;

							// Activation du détecteur de présence
							bDetectionPresenceActivation = true;

							bLastDectionPirSensor = false;
						}
						break;

					case 2:
						// Ici, on test si l'heure n'a pas dépassée l'heure d'allumage de la seconde séquence
						if (getRealTimeHourValueInteger() == getTimeScenario2On())
						{
							// Si oui, on démarre le mode timing control
							// On demande à envoyer la commande vers l'EPEVER
							*bSendNewScenario = true;

							// Etape 4 pour le scénario 3
							*u08StateOfScenario = 4;

							u08TableOfScenarioState[3] = 3;

							activateReceive_rs485();
							printf("Scen 3 : etape 2\r\n");
							nrf_delay_ms(50);

							// Désactivation du détecteur de présence
							bDetectionPresenceActivation = false;
						}
						else
						{
							if ((bDectionPirSensor == true) && (bLastDectionPirSensor == false))
							{
								// On demande à envoyer la commande vers l'EPEVER
								*bSendNewScenario = true;

								// Etape 2 pour le scénario 3
								*u08StateOfScenario = 2;

								// Pour indiquer que la commande a été envoyée
								bLastDectionPirSensor = true;

								activateReceive_rs485();
								printf("PIR detecte\r\n");
								nrf_delay_ms(50);
							}

							if (u08CounterToStopLedAfterPirDetection > 0)
							{
								u08CounterToStopLedAfterPirDetection--;
								if (u08CounterToStopLedAfterPirDetection == 0)
								{
									bDectionPirSensor = false;
									bLastDectionPirSensor = false;

									activateReceive_rs485();
									printf("Fin Pir Allumage\r\n");
									nrf_delay_ms(500);

									// On demande à envoyer la commande vers l'EPEVER
									*bSendNewScenario = true;

									// Etape 3 pour le scénario 3
									*u08StateOfScenario = 3;
								}
							}
						}
						break;

					case 3:
						// Ne rien faire
						break;
				}
				break;
		}
	}
}

void computeScenarioValue(void)
{
	// On va mettre à jour les valeurs des énergies emmagasinées et faire le calcul du PDU
	unsigned int u16GeneratedEnergy = 0;

	activateReceive_rs485();
	//NRF_LOG_INFO("Read Synchro\r\n");
	printf("Read Synchro\r\n");
	nrf_delay_ms(10);

	if (u08ScenarioNumber > 0)
	{
		u16GeneratedEnergy = getGeneratedEnergyValue();
		//NRF_LOG_INFO("u16GeneratedEnergy=%d\r\n", u16GeneratedEnergy);
		printf("u16GeneratedEnergy=%d\r\n", u16GeneratedEnergy);
		nrf_delay_ms(10);

		ComputeAverageLedEnergy();

		// Calcul du PDU
		//nrf_gpio_pin_toggle(15);

		unsigned int u16SOC = getBatterySOCValue();
		unsigned int u16CurrentPDU;
		u16CurrentPDU = FindLEDmALevel((unsigned char)(u16SOC & 0xFF));

		saveLEDCurrent(u16CurrentPDU);

		//NRF_LOG_INFO("Result : PDU = %d", getPduValue());
		printf("Result : PDU = %d\r\n", getPduValue());
		//NRF_LOG_INFO("Time = %d\r\n", getTimeToLightOnValue());
		printf("Time = %d\r\n", getTimeLedValue());

		nrf_delay_ms(200);	// Attente pour printf


	}
}

unsigned int getTimeScenario2Off(void)
{	// On recherche l'heeure d'extinction après le coucher du soleil
	unsigned int u16Time = 0;

	u16Time = u16HourCoucher + u16TimeScenario2Off;
	if ((u16Time & 0x00FF) > 0x3B)
	{	// On regarde si on dépasse les 60 minutes
		u16Time += 0x0100;	// On rajoute 1 heure
		u16Time = u16Time - 60;	// On enlève 60 minutes pour trouver le reste
	}

	// On prend le modulo de l'addition de l'heure du coucher + le temps d'extinction après coucher
	// et on fait le modulo sur 24h00 pour trouver le reste
	return (u16Time % 0x1800);
}

unsigned int getTimeScenario2On(void)
{	// On renvoi l'heure de l'allumage avant le lever du soleil
	unsigned int u16Time = 0;
	unsigned int u16SoustractHour = 0;

	// On soustrait d'abord les minutes
	if ((u16HourLever & 0x00FF) >= (u16TimeScenario2On & 0x00FF))
	{	// On fait une soustraction simple
		u16Time = (u16HourLever & 0x00FF) - (u16TimeScenario2On & 0x00FF);
	}
	else
	{	// On rajoute 60 minutes à la soustraction
		u16Time = 0x3C + (u16HourLever & 0x00FF) - (u16TimeScenario2On & 0x00FF);
		// On devra enlever 1h sur la soustraction des heures
		u16SoustractHour = 0x0100;
	}

	// On soustrait après les heures
	if ((u16HourLever & 0xFF00) >= (u16TimeScenario2On & 0xFF00))
	{
		u16Time += (u16HourLever & 0xFF00) - (u16TimeScenario2On & 0xFF00) - u16SoustractHour;
	}
	else
	{	// On rajoute minuit=24h à la soustraction
		u16Time += (u16HourLever & 0xFF00) + 0x1800 - (u16TimeScenario2On & 0xFF00) - u16SoustractHour;
	}
	return u16Time;
}

void setHourLever(void)
{
	// Enregistrement de l'heure de lever
	u16HourLever = getRealTimeHourValueInteger();
}

unsigned int getHourLever(void)
{
	return u16HourLever;
}

void setHourCoucher(void)
{
	// Enregistrement de l'heure de lever
	u16HourCoucher = getRealTimeHourValueInteger();
}

unsigned int getHourCoucher(void)
{
	return u16HourCoucher;
}
