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

unsigned char u08TableOfScenarioState[4];		// Tableau avec les diff�rents state de chaque sc�nario

unsigned int u16TimeScenario2Off = 0x0400;			// Temps d'extinction du sc�nario 2 apr�s heure de coucher : par d�faut � 4h00 apr�s le coucher

unsigned int u16TimeScenario2On = 0x0200;			// Temps d'allumage du sc�nario 2 avant heure de lever : par d�faut � 2h00 avant le lever

unsigned int u16HourCoucher = 0x1200;					// L'heure du coucher du soleil, par d�faut � 18h00

unsigned int u16HourLever = 0x0800;					// L'heure de lever du soleil, par d�faut � 8h00

bool bDetectionPresenceActivation = false;
unsigned int u16CounterToStartPirDetection = 0;
unsigned char u08CounterToStopLedAfterPirDetection = 0;
unsigned int u16CounterToStopPirDetection = 0;
bool bDectionPirSensor = false;
bool bLastDectionPirSensor = false;

#define MAXIMUM_COUNTER_TO_START_PIR_DETECTION	65000		// Temps pour la d�tection du capteur PIR

#define MAXIMUM_COUNTER_TO_STOP_PIR_DETECTION	65000		// Temps pour la non d�tection du capteur PIR

/**@brief Function to manage the scenario
 */
void Init_Scenario(void)
{
	uint8_t scenarioNumber_char[4];
	uint8_t u08ScenarioNumberValue = 0;

	// Lecture du num�ro de sc�nario
	memcpy(scenarioNumber_char, (uint32_t *)0x10001088, 4);
	u08ScenarioNumberValue = ((uint8_t)scenarioNumber_char[0]);

	// Sc�nario : par exemple, sc�nario 2
	// nrfjprog.exe -f nrf52 --memwr 0x10001088 --val 0xFFFFFF02

	if (u08ScenarioNumberValue == 0xFF)
	{	// Ici, pas de sc�nario enregitr� en m�moire
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
		{	// Capteur PIR renvoi 1, d�tection de pr�sence
			u16CounterToStartPirDetection++;
			if (u16CounterToStartPirDetection >= MAXIMUM_COUNTER_TO_START_PIR_DETECTION)
			{
				activateReceive_rs485();
				printf("PIR detecte\r\n");
				nrf_delay_ms(500);

				u08CounterToStopLedAfterPirDetection = 40;	// Pour 2 minutes d'allumage pendant la phase de d�tection pr�sence

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

	// TODO : en cas de changement de sc�nario, le u08TableOfScenarioState de l'ancien sc�nario revient � 0 !!!

	if (bDetectJour == true)
	{	// Ici, nous sommes dans la journ�e
		if (u08TableOfScenarioState[u08ScenarioNumber] != 0)
		{	// On test si le sc�nario �tait d�j� en cours
			// Si oui, on �teint les leds
			*bSwitchOffLed = true;

			// On enverra la commande � l'EPEVER
			*bSendNewScenario = true;

			// L'�tat du sc�nario en cours est aussi remis � 0
			*u08StateOfScenario = 0;

			// On indique que le sc�nario est remis � 0
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

					// Indiquer un �tat autre car on est pass� par l�
					u08TableOfScenarioState[0] = 1;

					// L'�tat du sc�nario 0 est mis � 1
					*u08StateOfScenario = 1;

					// On demande � envoyer la commande vers l'EPEVER
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
						// Le sc�nario 1 n'est pas d�marr�, donc, on va envoyer la premi�re commande
						break;
				}
				break;

			case 2:
				switch(u08TableOfScenarioState[2])
				{
					case 0:
						// Le sc�nario 1 n'est pas d�marr�, donc, on va envoyer la premi�re commande
						u16HourCoucher = getRealTimeHourValueInteger();

						// On demande � envoyer la commande vers l'EPEVER
						*bSendNewScenario = true;

						// Etape 1 pour le sc�nario 2
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
						// Le sc�nario 1 n'est pas d�marr�, donc, on va envoyer la premi�re commande
						u16HourCoucher = getRealTimeHourValueInteger();

						// On demande � envoyer la commande vers l'EPEVER
						*bSendNewScenario = true;

						// Etape 1 pour le sc�nario 3
						*u08StateOfScenario = 1;

						u08TableOfScenarioState[3] = 1;

						activateReceive_rs485();
						printf("Scen 3 : etape 0\r\n");
						nrf_delay_ms(50);
						break;

					case 1:
						// Ici, on test si l'heure n'a pas d�pass�e l'heure d'extinction de la premi�re s�quence
						if (getRealTimeHourValueInteger() == getTimeScenario2Off())
						{	// Si oui, on d�marre la d�tection de pr�sence
							activateReceive_rs485();
							printf("Scen 3 : etape 1\r\n");
							nrf_delay_ms(50);

							u08TableOfScenarioState[3] = 2;

							// Activation du d�tecteur de pr�sence
							bDetectionPresenceActivation = true;

							bLastDectionPirSensor = false;
						}
						break;

					case 2:
						// Ici, on test si l'heure n'a pas d�pass�e l'heure d'allumage de la seconde s�quence
						if (getRealTimeHourValueInteger() == getTimeScenario2On())
						{
							// Si oui, on d�marre le mode timing control
							// On demande � envoyer la commande vers l'EPEVER
							*bSendNewScenario = true;

							// Etape 4 pour le sc�nario 3
							*u08StateOfScenario = 4;

							u08TableOfScenarioState[3] = 3;

							activateReceive_rs485();
							printf("Scen 3 : etape 2\r\n");
							nrf_delay_ms(50);

							// D�sactivation du d�tecteur de pr�sence
							bDetectionPresenceActivation = false;
						}
						else
						{
							if ((bDectionPirSensor == true) && (bLastDectionPirSensor == false))
							{
								// On demande � envoyer la commande vers l'EPEVER
								*bSendNewScenario = true;

								// Etape 2 pour le sc�nario 3
								*u08StateOfScenario = 2;

								// Pour indiquer que la commande a �t� envoy�e
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

									// On demande � envoyer la commande vers l'EPEVER
									*bSendNewScenario = true;

									// Etape 3 pour le sc�nario 3
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
	// On va mettre � jour les valeurs des �nergies emmagasin�es et faire le calcul du PDU
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
{	// On recherche l'heeure d'extinction apr�s le coucher du soleil
	unsigned int u16Time = 0;

	u16Time = u16HourCoucher + u16TimeScenario2Off;
	if ((u16Time & 0x00FF) > 0x3B)
	{	// On regarde si on d�passe les 60 minutes
		u16Time += 0x0100;	// On rajoute 1 heure
		u16Time = u16Time - 60;	// On enl�ve 60 minutes pour trouver le reste
	}

	// On prend le modulo de l'addition de l'heure du coucher + le temps d'extinction apr�s coucher
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
	{	// On rajoute 60 minutes � la soustraction
		u16Time = 0x3C + (u16HourLever & 0x00FF) - (u16TimeScenario2On & 0x00FF);
		// On devra enlever 1h sur la soustraction des heures
		u16SoustractHour = 0x0100;
	}

	// On soustrait apr�s les heures
	if ((u16HourLever & 0xFF00) >= (u16TimeScenario2On & 0xFF00))
	{
		u16Time += (u16HourLever & 0xFF00) - (u16TimeScenario2On & 0xFF00) - u16SoustractHour;
	}
	else
	{	// On rajoute minuit=24h � la soustraction
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
