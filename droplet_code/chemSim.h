#ifndef CHEM_SIM
#define CHEM_SIM

#include "droplet_init.h"
#include "Atom.h"

#define RNB_BROADCAST_PERIOD 3000
#define UPDATE_ATOMS_PERIOD 100

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

Near_Atom near_atoms[12]; //this number is pretty arbitrary.
Atom NULL_ATOM = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0},{0,0},0,0,0};
Near_Atom NULL_NEAR_ATOM = {{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0},{0,0},0,0,0}, 0, 0, 0, 0, 0};
uint8_t bond_broken_flag;
uint8_t bond_broken_flag_2;
void init();
void loop();
void handle_msg(ir_msg* msg_struct);

void periodic_rnb_broadcast();
void add_to_near_atoms();
void update_near_atoms();
Atom getAtomFromAtomicNum(uint8_t atomicNum);
void found_diatomic_routine();
void setAtomColor(Atom ID);
void broadcastChemID(Atom ID);
void sendChemID(Atom ID, uint8_t channels[]);
uint8_t valenceFull();
void detectOtherDroplets();
void periodic_rnb_broadcast();
void print_near_atoms();

Atom myID;

#endif
