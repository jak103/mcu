/*
 * state.h
 *
 * Created: 12/21/2020 1:32:50 PM
 *  Author: jacob
 */ 


#ifndef STATE_H_
#define STATE_H_

enum STATE
{
	INIT,
	LISTEN,
	SLEEP,
	RFID_EVENT,
	RF_TRANSMITTING,
	RF_WAIT_FOR_REPLY,
	RF_MESSAGE_READY,
	STATUS,
	BUZZ,
};

extern enum STATE currentState;

#endif /* STATE_H_ */