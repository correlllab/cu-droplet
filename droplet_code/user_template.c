//#include "user_template.h"
//
//
//int comparison(uint8_t* aPtr, uint8_t* bPtr){
	//uint8_t a = *aPtr;
	//uint8_t b = *bPtr;
	//if(a<b){
		//return 1;
	//}else if(a==b){
		//return 0;
	//}else if(a>b){
		//return -1;
	//}
//}
//
///*
 //* Any code in this function will be run once, when the robot starts.
 //*/
//void init()
//{
	//printf("Hello!\r\n");
	//uint8_t cleanMolecule[15] = {0,6,0,0,6,0,0,0,6,0,0,0,0,0,0};
	//qsort(cleanMolecule, 15, 1, comparison);
	////uint8_t swapped=0;
	////do{
		////swapped=0;
		////for(uint8_t i=0;i<14;i++){
			////if(cleanMolecule[i]==0){
				////cleanMolecule[i]=cleanMolecule[i+1];
				////cleanMolecule[i+1]=0;
				////swapped=1;
			////}
		////}
	////}while(swapped);
	//printf("After:\t");
	//for(uint8_t i=0;i<15;i++){
		//printf("%hu ", cleanMolecule[i]);
	//}
	//printf("\r\n");
//}
//
///*
 //* The code in this function will be called repeatedly, as fast as it can execute.
 //*/
//void loop()
//{
//
//}
//
///*
 //* After each pass through loop(), the robot checks for all messages it has 
 //* received, and calls this function once for each message.
 //*/
//void handle_msg(ir_msg* msg_struct)
//{
//
//}
//
/////*
 ////*	The function below is optional - commenting it in can be useful for debugging if you want to query
 ////*	user variables over a serial connection.
 ////*/
////uint8_t user_handle_command(char* command_word, char* command_args)
////{
	////return 0;
////}
//
/////*
 ////*	The function below is optional - if it is commented in, and the leg interrupts have been turned on
 ////*	with enable_leg_status_interrupt(), this function will get called when that interrupt triggers.
 ////*/	
////void	user_leg_status_interrupt()
////{
	////
////}
//
