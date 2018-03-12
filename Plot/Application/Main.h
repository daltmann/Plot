/*
 * Main.h
 *
 *  Created on: 09 Mars 2018
 *      Author: Damien
 */

#ifndef APPLICATION_MAIN_H_
#define APPLICATION_MAIN_H_

#include <stdbool.h>
#include <stdint.h>

void forcePWMColor(unsigned char u08Red, unsigned char u08Green, unsigned char u08Blue, unsigned char u08White);
void ActuelPWMColor(unsigned char *u08Red, unsigned char *u08Green, unsigned char *u08Blue, unsigned char *u08White);

void updateBLEHour(void);

#endif /* APPLICATION_MAIN_H_ */
