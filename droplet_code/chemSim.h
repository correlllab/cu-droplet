#ifndef CHEM_SIM
#define CHEM_SIM

#include "droplet_init.h"
#include "Atom.h"

#define RNB_BROADCAST_PERIOD 11000
#define BLINK_PERIOD 2500
#define MIN_INTER_CHEM_ID_BROADCAST_DELAY 3000
#define DETECT_OTHER_DROPLETS_PERIOD 1000
#define UPDATE_ATOMS_PERIOD 100
#define LOOP_PERIOD 50

typedef struct
{
	Atom atom;
	uint16_t id;
	uint8_t range; //in theory you should reorder this at some point.
	int16_t bearing;
	int16_t heading;
	uint8_t last_msg_t; //time that this atom last sent a message to me
	uint8_t bonded;
}Near_Atom;

typedef struct 
{
	uint16_t bonded_atoms[6];
	uint16_t blink_timer;
}Bonded_Atoms_Msg;

Near_Atom near_atoms[12]; //this number is pretty arbitrary.
Atom NULL_ATOM = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0},{'0','0'},0,0,0};
Near_Atom NULL_NEAR_ATOM = {{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0},{'0','0'},0,0,0}, 0, 0, 0, 0, 0};
volatile uint32_t bondDelay;
volatile uint16_t potentialPartner;
volatile uint32_t tap_delay;
volatile uint32_t bonded_atoms_delay;

uint32_t last_chem_ID_broadcast;

uint16_t global_blink_timer;
void init();
void loop();
void handle_msg(ir_msg* msg_struct);

void periodic_rnb_broadcast();
void add_to_near_atoms();
void update_near_atoms();
void add_to_bonded_atoms(uint16_t ID);
void printValence(int8_t valence[]);
Atom getAtomFromAtomicNum(uint8_t atomicNum);
void checkPossibleBonds(Atom* near_atom, uint16_t senderID);
void found_bond_routine(char flag);
void setAtomColor(Atom ID);
void broadcastChemID(Atom ID);
void sendChemID(Atom ID, uint8_t channels[]);
uint8_t valenceState();
void detectOtherDroplets();
void periodic_rnb_broadcast();
void print_near_atoms();

Atom myID;

#endif
