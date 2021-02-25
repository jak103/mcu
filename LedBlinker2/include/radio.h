/*
 * radio.h
 *
 * Created: 12/27/2020 8:59:45 PM
 *  Author: jacob
 */ 

#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

#define RADIO_MODE_RECEIVE  0x60

void radioInit(void);
void radioSetMode(uint8_t mode);
void SendData(uint8_t* packet, uint8_t length);
uint8_t ReadData(uint8_t* packet, uint8_t maxLength);

#endif
