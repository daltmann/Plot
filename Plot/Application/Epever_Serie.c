/*
 * Epever_Serie.c
 *
 *  Created on: 25 oct. 2017
 *      Author: Damien
 */

#include "Epever_Serie.h"

#include "app_uart.h"
#include <stdio.h>
#include "nrf_drv_uart.h"
#include "nrf_gpio.h"

#include "stdbool.h"
#include "string.h"

#include "ble_Scenario.h"

#include "nrf_delay.h"

#include "Compute.h"

#include "Scenario.h"

stateSendProtocol u08IndexStateSend = STATE_END;

#define NUMBER_MAX_OF_RECEIVE_DATA		20
unsigned char u08NumberOfDataReceive = 0;
unsigned char u08TableDataReceive[NUMBER_MAX_OF_RECEIVE_DATA];
unsigned char u08NumberMaxReceiveData = NUMBER_MAX_OF_RECEIVE_DATA;

unsigned char u08TableauSend[15];

unsigned int crc16Modbus = 0xFF;

nrf_drv_uart_t m_uart_EpeverSerie = NRF_DRV_UART_INSTANCE(0);

bool bFlagUpdateBLEValue = false;

// Valeur provenant de l'EPEVER
unsigned int u16BatteryVoltageValue = 0;
unsigned int u16BatteryMaximumVoltageValue = 0;
signed long s32BatteryCurrentValue = 0;
unsigned int u16PvVoltageValue = 0;
unsigned int u16PvMaximumVoltageValue = 200;	// TODO : à remettre : 0;
unsigned long u32GeneratedEnergyValue = 0;
unsigned int u16BatterySOCValue = 0;
unsigned int u16BatteryStatusValue = 0;
unsigned int u16PvCurrentValue = 0;
unsigned int u16LoadCurrentValue = 0;
unsigned int u16LoadVoltageValue = 0;
unsigned int u16LoadMaximumVoltageValue = 0;
unsigned long u32ConsumedEnergyValue = 0;
unsigned long long u64RealTimeHourValue = 0;

unsigned char u08ScenarioNumberEpever = 0;

unsigned int u16CapacityBattery = 0;

unsigned long long u64RealTimeClock = 0;

unsigned int u16CounterToSendAgain = 0;
#define COUNTER_TO_SEND_AGAIN	50000

//uint8_t u08TableDataScenario[20];	// TODO : à supprimer

void Manage_Epever_RS485(bool resetProtocol, bool bNewScenario, bool bArretLed, unsigned char u08StateOfScenario, bool bHour)
{
	//uint8_t u08NumberOfData = 0;
	unsigned int u16Hour = 0;

	//int s16NumberOfCharacter = 0;
	//unsigned char tabCharacter[100];
	stateSendProtocol u08IndexStateSendAvant = u08IndexStateSend;

	if (resetProtocol == true)
	{
		u16CounterToSendAgain = 0;

		if (bNewScenario == true)
		{	// Ici, on envoi un nouveau scénario
			//u08NumberOfData = ble_scenario_readScenarioData(&u08TableDataScenario);	// TODO : à surpprimer

			//s16NumberOfCharacter = sprintf(tabCharacter, "\r\nRecv : %d %d %d %d %d\r\n", u08NumberOfData, u08TableDataScenario[0], u08TableDataScenario[1], u08TableDataScenario[2], u08TableDataScenario[3]);
			//nrf_drv_uart_tx(&m_uart_EpeverSerie, (uint8_t *)&tabCharacter[0], (uint8_t)s16NumberOfCharacter);

			// On test si on doit arrêt l'allumage des leds
			if (bArretLed == true)
			{	// Si on doit arrêter l'allumage, on envoi la commande manuelle de stop
				u08IndexStateSend = STATE_SCENARIO_0_MANUEL;
			}
			else
			{	// Sinon, on test et on envoi la commande en fonction du numéro de scénario
				switch(u08ScenarioNumberEpever)
				{
					case 0:
						u08IndexStateSend = STATE_SCENARIO_0_MANUEL;
						//u08IndexStateSend = STATE_END;
						break;

					case 1:
						u08IndexStateSend = STATE_SCENARIO_1_LOAD_CONTROL;
						/*switch(u08StateOfScenario)
						{

						}*/
						break;

					case 2:
						switch(u08StateOfScenario)
						{
							case 0:
								break;

							case 1:
								u08IndexStateSend = STATE_SCENARIO_2_LOAD_CONTROL;
								break;

							case 2:
								break;
						}
						break;

					case 3:
						switch(u08StateOfScenario)
						{
							case 0:
								break;

							case 1:
								// Envoi des heures pour les timing control
								u08IndexStateSend = STATE_SCENARIO_2_LOAD_CONTROL;
								break;

							case 2:
								// Allumage des leds en cas de détection présence
								u08IndexStateSend = STATE_SCENARIO_3_MANUEL_ON;
								break;

							case 3:
								// Extinction des leds en cas de non détection présence
								u08IndexStateSend = STATE_SCENARIO_3_MANUEL_OFF;
								break;

							case 4:
								// Mode timing control réactiver
								u08IndexStateSend = STATE_SCENARIO_3_ACTIVE_TIMING_CTRL;
								break;
						}
						break;

					default:
						// Aucun scénario correcte, on ne fait rien et on relance une lecture à la place
						u08IndexStateSend = STATE_BATTERY_VOLTAGE;
						//u08IndexStateSend = STATE_END;
						break;
				}
			}
		}
		else
		{	// on reste dans la lecture des paramètres de l'EPEVER
			if (bHour == true)
			{
				// Ecriture de l'heure dans l'EPEVER
				u08IndexStateSend = STATE_REAL_TIME_MIN_WRITE;
			}
			else
			{
				u08IndexStateSend = STATE_BATTERY_VOLTAGE;
				//u08IndexStateSend = STATE_END;
			}
		}
	}
	else
	{
		switch(u08IndexStateSend)
		{
			case STATE_BATTERY_VOLTAGE:
				sendEpeverRS(01, 04, BATTERY_VOLTAGE, 0001);
				u08IndexStateSend = STATE_BATTERY_VOLTAGE_WAIT;
				break;

			case STATE_BATTERY_VOLTAGE_WAIT:
				break;

			case STATE_BATTERY_CURRENT:
				sendEpeverRS(01, 04, BATTERY_CURRENT, 0002);
				u08IndexStateSend = STATE_BATTERY_CURRENT_WAIT;
				break;

			case STATE_BATTERY_CURRENT_WAIT:
				break;

			case STATE_PV_VOLTAGE:
				sendEpeverRS(01, 04, PV_VOLTAGE, 0001);
				u08IndexStateSend = STATE_PV_VOLTAGE_WAIT;
				break;

			case STATE_PV_VOLTAGE_WAIT:
				break;

			case STATE_GENERATED_ENERGY:
				sendEpeverRS(01, 04, GENERATED_ENERGY, 0002);
				u08IndexStateSend = STATE_GENERATED_ENERGY_WAIT;
				break;

			case STATE_GENERATED_ENERGY_WAIT:
				break;

			case STATE_BATTERY_SOC:
				sendEpeverRS(01, 04, BATTERY_SOC, 0001);
				u08IndexStateSend = STATE_BATTERY_SOC_WAIT;
				break;

			case STATE_BATTERY_SOC_WAIT:
				break;

			case STATE_BATTERY_STATUS:
				sendEpeverRS(01, 04, BATTERY_STATUS, 0001);
				u08IndexStateSend = STATE_BATTERY_STATUS_WAIT;
				break;

			case STATE_BATTERY_STATUS_WAIT:
				break;

			case STATE_PV_CURRENT:
				sendEpeverRS(01, 04, PV_CURRENT, 0001);
				u08IndexStateSend = STATE_PV_CURRENT_WAIT;
				break;

			case STATE_PV_CURRENT_WAIT:
				break;

			case STATE_LOAD_CURRENT:
				sendEpeverRS(01, 04, LOAD_CURRENT, 0001);
				u08IndexStateSend = STATE_LOAD_CURRENT_WAIT;
				break;

			case STATE_LOAD_CURRENT_WAIT:
				break;

			case STATE_LOAD_VOLTAGE:
				sendEpeverRS(01, 04, LOAD_VOLTAGE, 0001);
				u08IndexStateSend = STATE_LOAD_VOLTAGE_WAIT;
				break;

			case STATE_LOAD_VOLTAGE_WAIT:
				break;

			case STATE_CONSUMED_ENERGY_TODAY:
				sendEpeverRS(01, 04, CONSUMED_ENERGY_TODAY, 0002);
				u08IndexStateSend = STATE_CONSUMED_ENERGY_TODAY_WAIT;
				break;

			case STATE_CONSUMED_ENERGY_TODAY_WAIT:
				break;

			case STATE_REAL_TIME_CLOCK:
				sendEpeverRS(01, 03, REAL_TIME_CLOCK, 0003);
				u08IndexStateSend = STATE_REAL_TIME_CLOCK_WAIT;
				break;

			case STATE_REAL_TIME_CLOCK_WAIT:
				break;

			// ------------------- Ecriture de l'heure dans le Real Time Clock -------------------
			case STATE_REAL_TIME_MIN_WRITE:
				sendEpeverRSWriteHour(01, 0x10, REAL_TIME_CLOCK, getRealTimeClock());
				u08IndexStateSend = STATE_REAL_TIME_MIN_WRITE_WAIT;

				break;

			case STATE_REAL_TIME_MIN_WRITE_WAIT:
				break;

			case STATE_REAL_TIME_HOUR_WRITE:
				//u16Hour = getRealTimeClock();
				//u16Hour = (((unsigned int)(u32RealTimeHourValue & 0xFF000000)) >> 16) + ((u16Hour & 0xFF00) >> 8);

				//sendEpeverRSWrite(01, 0x10, REAL_TIME_CLOCK+1, u16Hour);
				sendEpeverRS(01, 03, REAL_TIME_CLOCK, 0003);
				u08IndexStateSend = STATE_REAL_TIME_HOUR_WRITE_WAIT;
				break;

			case STATE_REAL_TIME_HOUR_WRITE_WAIT:
				break;


			// ------------------- Scénario 0 : envoi des commandes -------------------
			case STATE_SCENARIO_0_MANUEL:
				//sendEpeverRSCommand(01, 05, CMD_MANUEL_CONTROL, 0x0000);
				//sendEpeverRSCommand(01, 05, CMD_FORCE_LOAD, 0x0000);
				sendEpeverRSWrite(01, 0x10, CMD_LOAD_CONTROL, 0x0000);
				u08IndexStateSend = STATE_SCENARIO_0_MANUEL_WAIT;
				break;

			case STATE_SCENARIO_0_MANUEL_WAIT:
				break;

			case STATE_SCENARIO_0_FORCE:
				// Force la sortie à 0
				//sendEpeverRSCommand(01, 05, CMD_FORCE_LOAD, 0x0000);
				//sendEpeverRSCommand(01, 05, CMD_LOAD_TEST, 0x0000);
				sendEpeverRSWrite(01, 0x10, DEFAULT_LOAD_IN_MANUAL, 0x0000);
				u08IndexStateSend = STATE_SCENARIO_0_FORCE_WAIT;
				break;

			case STATE_SCENARIO_0_FORCE_WAIT:
				break;

			case STATE_SCENARIO_0_RESET_PERC:
				// Force la sortie à 1
				sendEpeverRSWrite(01, 0x10, CMD_LED_RATE_MANUEL_PERC, 0);
				u08IndexStateSend = STATE_SCENARIO_0_RESET_PERC_WAIT;
				break;

			case STATE_SCENARIO_0_RESET_PERC_WAIT:
				break;

			// ------------------- Scénario 1 : envoi des commandes -------------------
			case STATE_SCENARIO_1_LOAD_CONTROL:
				sendEpeverRSWrite(01, 0x10, CMD_LOAD_CONTROL, 0x0004);
				u08IndexStateSend = STATE_SCENARIO_1_LOAD_CONTROL_WAIT;
				break;

			case STATE_SCENARIO_1_LOAD_CONTROL_WAIT:
				break;

			case STATE_SCENARIO_1_NB_PERIODE:
				sendEpeverRSWrite(01, 0x10, CMD_NB_PERIODE, 0x0003);
				u08IndexStateSend = STATE_SCENARIO_1_NB_PERIODE_WAIT;
				break;

			case STATE_SCENARIO_1_NB_PERIODE_WAIT:
				break;

			case STATE_SCENARIO_1_LED_RATE:
				sendEpeverRSWrite(01, 0x10, CMD_LED_RATE, 100);	// Ici, on mets toujours 1A car il s'agit du courant minimum
				u08IndexStateSend = STATE_SCENARIO_1_LED_RATE_WAIT;
				break;

			case STATE_SCENARIO_1_LED_RATE_WAIT:
				break;

			case STATE_SCENARIO_1_TIMER_1:
				if (getTimeLedValue() >=1)
				{
					sendEpeverRSWrite(01, 0x10, CMD_TIMER_1, 0x001E); // Ici, on mets 30 minutes
				}
				else
				{
					sendEpeverRSWrite(01, 0x10, CMD_TIMER_1, 0);	// 0 minutes
				}
				u08IndexStateSend = STATE_SCENARIO_1_TIMER_1_WAIT;

				break;

			case STATE_SCENARIO_1_TIMER_1_WAIT:
				break;

			case STATE_SCENARIO_1_TIMER_2:
				if (getTimeLedValue() > 1)
				{	// Ici, on a au moins 1 heure d'éclairage, on enlève donc 1 heure et on garde le restant d'heure
					sendEpeverRSWrite(01, 0x10, CMD_TIMER_2, (((unsigned int)((getTimeLedValue() - 1)) << 8) & 0xFF00));
				}
				else
				{
					sendEpeverRSWrite(01, 0x10, CMD_TIMER_2, 0);
				}

				u08IndexStateSend = STATE_SCENARIO_1_TIMER_2_WAIT;
				break;

			case STATE_SCENARIO_1_TIMER_2_WAIT:
				break;

			case STATE_SCENARIO_1_TIMER_3:
				if (getTimeLedValue() >=1)
				{
					sendEpeverRSWrite(01, 0x10, CMD_TIMER_3, 0x001E); // Ici, on mets 30 minutes
				}
				else
				{
					sendEpeverRSWrite(01, 0x10, CMD_TIMER_3, 0);	// 0 minutes
				}
				u08IndexStateSend = STATE_SCENARIO_1_TIMER_3_WAIT;
				break;

			case STATE_SCENARIO_1_TIMER_3_WAIT:
				break;

			case STATE_SCENARIO_1_RATED_CURRENT_1:
				// Ici, on met toujours 50% du PDU
				// La valeur en % doit être * 100 mais on prend 50% donc on divise par 2
				// Sauf que l'on veux un pourcentage par rapport à 1A
				// Ex : si PDU = 600 mA, celà représente 60% de 1A, valeur qui est *100 (=6000) mais on prend que la moitié donc / 2
				// cela donne : ((PDU / 10) * 100) / 2 = PDU * 5
				sendEpeverRSWrite(01, 0x10, CMD_RATE_CURRENT_1, (getPduValue() * 5));
				u08IndexStateSend = STATE_SCENARIO_1_RATED_CURRENT_1_WAIT;
				break;

			case STATE_SCENARIO_1_RATED_CURRENT_1_WAIT:
				break;

			case STATE_SCENARIO_1_RATED_CURRENT_2:
				// Ici, on met toujours 100% du PDU
				// La valeur en % doit être * 100
				// Sauf que l'on veux un pourcentage par rapport à 1A
				// Ex : si PDU = 600 mA, cela représente 60% de 1A, valeur qui est *100 (=6000)
				// cela donne : ((PDU / 10) * 100) = PDU * 10
				sendEpeverRSWrite(01, 0x10, CMD_RATE_CURRENT_2, (getPduValue() * 10));
				u08IndexStateSend = STATE_SCENARIO_1_RATED_CURRENT_2_WAIT;
				break;

			case STATE_SCENARIO_1_RATED_CURRENT_2_WAIT:
				break;

			case STATE_SCENARIO_1_RATED_CURRENT_3:
				// Ici, on met toujours 50% du PDU
				// La valeur en % doit être * 100 mais on prend 50% donc on divise par 2
				// Sauf que l'on veux un pourcentage par rapport à 1A
				// Ex : si PDU = 600 mA, celà représente 60% de 1A, valeur qui est *100 (=6000) mais on prend que la moitié donc / 2
				// cela donne : ((PDU / 10) * 100) / 2 = PDU * 5
				sendEpeverRSWrite(01, 0x10, CMD_RATE_CURRENT_3, (getPduValue() * 5));
				u08IndexStateSend = STATE_SCENARIO_1_RATED_CURRENT_3_WAIT;
				break;

			case STATE_SCENARIO_1_RATED_CURRENT_3_WAIT:
				break;


			// ------------------- Scénario 2 : envoi des commandes -------------------
			case STATE_SCENARIO_2_LOAD_CONTROL:
				// Ecriture de mode "Timing Control"
				sendEpeverRSWrite(01, 0x10, CMD_LOAD_CONTROL, 0x0003);
				u08IndexStateSend = STATE_SCENARIO_2_LOAD_CONTROL_WAIT;
				break;

			case STATE_SCENARIO_2_LOAD_CONTROL_WAIT:
				break;

			case STATE_SCENARIO_2_NB_TIMING_CONTROL:
				nrf_delay_ms(50);
				sendEpeverRSWrite(01, 0x10, CMD_NB_PERIODE, 0x0003);
				u08IndexStateSend = STATE_SCENARIO_2_NB_TIMING_CONTROL_WAIT;
				break;

			case STATE_SCENARIO_2_NB_TIMING_CONTROL_WAIT:
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_ON_1:
				nrf_delay_ms(50);
				sendEpeverRSWriteTimingControl(01, 0x10, TIMING_CONTROL_ON_1, getRealTimeHourValueInteger());
				u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_ON_1_WAIT;
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_ON_1_WAIT:
				break;

			// TODO : gérer le temps en fonction de "getTimeLedValue"

			case STATE_SCENARIO_2_TIMING_CONTROL_OFF_1:
				nrf_delay_ms(50);
				u16Hour = getRealTimeHourValueInteger();
				u16Hour += 30;	// Ajout de 30 minutes sur l'heure actuelle
				if ((u16Hour & 0x00FF) > 0x3C)
				{
					u16Hour += 0x0100;	// On rajoute 1 heure
					u16Hour = u16Hour - 60;	// On enlève 60 pour avoir des minutes restantes
				}

				sendEpeverRSWriteTimingControl(01, 0x10, TIMING_CONTROL_OFF_1, u16Hour);
				u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_OFF_1_WAIT;
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_OFF_1_WAIT:
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_ON_2:
				nrf_delay_ms(50);
				u16Hour = getRealTimeHourValueInteger();
				u16Hour += 30;	// Ajout de 30 minutes sur l'heure actuelle
				if ((u16Hour & 0x00FF) > 0x3C)
				{
					u16Hour += 0x0100;	// On rajoute 1 heure
					u16Hour = u16Hour - 60;	// On enlève 60 pour avoir des minutes restantes
				}

				sendEpeverRSWriteTimingControl(01, 0x10, TIMING_CONTROL_ON_2, u16Hour);
				u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_ON_2_WAIT;
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_ON_2_WAIT:
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_OFF_2:
				nrf_delay_ms(50);
				sendEpeverRSWriteTimingControl(01, 0x10, TIMING_CONTROL_OFF_2, getTimeScenario2Off());
				u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_OFF_2_WAIT;
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_OFF_2_WAIT:
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_ON_3:
				nrf_delay_ms(50);
				sendEpeverRSWriteTimingControl(01, 0x10, TIMING_CONTROL_ON_3, getTimeScenario2On());
				u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_ON_3_WAIT;
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_ON_3_WAIT:
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_OFF_3:
				nrf_delay_ms(50);
				u16Hour = getHourLever();

				if (u16TimeScenario2On != 0)
				{
					if ((u16Hour & 0x00FF) >= 30)
					{	// on enlève simplement les 30 minutes de l'heure du lever
						u16Hour = u16Hour - 30;
					}
					else
					{	// On doit enlever 1 heure, rajouter 60 minutes  et enlever les 30 minutes
						// Normalement, la formule serait : u16Hour = u16Hour + (60 - 30 - 0x0100)
						u16Hour = u16Hour + 30;
						if (u16Hour > 0x0100)
						{	// on soustrait les 1 heures ici
							u16Hour = u16Hour - 0x0100;
						}
						else
						{	// sinon, on rajouter 24h et on enlève les 1h
							u16Hour = 0x1800 + u16Hour - 0x0100;
						}
					}
				}

				sendEpeverRSWriteTimingControl(01, 0x10, TIMING_CONTROL_OFF_3, u16Hour);
				u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_OFF_3_WAIT;
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_OFF_3_WAIT:
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_ON_4:
				nrf_delay_ms(50);
				u16Hour = getHourLever();

				if (u16TimeScenario2On != 0)
				{
					if ((u16Hour & 0x00FF) >= 30)
					{	// on enlève simplement les 30 minutes de l'heure du lever
						u16Hour = u16Hour - 30;
					}
					else
					{	// On doit enlever 1 heure, rajouter 60 minutes  et enlever les 30 minutes
						// Normalement, la formule serait : u16Hour = u16Hour + (60 - 30 - 0x0100)
						u16Hour = u16Hour + 30;
						if (u16Hour > 0x0100)
						{	// on soustrait les 1 heures ici
							u16Hour = u16Hour - 0x0100;
						}
						else
						{	// sinon, on rajouter 24h et on enlève les 1h
							u16Hour = 0x1800 + u16Hour - 0x0100;
						}
					}
				}
				sendEpeverRSWriteTimingControl(01, 0x10, TIMING_CONTROL_ON_4, u16Hour);
				u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_ON_4_WAIT;
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_ON_4_WAIT:
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_OFF_4:
				nrf_delay_ms(50);
				sendEpeverRSWriteTimingControl(01, 0x10, TIMING_CONTROL_OFF_4, getHourLever());
				u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_OFF_4_WAIT;
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_OFF_4_WAIT:
				break;

			case STATE_SCENARIO_2_LED_RATE_CURRENT:
				nrf_delay_ms(50);
				// Ici, on met toujours le courant correspondant a 1A
				sendEpeverRSWrite(01, 0x10, CMD_LED_RATE, 100);
				u08IndexStateSend = STATE_SCENARIO_2_LED_RATE_CURRENT_WAIT;
				break;

			case STATE_SCENARIO_2_LED_RATE_CURRENT_WAIT:
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_1_PERC:
				nrf_delay_ms(50);
				// Ici, on met toujours 50% du PDU
				// La valeur en % doit être * 100 mais on prend 50% donc on divise par 2
				// Sauf que l'on veux un pourcentage par rapport à 1A
				// Ex : si PDU = 600 mA, celà représente 60% de 1A, valeur qui est *100 (=6000) mais on prend que la moitié donc / 2
				// cela donne : ((PDU / 10) * 100) / 2 = PDU * 5
				sendEpeverRSWrite(01, 0x10, TIMING_CONTROL_1_PERCENTAGE, (getPduValue() * 5));
				u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_1_PERC_WAIT;
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_1_PERC_WAIT:
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_2_PERC:
				nrf_delay_ms(50);
				// Ici, on met toujours 100% du PDU
				// La valeur en % doit être * 100
				// Sauf que l'on veux un pourcentage par rapport à 1A
				// Ex : si PDU = 600 mA, cela représente 60% de 1A, valeur qui est *100 (=6000)
				// cela donne : ((PDU / 10) * 100) = PDU * 10
				sendEpeverRSWrite(01, 0x10, TIMING_CONTROL_2_PERCENTAGE, (getPduValue() * 10));
				u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_2_PERC_WAIT;
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_2_PERC_WAIT:
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_3_PERC:
				nrf_delay_ms(50);
				// Ici, on met toujours 100% du PDU
				// La valeur en % doit être * 100
				// Sauf que l'on veux un pourcentage par rapport à 1A
				// Ex : si PDU = 600 mA, cela représente 60% de 1A, valeur qui est *100 (=6000)
				// cela donne : ((PDU / 10) * 100) = PDU * 10
				sendEpeverRSWrite(01, 0x10, TIMING_CONTROL_3_PERCENTAGE, (getPduValue() * 10));
				u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_3_PERC_WAIT;
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_3_PERC_WAIT:
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_4_PERC:
				nrf_delay_ms(50);
				// Ici, on met toujours 50% du PDU
				// La valeur en % doit être * 100 mais on prend 50% donc on divise par 2
				// Sauf que l'on veux un pourcentage par rapport à 1A
				// Ex : si PDU = 600 mA, celà représente 60% de 1A, valeur qui est *100 (=6000) mais on prend que la moitié donc / 2
				// cela donne : ((PDU / 10) * 100) / 2 = PDU * 5
				sendEpeverRSWrite(01, 0x10, TIMING_CONTROL_4_PERCENTAGE, (getPduValue() * 5));
				u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_4_PERC_WAIT;
				break;

			case STATE_SCENARIO_2_TIMING_CONTROL_4_PERC_WAIT:
				break;


			// ------------------- Scénario 3 : envoi des commandes -------------------
			// Détectino de présence ON
			case STATE_SCENARIO_3_MANUEL_ON:
				//sendEpeverRSCommand(01, 05, CMD_MANUEL_CONTROL, 0xFF00);
				//sendEpeverRSCommand(01, 05, CMD_LOAD_TEST, 0xFF00);
				sendEpeverRSWrite(01, 0x10, CMD_LOAD_CONTROL, 0x0000);	// Activation du mode manuel
				u08IndexStateSend = STATE_SCENARIO_3_MANUEL_ON_WAIT;
				break;

			case STATE_SCENARIO_3_MANUEL_ON_WAIT:
				break;

			case STATE_SCENARIO_3_FORCE_PERC:
				// Force la sortie à 1
				sendEpeverRSWrite(01, 0x10, CMD_LED_RATE_MANUEL_PERC, (getPduValue() * 10));
				u08IndexStateSend = STATE_SCENARIO_3_FORCE_PERC_WAIT;
				break;

			case STATE_SCENARIO_3_FORCE_PERC_WAIT:
				break;

			case STATE_SCENARIO_3_LOAD_MANUAL_ON:
				//sendEpeverRSWrite(01, 0x10, DEFAULT_LOAD_IN_MANUAL, 0x0001);
				sendEpeverRSCommand(01, 05, CMD_MANUEL_CONTROL, 0xFF00);
				u08IndexStateSend = STATE_SCENARIO_3_LOAD_MANUAL_ON_WAIT;
				break;

			case STATE_SCENARIO_3_LOAD_MANUAL_ON_WAIT:
				break;

			// Détectino de présence OFF
			case STATE_SCENARIO_3_MANUEL_OFF:
				sendEpeverRSWrite(01, 0x10, CMD_LOAD_CONTROL, 0x0000);	// Activation du mode manuel
				u08IndexStateSend = STATE_SCENARIO_3_MANUEL_OFF_WAIT;
				break;

			case STATE_SCENARIO_3_MANUEL_OFF_WAIT:
				break;

			case STATE_SCENARIO_3_RESET_PERC:
				// Force la sortie à 1
				sendEpeverRSWrite(01, 0x10, CMD_LED_RATE_MANUEL_PERC, 0);
				u08IndexStateSend = STATE_SCENARIO_3_RESET_PERC_WAIT;
				break;

			case STATE_SCENARIO_3_RESET_PERC_WAIT:
				break;

				// Détection de présence : retour vers le timing control
			case STATE_SCENARIO_3_ACTIVE_TIMING_CTRL:
				// Ecriture de mode "Timing Control"
				sendEpeverRSWrite(01, 0x10, CMD_LOAD_CONTROL, 0x0003);
				u08IndexStateSend = STATE_SCENARIO_3_ACTIVE_TIMING_CTRL_WAIT;
				break;

			case STATE_SCENARIO_3_ACTIVE_TIMING_CTRL_WAIT:
				break;

			case STATE_SCENARIO_3_LOAD_MANUAL_OFF:
				sendEpeverRSWrite(01, 0x10, DEFAULT_LOAD_IN_MANUAL, 0x0000);
				u08IndexStateSend = STATE_SCENARIO_3_LOAD_MANUAL_OFF_WAIT;
				break;

			case STATE_SCENARIO_3_LOAD_MANUAL_OFF_WAIT:
				break;

			// ------------------- FIN DES ENVOIS -------------------
			case STATE_END:
				break;

			default:
				break;
		}
	}

	if ((u08IndexStateSendAvant == u08IndexStateSend) && (u08IndexStateSend != STATE_END))
	{	// Dans ce cas, l'état n'a paq changé
		u16CounterToSendAgain++;
		if (u16CounterToSendAgain >= COUNTER_TO_SEND_AGAIN)
		{
			u08IndexStateSend--;
			u16CounterToSendAgain = 0;
		}
	}
	else
	{
		u16CounterToSendAgain = 0;
	}
}

void ManageReceiveData(void)
{
	int s16NumberOfCharacter = 0;
	char tabCharacter[150];
	bool bFlagDebugMessageReceive = false;

	switch(u08IndexStateSend)
	{
		case STATE_BATTERY_VOLTAGE:
			break;

		case STATE_BATTERY_VOLTAGE_WAIT:
			u16BatteryVoltageValue = (((unsigned int)u08TableDataReceive[3]) << 8) + ((unsigned int)u08TableDataReceive[4]);
			u08IndexStateSend = STATE_BATTERY_CURRENT;
			if (u16BatteryVoltageValue > u16BatteryMaximumVoltageValue)
			{
				u16BatteryMaximumVoltageValue = u16BatteryVoltageValue;
			}
			break;

		case STATE_BATTERY_CURRENT:
			break;

		case STATE_BATTERY_CURRENT_WAIT:
			s32BatteryCurrentValue = (((signed long)u08TableDataReceive[3]) << 8) + (signed long)(u08TableDataReceive[4])
									+(((signed long)u08TableDataReceive[5]) << 24) + (((signed long)u08TableDataReceive[6]) << 16);

			u08IndexStateSend = STATE_PV_VOLTAGE;
			break;

		case STATE_PV_VOLTAGE:
			break;

		case STATE_PV_VOLTAGE_WAIT:
			u16PvVoltageValue = (((unsigned int)u08TableDataReceive[3]) << 8) + ((unsigned int)u08TableDataReceive[4]);
			u08IndexStateSend = STATE_GENERATED_ENERGY;
			if (u16PvVoltageValue > u16PvMaximumVoltageValue)
			{
				u16PvMaximumVoltageValue = u16PvVoltageValue;
			}
			break;

		case STATE_GENERATED_ENERGY:
			break;

		case STATE_GENERATED_ENERGY_WAIT:
			u32GeneratedEnergyValue = (((unsigned long)u08TableDataReceive[3]) << 8) + (unsigned long)(u08TableDataReceive[4])
									+ (((unsigned long)u08TableDataReceive[5]) << 24) + (((unsigned long)u08TableDataReceive[6]) << 16);
			u08IndexStateSend = STATE_BATTERY_SOC;
			break;

		case STATE_BATTERY_SOC:
			break;

		case STATE_BATTERY_SOC_WAIT:
			u16BatterySOCValue = (((unsigned int)u08TableDataReceive[3]) << 8) + ((unsigned int)u08TableDataReceive[4]);
			u08IndexStateSend = STATE_BATTERY_STATUS;
			break;

		case STATE_BATTERY_STATUS:
			break;

		case STATE_BATTERY_STATUS_WAIT:
			u16BatteryStatusValue = (((unsigned int)u08TableDataReceive[3]) << 8) + ((unsigned int)u08TableDataReceive[4]);
			u08IndexStateSend = STATE_PV_CURRENT;
			break;

		case STATE_PV_CURRENT:
			break;

		case STATE_PV_CURRENT_WAIT:
			u16PvCurrentValue = (((unsigned int)u08TableDataReceive[3]) << 8) + ((unsigned int)u08TableDataReceive[4]);
			u08IndexStateSend = STATE_LOAD_CURRENT;
			break;

		case STATE_LOAD_CURRENT:
			break;

		case STATE_LOAD_CURRENT_WAIT:
			u16LoadCurrentValue = (((unsigned int)u08TableDataReceive[3]) << 8) + ((unsigned int)u08TableDataReceive[4]);
			u08IndexStateSend = STATE_LOAD_VOLTAGE;
			break;

		case STATE_LOAD_VOLTAGE:
			break;

		case STATE_LOAD_VOLTAGE_WAIT:
			u16LoadVoltageValue = (((unsigned int)u08TableDataReceive[3]) << 8) + ((unsigned int)u08TableDataReceive[4]);
			u08IndexStateSend = STATE_CONSUMED_ENERGY_TODAY;
			if (u16LoadVoltageValue > u16LoadMaximumVoltageValue)
			{
				u16LoadMaximumVoltageValue = u16LoadVoltageValue;
			}
			break;

		case STATE_CONSUMED_ENERGY_TODAY:
			break;

		case STATE_CONSUMED_ENERGY_TODAY_WAIT:
			u32ConsumedEnergyValue = (((unsigned long)u08TableDataReceive[3]) << 8) + (unsigned long)(u08TableDataReceive[4])
									+(((unsigned long)u08TableDataReceive[5]) << 24) + (((unsigned long)u08TableDataReceive[6]) << 16);
			u08IndexStateSend = STATE_REAL_TIME_CLOCK;
			break;

		case STATE_REAL_TIME_CLOCK:
			break;

		case STATE_REAL_TIME_CLOCK_WAIT:
			// On récupère les secondes / minutes / heures / jour
			u64RealTimeHourValue = 	(((unsigned long long)u08TableDataReceive[3]) << 8) + (unsigned long long)(u08TableDataReceive[4])
								   +(((unsigned long long)u08TableDataReceive[5]) << 24) + (((unsigned long long)u08TableDataReceive[6]) << 16)
								   +(((unsigned long long)u08TableDataReceive[7]) << 40) + (((unsigned long long)u08TableDataReceive[8]) << 32);

			//bFlagDebugMessageReceive = true;
			bFlagUpdateBLEValue = true;
			u08IndexStateSend = STATE_END;
			break;

		// ------------------- Ecriture dans le Real Time CLock : réceptions des commandes -------------------
		case STATE_REAL_TIME_MIN_WRITE:
			break;

		case STATE_REAL_TIME_MIN_WRITE_WAIT:
			//u08IndexStateSend = STATE_REAL_TIME_HOUR_WRITE;
			u08IndexStateSend = STATE_END;
			break;

		case STATE_REAL_TIME_HOUR_WRITE:
			break;

		case STATE_REAL_TIME_HOUR_WRITE_WAIT:
			u08IndexStateSend = STATE_END;
			break;

		// ------------------- Scénario 0 : réceptions des commandes -------------------
		case STATE_SCENARIO_0_MANUEL:
			break;

		case STATE_SCENARIO_0_MANUEL_WAIT:
			u08IndexStateSend = STATE_SCENARIO_0_FORCE;
			break;

		case STATE_SCENARIO_0_FORCE:
			break;

		case STATE_SCENARIO_0_FORCE_WAIT:
			u08IndexStateSend = STATE_SCENARIO_0_RESET_PERC;
			break;

		case STATE_SCENARIO_0_RESET_PERC:
			break;

		case STATE_SCENARIO_0_RESET_PERC_WAIT:
			u08IndexStateSend = STATE_END;
			break;

		// ------------------- Scénario 1 : réceptions des commandes -------------------
		case STATE_SCENARIO_1_LOAD_CONTROL:
			break;

		case STATE_SCENARIO_1_LOAD_CONTROL_WAIT:
			u08IndexStateSend = STATE_SCENARIO_1_NB_PERIODE;
			break;

		case STATE_SCENARIO_1_NB_PERIODE:
			break;

		case STATE_SCENARIO_1_NB_PERIODE_WAIT:
			u08IndexStateSend = STATE_SCENARIO_1_LED_RATE;
			break;

		case STATE_SCENARIO_1_LED_RATE:
			break;

		case STATE_SCENARIO_1_LED_RATE_WAIT:
			u08IndexStateSend = STATE_SCENARIO_1_TIMER_1;
			break;

		case STATE_SCENARIO_1_TIMER_1:
			break;

		case STATE_SCENARIO_1_TIMER_1_WAIT:
			u08IndexStateSend = STATE_SCENARIO_1_TIMER_2;
			break;

		case STATE_SCENARIO_1_TIMER_2:
			break;

		case STATE_SCENARIO_1_TIMER_2_WAIT:
			u08IndexStateSend = STATE_SCENARIO_1_TIMER_3;
			break;

		case STATE_SCENARIO_1_TIMER_3:
			break;

		case STATE_SCENARIO_1_TIMER_3_WAIT:
			u08IndexStateSend = STATE_SCENARIO_1_RATED_CURRENT_1;
			break;

		case STATE_SCENARIO_1_RATED_CURRENT_1:
			break;

		case STATE_SCENARIO_1_RATED_CURRENT_1_WAIT:
			u08IndexStateSend = STATE_SCENARIO_1_RATED_CURRENT_2;
			break;

		case STATE_SCENARIO_1_RATED_CURRENT_2:
			break;

		case STATE_SCENARIO_1_RATED_CURRENT_2_WAIT:
			u08IndexStateSend = STATE_SCENARIO_1_RATED_CURRENT_3;
			break;

		case STATE_SCENARIO_1_RATED_CURRENT_3:
			break;

		case STATE_SCENARIO_1_RATED_CURRENT_3_WAIT:
			u08IndexStateSend = STATE_END;
			break;

		// ------------------- Scénario 2 : réceptions des commandes -------------------
		case STATE_SCENARIO_2_LOAD_CONTROL:
			break;

		case STATE_SCENARIO_2_LOAD_CONTROL_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_NB_TIMING_CONTROL;
			break;

		case STATE_SCENARIO_2_NB_TIMING_CONTROL:
			break;

		case STATE_SCENARIO_2_NB_TIMING_CONTROL_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_ON_1;
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_ON_1:
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_ON_1_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_OFF_1;
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_OFF_1:
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_OFF_1_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_ON_2;
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_ON_2:
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_ON_2_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_OFF_2;
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_OFF_2:
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_OFF_2_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_ON_3;
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_ON_3:
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_ON_3_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_OFF_3;
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_OFF_3:
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_OFF_3_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_ON_4;
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_ON_4:
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_ON_4_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_OFF_4;
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_OFF_4:
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_OFF_4_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_LED_RATE_CURRENT;
			break;

		case STATE_SCENARIO_2_LED_RATE_CURRENT:
			break;

		case STATE_SCENARIO_2_LED_RATE_CURRENT_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_1_PERC;
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_1_PERC:
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_1_PERC_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_2_PERC;
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_2_PERC:
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_2_PERC_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_3_PERC;
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_3_PERC:
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_3_PERC_WAIT:
			u08IndexStateSend = STATE_SCENARIO_2_TIMING_CONTROL_4_PERC;
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_4_PERC:
			break;

		case STATE_SCENARIO_2_TIMING_CONTROL_4_PERC_WAIT:
			u08IndexStateSend = STATE_END;
			break;


		// ------------------- Scénario 3 : réception des commandes -------------------
		case STATE_SCENARIO_3_MANUEL_ON:
			break;

		case STATE_SCENARIO_3_MANUEL_ON_WAIT:
			u08IndexStateSend = STATE_SCENARIO_3_FORCE_PERC;
			break;

		case STATE_SCENARIO_3_FORCE_PERC:
			break;

		case STATE_SCENARIO_3_FORCE_PERC_WAIT:
			u08IndexStateSend = STATE_SCENARIO_3_LOAD_MANUAL_ON;
			break;

		case STATE_SCENARIO_3_LOAD_MANUAL_ON:
			break;

		case STATE_SCENARIO_3_LOAD_MANUAL_ON_WAIT:
			u08IndexStateSend = STATE_END;
			break;

		case STATE_SCENARIO_3_MANUEL_OFF:
			break;

		case STATE_SCENARIO_3_MANUEL_OFF_WAIT:
			u08IndexStateSend = STATE_SCENARIO_3_RESET_PERC;
			break;

		case STATE_SCENARIO_3_RESET_PERC:
			break;

		case STATE_SCENARIO_3_RESET_PERC_WAIT:
			u08IndexStateSend = STATE_END;
			break;

		case STATE_SCENARIO_3_ACTIVE_TIMING_CTRL:
			break;

		case STATE_SCENARIO_3_ACTIVE_TIMING_CTRL_WAIT:
			u08IndexStateSend = STATE_SCENARIO_3_LOAD_MANUAL_OFF;
			break;

		case STATE_SCENARIO_3_LOAD_MANUAL_OFF:
			break;

		case STATE_SCENARIO_3_LOAD_MANUAL_OFF_WAIT:
			u08IndexStateSend = STATE_END;
			break;


		// ------------------- FIN DES RECEPTIONS -------------------
		case STATE_END:
			break;

		default:
			break;
	}

	if (bFlagDebugMessageReceive == true)
	{
		s16NumberOfCharacter = sprintf(tabCharacter, "\r\nRecv : %d %lx %d %d %lx %d %d %d %d %d %lx\r\n", u16BatteryVoltageValue, s32BatteryCurrentValue, (s32BatteryCurrentValue > 0) ? 1:0 , u16PvVoltageValue, u32GeneratedEnergyValue, u16BatterySOCValue,
																u16BatteryStatusValue, u16PvCurrentValue, u16LoadCurrentValue, u16LoadVoltageValue, u32ConsumedEnergyValue);
		nrf_drv_uart_tx(&m_uart_EpeverSerie, (uint8_t *)&tabCharacter[0], (uint8_t)s16NumberOfCharacter);
	}
}

void sendEpeverRS(unsigned char deviceID, unsigned char functionCode, adresseEpeverRegister startAdress, unsigned int numberAdress)
{
	unsigned char index = 0;

	u08TableauSend[0] = deviceID;
	u08TableauSend[1] = functionCode;
	u08TableauSend[2] = ((startAdress >> 8) & 0xFF);
	u08TableauSend[3] = (startAdress & 0xFF);
	u08TableauSend[4] = ((numberAdress >> 8) & 0xFF);
	u08TableauSend[5] = (numberAdress & 0xFF);

	initCRC();
	for (index = 0; index < 6; index++)
	{
		computerCRC(u08TableauSend[index]);
	}
	u08TableauSend[6] = (unsigned char)(readCRC() & 0xFF);
	u08TableauSend[7] = (unsigned char)((readCRC() >> 8) & 0xFF);

	activateTransmit_rs485();
	/*for (index=0; index < 8; index++)
	{
		printf("%c", u08TableauSend[index]);
	}*/
	nrf_drv_uart_tx(&m_uart_EpeverSerie, (uint8_t *)&u08TableauSend[0], 8);
	//activateReceive_rs485();

	u08NumberOfDataReceive = 0;
	u08NumberMaxReceiveData = NUMBER_MAX_OF_RECEIVE_DATA;
	memset(u08TableDataReceive, 0, NUMBER_MAX_OF_RECEIVE_DATA);

	// Envoyer sur la sortie le résultat
	/*
	unsigned char u08NumberOfCharacter = 0;
	unsigned char tabCharacter[50];
	u08NumberOfCharacter = sprintf(tabCharacter, "Send : %x %x %x %x %x %x %02x %02x \r\n", tableau[0], tableau[1], tableau[2], tableau[3], tableau[4], tableau[5], tableau[6], tableau[7]);
	nrf_drv_uart_tx(&m_uart_EpeverSerie, (uint8_t *)&tabCharacter[0], u08NumberOfCharacter);
	*/
}

void sendEpeverRSCommand(unsigned char deviceID, unsigned char functionCode, adresseEpeverRegister startAdress, unsigned int u16commande)
{
	unsigned char index = 0;

	u08TableauSend[0] = deviceID;
	u08TableauSend[1] = functionCode;
	u08TableauSend[2] = ((startAdress >> 8) & 0xFF);
	u08TableauSend[3] = (startAdress & 0xFF);
	u08TableauSend[4] = ((u16commande >> 8) & 0xFF);
	u08TableauSend[5] = (u16commande & 0xFF);

	initCRC();
	for (index = 0; index < 6; index++)
	{
		computerCRC(u08TableauSend[index]);
	}
	u08TableauSend[6] = (unsigned char)(readCRC() & 0xFF);
	u08TableauSend[7] = (unsigned char)((readCRC() >> 8) & 0xFF);

	activateTransmit_rs485();
	/*for (index=0; index < 8; index++)
	{
		printf("%c", u08TableauSend[index]);
	}*/
	nrf_drv_uart_tx(&m_uart_EpeverSerie, (uint8_t *)&u08TableauSend[0], 8);
	//activateReceive_rs485();

	u08NumberOfDataReceive = 0;
	u08NumberMaxReceiveData = NUMBER_MAX_OF_RECEIVE_DATA;
	memset(u08TableDataReceive, 0, NUMBER_MAX_OF_RECEIVE_DATA);

	// Envoyer sur la sortie le résultat
	/*
	unsigned char u08NumberOfCharacter = 0;
	unsigned char tabCharacter[50];
	u08NumberOfCharacter = sprintf(tabCharacter, "Send : %x %x %x %x %x %x %02x %02x \r\n", tableau[0], tableau[1], tableau[2], tableau[3], tableau[4], tableau[5], tableau[6], tableau[7]);
	nrf_drv_uart_tx(&m_uart_EpeverSerie, (uint8_t *)&tabCharacter[0], u08NumberOfCharacter);
	*/
}

void sendEpeverRSWrite(unsigned char deviceID, unsigned char functionCode, adresseEpeverRegister startAdress, unsigned int u16data)
{
	unsigned char index = 0;

	u08TableauSend[0] = deviceID;
	u08TableauSend[1] = functionCode;
	u08TableauSend[2] = ((startAdress >> 8) & 0xFF);
	u08TableauSend[3] = (startAdress & 0xFF);
	u08TableauSend[4] = 0x00;	//
	u08TableauSend[5] = 0x01;	// The number of adress
	u08TableauSend[6] = 0x02;	// Number of bytes
	u08TableauSend[7] = ((u16data >> 8) & 0xFF);
	u08TableauSend[8] = (u16data & 0xFF);

	initCRC();
	for (index = 0; index < 9; index++)
	{
		computerCRC(u08TableauSend[index]);
	}
	u08TableauSend[9] = (unsigned char)(readCRC() & 0xFF);
	u08TableauSend[10] = (unsigned char)((readCRC() >> 8) & 0xFF);

	activateTransmit_rs485();
	/*for (index=0; index < 11; index++)
	{
		printf("%c", u08TableauSend[index]);
	}*/
	nrf_drv_uart_tx(&m_uart_EpeverSerie, (uint8_t *)&u08TableauSend[0], 11);
	//activateReceive_rs485();

	u08NumberOfDataReceive = 0;
	u08NumberMaxReceiveData = NUMBER_MAX_OF_RECEIVE_DATA;
	memset(u08TableDataReceive, 0, NUMBER_MAX_OF_RECEIVE_DATA);

	// Envoyer sur la sortie le résultat
	/*
	unsigned char u08NumberOfCharacter = 0;
	unsigned char tabCharacter[50];
	u08NumberOfCharacter = sprintf(tabCharacter, "Send : %x %x %x %x %x %x %02x %02x \r\n", tableau[0], tableau[1], tableau[2], tableau[3], tableau[4], tableau[5], tableau[6], tableau[7]);
	nrf_drv_uart_tx(&m_uart_EpeverSerie, (uint8_t *)&tabCharacter[0], u08NumberOfCharacter);
	*/
}

void sendEpeverRSWriteHour(unsigned char deviceID, unsigned char functionCode, adresseEpeverRegister startAdress, unsigned long long u64data)
{
	unsigned char index = 0;

	u08TableauSend[0] = deviceID;
	u08TableauSend[1] = functionCode;
	u08TableauSend[2] = ((startAdress >> 8) & 0xFF);
	u08TableauSend[3] = (startAdress & 0xFF);
	u08TableauSend[4] = 0x00;	//
	u08TableauSend[5] = 0x03;	// The number of adress
	u08TableauSend[6] = 0x06;	// Number of bytes

	u08TableauSend[7] = ((u64data >> 8) & 0xFF);
	u08TableauSend[8] = (u64data & 0xFF);

	u08TableauSend[9] = ((u64data >> 24) & 0xFF);
	u08TableauSend[10] = ((u64data >> 16) & 0xFF);

	u08TableauSend[11] = ((u64data >> 40) & 0xFF);
	u08TableauSend[12] = ((u64data >> 32) & 0xFF);

	initCRC();
	for (index = 0; index < 13; index++)
	{
		computerCRC(u08TableauSend[index]);
	}
	u08TableauSend[13] = (unsigned char)(readCRC() & 0xFF);
	u08TableauSend[14] = (unsigned char)((readCRC() >> 8) & 0xFF);

	activateTransmit_rs485();
	/*for (index=0; index < 11; index++)
	{
		printf("%c", u08TableauSend[index]);
	}*/
	nrf_drv_uart_tx(&m_uart_EpeverSerie, (uint8_t *)&u08TableauSend[0], 15);
	//activateReceive_rs485();

	u08NumberOfDataReceive = 0;
	u08NumberMaxReceiveData = NUMBER_MAX_OF_RECEIVE_DATA;
	memset(u08TableDataReceive, 0, NUMBER_MAX_OF_RECEIVE_DATA);

	// Envoyer sur la sortie le résultat
	/*
	unsigned char u08NumberOfCharacter = 0;
	unsigned char tabCharacter[50];
	u08NumberOfCharacter = sprintf(tabCharacter, "Send : %x %x %x %x %x %x %02x %02x \r\n", tableau[0], tableau[1], tableau[2], tableau[3], tableau[4], tableau[5], tableau[6], tableau[7]);
	nrf_drv_uart_tx(&m_uart_EpeverSerie, (uint8_t *)&tabCharacter[0], u08NumberOfCharacter);
	*/
}

void sendEpeverRSWriteTimingControl(unsigned char deviceID, unsigned char functionCode, adresseEpeverRegister startAdress, unsigned int u16Hour)
{
	unsigned char index = 0;

	u08TableauSend[0] = deviceID;
	u08TableauSend[1] = functionCode;
	u08TableauSend[2] = ((startAdress >> 8) & 0xFF);
	u08TableauSend[3] = (startAdress & 0xFF);
	u08TableauSend[4] = 0x00;	//
	u08TableauSend[5] = 0x03;	// The number of adress
	u08TableauSend[6] = 0x06;	// Number of bytes

	u08TableauSend[7] = 0;		// Secondes
	u08TableauSend[8] = 0;		// Secondes

	u08TableauSend[9] = 0;		// Minutes
	u08TableauSend[10] = (u16Hour & 0x00FF);	// Minutes

	u08TableauSend[11] = 0;		// Heures
	u08TableauSend[12] = ((u16Hour & 0xFF00) >> 8);	// Heure

	initCRC();
	for (index = 0; index < 13; index++)
	{
		computerCRC(u08TableauSend[index]);
	}
	u08TableauSend[13] = (unsigned char)(readCRC() & 0xFF);
	u08TableauSend[14] = (unsigned char)((readCRC() >> 8) & 0xFF);

	activateTransmit_rs485();
	/*for (index=0; index < 11; index++)
	{
		printf("%c", u08TableauSend[index]);
	}*/
	nrf_drv_uart_tx(&m_uart_EpeverSerie, (uint8_t *)&u08TableauSend[0], 15);
	//activateReceive_rs485();

	u08NumberOfDataReceive = 0;
	u08NumberMaxReceiveData = NUMBER_MAX_OF_RECEIVE_DATA;
	memset(u08TableDataReceive, 0, NUMBER_MAX_OF_RECEIVE_DATA);

	// Envoyer sur la sortie le résultat
	/*
	unsigned char u08NumberOfCharacter = 0;
	unsigned char tabCharacter[50];
	u08NumberOfCharacter = sprintf(tabCharacter, "Send : %x %x %x %x %x %x %02x %02x \r\n", tableau[0], tableau[1], tableau[2], tableau[3], tableau[4], tableau[5], tableau[6], tableau[7]);
	nrf_drv_uart_tx(&m_uart_EpeverSerie, (uint8_t *)&tabCharacter[0], u08NumberOfCharacter);
	*/
}

void initCRC()
{
	crc16Modbus = 0xFFFF;
}


void computerCRC(unsigned char byteToCompute)
{
	unsigned char i = 0;

	crc16Modbus ^= (unsigned char)byteToCompute;	// XOR byte into least sig. byte of crc

	for (i = 8; i != 0; i--)
	{	// Loop over each bit
		if ((crc16Modbus & 0x0001) != 0)
		{	// If the LSB is set
			crc16Modbus >>= 1;	// Shift right and XOR 0xA001
			crc16Modbus ^= 0xA001;
		}
		else
		{
			crc16Modbus >>=1;		// Just shift right
		}
	}
}

unsigned int readCRC(void)
{
	return crc16Modbus;
}

/**@brief Function for activate the transmit into the RS485 communication.
 */
void activateTransmit_rs485(void)
{
	// Signal DE à 1
	nrf_gpio_pin_set(24);

	// Signal /RE à 1
	nrf_gpio_pin_set(23);
}

/**@brief Function for activate the reception into the RS485 communication.
 */
void activateReceive_rs485(void)
{
	// Signal DE à 0
	nrf_gpio_pin_clear(24);

	// Signal /RE à 0
	nrf_gpio_pin_clear(23);
}

void receiveData(unsigned char caractereReceive)
{
	u08TableDataReceive[u08NumberOfDataReceive] = caractereReceive;

	if (u08NumberOfDataReceive == 2)
	{
		if (u08TableDataReceive[1] == 5)
		{	// The function code is a write command, the number of byte is always the same : 8 bytes
			u08NumberMaxReceiveData = 8;
		}
		else if (u08TableDataReceive[1] == 0x10)
		{	// The function code is a write data, the number of byte is always the same : 8 bytes
			u08NumberMaxReceiveData = 8;
		}
		else
		{
			// The data is the number of receive byte
			// Add this number of byte + 1 byte for deviceID + 1 byte for function code + 2 bytes of CRC + 1 bytes for this (number of byte)
			u08NumberMaxReceiveData = u08TableDataReceive[u08NumberOfDataReceive] + 1 + 1 + 2 + 1;
		}
	}

	u08NumberOfDataReceive++;
	if (u08NumberOfDataReceive >= u08NumberMaxReceiveData)
	{	// This is the end of the receive data
		ManageReceiveData();
	}
}

bool getUpdateBLEValue(void)
{
	if (bFlagUpdateBLEValue == true)
	{
		bFlagUpdateBLEValue = false;
		return true;
	}
	return bFlagUpdateBLEValue;
}

unsigned int getBatteryVoltageValue(void)
{
	return u16BatteryVoltageValue;
}

unsigned int getBatteryMaximumVoltageValue(void)
{
	return u16BatteryMaximumVoltageValue;
}

signed long getBatteryCurrentValue(void)
{
	return s32BatteryCurrentValue;
}

unsigned int getPvVoltageValue(void)
{
	return u16PvVoltageValue;
}

unsigned int getPvMaximumVoltageValue(void)
{
	return u16PvMaximumVoltageValue;
}

void setPvVoltageMaxValue(unsigned int u16Value)	// TODO : à supprimer
{
	u16PvMaximumVoltageValue = u16Value;
}

unsigned long getGeneratedEnergyValue(void)
{
	return u32GeneratedEnergyValue;
}

void setGeneratedEnergyValue(unsigned long u32Value)	// TODO : à supprimer
{
	u32GeneratedEnergyValue = u32Value;
}

unsigned int getBatterySOCValue(void)
{
	return u16BatterySOCValue;
}

void setSOCValue(unsigned long u32Value)	// TODO : à supprimer
{
	u16BatterySOCValue = u32Value;
}

unsigned int getBatteryStatusValue(void)
{
	return  u16BatteryStatusValue;
}

unsigned int getPvCurrentValue(void)
{
	return  u16PvCurrentValue;
}

unsigned int getLoadCurrentValue(void)
{
	return u16LoadCurrentValue;
}

unsigned int getLoadVoltageValue(void)
{
	return u16LoadVoltageValue;
}

void initMaximumVoltageValue()
{
	uint8_t maxLoadVoltage_char[4];
	unsigned int u16Value = 0;

	// Initialisation du maximumLoadVoltage avec la mémoire
	memcpy(maxLoadVoltage_char, (uint32_t *)0x1000108C, 4);
	u16Value = (((uint16_t)maxLoadVoltage_char[1]) << 8) + ((uint16_t)maxLoadVoltage_char[0]);

	// Tension maximale du load : par exemple, 50V
	// nrfjprog.exe -f nrf52 --memwr 0x1000108C --val 0xFFFF1388

	if (u16Value == 0xFFFF)
	{
		printf("Max Load Volt vide : 5600\r\n");
		u16Value = 5600;
	}
	else
	{
		if (u16Value > 5600)
		{
			u16Value = 5600;
		}
		printf("Max Load Volt = %d\r\n", u16Value);
	}
	nrf_delay_ms(50);	// Attente pour printf

	u16LoadMaximumVoltageValue = u16Value;
}

void setLoadMaximumVoltageValue(unsigned int u16Value)
{
	u16LoadMaximumVoltageValue = u16Value;
}

unsigned int getLoadMaximumVoltageValue(void)
{
	return u16LoadMaximumVoltageValue;
}

unsigned long getConsumedEnergyValue(void)
{
	return  u32ConsumedEnergyValue;
}

unsigned long long getRealTimeHourValue(void)
{
	// u64RealTimeHourValue : Année / Mois / JOUR / Heure / Minutes / Secondes
	return u64RealTimeHourValue;
}

// Renvoi seulement l'Heure et la minute
unsigned int getRealTimeHourValueInteger(void)
{
	// u64RealTimeHourValue : Année / Mois / JOUR / Heure / Minutes / Secondes
	unsigned long long u64Value = 0;

	u64Value = ((u64RealTimeHourValue & 0x0000000000FFFF00) >> 8);
	return (unsigned int)u64Value;
}


void setScenarioNumberEpever(unsigned char u08Value)
{
	u08ScenarioNumberEpever = u08Value;
}

unsigned char getScenarioNumberEpever(void)
{
	return u08ScenarioNumberEpever;
}

void setCapacityBatteryValue(unsigned int u16Value)
{
	u16CapacityBattery = u16Value;
}

unsigned int getCapacityBatteryValue(void)
{
	return u16CapacityBattery;
}

void setRealTimeClock(unsigned long long u64Value)
{
	u64RealTimeClock = u64Value;
}

unsigned long long getRealTimeClock(void)
{
	return u64RealTimeClock;
}

void initBatteryCapacity(unsigned int u16Value)
{
	sendEpeverRSWrite(01, 0x10, CMD_BATTERY_CAPACITY, u16Value);
}
