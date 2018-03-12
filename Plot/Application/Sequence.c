/*
 * Sequence.c
 *
 *  Created on: 25 Février 2018
 *      Author: Damien
 */

#include "Sequence.h"

#include "Main.h"

#include "app_uart.h"
#include <stdio.h>

#include "stdbool.h"
#include "string.h"

#include "nrf_delay.h"

#include "app_timer.h"


uint8_t u08IDGroup = 0;
uint8_t u08SequenceNumberGroup = 0;
uint8_t u08RepartitionNuitGroup = 0;

bool bFlagNewConfigurationSequence = false;

uint8_t u08TableSequence[NUMBER_MAX_SEQUENCE][NUMBER_BYTE_BY_SEQUENCE];	// Tableau de sauvegarde de toutes les séquences d'un même groupe
uint8_t u08SequenceNumber = 0;
bool bFlagNewSequence = false;
uint8_t u08CounterOfSequenceWriting = 0;

bool bFlagNewSequenceC4 = false;
uint8_t u08NumberSequenceC4 = 0;
uint8_t u08SequenceNumberMaxC4 = 0;

bool bFlagNewSequenceToSendRadio = false;

bool bFlagEndSendScenario_ModeScenario = false;

strucDemoMode stDemoMode;
bool bFlagNewDemoMode = false;

structHour stHour;

APP_TIMER_DEF(m_notif_timer_sequence);
static void timer_timeout_handler_sequence(void * p_context);
static void startTimerSequence(uint32_t u32TimerValue);
static void stopTimerSequence(void);

unsigned char u08ActuelSequenceNumber = 0;	// valeur actuelle de la séquence
unsigned char u08NbLoopSequence = 0;

unsigned char u08RedOrigine = 0;		// Permet de sauvegarder les couleurs d'origines des séquences de fondues
unsigned char u08GreenOrigine = 0;
unsigned char u08BlueOrigine = 0;
unsigned char u08WhiteOrigine = 0;

#define NB_CYCLE_FONDUE		20		// 20 tranches pour une fondue

bool bNewSequence = false;


/**@brief Function to manage the Sequence
 */
void Init_Sequence(void)
{
//	nrf_delay_ms(50);	// Attente pour printf
	uint8_t u08IndexPremier = 0;
	uint8_t u08IndexSecond = 0;

	// Initialisation du tableau de sauvegarde des séquences
	for (u08IndexPremier = 0; u08IndexPremier < NUMBER_MAX_SEQUENCE; u08IndexPremier++)
	{
		for (u08IndexSecond = 0; u08IndexSecond < NUMBER_BYTE_BY_SEQUENCE; u08IndexSecond++)
		{
			u08TableSequence[u08IndexPremier][u08IndexSecond] = 0;
		}
	}

	u08CounterOfSequenceWriting = 0;
}

void Init_Timer_Sequence(void)
{
	uint32_t err_code = 0;

	// Timer pour les sequences
	err_code = app_timer_create(&m_notif_timer_sequence, APP_TIMER_MODE_REPEATED, timer_timeout_handler_sequence);
	APP_ERROR_CHECK(err_code);
}

void Init_DemoMode(void)
{
	stDemoMode.u08Mode = 0;
	stDemoMode.u08Hour = 0;
	stDemoMode.u08Minute = 0;
	stDemoMode.u08Seconde = 0;
}

void Init_Hour(void)
{
	stHour.u08Hour = 0;
	stHour.u08Minute = 0;
	stHour.u08Second = 0;
	stHour.u16MilliSecond = 0;
}

// ------- Gestion de l'univers --------
/**@brief Function to save the Univers Numbers table for the new sequence configuration
 */
void saveUniversNumber(const uint8_t *u08DataUniversNumber)
{
	if ((u08DataUniversNumber[0] == 0xFF) && (u08DataUniversNumber[1] == 0xFF) && (u08DataUniversNumber[2] == 0xFF))
	{
		bFlagEndSendScenario_ModeScenario = true;
	}

	if ((u08DataUniversNumber[1] <= NUMBER_MAX_SEQUENCE) && (u08DataUniversNumber[1] > 0))
	{	// Test si le nombre de sequence est bien inférieur au maximum de sequence maximum possible !!!
		// On sauvegarde les infos dans les variable de la gestion de cette configuration
		u08IDGroup = u08DataUniversNumber[0];

		u08SequenceNumberGroup = u08DataUniversNumber[1];

		u08RepartitionNuitGroup = u08DataUniversNumber[2];

		bFlagNewConfigurationSequence = true;
	}
	else
	{	// Le nombre max de sequence est supérieur au maximum
		// On indique une erreur du numéro
		u08IDGroup = 0;
		u08SequenceNumberGroup = 0;
	}
}

/**@brief Function to save the Univers Numbers table for the new sequence configuration, value by value
 */
void saveUniversNumberUnit(uint8_t u08IdGroupValue, uint8_t u08SeqNbValue, uint8_t u08RepNuitValue)
{
	u08IDGroup = u08IdGroupValue;

	u08SequenceNumberGroup = u08SeqNbValue;

	u08RepartitionNuitGroup = u08RepNuitValue;
}


uint8_t returnLastIDGroup(void)
{
	return u08IDGroup;
}

uint8_t returnLastSequenceNumberGroup(void)
{
	return u08SequenceNumberGroup;
}

uint8_t returnLastRepartitionNuitGroup(void)
{
	return u08RepartitionNuitGroup;
}


bool checkIfNewUniversNumberConfiguration(void)
{
	return bFlagNewConfigurationSequence;
}

void resetFlagNewUniversNumberConfiguration(void)
{
	bFlagNewConfigurationSequence = false;
}


bool readUniversNumber(uint8_t *u08IdGroupValue, uint8_t *u08SeqNbValue, uint8_t *u08RepNuitValue)
{
	*u08IdGroupValue = u08IDGroup;

	*u08SeqNbValue = u08SequenceNumberGroup;

	*u08RepNuitValue = u08RepartitionNuitGroup;

	return true;
}

// ------- Gestion de la Sequence --------
/**@brief Function to save the Univers Numbers table for the new sequence configuration
 */
void saveSequence(const uint8_t *u08DataSequence)
{
	uint8_t u08Index = 0;

	if ((u08DataSequence[0] <= u08SequenceNumberGroup) && (u08DataSequence[0] <= NUMBER_MAX_SEQUENCE))
	{	// Test si le numéro de séquence est bien inférieur au nombre MAX de séquence du Groupe
		if (u08TableSequence[u08DataSequence[0] - 1][0] == 0)
		{
			u08CounterOfSequenceWriting++;
		}

		for (u08Index = 0; u08Index < NUMBER_BYTE_BY_SEQUENCE; u08Index++)
		{
			u08TableSequence[u08DataSequence[0] - 1][u08Index] = u08DataSequence[u08Index];
		}

		u08SequenceNumber = u08DataSequence[0];

		bFlagNewSequence = true;

		if (u08CounterOfSequenceWriting >= returnLastSequenceNumberGroup())
		{
			bFlagNewSequenceToSendRadio = true;
		}
	}
	else
	{
		u08SequenceNumber = 0;

		bFlagNewSequence = true;
	}
}

uint8_t returnLastSequenceNumber(void)
{
	return u08SequenceNumber;
}

bool checkIfNewSequence(void)
{
	return bFlagNewSequence;
}

void resetFlagNewSequence(void)
{
	bFlagNewSequence = false;
}


bool checkIfNewSequenceToSendRadio(void)
{
	return bFlagNewSequenceToSendRadio;
}

void resetFlagNewSequenceToSendRadio(void)
{
	bFlagNewSequenceToSendRadio = false;
}

bool readTableSequence(uint8_t u08SequenceNumber, uint8_t *table)
{
	uint8_t u08Index = 0;

	if ((u08SequenceNumber <= NUMBER_MAX_SEQUENCE) && (u08SequenceNumber > 0))
	{
		//printf("Seq  %d read\r\n", u08SequenceNumber);
		for (u08Index = 0; u08Index < NUMBER_BYTE_BY_SEQUENCE; u08Index++)
		{
			table[u08Index] = u08TableSequence[u08SequenceNumber - 1][u08Index];
		}

		return true;
	}
	else
	{
		printf("Wrong Read seq Nb\r\n");
		return false;
	}
}

// ------- C4 --------
/**@brief Function to save the Sequence for the C4 mode table for the new sequence configuration
 */
void saveSequenceC4(const uint8_t *u08DataSequence)
{
	uint8_t u08Index = 0;

	if ((u08DataSequence[0] <= u08SequenceNumberMaxC4) && (u08DataSequence[0] <= NUMBER_MAX_SEQUENCE))
	{	// Test si le numéro de séquence est bien inférieur au nombre MAX de séquence du Groupe
		printf("Save Seq C4 : %d\r\n", u08DataSequence[0]);
		if (u08TableSequence[u08DataSequence[0] - 1][0] == 0)
		{
			u08CounterOfSequenceWriting++;
		}

		for (u08Index = 0; u08Index < NUMBER_BYTE_BY_SEQUENCE; u08Index++)
		{
			printf("[%d] = %x, ", u08Index, u08DataSequence[u08Index]);
			u08TableSequence[u08DataSequence[0] - 1][u08Index] = u08DataSequence[u08Index];
		}
		printf("\r\n");

		u08NumberSequenceC4 = u08DataSequence[0];

		bFlagNewSequenceC4 = true;
	}
	else
	{
		printf("Wrong Write seq Nb\r\n");
		u08NumberSequenceC4 = 0;

		bFlagNewSequenceC4 = true;
	}
}

uint8_t returnLastSequenceNumberC4(void)
{
	return u08NumberSequenceC4;
}

bool checkIfNewSequenceC4(void)
{
	return bFlagNewSequenceC4;
}

void resetFlagNewSequenceC4(void)
{
	bFlagNewSequenceC4 = false;
}

void saveNumberSequenceC4(unsigned char u08DataValue)
{
	u08SequenceNumberMaxC4 = u08DataValue;
}

unsigned char returnNumberSequenceMaxC4(void)
{
	return u08SequenceNumberMaxC4;
}


bool checkIfEndSendModeScenario(void)
{
	return bFlagEndSendScenario_ModeScenario;
}

void resetFlagEndSendModeScenario(void)
{
	bFlagEndSendScenario_ModeScenario = false;
}

// ------- Gestion du mode Démo --------
/**@brief Function to save the Demo Mode value
 */
void saveDemoMode(const uint8_t *u08DataDemoMode)
{
	stDemoMode.u08Mode = u08DataDemoMode[0];
	stDemoMode.u08Hour = u08DataDemoMode[1];
	stDemoMode.u08Minute = u08DataDemoMode[2];
	stDemoMode.u08Seconde = u08DataDemoMode[3];

	bFlagNewDemoMode = true;
}

bool checkIfNewDemoMode(void)
{
	return bFlagNewDemoMode;
}

void resetFlagNewDemoMode(void)
{
	bFlagNewDemoMode = false;
}


// ------- Gestion des séquences et des scénarios --------
void initNewSequenceC5(void)
{
	// TODO :
	// Lancer une nouvelle séquence à partir de la valeur de ce DemoMode !!!
	switch((enumModeFonctionnement)stDemoMode.u08Mode)
	{
		case MODE_ARRET:
			break;
		case MODE_DEMO:
			printf("Mode Demo C5\r\n");
			break;

		case MODE_JOUR:
			break;

		case MODE_NUIT:
			break;

		case MODE_UPDATE:
			break;

		case MODE_DEFAULT:
			break;

		default:
			break;
	}

}

void initNewSequenceC4(unsigned char u08FonctionnementValue)
{
	unsigned char u08Index, u08IndexSec;
	unsigned char u08TableSequence[NUMBER_BYTE_BY_SEQUENCE];

	// TODO :
	// Lancer une nouvelle séquence à partir de la valeur de ce DemoMode !!!
	stDemoMode.u08Mode = u08FonctionnementValue;

	switch((enumModeFonctionnement)stDemoMode.u08Mode)
	{
		case MODE_ARRET:
			printf("Arret Seq C4\r\n");
			stopTimerSequence();

			forcePWMColor(0, 0, 0, 0);
			break;
		case MODE_DEMO:
			printf("Mode Demo C4\r\n");
			if (returnNumberSequenceMaxC4() > 0)
			{
				// On parcourd tout le tableau des configurations
				for (u08Index = 1; u08Index <= returnNumberSequenceMaxC4(); u08Index++)
				{
					readTableSequence(u08Index, u08TableSequence);
					printf("Seq %d : ", u08Index);
					for (u08IndexSec = 0; u08IndexSec < NUMBER_BYTE_BY_SEQUENCE; u08IndexSec++)
					{
						printf("[%d]=%x", u08IndexSec, u08TableSequence[u08IndexSec]);
					}
					printf("\r\n");
					nrf_delay_ms(50);
				}

				u08ActuelSequenceNumber = 1;
				bNewSequence = true;

				startTimerSequence(1000);
			}
			break;

		case MODE_JOUR:
			break;

		case MODE_NUIT:
			break;

		case MODE_UPDATE:
			break;

		case MODE_DEFAULT:
			break;

		default:
			break;
	}
}

void incrementHour(void)
{
	// On incrémente les millisecondes de 125 ms
	stHour.u16MilliSecond += 125;
	if (stHour.u16MilliSecond >= 1000)
	{
		stHour.u08Second++;
		stHour.u16MilliSecond = 0;
	}

	if (stHour.u08Second >= 60)
	{
		stHour.u08Minute++;
		stHour.u08Second = 0;
	}

	if (stHour.u08Minute >= 60)
	{
		stHour.u08Hour++;
		stHour.u08Minute = 0;
	}

	if (stHour.u08Hour >= 24)
	{
		//stHour.++;
		// TODO : augmenter la date !! le jour
		stHour.u08Hour = 0;
	}
}

void saveHour(unsigned char u08Hour, unsigned char u08Minutes, unsigned char u08Seconds, unsigned int u16MilliSeconds)
{
	stHour.u08Hour = u08Hour;
	stHour.u08Minute = u08Minutes;
	stHour.u08Second = u08Seconds;
	stHour.u16MilliSecond = u16MilliSeconds;
}

structHour readHour(void)
{
	return stHour;
}


static void timer_timeout_handler_sequence(void * p_context)
{
	// On exécute la gestion des séquences
	manageScenario();
}

static void startTimerSequence(uint32_t u32TimerValue)
{
	uint32_t err_code = 0;

	err_code = app_timer_start(m_notif_timer_sequence, APP_TIMER_TICKS(u32TimerValue), NULL);
	APP_ERROR_CHECK(err_code);
}

static void stopTimerSequence(void)
{
	uint32_t err_code = 0;

	err_code = app_timer_stop(m_notif_timer_sequence);
	APP_ERROR_CHECK(err_code);
}

static unsigned char fondue(unsigned char u08BeginColor, unsigned char u08FinalColor, unsigned char u08LoopNumber)
{
	unsigned char u08ResultColor = 0;

	if (u08LoopNumber == 0)
	{	// Ici, on a atteint la dernière boucle, le résultat doit être la valeur final de la couleur
		u08ResultColor = u08FinalColor;
	}
	else
	{
		if (u08BeginColor >= u08FinalColor)
		{	// Ici, nosu avons une fondues en descente
			u08ResultColor = ((u08BeginColor - u08FinalColor) / NB_CYCLE_FONDUE) * u08LoopNumber;
			u08ResultColor = u08FinalColor + u08ResultColor;
		}
		else
		{	// Ici, la fondue sera en montée
			u08ResultColor = ((u08FinalColor - u08BeginColor) / NB_CYCLE_FONDUE) * u08LoopNumber;
			u08ResultColor = u08BeginColor - u08ResultColor;
		}
	}

	return u08ResultColor;
}

void manageScenario(void)
{
	//unsigned char u08Index, u08IndexSec;
	unsigned char u08TableSequence[NUMBER_BYTE_BY_SEQUENCE];
	bool bFlagNextSequence = false;
	bool fForceContinueSequence = false;
	unsigned char u08Red, u08Green, u08Blue, u08White;
	unsigned long u32Time;

	// Lecture de la séquence suivante
	readTableSequence(u08ActuelSequenceNumber, u08TableSequence);

	if (bNewSequence == true)
	{
		printf("Seq : %d\r\n", u08ActuelSequenceNumber);

		// TODO : prévoir le timeout de 2h pour l'arrêt de la séquence
		structHour stActuelHour;
		stActuelHour = readHour();
		printf("Hour = %d:%d:%d:%d\r\n", stActuelHour.u08Hour, stActuelHour.u08Minute, stActuelHour.u08Second, stActuelHour.u16MilliSecond);

		bNewSequence = false;
	}

	// Gestion du type de scénario à faire
	// Sur les 4 octets de poids fort
	switch((u08TableSequence[1] & 0xF0))
	{
		case 0x00:
			// Couleur fixe avec un temps d'attente
			forcePWMColor(u08TableSequence[2], u08TableSequence[3], u08TableSequence[4], u08TableSequence[5]);
			stopTimerSequence();
			printf("Fix time color : %d\r\n", (u08TableSequence[1] & 0x0F));
			startTimerSequence((u08TableSequence[1] & 0x0F) * 1000);

			bFlagNextSequence = true;
			fForceContinueSequence = false;
			break;

		case 0x10:
			// Transition fondue
			if (u08NbLoopSequence == 0)
			{	// Ici, il s'agit de la première fois que nous allons traiter la séquence
				// On indique que le nombre de boucle sera de 5
				u08NbLoopSequence = NB_CYCLE_FONDUE;

				// On prend la valeur actuel de chaque couleur de PWM
				ActuelPWMColor(&u08RedOrigine, &u08GreenOrigine, &u08BlueOrigine, &u08WhiteOrigine);
				u08Red = fondue(u08RedOrigine, u08TableSequence[2], u08NbLoopSequence);
				u08Green = fondue(u08GreenOrigine, u08TableSequence[3], u08NbLoopSequence);
				u08Blue = fondue(u08BlueOrigine, u08TableSequence[4], u08NbLoopSequence);
				u08White = fondue(u08WhiteOrigine, u08TableSequence[5], u08NbLoopSequence);

				// On force les nouvelles couelurs trouvées
				forcePWMColor(u08Red, u08Green, u08Blue, u08White);

				// On doit diviser le temps par NB_CYCLE_FONDUE (5)
				stopTimerSequence();
				u32Time = (((u08TableSequence[1] & 0x0F) * 1000) / NB_CYCLE_FONDUE);
				printf("Fondue time : %ld\r\n", u32Time);
				startTimerSequence(u32Time);
			}
			else
			{
				// On décrémente le numéro de séquence
				u08NbLoopSequence--;
				if (u08NbLoopSequence == 0)
				{
					bFlagNextSequence = true;
					fForceContinueSequence = true;
				}

				u08Red = fondue(u08RedOrigine, u08TableSequence[2], u08NbLoopSequence);
				u08Green = fondue(u08GreenOrigine, u08TableSequence[3], u08NbLoopSequence);
				u08Blue = fondue(u08BlueOrigine, u08TableSequence[4], u08NbLoopSequence);
				u08White = fondue(u08WhiteOrigine, u08TableSequence[5], u08NbLoopSequence);

				// On force les nouvelles couelurs trouvées
				forcePWMColor(u08Red, u08Green, u08Blue, u08White);
			}
			break;

		case 0x20:
			// Transition SEC
			forcePWMColor(u08TableSequence[2], u08TableSequence[3], u08TableSequence[4], u08TableSequence[5]);
			stopTimerSequence();
			printf("Fix time color : %d\r\n", (u08TableSequence[1] & 0x0F));
			startTimerSequence((u08TableSequence[1] & 0x0F) * 1000);

			bFlagNextSequence = true;
			fForceContinueSequence = false;
			break;

		case 0x30:
			// Attente
			stopTimerSequence();
			printf("Attente : %d\r\n", (u08TableSequence[1] & 0x0F));
			startTimerSequence((u08TableSequence[1] & 0x0F) * 1000);

			// On force les leds à 0%
			forcePWMColor(0, 0, 0, 0);

			bFlagNextSequence = true;
			fForceContinueSequence = true;
			break;

		case 0x40:
			// Clignotement
			if (u08NbLoopSequence == 0)
			{	// Ici, il s'agit de la première fois que nous allons traiter la séquence
				// On indique que le nombre de boucle sera de 5
				u08NbLoopSequence = (u08TableSequence[1] & 0x0F) * 2;

				// On force les nouvelles couleurs trouvées pour le premier allumage du clignotement
				forcePWMColor(u08TableSequence[2], u08TableSequence[3], u08TableSequence[4], u08TableSequence[5]);
				// On sauvegarde le niveau de chaque couleur
				u08RedOrigine = u08TableSequence[2];
				u08GreenOrigine = u08TableSequence[3];
				u08BlueOrigine = u08TableSequence[4];
				u08WhiteOrigine = u08TableSequence[5];

				// On doit diviser par le nombre de clignotement par secondes mais aussi multiplié par 2 pour avoir un coup un allumage, un coup rien
				stopTimerSequence();
				u32Time = ((1000 / (u08TableSequence[1] & 0x0F)) * 2);
				printf("Clignot time : %ld\r\n", u32Time);
				startTimerSequence(u32Time);
			}
			else
			{
				u08NbLoopSequence--;
				if (u08NbLoopSequence == 0)
				{
					bFlagNextSequence = true;
					fForceContinueSequence = true;
				}
				else
				{
					// On détecte si il s'agit d'un nombre de boucle paire ou impaire
					if ((u08NbLoopSequence & 0x01) == 0x01)
					{	// Nombre impaire de boucle, donc, on arrête l'éclairage
						forcePWMColor(0, 0, 0, 0);
					}
					else
					{	// Si le nombre est paire, on allume au niveau demandé
						forcePWMColor(u08RedOrigine, u08GreenOrigine, u08BlueOrigine, u08WhiteOrigine);
					}
				}
			}
			break;

		default:
			break;
	}

	if (bFlagNextSequence == true)
	{
		// On incrémente la valeur du numéro de séquence actuel jusqu'à sa séquence maximum
		u08ActuelSequenceNumber++;
		if (u08ActuelSequenceNumber > returnNumberSequenceMaxC4())
		{	// On revient sur la première séquence
			u08ActuelSequenceNumber = 1;
		}

		if (fForceContinueSequence == true)
		{
			// On arrête le timer et on le redémarre poru que les séquences puisse être collées le plus possible
			stopTimerSequence();
			startTimerSequence(20);
		}

		bNewSequence = true;
	}
}
