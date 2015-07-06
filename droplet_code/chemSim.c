#include "chemSim.h"

uint8_t MY_CHEM_ID; //This will eventually be written to the file by writeFiles.c!
char* global_Atom_str;

//This is a list of Atoms. Should they be static or constant or something?
//Variables are valence shell, current bonded atoms, Mulliken electronegativity, symbol, diatomic, atomic number
Atom H =  {{1, 0, -1, -1, -1, -1, -1, -1}, {0,0,0,0,0,0}, 2.25, "H", 0, 1, 1};
Atom He = {{1, 1, -1, -1, -1, -1, -1, -1}, {0,0,0,0,0,0}, 3.49, "He", 0, 0, 2};
Atom Li = {{1, 0, 0, 0, 0, 0, 0, 0},{0,0,0,0,0,0}, 0.97, "Li", 0, 0, 3};
Atom Be = {{1, 1, 0, 0, 0, 0, 0, 0}, {0,0,0,0,0,0}, 1.54, "Be", 0, 0, 4};
Atom C =  {{1, 1, 1, 1, 0, 0, 0, 0}, {0,0,0,0,0,0}, 2.48, "C", 0, 0, 6};
Atom N =  {{1, 1, 1, 1, 1, 0, 0, 0}, {0,0,0,0,0,0}, 2.90, "N", 0, 1, 7};
Atom O =  {{1, 1, 1, 1, 1, 1, 0, 0}, {0,0,0,0,0,0}, 3.41, "O", 0, 1, 8};
Atom F =  {{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0,0,0}, 3.91, "F", 0, 1, 9};
Atom Na = {{1, 0, 0, 0, 0, 0, 0, 0}, {0,0,0,0,0,0}, 0.91, "Na", 0, 0, 11};
Atom Mg = {{1, 1, 0, 0, 0, 0, 0, 0}, {0,0,0,0,0,0}, 1.37, "Mg", 0, 0, 12};
Atom Cl = {{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0,0,0}, 3.10, "Cl", 0, 1, 17};
Atom Br = {{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0,0,0}, 2.95, "Br", 0, 1, 35};
Atom I =  {{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0,0,0}, 2.74, "I", 0, 1, 53};


void print_near_atoms()
{
	//printf("PRINTING NEAR_ATOMS: \r\n");
	for(uint8_t i = 0; i < 12; i++)
	{
		if(near_atoms[i].id==0) break;
		//printf("Atom: %s  Rng: %hu ID: %04X \r\n", near_atoms[i].atom.name, near_atoms[i].range, near_atoms[i].id);
	}
	
	printValence(myID.valence);
	//printf("\r\n");
	//printf("\r\n");
	printf("\tBonded droplets: ");
	uint8_t any_bonded=0;
	for(uint8_t i = 0; i < 6; i++)
	{
		if(myID.bonded_atoms[i]){
			any_bonded=1;
			printf("%04X, ", myID.bonded_atoms[i]);
		}
	}
	if(any_bonded)	printf("\b\b\r\n");
	else			printf("None\r\n");
}

void add_to_near_atoms(Near_Atom near_atom)
{
	uint8_t isSpace = 0;
	for(uint8_t i = 0; i < 12; i++)
	{
		if(near_atoms[i].id == 0) 
		{
			near_atoms[i] = near_atom; //This is probably okay but if the data structure is getting corrupted take a look at what exactly changes when you make this assignment.
			isSpace = 1;
			//printf("Adding %s \r\n", near_atom.atom.name);
			break;
		}
	}	
	if (isSpace == 0) printf("No space to add another Droplet \r\n");
}

void update_near_atoms()
{
	for(uint8_t i = 0; i < 12; i++)
	{
		if(near_atoms[i].id != 0 && near_atoms[i].last_msg_t > 10000) //assuming this is in ms
		{
			near_atoms[i] = NULL_NEAR_ATOM;
			printf("Removing %c%c \r\n", near_atoms[i].atom.name[0], near_atoms[i].atom.name[1]);
		}
		else if(near_atoms[i].id != 0)
		{
			near_atoms->last_msg_t += UPDATE_ATOMS_PERIOD; //assuming update is called every 100 ms
		}
	}
}

Atom getAtomFromAtomicNum(uint8_t atomicNum)
{
	switch(atomicNum){
		case 1:
		{
			//printf("case 1");
			set_rgb(255, 200, 0); //yellow
			return H;
		}
		case 2:
		{
			//printf("case 2");
			set_rgb(255, 50, 0); //orange
			return He;
		}
		case 3:
		{
			//printf("case 3");
			set_rgb(100, 100, 255); //ice white
			return Li;
		}
		case 4:
		{
			//printf("case 4");
			set_rgb(100, 255, 100); //ice green
			return Be;
		}
		case 6:
		{
			set_rgb(100, 0, 255); //indigo
			return C;
		}
		case 7:
		{
			set_rgb(200, 10, 10); //red-pink
			return N;
		}
		case 8:
		{
			set_rgb(0, 0, 255); //blue
			return O;
		}
		//All halogens are green
		case 9:
		{
			set_rgb(0, 255, 0); //green
			return F;
		}
		case 17:
		{
			set_rgb(0, 255, 0); //green
			return Cl;
		}
		case 35:
		{
			set_rgb(0, 255, 0); //green
			return Br;
		}
		case 53:
		{
			set_rgb(0, 255, 0); //green
			return I;
		}
		default:
			printf("No such element");
	}
}

void found_diatomic_routine()
{
	//printf("IN FOUND_DIATOMIC_ROUTINE \r\n");
	set_rgb(255, 0, 0);
	delay_ms(300);
	setAtomColor(myID);
	delay_ms(300);
	set_rgb(255, 0, 0);
	delay_ms(300);
	set_rgb(255, 255, 0);
}

void found_bond_routine()
{
	set_rgb(255, 0, 255);
	delay_ms(300);
	set_rgb(255,255,255);
	delay_ms(300);
	set_rgb(255, 0, 255);
	delay_ms(300);
	set_rgb(255,0,0);
	delay_ms(300);
	//setAtomColor(myID);
}

void setAtomColor(Atom ID)
{
	uint8_t atomicNum = ID.atomicNum;
	switch(atomicNum){
		case 1:
		{
			//printf("case 1");
			set_rgb(255, 200, 0); //yellow
			break;
		}
		case 2:
		{
			//printf("case 2");
			set_rgb(255, 50, 0); //orange
			break;
		}
		case 3:
		{
			//printf("case 3");
			set_rgb(100, 100, 255); //purple
			break;
		}
		case 4:
		{
			//printf("case 4");
			set_rgb(100, 255, 100); //green
			break;
		}
		case 6:
		{
			set_rgb(100, 0, 255); //indigo
			break;
		}
		case 7:
		{
			set_rgb(200, 10, 10); //red-pink
			break;
		}
		case 8:
		{
			set_rgb(0, 0, 255); //blue
			break;
		}
		//All halogens are green
		case 9:
		{
			set_rgb(0, 255, 0); //green 
			break;
		}
		case 17:
		{
			set_rgb(0, 255, 0); //green
			break;
		}
		case 35:
		{
			set_rgb(0, 255, 0); //green
			break;
		}
		case 53:
		{
			set_rgb(0, 255, 0); //green
			break;
		}
		default:
		printf("No such element");
	}
}

void broadcastChemID(Atom ID)
{
	//send the character array associated with this atom to all nearby droplets
	//For now, it needs to go to every droplet on the board. Later, possibly change that.
	//global_Atom_str = (char*)(&ID);
	//printf("\r\n broadcastChemID called \r\n");
	//uint8_t r=get_red_led(), g=get_green_led(), b=get_blue_led();
	//set_rgb(255,255,255);	
	ir_send(ALL_DIRS, (char*)(&ID), sizeof(Atom));
	delay_ms(100);
	//set_rgb(r,g,b);
	//printf("sending chem ID \r\n");
}

void sendChemID(Atom ID, uint8_t channels[])
{
	global_Atom_str = (char*)(&ID);
	ir_send(channels[0], (char*)(&ID), sizeof(Atom));
	ir_send(channels[1], (char*)(&ID), sizeof(Atom));
}

uint8_t valenceState() //returns 0 if empty, 2 if full, 1 if in between
{
	uint8_t state = 0;
	for(uint8_t i = 0; i < 8; i++)
	{
		if(myID.valence[i] != 0) state++; 
	}
	if(state == 8) return 2;
	else if(state == 0) return 0;
	else return 1;
}

void detectOtherDroplets()
{
	//printf("detectOtherDroplets called \r\n");
	uint16_t received_id;
	float received_range;
	float received_bearing;
	float received_heading;
	uint8_t dir_mask = check_collisions();
	//printf("Collisions: %02hX \r\n", dir_mask);
	
	if(rnb_updated)
	{
		printf("***********************************rnb_updated is true\r\n");
		received_id = last_good_rnb.id_number;
		received_range = last_good_rnb.range;
		received_bearing = last_good_rnb.bearing;
		received_heading = last_good_rnb.heading;
		//convert to degrees from radians
		received_bearing = rad_to_deg(received_bearing);
		received_heading = rad_to_deg(received_heading);
		//scaling the range to mm.
		received_range = received_range*10;
		//printf("range: %f\r\n", received_range);
		uint8_t i;
		for(i = 0; i < 12; i++)
		{
			if(near_atoms[i].id == received_id)
			{
				near_atoms[i].bearing = (int16_t)received_bearing;
				near_atoms[i].heading = (int16_t)received_heading;
				near_atoms[i].range = (uint8_t)received_range;
			}
		}
		checkPossibleBonds(near_atoms[i]);
		print_near_atoms();
		broadcastChemID(myID);
		rnb_updated=0;
	}
}

void formDiatomicBond(uint16_t senderID, Atom near_atom, uint8_t my_empty, uint8_t other_empty)
{
	printf("\tEntered diatomic if statement.\r\n");
	unsigned char diatomic[9];
	diatomic[0] = 'd';
	//Set bonded flag in near_atoms to true
	for(uint8_t k = 0; k < 12; k++)
	{
		if(near_atoms[k].id == senderID)
		{
			near_atoms[k].bonded = 1;
			break;
		}
	}
	myID.bondType = 2;
	uint8_t empty_slot_counter = other_empty;
	uint8_t free_electron_counter = other_empty;
	uint8_t empty_slot_counter_me = my_empty;
	uint8_t free_electron_counter_me = my_empty;
	//Fill the array diatomic to send to my partner
	for(uint8_t i = 1; i < 9; i++)
	{
		//printf("near_atom.valence[i] = %hd \r\n ", near_atom.valence[i-1]);
		if(near_atom.valence[i-1] == 0 && empty_slot_counter > 0)
		{
			diatomic[i] = 2;
			empty_slot_counter--;
		}
		else if(near_atom.valence[i-1] == 1 && free_electron_counter > 0)
		{
			diatomic[i] = 2;
			free_electron_counter --;
		}
		else diatomic[i] = near_atom.valence[i-1];
	}
	//Change my own valence shell
	for(uint8_t i = 0; i < 8; i++)
	{
		if(myID.valence[i] == 0 && empty_slot_counter_me > 0)
		{
			myID.valence[i] = 2;
			empty_slot_counter_me--;
		}
		else if(myID.valence[i] == 1 && free_electron_counter_me > 0)
		{
			myID.valence[i] = 2;
			free_electron_counter_me --;
		}
	}
	found_diatomic_routine();
	add_to_bonded_atoms(senderID);
	ir_targeted_send(ALL_DIRS, diatomic, 9, senderID);
}

formIonicBond(uint16_t senderID, Atom near_atom)
{
	unsigned char newValence[9];
	newValence[0] = 'i';
	myID.bondType = 1;
	for(uint8_t k = 0; k < 12; k++)
	{
		if(near_atoms[k].id == senderID)
		{
			near_atoms[k].bonded = 1;
			break;
		}
	}
	
	uint8_t zero = 1;
	uint8_t one = 1;
	if(myID.chi > near_atom.chi)
	{
		uint8_t zero = 1;
		uint8_t one = 1;

		//Turn one of my electrons and one of my free slots into bonded electrons, and tell near_atom to remove a free electron
		for(uint8_t i = 0; i < 8; i++)
		{
			if(myID.valence[i] == 0 && zero != 0)
			{
				myID.valence[i] = 2;
				zero--;
			}
			else if(myID.valence[i] == 1 && one != 0)
			{
				myID.valence[i] = 2;
				one--;
			}
		}
		//Fill newValence by copying near_atom's current valence shell into newValence and removing one of its free electrons.
		//newValence starts with the char 'i', so index 0 of near_atom.valence is index 1 of newValence
		if(near_atom.valence[1] == 0 && near_atom.valence[0] == 1) newValence[1] = 0;
		else newValence[1] = near_atom.valence[0];
		for(uint8_t i = 1; i < 8; i++)
		{
			if(near_atom.valence[i] == 0 && near_atom.valence[i-1] == 1) newValence[i-1] = 0;
			else newValence[i+1] = near_atom.valence[i];
		}
		add_to_bonded_atoms(senderID);
		found_bond_routine();
		ir_targeted_send(ALL_DIRS, newValence, 9, senderID);
	}
	else
	{
		uint8_t zero = 1;
		uint8_t one = 1;
		//Fill newValence with modified shell
		for(uint8_t i = 0; i < 8; i++)
		{
			if(near_atom.valence[i] == 0 && zero != 0)
			{
				newValence[i+1] = 2;
				zero--;
			}
			else if(near_atom.valence[i] == 1 && one != 0)
			{
				newValence[i+1] = 2;
				one--;
			}
			else newValence[i+1] = near_atom.valence[i];
		}
		//Take away an electron from me
		for(uint8_t i = 0; i < 8; i++)
		{
			if(myID.valence[i+1] ==0 && myID.valence[i] == 1)
			{
				myID.valence[i] = 0;
				break;
			}
		}
		add_to_bonded_atoms(senderID);
		found_bond_routine();
		ir_targeted_send(ALL_DIRS, newValence, 9, senderID);
	}
}


void makePossibleBonds(Atom near_atom, char flag, uint16_t senderID)
{
	uint8_t myBonds = 0;
	uint8_t otherBonds = 0;
	uint8_t my_empty = 0; //number of empty electron slots in my valence shell
	uint8_t other_empty = 0; //number of empty electron slots in near_atom's valence shell
	uint8_t nearAtomBonded = 0; //Flag that determines if near_atom is already bonded to me, and therefore a bond should form regardless of other checks
	float deltaChi;
	
	if(myID.chi > near_atom.chi) deltaChi = myID.chi - near_atom.chi;
	else deltaChi = near_atom.chi - myID.chi;
	
	for(uint8_t i = 0; i < 8; i++) {
		if(near_atom.valence[i] == 0) other_empty++;
		if(myID.valence[i] == 0) my_empty++;
		if(near_atom.valence[i] == 2) otherBonds++;
		if(myID.valence[i] == 2) myBonds++;
	}
	printf("\tmyBonds is %u and otherBonds is %u \r\n", myBonds/2, otherBonds/2);
	myBonds/=2;
	otherBonds/=2;
	
	//Check to see if this atom thinks he's bonded to me already, or if I'm already bonded to him
	for(uint8_t i = 0; i < 6; i++) {
		if(near_atom.bonded_atoms[i] == get_droplet_id()) nearAtomBonded = 1;
		if(myID.bonded_atoms[i] == senderID) return;
	}
	
	//Check for full valence shell
	if(my_empty == 0 || other_empty == 0) return;
	
	//Diatomic bond
	if(near_atom.diatomic == 1 && near_atom.atomicNum == myID.atomicNum && my_empty != 0 && ((other_empty != 0  && otherBonds == 0) || nearAtomBonded == 1) && myBonds == 0) {
		formDiatomicBond(senderID, near_atom, my_empty, other_empty); 
		return;
	}
	else if(deltaChi >= 1.5) { //Ionic bond
		if(near_atom.bondType == 2 || myID.bondType == 2) {
			printf("near_atom is already bonded covalently, or I am. \r\n");
			return;
		}
		//How do I check if there's more than one type of cation to each anion?
		if(my_empty != 0 && (other_empty != 0 || nearAtomBonded == 1)) {
			formIonicBond(senderID, near_atom);
			return;
		}
	}
}

//Checks to see what kind of bonds the newly discovered near_atom can make with me. 
void checkPossibleBonds(Atom near_atom, uint16_t senderID)
{
	uint8_t myBonds = 0;
	uint8_t otherBonds = 0;
	uint8_t my_empty = 0; //number of empty electron slots in my valence shell
	uint8_t other_empty = 0; //number of empty electron slots in near_atom's valence shell
	uint8_t nearAtomBonded = 0; //Flag that determines if near_atom is already bonded to me, and therefore a bond should form regardless of other checks
	float deltaChi;
	//Set deltaChi
	if(myID.chi > near_atom.chi) deltaChi = myID.chi - near_atom.chi;
	else deltaChi = near_atom.chi - myID.chi;
	//Set number of empty slots and bonds in each valence shell
	for(uint8_t i = 0; i < 8; i++) {
		if(near_atom.valence[i] == 0) other_empty++;
		if(myID.valence[i] == 0) my_empty++;
		if(near_atom.valence[i] == 2) otherBonds++;
		if(myID.valence[i] == 2) myBonds++;
	}
	myBonds/=2;
	otherBonds/=2;
	//Check to see if this atom thinks he's bonded to me already, or if I'm already bonded to him
	for(uint8_t i = 0; i < 6; i++) {
		if(near_atom.bonded_atoms[i] == get_droplet_id()) nearAtomBonded = 1;
		if(myID.bonded_atoms[i] == senderID) return;
	}
	//Check for full or empty valence shell
	if(my_empty == 0 || other_empty == 0 || my_empty == 8 || other_empty == 8) return;
	//Diatomic bond
	if(near_atom.diatomic == 1 && near_atom.atomicNum == myID.atomicNum && my_empty != 0 && ((other_empty != 0  && otherBonds == 0) || nearAtomBonded == 1) && myBonds == 0) {
		bondDelay = get_time();
		char msg[2] = {'p', 'd'};
		ir_targeted_send(ALL_DIRS, msg, sizeof(msg), senderID);
		return;
	}
	else if(deltaChi >= 1.5)  {  //Ionic bond
		//How do I check if there's more than one type of cation to each anion?
		if(near_atom.bondType == 2 || myID.bondType == 2) { 
			printf("near_atom is already bonded covalently, or I am. \r\n");
			return; 
		}		
		if(my_empty != 0 && (other_empty != 0 || nearAtomBonded == 1)) {
			//formIonicBond(senderID, near_atom);
			bondDelay = get_time();
			char msg[2] = {'p', 'i'};
			ir_targeted_send(ALL_DIRS, msg, sizeof(msg), senderID);
			return;
		}
	}
}

void repairBondedAtoms()
{
	uint8_t broken = 0;
	if(myID.bonded_atoms[1] != 0)
		printf("Out of place or too many bonds\r\n"); //REMEMBER TO REMOVE THIS LINE AFTER CURRENT BUG IS FIXED
	for(uint8_t i = 0; i < 6; i++)
	{
		for(uint8_t j = i; j < 6; j++)
		{
			if(myID.bonded_atoms[i] == myID.bonded_atoms[j] && i != j && myID.bonded_atoms[i] != 0 && myID.bonded_atoms[j] != 0)
			{
				printf("ERROR: Bonded atoms has a repeated atom. \r\n");
				broken = 1;
				break;
			}
		}
		if(broken == 1) break;
	}
}

void repairValence()
{
	for(uint8_t i = 0; i < 8; i++)
	{
		if(!(myID.valence[i] == -1 || myID.valence[i] == 0 ||  myID.valence[i] == 1 || myID.valence[i] == 2))
		{
			printf("ERROR: Corrupted valence shell.");
			printValence(myID.valence);
			printf("\r\n");
		}
		if(myID.atomicNum == 3 && i >=1 &&myID.valence[i] != 0)
		{
			printf("ERROR: Too many valence electrons for lithium. \r\n");
		}
	}
}

void add_to_bonded_atoms(uint16_t ID)
{
	uint8_t slotFound = 0;
	for(uint8_t i = 0; i < 6; i++)
	{
		if(myID.bonded_atoms[i] == 0)
		{
			myID.bonded_atoms[i] = ID;
			slotFound = 1;
			break;
		}
		else if(myID.bonded_atoms[i] == ID) 
		{
			printf("ERROR: Tried to add ID to bonded_atoms while it was already there. \r\n");
			set_rgb(255, 0, 255);
			slotFound = 1;
			break;
		}
	}
	if(slotFound == 0) 
	{
		printf("ERROR: Tried to add an ID to bonded_atoms but the array was full. \r\n");
		set_rgb(255, 0, 255);
	}
}

void printValence(int8_t valence[])
{
	printf("\tValence is:\t");
	for(uint8_t i = 0; i < 8; i++)
	{
		printf(" %hd ", valence[i]);
	}
	printf("\r\n");
}

float getChiFromID(uint16_t ID)
{
	for(uint8_t i = 0; i < 12; i++)
	{
		if(near_atoms[i].id == ID)
		{
			return near_atoms[i].atom.chi;
		}
	}
	printf("getChiFromId failed, I'm not forming the bond I was told to form until that atom is in my near_atoms array \r\n");
	return -1.00;
}

Atom getAtomFromID(uint16_t ID)
{
	for(uint8_t i = 0; i < 12; i++) {
		if(near_atoms[i].id == ID) return near_atoms[i].atom;
	}
	return NULL_ATOM;
}

void msgAtom(ir_msg* msg_struct)
{
	printf("\t\t\t Received atom struct \r\n");
	Atom* near_atom;
	near_atom = (Atom*)(msg_struct->msg); //do some kind of check to make sure this is actually an atom.
	
	//If this droplet isn't in our list, add it. If it is, update it.
	uint8_t found = 0;
	uint8_t i;
	for(i = 0; i < 12; i++) {
		if(near_atoms[i].id ==msg_struct->sender_ID) {
			near_atoms[i].last_msg_t = 0;
			found = 1;
			break;
		}
	}
	if(bondDelay == 0) checkPossibleBonds(*near_atom, msg_struct->sender_ID);
	if (found == 0) { //add new droplet to near_atoms 
		Near_Atom close_atom = {*near_atom, msg_struct->sender_ID, 0, 0, 0, 0, 0};
		add_to_near_atoms(close_atom);
	}
	else if(found == 1) {
		for(uint8_t j = 0; j < 6; j++) near_atoms[i].atom.bonded_atoms[j] = near_atom->bonded_atoms[i];
		for(uint8_t j = 0; j < 8; j++) near_atoms[i].atom.valence[j] = near_atom->valence[j];
		near_atoms[i].atom.bondType = near_atom->bondType;
		near_atoms[i].last_msg_t = 0;
	}
}

void msgDiatomic(ir_msg* msg_struct)
{
	printf("Got 'diatomic bond made' message.\r\n");
	uint8_t bondAlreadyExists = 0;
	for(uint8_t i = 0; i < 6; i++) {
		if(myID.bonded_atoms[i] == msg_struct->sender_ID) bondAlreadyExists = 1;
	}
	float senderChi = getChiFromID(msg_struct->sender_ID);
	float deltaChi;
	if(myID.chi < senderChi) deltaChi = senderChi-myID.chi;
	else deltaChi = myID.chi-senderChi;
	if(deltaChi >= 1.5 && myID.bondType == 2) return;
	else if (deltaChi < 1.5 && myID.bondType == 1) return;
	else if(bondAlreadyExists == 1) return;
	else {
		found_diatomic_routine();
		myID.bondType = 2;
		for(uint8_t i = 0; i < 8; i++) myID.valence[i] = msg_struct->msg[i+1];
		//printf("After receiving the 'bond found' message, valence is: ");
		//printValence(myID.valence);
		add_to_bonded_atoms(msg_struct->sender_ID);
		uint8_t found = 0;
		for(uint8_t i = 0; i < 12; i++) {
			if(near_atoms[i].id == msg_struct->sender_ID) {
				found = 1;
				near_atoms[i].bonded = 1;
				break;
			}
		}
		if(found == 0) printf("ERROR: Someone tried to bond with me who isn't in my near_atoms array.");
		print_near_atoms();
	}
}

void msgBondMade(ir_msg* msg_struct)
{
	printf("Got 'bond made' message.\r\n");
	uint8_t bondAlreadyExists = 0;
	float senderChi = getChiFromID(msg_struct->sender_ID);
	if (senderChi == -1) return;
	float deltaChi;
	if(myID.chi < senderChi) deltaChi = senderChi-myID.chi;
	else deltaChi = myID.chi-senderChi;
	for(uint8_t i = 0; i < 6; i++)
	{
		if(myID.bonded_atoms[i] == msg_struct->sender_ID) bondAlreadyExists = 1;
	}
	if(deltaChi >= 1.5 && myID.bondType == 2) return;
	else if (deltaChi < 1.5 && myID.bondType == 1) return;
	else if (bondAlreadyExists == 1) return;
	else if (valenceState() == 2 || valenceState() == 0) return;
	else
	{
		found_bond_routine();
		if(msg_struct->msg[0] == 'c') myID.bondType = 2;
		else myID.bondType = 1;
		for(uint8_t i = 0; i < 8; i++)
		{
			myID.valence[i] = msg_struct->msg[i+1];
		}
		//printf("After receiving the 'bond found' message, valence is: ");
		//printValence(myID.valence);
		add_to_bonded_atoms(msg_struct->sender_ID);
		uint8_t found = 0;
		for(uint8_t i = 0; i < 12; i++)
		{
			if(near_atoms[i].id == msg_struct->sender_ID)
			{
				found = 1;
				near_atoms[i].bonded = 1;
				break;
			}
		}
		if(found == 0) printf("ERROR: Someone tried to bond with me who isn't in my near_atoms array.");
		print_near_atoms();
	}
}

void msgPossibleBond(ir_msg* msg_struct)
{
	printf("msgPossibleBond called \r\n");
	Atom senderAtom = getAtomFromID(msg_struct->sender_ID);
	if(senderAtom.atomicNum == 0)
	{
		printf("Atom who wants to bond with me isn't in my near_atoms array \r\n");
		return;
	}
	if(msg_struct->msg[1] == 'd') makePossibleBonds(senderAtom, 'd', msg_struct->sender_ID);
	else if (msg_struct->msg[1] == 'i')  makePossibleBonds(senderAtom, 'i', msg_struct->sender_ID);
}

void msgBondedAtoms(ir_msg* msg_struct)
{
	printf("Received bonded_atoms from %x \r\n", msg_struct->sender_ID);
	//Check to see if I'm bonded to the droplet who just sent me his bonded_atoms array
	uint8_t senderIDFound = 0;
	uint8_t myIdFound = 0;
	uint8_t i;
	uint16_t* recast_bonded_atoms = (uint16_t*)(msg_struct->msg);
	for(i = 0; i < 6; i++)  {
		if(myID.bonded_atoms[i] == msg_struct->sender_ID) {
			printf("He's in my bonded_atoms array \r\n");
			senderIDFound = 1;
			break;
		}
	}
		
	//If so, check to see if he's bonded to me. If he isn't, break my bond.
	for(uint8_t j = 0; j < 6; j++)  {
		if(recast_bonded_atoms[j] == get_droplet_id())  {
			printf("\r\n I'm in his bonded_atoms array \r\n");
			myIdFound = 1;
			break;
		}
	}	
	if(myIdFound == 1 && senderIDFound == 0)  {
		printf("Droplet %x thinks he's bonded to me but I don't think I'm bonded to him. I'm sending my bonded_atoms to try to break the bond.\r\n", msg_struct->sender_ID);
		ir_targeted_send(ALL_DIRS, myID.bonded_atoms, sizeof(myID.bonded_atoms), msg_struct->sender_ID);
	}
	else if(myIdFound == 0 && senderIDFound == 1)
	{
		printf("I think I'm bonded to a droplet who doesn't think he's bonded to me. I'm breaking the bond. \r\n");
		setAtomColor(myID);
		//Set my bondType based on what bonds I have remaining
		//Possible error here if a droplet I'm bonded to isn't in near_atoms, but unlikely.
		uint8_t foundBond = 0;
		for(uint8_t k = 0; k < 6; k++)  {
			if(myID.bonded_atoms[k] != 0)  {
				foundBond = 1;
				float otherChi = getChiFromID(myID.bonded_atoms[k]); 
				if(otherChi != -1)  {
					float deltaChi;
					if(myID.chi < otherChi) deltaChi = otherChi-myID.chi;
					else deltaChi = myID.chi-otherChi;
					if(deltaChi > 1.5) myID.bondType = 1;
					else myID.bondType = 2;
				}
			}
		}
		if (foundBond == 0) myID.bondType == 0;
		//Remove other droplet from bonded_atoms and remove the bonded flag from near_atoms
		myID.bonded_atoms[i] = 0;
		print_near_atoms();
		for(uint8_t k = 0; k < 12; k++)  {
			if(near_atoms[k].id == msg_struct->sender_ID) {
				near_atoms[k].bonded = 0;
				break;
			}
		}
		//Change my valence shell to reflect the absence of that bond.
		//If I'm a cation, I get my electron back:
		if(myID.chi <= 2)  {
			for(uint8_t i = 0; i < 8; i++)  {
				if(myID.valence[i] == 0)  {
					myID.valence[i] = 1;
					break;
				}
			}
		}
		else {   //I'm an anion or covalently bonded, my 2 bonded e's get turned into a free e and an empty space
			int8_t o = 1;
			int8_t z = 1;
			for(int8_t i = 7; i >= 0; i--)  {
				if(myID.valence[i] == 2 && z != 0)  {
					myID.valence[i] = 0;
					z--;
				}
				if(myID.valence[i] == 2 && o != 0)  {
					myID.valence[i] = 1;
					o--;
				}
			}
		}				
	}
}

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	switch(get_droplet_id()){
		case 0xC806: MY_CHEM_ID = 3; break;
		case 0x1F08: MY_CHEM_ID = 3; break;
		case 0x4177: MY_CHEM_ID = 3; break;	
		default:     MY_CHEM_ID = 17; break;
	}
	
	for(uint8_t i = 0; i < 12; i++)
	{
		near_atoms[i] = NULL_NEAR_ATOM;
	}
	//set_rgb(10, 255, 255); //this is a test line
	myID = getAtomFromAtomicNum(MY_CHEM_ID);
	schedule_periodic_task(300, update_near_atoms, NULL);
	enable_leg_status_interrupt();
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */

void loop()
{
	delay_ms(LOOP_PERIOD);
	//broadcastChemID(myID);
	
	uint32_t time_floor = ((get_time()/LOOP_PERIOD));
	if((time_floor%(DETECT_OTHER_DROPLETS_PERIOD/LOOP_PERIOD))==0){
		detectOtherDroplets();
	}
	if((get_time()-bondDelay) > 1000) bondDelay = 0;	
	if((time_floor%(RNB_BROADCAST_PERIOD/LOOP_PERIOD))==0){
		//printf("\r\n sent bonded_atoms\r\n");
		broadcast_rnb_data();
		ir_send(ALL_DIRS, (char*)(myID.bonded_atoms), 12); 
	}
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */

void handle_msg(ir_msg* msg_struct)
{
	printf("Beginning handle_msg.\r\n");
	printValence(myID.valence);
	repairValence();
	//printf("Bonded atoms are: \r\n");
	print_near_atoms();
	repairBondedAtoms();
	uint8_t r=get_red_led(), g=get_green_led(), b=get_blue_led();
	if(msg_struct->length==0)  printf("ERROR: Message length 0.\r\n");
	
	//Message is an Atom struct
	if(msg_struct->length==sizeof(Atom))  msgAtom(msg_struct);
	//Message is that a diatomic bond was formed
	else if(msg_struct->msg[0] == 'd')  msgDiatomic(msg_struct);
	//Message is that a bond was formed
	else if(msg_struct->msg[0] == 'c' || msg_struct->msg[0] == 'i')  msgBondMade(msg_struct);
	//Message is a possible bond
	else if(msg_struct->msg[0] == 'p' && bondDelay == 0)  msgPossibleBond(msg_struct);
	//Message is another Droplet's bonded_atoms array. T
	else if(msg_struct->length == sizeof(myID.bonded_atoms)) msgBondedAtoms(msg_struct);
	
	printf("Ending handle_msg.\r\n");
	printValence(myID.valence);
	repairValence();
	//printf("Bonded atoms are: \r\n");
	print_near_atoms();
	repairBondedAtoms();
}



