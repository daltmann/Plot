/*
 * Epever_Serie.h
 *
 *  Created on: 25 oct. 2017
 *      Author: Damien
 */

#ifndef APPLICATION_EPEVER_SERIE_H_
#define APPLICATION_EPEVER_SERIE_H_

#include <stdbool.h>

typedef enum
{
	BATTERY_VOLTAGE 			=	0x331A,
	BATTERY_CURRENT 			=	0x331B,
	PV_VOLTAGE 					=	0x3100,
	GENERATED_ENERGY 			= 	0x330C,
	BATTERY_SOC					= 	0x311A,
	BATTERY_STATUS				=	0x3200,
	PV_CURRENT					= 	0x3101,
	LOAD_CURRENT				=	0x310D,
	LOAD_VOLTAGE				= 	0x310C,
	CONSUMED_ENERGY_TODAY		=	0x3304,

	REAL_TIME_CLOCK				=	0x9013,
	DEFAULT_LOAD_IN_MANUAL		=	0x906A,
	TIMING_CONTROL_ON_1			=	0x9042,
	TIMING_CONTROL_OFF_1		=	0x9045,
	TIMING_CONTROL_ON_2			=	0x9048,
	TIMING_CONTROL_OFF_2		=	0x904B,
	TIMING_CONTROL_ON_3			=	0x904E,
	TIMING_CONTROL_OFF_3		=	0x9051,
	TIMING_CONTROL_ON_4			=	0x9054,
	TIMING_CONTROL_OFF_4		=	0x9057,
	TIMING_CONTROL_1_PERCENTAGE	=	0x9080,
	TIMING_CONTROL_2_PERCENTAGE	=	0x9081,
	TIMING_CONTROL_3_PERCENTAGE	=	0x911C,
	TIMING_CONTROL_4_PERCENTAGE	=	0x911D,

	// Scénario 0 et 3
	CMD_MANUEL_CONTROL			=	0x0002,
	CMD_LOAD_TEST				=	0x0005,
	CMD_FORCE_LOAD				=	0x0006,

	// Scénario 1
	CMD_NB_PERIODE			=	0x9069,
	CMD_LOAD_CONTROL		=	0x903D,
	CMD_LED_RATE			=	0x9078,
	CMD_LED_RATE_MANUEL_PERC=	0x9079,
	CMD_TIMER_1				=	0x907A,
	CMD_TIMER_2				=	0x907B,
	CMD_TIMER_3				=	0x907C,
	CMD_RATE_CURRENT_1		=	0x907D,
	CMD_RATE_CURRENT_2		=	0x907E,
	CMD_RATE_CURRENT_3		=	0x907F,

	// Battery capacity
	CMD_BATTERY_CAPACITY	=	0x9001,
} adresseEpeverRegister;

typedef enum
{
	STATE_BATTERY_VOLTAGE 					=	0,
	STATE_BATTERY_VOLTAGE_WAIT 				=	1,
	STATE_BATTERY_CURRENT 					=	2,
	STATE_BATTERY_CURRENT_WAIT				=	3,
	STATE_PV_VOLTAGE 						=	4,
	STATE_PV_VOLTAGE_WAIT					=	5,
	STATE_GENERATED_ENERGY		 			= 	6,
	STATE_GENERATED_ENERGY_WAIT				= 	7,
	STATE_BATTERY_SOC						= 	10,
	STATE_BATTERY_SOC_WAIT					= 	11,
	STATE_BATTERY_STATUS					=	12,
	STATE_BATTERY_STATUS_WAIT				=	13,
	STATE_PV_CURRENT						= 	14,
	STATE_PV_CURRENT_WAIT					= 	15,
	STATE_LOAD_CURRENT						=	16,
	STATE_LOAD_CURRENT_WAIT					=	17,
	STATE_LOAD_VOLTAGE						= 	18,
	STATE_LOAD_VOLTAGE_WAIT					= 	19,
	STATE_CONSUMED_ENERGY_TODAY				=	20,
	STATE_CONSUMED_ENERGY_TODAY_WAIT		=	21,
	STATE_REAL_TIME_CLOCK					=	22,
	STATE_REAL_TIME_CLOCK_WAIT				=	23,

	STATE_SCENARIO_0_MANUEL					= 	50,
	STATE_SCENARIO_0_MANUEL_WAIT			= 	51,
	STATE_SCENARIO_0_FORCE					= 	52,
	STATE_SCENARIO_0_FORCE_WAIT				= 	53,
	STATE_SCENARIO_0_RESET_PERC				=	54,
	STATE_SCENARIO_0_RESET_PERC_WAIT		=	55,


	STATE_SCENARIO_1_LOAD_CONTROL			= 	70,
	STATE_SCENARIO_1_LOAD_CONTROL_WAIT		= 	71,
	STATE_SCENARIO_1_NB_PERIODE				= 	72,
	STATE_SCENARIO_1_NB_PERIODE_WAIT		= 	73,
	STATE_SCENARIO_1_LED_RATE				= 	74,
	STATE_SCENARIO_1_LED_RATE_WAIT			= 	75,
	STATE_SCENARIO_1_TIMER_1				= 	76,
	STATE_SCENARIO_1_TIMER_1_WAIT			= 	77,
	STATE_SCENARIO_1_TIMER_2				= 	78,
	STATE_SCENARIO_1_TIMER_2_WAIT			= 	79,
	STATE_SCENARIO_1_TIMER_3				= 	80,
	STATE_SCENARIO_1_TIMER_3_WAIT			= 	81,
	STATE_SCENARIO_1_RATED_CURRENT_1		= 	82,
	STATE_SCENARIO_1_RATED_CURRENT_1_WAIT	= 	83,
	STATE_SCENARIO_1_RATED_CURRENT_2		= 	84,
	STATE_SCENARIO_1_RATED_CURRENT_2_WAIT	= 	85,
	STATE_SCENARIO_1_RATED_CURRENT_3		= 	86,
	STATE_SCENARIO_1_RATED_CURRENT_3_WAIT	= 	87,


	STATE_SCENARIO_2_LOAD_CONTROL				= 	120,
	STATE_SCENARIO_2_LOAD_CONTROL_WAIT			= 	121,
	STATE_SCENARIO_2_NB_TIMING_CONTROL			=	122,
	STATE_SCENARIO_2_NB_TIMING_CONTROL_WAIT		=	123,
	STATE_SCENARIO_2_TIMING_CONTROL_ON_1		= 	124,
	STATE_SCENARIO_2_TIMING_CONTROL_ON_1_WAIT	= 	125,
	STATE_SCENARIO_2_TIMING_CONTROL_OFF_1		= 	126,
	STATE_SCENARIO_2_TIMING_CONTROL_OFF_1_WAIT	= 	127,
	STATE_SCENARIO_2_TIMING_CONTROL_ON_2		= 	128,
	STATE_SCENARIO_2_TIMING_CONTROL_ON_2_WAIT	= 	129,
	STATE_SCENARIO_2_TIMING_CONTROL_OFF_2		= 	130,
	STATE_SCENARIO_2_TIMING_CONTROL_OFF_2_WAIT	= 	131,
	STATE_SCENARIO_2_TIMING_CONTROL_ON_3		= 	132,
	STATE_SCENARIO_2_TIMING_CONTROL_ON_3_WAIT	= 	133,
	STATE_SCENARIO_2_TIMING_CONTROL_OFF_3		= 	134,
	STATE_SCENARIO_2_TIMING_CONTROL_OFF_3_WAIT	= 	135,
	STATE_SCENARIO_2_TIMING_CONTROL_ON_4		= 	136,
	STATE_SCENARIO_2_TIMING_CONTROL_ON_4_WAIT	= 	137,
	STATE_SCENARIO_2_TIMING_CONTROL_OFF_4		= 	138,
	STATE_SCENARIO_2_TIMING_CONTROL_OFF_4_WAIT	= 	139,
	STATE_SCENARIO_2_LED_RATE_CURRENT			= 	140,
	STATE_SCENARIO_2_LED_RATE_CURRENT_WAIT		= 	141,
	STATE_SCENARIO_2_TIMING_CONTROL_1_PERC		= 	142,
	STATE_SCENARIO_2_TIMING_CONTROL_1_PERC_WAIT	= 	143,
	STATE_SCENARIO_2_TIMING_CONTROL_2_PERC		= 	144,
	STATE_SCENARIO_2_TIMING_CONTROL_2_PERC_WAIT	= 	145,
	STATE_SCENARIO_2_TIMING_CONTROL_3_PERC		= 	146,
	STATE_SCENARIO_2_TIMING_CONTROL_3_PERC_WAIT	= 	147,
	STATE_SCENARIO_2_TIMING_CONTROL_4_PERC		= 	148,
	STATE_SCENARIO_2_TIMING_CONTROL_4_PERC_WAIT	= 	149,


	STATE_SCENARIO_3_MANUEL_ON				= 	190,
	STATE_SCENARIO_3_MANUEL_ON_WAIT			= 	191,
	STATE_SCENARIO_3_FORCE_PERC				= 	192,
	STATE_SCENARIO_3_FORCE_PERC_WAIT		= 	193,
	STATE_SCENARIO_3_LOAD_MANUAL_ON			=	194,
	STATE_SCENARIO_3_LOAD_MANUAL_ON_WAIT	=	195,

	STATE_SCENARIO_3_MANUEL_OFF				=	200,
	STATE_SCENARIO_3_MANUEL_OFF_WAIT		=	201,
	STATE_SCENARIO_3_RESET_PERC				=	202,
	STATE_SCENARIO_3_RESET_PERC_WAIT		=	203,

	STATE_SCENARIO_3_ACTIVE_TIMING_CTRL		=	210,
	STATE_SCENARIO_3_ACTIVE_TIMING_CTRL_WAIT=	211,
	STATE_SCENARIO_3_LOAD_MANUAL_OFF		=	212,
	STATE_SCENARIO_3_LOAD_MANUAL_OFF_WAIT	=	213,

	STATE_REAL_TIME_MIN_WRITE				= 	240,
	STATE_REAL_TIME_MIN_WRITE_WAIT			= 	241,
	STATE_REAL_TIME_HOUR_WRITE				= 	242,
	STATE_REAL_TIME_HOUR_WRITE_WAIT			= 	243,

	STATE_END								=	250,
} stateSendProtocol;



void Manage_Epever_RS485(bool resetProtocol, bool bNewScenario, bool bArretLed, unsigned char u08StateOfScenario, bool bHour);

void sendEpeverRS(unsigned char deviceID, unsigned char functionCode, adresseEpeverRegister startAdress, unsigned int numberAdress);

void sendEpeverRSCommand(unsigned char deviceID, unsigned char functionCode, adresseEpeverRegister startAdress, unsigned int u16commande);

void sendEpeverRSWrite(unsigned char deviceID, unsigned char functionCode, adresseEpeverRegister startAdress, unsigned int u16data);

void sendEpeverRSWriteHour(unsigned char deviceID, unsigned char functionCode, adresseEpeverRegister startAdress, unsigned long long u64data);

void sendEpeverRSWriteTimingControl(unsigned char deviceID, unsigned char functionCode, adresseEpeverRegister startAdress, unsigned int u16Hour);

void initCRC();

void computerCRC(unsigned char byteToCompute);

unsigned int readCRC(void);

void activateTransmit_rs485(void);

void activateReceive_rs485(void);

void receiveData(unsigned char caractereReceive);

void ManageReceiveData(void);

bool getUpdateBLEValue(void);

unsigned int getBatteryVoltageValue(void);
unsigned int getBatteryMaximumVoltageValue(void);

signed long getBatteryCurrentValue(void);

unsigned int getPvVoltageValue(void);

unsigned int getPvMaximumVoltageValue(void);
void setPvVoltageMaxValue(unsigned int u16Value);

unsigned long getGeneratedEnergyValue(void);
void setGeneratedEnergyValue(unsigned long u32Value);

unsigned int getBatterySOCValue(void);
void setSOCValue(unsigned long u32Value);

unsigned int getBatteryStatusValue(void);

unsigned int getPvCurrentValue(void);

unsigned int getLoadCurrentValue(void);

unsigned int getLoadVoltageValue(void);
void initMaximumVoltageValue();
void setLoadMaximumVoltageValue(unsigned int u16Value);
unsigned int getLoadMaximumVoltageValue(void);

unsigned long getConsumedEnergyValue(void);

unsigned long long getRealTimeHourValue(void);
unsigned int getRealTimeHourValueInteger(void);

void setScenarioNumberEpever(unsigned char u08Value);
unsigned char getScenarioNumberEpever(void);

void setCapacityBatteryValue(unsigned int u16Value);
unsigned int getCapacityBatteryValue(void);

void setRealTimeClock(unsigned long long u64Value);
unsigned long long getRealTimeClock(void);

void initBatteryCapacity(unsigned int u16Value);

#endif /* APPLICATION_EPEVER_SERIE_H_ */
