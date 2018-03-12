/*
 * Compute.c
 *
 *  Created on: 02 novembre 2017
 *      Author: Damien
 */

#include "Compute.h"

#include "app_uart.h"
#include <stdio.h>
#include "nrf_drv_uart.h"
#include "nrf_gpio.h"

#include "stdbool.h"
#include "string.h"

#include "Epever_Serie.h"
#include "ble_Scenario.h"

//#include "nrf_log.h"
#include "nrf_delay.h"

#include "Scenario.h"


#define OPTIMISATION_PUISSANCE_ECLAIRAGE    TRUE    // Optimisation de la puissance d'éclairage avec minimum d'éclairage objectif

#ifdef PLOT_BOARD
#define MAXIMUM_BATTERY_CAPACITY	1500
#endif

#ifdef SPLIT_BOARD
#define MAXIMUM_BATTERY_CAPACITY	10000
#endif

#define NB_DAY_MAXIMUM	5
//unsigned long u32TableOfEnergy[NB_DAY_MAXIMUM];

unsigned int u16TableLEDCurrent[NB_DAY_MAXIMUM];

unsigned int Battery_voltage = 1280;	// Equivalent à 12.8V
unsigned long coeff_conversion_battery_capacity = MAXIMUM_BATTERY_CAPACITY;
unsigned int AvgCurrentmA = 0;

unsigned int Received_Solar_Pw0 = 0;
unsigned int Received_Solar_Pw1 = 0;
unsigned int Received_Solar_Pw2 = 0;
unsigned int Received_Solar_Pw3 = 0;
unsigned int Received_Solar_Pw4 = 0;

unsigned char Scen_Delay = 6;	// V1.1 : remplacement de 0 par 6

#ifdef OPTIMISATION_PUISSANCE_ECLAIRAGE
#define POWER_MINIMAL_OBJECTIF  (unsigned int)900   // puissance minimal objectif, en mAh 
#define SEUIL_ACTIVATION        (unsigned int)70      // Seuil d'activation : 70% de la capacité batterie
#endif

unsigned int LEDmAValue = 0;	// Valeur de la PDU

unsigned int u16Duree_Eclairage_Calculee = 6;   // Dc : Durée d'éclairage calculée, initialisée à 6 heures

unsigned char u08ScenarioNumberComputer = 0;	// Numéro du scénario dans Computer

bool bInitFindLEDmALevel = false;			// Juste pour initialiser les variables la premières fois

unsigned int initCapacityBattery()
{
	uint8_t batteryCapa_char[4];
	uint16_t u16BatteryCapacityValue = 0;

	memcpy(batteryCapa_char, (uint32_t *)0x10001084, 4);
	u16BatteryCapacityValue = (((uint16_t)batteryCapa_char[1]) << 8) + ((uint16_t)batteryCapa_char[0]);

	// Capacité de la batterie : par exemple, 10 Ah
	// nrfjprog.exe -f nrf52 --memwr 0x10001084 --val 0xFFFF2710

	if (u16BatteryCapacityValue == 0xFFFF)
	{
		printf("Capa mem vide : 10Ah\r\n");
		u16BatteryCapacityValue = MAXIMUM_BATTERY_CAPACITY;
	}
	else
	{
		printf("Capa Battery = %d\r\n", u16BatteryCapacityValue);
	}
	nrf_delay_ms(50);	// Attente pour printf

	// Envoi la valeur vers la variable interne
	setCapacityBatteryValue(u16BatteryCapacityValue);

	// Send the new value to the Epever
	initBatteryCapacity(u16BatteryCapacityValue / 1000);

	return u16BatteryCapacityValue;
}

unsigned int initCapacityBatteryPlot()
{
	uint8_t batteryCapa_char[4];
	uint16_t u16BatteryCapacityValue = 0;

	memcpy(batteryCapa_char, (uint32_t *)0x10001084, 4);
	u16BatteryCapacityValue = (((uint16_t)batteryCapa_char[1]) << 8) + ((uint16_t)batteryCapa_char[0]);

	// Capacité de la batterie : par exemple, 10 Ah
	// nrfjprog.exe -f nrf52 --memwr 0x10001084 --val 0xFFFF2710

	if (u16BatteryCapacityValue == 0xFFFF)
	{
//		printf("Capa mem vide : %d mAh\r\n", MAXIMUM_BATTERY_CAPACITY);
		u16BatteryCapacityValue = MAXIMUM_BATTERY_CAPACITY;
	}
	else
	{
//		printf("Capa Battery = %d\r\n", u16BatteryCapacityValue);
	}
	nrf_delay_ms(50);	// Attente pour printf

	return u16BatteryCapacityValue;
}

void initGeneratedEnergy(void)
{
	unsigned char u08Index = 0;

/*	for (u08Index = 0; u08Index < NB_DAY_MAXIMUM; u08Index++)
	{
		u32TableOfEnergy[u08Index] = (800 * (u08Index + 1));
	}*/

	AvgCurrentmA = 0;

	// Initialisation des valeurs de charges solaire des 5 derniers jours
	Received_Solar_Pw0 = (((unsigned int)coeff_conversion_battery_capacity * 12) / 100);
	Received_Solar_Pw1 = (((unsigned int)coeff_conversion_battery_capacity * 12) / 100);
	Received_Solar_Pw2 = (((unsigned int)coeff_conversion_battery_capacity * 12) / 100);
	Received_Solar_Pw3 = (((unsigned int)coeff_conversion_battery_capacity * 12) / 100);
	Received_Solar_Pw4 = (((unsigned int)coeff_conversion_battery_capacity * 12) / 100);

	// Mise à 300 mA de tous les courants d'avant
	for (u08Index = 0; u08Index < NB_DAY_MAXIMUM; u08Index++)
	{
		u16TableLEDCurrent[u08Index] = ((((unsigned int)coeff_conversion_battery_capacity * 12) / 100) / 6);
	}
}

/*void saveGeneratedEnergy(unsigned long u32ValueEnergy)
{
	unsigned char u08Index = 0;

	NRF_LOG_INFO("saveGeneratedEnergy():");

	NRF_LOG_INFO("Add new Energy avant rotation : %d : %d : %d : %d : %d", u32TableOfEnergy[0], u32TableOfEnergy[1], u32TableOfEnergy[2], u32TableOfEnergy[3], u32TableOfEnergy[4]);

	for (u08Index = (NB_DAY_MAXIMUM - 1); u08Index > 0; u08Index--)
	{
		u32TableOfEnergy[u08Index] = u32TableOfEnergy[u08Index - 1];
	}

	u32TableOfEnergy[0] = u32ValueEnergy;

	NRF_LOG_INFO("Add new Energy apres rotation : %d : %d : %d : %d : %d\r\n", u32TableOfEnergy[0], u32TableOfEnergy[1], u32TableOfEnergy[2], u32TableOfEnergy[3], u32TableOfEnergy[4]);
}*/

void saveLEDCurrent(unsigned int u16ValueEnergy)
{
	unsigned char u08Index = 0;

	printf("saveLEDCurrent():\r\n");

	printf("Add LEDCurrent avant rotation : %d : %d : %d : %d : %d\r\n", u16TableLEDCurrent[0], u16TableLEDCurrent[1], u16TableLEDCurrent[2], u16TableLEDCurrent[3], u16TableLEDCurrent[4]);

	for (u08Index = (NB_DAY_MAXIMUM - 1); u08Index > 0; u08Index--)
	{
		u16TableLEDCurrent[u08Index] = u16TableLEDCurrent[u08Index - 1];
	}

	u16TableLEDCurrent[0] = u16ValueEnergy;

	printf("Add LEDCurrent apres rotation : %d : %d : %d : %d : %d\r\n", u16TableLEDCurrent[0], u16TableLEDCurrent[1], u16TableLEDCurrent[2], u16TableLEDCurrent[3], u16TableLEDCurrent[4]);

	nrf_delay_ms(400);	// Attente pour printf
}


void ComputeAverageLedEnergy(void)
{
    //-compute coeff avg in Led mA/5
//	unsigned int u16PvMaximumVoltage = 0;
//	unsigned int u32AverageGeneratedEnergy = 0;
	unsigned long u32AverageLEDCurrent = 0;
	unsigned char u08DivisorValue = 0;
	unsigned char u08Index = 0;
//	unsigned long u32ValueOFCurrent = 0;

	printf("ComputeAverageLedEnergy():\r\n");

	// Faire ce calcul quand l'EPEVER est bien stable
	printf("getLoadMaximumVoltageValue : %d\r\n", getLoadMaximumVoltageValue());
	if (getLoadMaximumVoltageValue() != 0)
	{
		coeff_conversion_battery_capacity = ((Battery_voltage * getCapacityBatteryValue()) / getLoadMaximumVoltageValue());
	}
	else
	{
		coeff_conversion_battery_capacity = getCapacityBatteryValue();
	}

	// Initialisation des variables la premières fois seulement
	if (bInitFindLEDmALevel == false)
	{
		initGeneratedEnergy();
		bInitFindLEDmALevel = true;
	}

	// Calcul de la moyenne d'énergie générée
//	u32AverageGeneratedEnergy = 0;
	// Affichage des valeurs de la table
	printf("u16TableLEDCurrent : %d, %d, %d, %d, %d\r\n", u16TableLEDCurrent[0], u16TableLEDCurrent[1], u16TableLEDCurrent[2], u16TableLEDCurrent[3], u16TableLEDCurrent[4]);

	u32AverageLEDCurrent = 0;
	u08DivisorValue = 0;
	for (u08Index = 0; u08Index < NB_DAY_MAXIMUM; u08Index++)
	{	// On calcul la moyenne pondurée des courants des autres jours
		u32AverageLEDCurrent += (u16TableLEDCurrent[u08Index] * (NB_DAY_MAXIMUM - u08Index));
		u08DivisorValue += (NB_DAY_MAXIMUM - u08Index);
	}
	printf("u32AverageLEDCurrent : %ld, u08DivisorValue : %d\r\n", u32AverageLEDCurrent, u08DivisorValue);

	// Calcul du courant moyen des 5 derniers jours
	AvgCurrentmA = u32AverageLEDCurrent / u08DivisorValue;

	printf("AvgCurrentmA : %d\r\n", AvgCurrentmA);

	nrf_delay_ms(200);	// Attente pour printf
}


unsigned int FindLEDmALevel(unsigned char BatPercentCharge)
{
    unsigned char Scen_Time_In_Hour = Scen_Delay;
    unsigned int u08ChoixForDebug = 0;
    unsigned int u16AvrReceiveSolar = 0;

#ifdef OPTIMISATION_PUISSANCE_ECLAIRAGE
    unsigned int u16Duree_Eclairage_Minimum = 0;   // Dm : Durée d'éclairage minimum
    unsigned int u16Eclairage_minimum_objectif = 0;   // Emo : Eclairage minimum objective
	unsigned int Pdj = 0;

	//bool bDureeOptimisationEclairage = false;
	unsigned int u16Duree_Eclairage_Difference = 0;
	unsigned int u16Battery_Remaining_Capacity = 0;

	unsigned char Scenario_ID = 0;

	unsigned long u32AvgSolarCap = 0;

	unsigned char u08Minimal_App_current = 50; // 50mA
	unsigned int u16Maximal_App_current = 700; // 700mA

	printf("FindLEDmALevel() : BatPercentCharge = %d\r\n", BatPercentCharge);

	Scenario_ID = ble_scenario_readScenarioNumber();

	// Dernière energie emmagasinée, ramenée en mA
	// Ici, on prends la valeur de tension maximum de la batterie pour avoir la capacité de la charge de la journée
	u32AvgSolarCap = (getGeneratedEnergyValue() * 10000);
	u32AvgSolarCap = (u32AvgSolarCap * 100)  /  getLoadMaximumVoltageValue();	// Pour ramener en tension en Volts
	printf("u32AvgSolarCap = %ld, getGeneratedEnergyValue = %ld, getLoadMaximumVoltageValue = %d\r\n", u32AvgSolarCap, getGeneratedEnergyValue(), getLoadMaximumVoltageValue());

	nrf_delay_ms(200);	// Attente pour printf

    if (Scenario_ID > 0)
    {
    	// Calcul du temps d'éclairage voulue
		Scen_Time_In_Hour = getTimeToLightOnValue();
		printf("Scen_Time_In_Hour = %d\r\n", Scen_Time_In_Hour);
    }
    else
    {
    	return LEDmAValue;
    }
#endif

    nrf_delay_ms(400);	// Attente pour printf

    // Sauvegarde et rotation des valeurs des charges solaires des jours précédents
    printf("Receive Solar avant rotation : %d : %d : %d : %d : %d\r\n", Received_Solar_Pw0, Received_Solar_Pw1, Received_Solar_Pw2, Received_Solar_Pw3, Received_Solar_Pw4);
    Received_Solar_Pw4 = Received_Solar_Pw3;
	Received_Solar_Pw3 = Received_Solar_Pw2;
	Received_Solar_Pw2 = Received_Solar_Pw1;
	Received_Solar_Pw1 = Received_Solar_Pw0;
    Received_Solar_Pw0 = u32AvgSolarCap;
	u16AvrReceiveSolar = (Received_Solar_Pw0 + Received_Solar_Pw1 + Received_Solar_Pw2 + Received_Solar_Pw3 + Received_Solar_Pw4) / 5; // MM moyenne de charge solaire.

	printf("Receive Solar après rotation (Moy=%d) : %d : %d : %d : %d : %d\r\n", u16AvrReceiveSolar, Received_Solar_Pw0, Received_Solar_Pw1, Received_Solar_Pw2, Received_Solar_Pw3, Received_Solar_Pw4);

	nrf_delay_ms(400);	// Attente pour printf

	//MM==> capacité batterie en mAh pour une batterie de 12.8V alors que la charge est par exemple a 24V. Donc on simplifie.
	u16Battery_Remaining_Capacity = (((unsigned int)coeff_conversion_battery_capacity / 100 ) * BatPercentCharge);
	printf("BatPercentCharge = %d, u16Battery_Remaining_Capacity = %d\r\n", BatPercentCharge, u16Battery_Remaining_Capacity);

    if (Scen_Time_In_Hour == 0)
        Scen_Time_In_Hour = 1;

    //- Limite basse de la capacité batterie
    if (BatPercentCharge < 5)
    {
        LEDmAValue = 0;
        u08ChoixForDebug = 1;
        printf("    BatPercentCharge < 5\r\n");
        return LEDmAValue;
    } //- Batterie pleine on augmente le courant moyen de 20%
    else if (BatPercentCharge >= 90)
    {
        LEDmAValue = AvgCurrentmA + 100;    //(AvgCurrentmA / 5);
        u08ChoixForDebug = 2;
        printf("    BatPercentCharge > 90 : LEDmAValue = %d\r\n", LEDmAValue);
    } //-reserve de batterie >1500maH
    else if ((u16Battery_Remaining_Capacity - (Scen_Time_In_Hour * AvgCurrentmA)) < (3 * (unsigned int)coeff_conversion_battery_capacity / 10))
    {
        //signed int s16AvrReceiveSolar = 0;
        //s16AvrReceiveSolar = ((Received_Solar_Pw0 + Received_Solar_Pw1 + Received_Solar_Pw2 + Received_Solar_Pw3 + Received_Solar_Pw4)*8);  // DAMIEN : 40) / 5; // DAMIEN : mettre une /40 au lieu de /100
        //DAMIEN : enlever cette ligne s32AvrReceiveSolar = s32AvrReceiveSolar * design_capacity;

        if (u16Battery_Remaining_Capacity > (3 * (unsigned int)coeff_conversion_battery_capacity / 10))    // DAMIEN
        {
            //LEDmAValue = (unsigned int) (FuelGaugeData.Battery_Remaining_Capacity - (0.2 * design_capacity)) / Scen_Time_In_Hour;   // DAMIEN : modif du 0.15 en 0.2
            // DAMIEN : nouvelle formule
            // V1.0.68 : remplacement de Scen_Delay par Scen_Time_In_Hour
            LEDmAValue = ((4 * u16AvrReceiveSolar) / (unsigned int)Scen_Time_In_Hour) / 10;      // DAMIEN : remplacement de 6 par Scen_Delay
            printf("    3-1:u16AvrReceiveSolar = %d, LEDmAValue = %d\r\n", u16AvrReceiveSolar, LEDmAValue);
        }
        else
        {
            LEDmAValue = ((3 * u16AvrReceiveSolar) / (unsigned int)Scen_Time_In_Hour) / 10;      // DAMIEN : remplacement de 6 par Scen_Delay
            printf("    3-2:u16AvrReceiveSolar = %d, LEDmAValue = %d\r\n", u16AvrReceiveSolar, LEDmAValue);
        }
        u08ChoixForDebug = 3;
    } //-Charge superieur au min autorisé
    else if (u32AvgSolarCap > ((unsigned int)Scen_Time_In_Hour * (unsigned int)u08Minimal_App_current))
    {
        if (u32AvgSolarCap > (AvgCurrentmA * (unsigned int)Scen_Time_In_Hour))
        {
            //- Charge solaire journée superieure à MoyenneEclairage
            LEDmAValue = AvgCurrentmA + ((2 *((u32AvgSolarCap / (unsigned int)Scen_Time_In_Hour) - AvgCurrentmA))/5);
            u08ChoixForDebug = 4;
            printf("    4:u32AvgSolarCap = %ld, LEDmAValue = %d\r\n", u32AvgSolarCap, LEDmAValue);
        }
        else
        {
            LEDmAValue = AvgCurrentmA - ((3 *(AvgCurrentmA - (u32AvgSolarCap / (unsigned int)Scen_Time_In_Hour)))/5);
            u08ChoixForDebug = 5;
            printf("    5:u32AvgSolarCap = %ld, LEDmAValue = %d\r\n", u32AvgSolarCap, LEDmAValue);
        }
    }
    else if (u32AvgSolarCap <= ((unsigned int)Scen_Time_In_Hour * (unsigned int)u08Minimal_App_current))
    {
        //-Utilisation de la capacité répartie sur les huits jours à venir, réactualisé chaque jour avec les 8 jours à venir
        //DAMIEN : remplacer par la nouvelle formule LEDmAValue = (FuelGaugeData.Battery_Remaining_Capacity) / ((Scen_Time_In_Hour)*8);      // DAMIEN : modif du 10 en 8
        // DAMIEN : s16AvrReceiveSolar = ((Received_Solar_Pw0 + Received_Solar_Pw1 + Received_Solar_Pw2 + Received_Solar_Pw3 + Received_Solar_Pw4)*8);  // DAMIEN : 40) / 5; // DAMIEN : mettre une /40 au lieu de /100
        //DAMIEN : la valeur est déjà en mA !!  s32AvrReceiveSolar = s32AvrReceiveSolar * design_capacity;

        LEDmAValue = u16AvrReceiveSolar / (unsigned int)Scen_Time_In_Hour;
        u08ChoixForDebug = 6;
        printf("    6:u16AvrReceiveSolar = %d, LEDmAValue = %d\r\n", u16AvrReceiveSolar, LEDmAValue);
    }
    else
    { //- Security restore defaut value
        LEDmAValue = 100;
        u08ChoixForDebug = 7;
        printf("    7:LEDmAValue = %d\r\n", LEDmAValue);
    }

    Scen_Delay = Scen_Time_In_Hour;	// V1.1 : remise à 6h du temps d'éclairage
    u16TimeScenario2Off = 0x0400;
	u16TimeScenario2On = 0x0200;

    nrf_delay_ms(200);	// Attente pour printf

    printf("u08ChoixForDebug = %d, LEDmAValue = %d, Scen_Delay = %d\r\n", u08ChoixForDebug, LEDmAValue, Scen_Delay);

#ifdef OPTIMISATION_PUISSANCE_ECLAIRAGE
    unsigned int u16PowerMinimalObjectif = 0;
    u16PowerMinimalObjectif = (unsigned int)(((unsigned int)coeff_conversion_battery_capacity * 9) / 100);
    u16Eclairage_minimum_objectif = u16PowerMinimalObjectif / ((unsigned char)Scen_Time_In_Hour);
    u16Duree_Eclairage_Minimum = ((unsigned char)Scen_Time_In_Hour * 2) / 3;
    printf("u16Eclairage_minimum_objectif = %d, u16Duree_Eclairage_Minimum = %d\r\n", u16Eclairage_minimum_objectif, u16Duree_Eclairage_Minimum);

    nrf_delay_ms(200);	// Attente pour printf

    if (LEDmAValue < u16Eclairage_minimum_objectif)
    {
		if (u32AvgSolarCap>= u16PowerMinimalObjectif)
        {
            LEDmAValue = u16Eclairage_minimum_objectif;
            u08ChoixForDebug += 60;
            Scen_Delay = u16Duree_Eclairage_Minimum;
            printf("    60:LEDmAValue = %d, Scen_Delay = %d\r\n", LEDmAValue, Scen_Delay);
        }
        else
        {
            if (BatPercentCharge >= SEUIL_ACTIVATION)
            {
                LEDmAValue = u16Eclairage_minimum_objectif;
                u08ChoixForDebug += 70;
                Scen_Delay = u16Duree_Eclairage_Minimum;
                printf("    70:LEDmAValue = %d, Scen_Delay = %d\r\n", LEDmAValue, Scen_Delay);
            }
            else
            {
                // Calcul de la puissance du jour
                Pdj = (u16PowerMinimalObjectif * (unsigned int)BatPercentCharge) / SEUIL_ACTIVATION;
                printf("    Pdj = %d\r\n", Pdj);
                if ((unsigned int)(BatPercentCharge * 3) > (unsigned int)(SEUIL_ACTIVATION * 2))
                {
                    LEDmAValue = u16Eclairage_minimum_objectif;
                    u16Duree_Eclairage_Calculee = ((unsigned int)(BatPercentCharge * Scen_Time_In_Hour) / SEUIL_ACTIVATION);
                    // V1.0.76 : Cela permet de réduire et non d?augmenter lorsque la durée du scénario est de 1h
                    if (u16Duree_Eclairage_Calculee == 1)
                    {
                        LEDmAValue = Pdj * 2 / 3;
                    }
                    /*else
                    {
                        LEDmAValue = Pdj / u16Duree_Eclairage_Minimum;
                    }*/

                    //u16Duree_Eclairage_Calculee = u16Duree_Eclairage_Minimum;
                    //u08ChoixForDebug = 11;
                    // V1.0.76

                    u08ChoixForDebug += 80;
                    printf("    80:u16Duree_Eclairage_Calculee = %d, u16Duree_Eclairage_Minimum = %d, LEDmAValue = %d\r\n", u16Duree_Eclairage_Calculee, u16Duree_Eclairage_Minimum, LEDmAValue);
                }
                else
                {
                    LEDmAValue = Pdj / u16Duree_Eclairage_Minimum;
                    u16Duree_Eclairage_Calculee = u16Duree_Eclairage_Minimum;
                    u08ChoixForDebug += 90;
                    printf("    90:u16Duree_Eclairage_Calculee = %d, u16Duree_Eclairage_Minimum = %d, LEDmAValue = %d\r\n", u16Duree_Eclairage_Calculee, u16Duree_Eclairage_Minimum, LEDmAValue);
                }
                // La nouvelle durée calculée est utilisée pour après
				if (Scenario_ID == 1)
				{	// On enregistre la nouvelle durée dans le poid faible
					Scen_Delay = u16Duree_Eclairage_Calculee;
					printf("-- Scenario 1 : Scen_Delay = %d\r\n", Scen_Delay);
				}
				if (Scenario_ID == 2)
				{
					// Calcul du temps d'éclairage après couché et avant lever
					u16Duree_Eclairage_Difference = 6 - u16Duree_Eclairage_Calculee;	// V1.1 : avant Scen_Time_In_Hour - u16Duree_Eclairage_Calculee;

					u16TimeScenario2Off = 0x0400;
					u16TimeScenario2On = 0x0200;

					while (u16Duree_Eclairage_Difference > 0)
					{
						if ((u16TimeScenario2Off & 0x00FF) == 0)
						{	// Ici, il s'agit d'une heure sans minute, on enlève 1 heure et on rajoute 30 minutes
							if (u16TimeScenario2Off >= 0x0100)
							{
								u16TimeScenario2Off = u16TimeScenario2Off - 0x0100;
								u16TimeScenario2Off = u16TimeScenario2Off + 0x0030;
							}
						}
						else
						{	// Ici, l'heure contient déjà des minutes
							if (u16TimeScenario2Off >= 0x0030)
							{
								u16TimeScenario2Off = u16TimeScenario2Off - 0x0030;
							}
						}

						if ((u16TimeScenario2On & 0x00FF) == 0)
						{	// Ici, il s'agit d'une heure sans minute, on enlève 1 heure et on rajoute 30 minutes
							if (u16TimeScenario2On >= 0x0100)
							{
								u16TimeScenario2On = u16TimeScenario2On - 0x0100;
								u16TimeScenario2On = u16TimeScenario2On + 0x0030;
							}
						}
						else
						{	// Ici, l'heure contient déjà des minutes
							if (u16TimeScenario2On >= 0x0030)
							{
								u16TimeScenario2On = u16TimeScenario2On - 0x0030;
							}
						}

						u16Duree_Eclairage_Difference--;
					}


/*					bDureeOptimisationEclairage = false;
					u16Duree_Eclairage_Difference = Scen_Time_In_Hour - u16Duree_Eclairage_Calculee;	// Calcul de la différence entre Scen_Delay (durée souhaitée) et u16Duree_Eclairage_Calculee (durée calculé optimale)
					while (u16Duree_Eclairage_Difference > 0)
					{
						if (bDureeOptimisationEclairage == false)
						{	// On enlève 1h au poid faible : représentant la durée après le coucher
							Scen_Delay = Scen_Delay - 1;
							bDureeOptimisationEclairage = true;
						}
						else
						{	// On enlève 1h au poid ford : représentant la durée avant le lever
							Scen_Delay = Scen_Delay - 0x10;
							bDureeOptimisationEclairage = false;
						}

						u16Duree_Eclairage_Difference--;
					}
*/
					Scen_Delay = u16Duree_Eclairage_Calculee;
					printf("-- Scenario 2 : Scen_Delay = %d, u16TimeScenario2Off = 0x%x, u16TimeScenario2On = 0x%x\r\n", Scen_Delay, u16TimeScenario2Off, u16TimeScenario2On);
				}
				if (Scenario_ID == 3)
				{
					Scen_Delay = u16Duree_Eclairage_Calculee;
					printf("-- Scenario 3 : Scen_Delay = %d\r\n", Scen_Delay);
				}
				if (Scenario_ID == 4)
				{
/*					if (Scen_Conf & 0x20)
					{	// Le bluetooth est activé, les durées sont en minutes...
                        Heures = ((unsigned int)Scen_Delay << 8) + Scen_Hour;
                        Minutes = ((unsigned int)Scen_Minute << 8) + Scen_WeekDays;
                        u16Duree_Eclairage_Difference = Scen_Time_In_Hour - u16Duree_Eclairage_Calculee;	// Calcul de la différence entre Scen_Delay (durée souhaitée) et u16Duree_Eclairage_Calculee (durée calculé optimale)
                        if (Scen_Conf & 0x04)
                        {   // Heure fixe
                            while (u16Duree_Eclairage_Difference > 0)
                            {
                                if (Heures > 30)
                                {
                                    Heures -= 30;   // On diminue de 30 minutes la partie heure fixe d'extinction
                                }
                                else
                                {
                                    Minutes += 30;  // On augmente de 30 minutes la partie heure fixe d'allumage
                                }
                                Minutes += 30;  // On augmente de 30 minutes la partie heure fixe d'allumage
                                u16Duree_Eclairage_Difference--;
                            }
                        }
                        else
                        {   // Durée
                            while (u16Duree_Eclairage_Difference > 0)
                            {
                                if (Heures > 30)    // V1.0.76
                                {
                                    Heures -= 30;   // On diminue de 30 minutes la partie durée d'extinction après coucher
                                }
                                else
                                {   // V1.0.76 : ici, si le temps entre le coucher et la fin d'éclairage est nul, on diminu la durée du lever
                                    if (Minutes > 30)
                                    {
                                        Minutes -= 30;  // On diminue de 30 minutes la partie durée d'allumage avant lever
                                    }
                                }
                                if (Minutes > 30)   // V1.0.76
                                {
                                    Minutes -= 30;  // On diminue de 30 minutes la partie durée d'allumage avant lever
                                }
                                else
                                {   // V1.0.76 : ici, si le temps entre le lever et la début d'éclairage est nul, on diminu la durée du coucher
                                    if (Heures > 30)
                                    {
                                        Heures -= 30;   // On diminue de 30 minutes la partie durée d'extinction après coucher
                                    }
                                }
                                u16Duree_Eclairage_Difference--;
                            }
                        }
                        Scen_Delay = (Heures >> 8);
                        Scen_Hour = (Heures & 0xFF);
                        Scen_Minute = (Minutes >> 8);
                        Scen_WeekDays = (Minutes & 0xFF);
					}
					else
					{	// Couché et levé activé...comme un scénario 2
						bDureeOptimisationEclairage = false;
                        u16Duree_Eclairage_Difference = Scen_Time_In_Hour - u16Duree_Eclairage_Calculee;	// Calcul de la différence entre Scen_Delay (durée souhaitée) et u16Duree_Eclairage_Calculee (durée calculé optimale)
                        while (u16Duree_Eclairage_Difference > 0)
                        {
                            if (bDureeOptimisationEclairage == false)
                            {	// On enlève 1h au poid faible : représentant la durée après le coucher
                                Scen_Delay = Scen_Delay - 1;
                                bDureeOptimisationEclairage = true;
                            }
                            else
                            {	// On enlève 1h au poid ford : représentant la durée avant le lever
                                Scen_Delay = Scen_Delay - 0x10;
                                bDureeOptimisationEclairage = false;
                            }

                            u16Duree_Eclairage_Difference--;
                        }
					}
*/
					printf("-- Scenario 4 : Scen_Delay = %d\r\n", Scen_Delay);
				}
                // V1.0.67 : Scen_Delay = u16Duree_Eclairage_Calculee;
            }
        }
    }

    nrf_delay_ms(200);	// Attente pour printf
#endif

    //- SECURITY limitation current
    // Faire une limitation dans l'augmentation de LEDmAValue !! de 40% pas plus
/*    u16LEDmAValueInCoucher = EEPROM_Read(Valeur_Moy_NRJ_Add) * 5;
    if (u16LEDmAValueInCoucher > 0)
    {   // Cas où la valeur du dernier coucher n'est pas nul
        if (LEDmAValue > (u16LEDmAValueInCoucher + 100))
        {
            LEDmAValue = u16LEDmAValueInCoucher + 100;
            u08ChoixForDebug += 100;
        }
        else
        {
            u08ChoixForDebug += 200;
        }
    }
*/
    if (LEDmAValue > u16Maximal_App_current)
        LEDmAValue = u16Maximal_App_current;

    if (LEDmAValue < u08Minimal_App_current)
        LEDmAValue = u08Minimal_App_current;

    printf("u08ChoixForDebug = %d, LEDmAValue = %d\r\n", u08ChoixForDebug, LEDmAValue);

    nrf_delay_ms(800);	// Attente pour printf

    return LEDmAValue;
}

unsigned int getPduValue(void)
{
	return LEDmAValue;
}

unsigned int getTimeLedValue(void)
{
	//return u16Duree_Eclairage_Calculee;
	return Scen_Delay;
}

unsigned int getTimeToLightOnValue(void)
{
	//uint8_t u08TableDataScenario[20];
	//uint8_t u08NumberOfData = 0;
	unsigned int u16TimeToLightOn = 0;

	//u08NumberOfData = ble_scenario_readScenarioData(&u08TableDataScenario);

	u08ScenarioNumberComputer = ble_scenario_readScenarioNumber();

	switch(u08ScenarioNumberComputer)
	{
		case 0:
			u16TimeToLightOn = 0;
			break;

		case 1:
			u16TimeToLightOn = 6;
			break;

		case 2:
			u16TimeToLightOn = 6;
			break;

		case 3:
			u16TimeToLightOn = 6;
			break;

		default:
			u16TimeToLightOn = 6;
			break;
	}

	return u16TimeToLightOn;
}
