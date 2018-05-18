#include "user_template.h"

//This function is called once, after all of the Droplet's systems have been initialized.
void init(){

}

/*
 * This function is called repeatedly, as fast as it can. Note that this droplet can only
 * receive new rnb measurements or ir messages after this function returns. Things work
 * better if you let it return frequently.
 */
void loop(){

}

/*
 * This function is called once for every range and bearing measurement this droplet has
 * received since the last time loop returned.
 */
void handleMeas(Rnb* meas){

}

/*
 * This function is called once for every message this droplet has received since the last
 * time loop returned, after handleMeas is called for any rnb measurements received.
 */
void handleMsg(irMsg* msgStruct){

}

/*
 * The two functions below are optional; they do not have to be defined. If they are defined, 
 * they will be called in response to the appropriate events.
 
 optional - commenting it in can be useful for debugging if you want to query
 *	user variables over a serial connection.
 */

/* If defined, this function will be called when the microphone detects a sharp rising edge.
 * In practice, this works well for things like detecting claps or someone tapping on the 
 * Droplet's shell.
 */
//void userMicInterrupt(){}

/*
 * If defined, this function will be called with any serial commandWords that do not match
 * other commands serial_handler.c checks for. See the serial_handler documentation for
 * details on commandWord and commandArgs.
 */
//uint8_t userHandleCommand(char* commandWord, char* commandArgs){}
