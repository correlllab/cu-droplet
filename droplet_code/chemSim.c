#include "chemSim.h"

uint8_t MY_CHEM_ID; 
char* global_Atom_str;

//This is a list of Atoms. Should they be static or constant or something?
//Variables are valence shell, current bonded atoms, Mulliken electronegativity, symbol, diatomic, atomic number
Atom H =  {{1, 0, -1, -1, -1, -1, -1, -1}, {0,0,0,0,0,0}, 2.25, "H", 0, 1, 1};
Atom He = {{1, 1, -1, -1, -1, -1, -1, -1}, {0,0,0,0,0,0}, 3.49, "He", 0, 0, 2};
Atom Li = {{1, 0, 0, 0, 0, 0, 0, 0},{0,0,0,0,0,0}, 0.97, "Li", 0, 0, 3};
Atom Be = {{1, 0, 1, 0, 0, 0, 0, 0}, {0,0,0,0,0,0}, 1.54, "Be", 0, 0, 4};
Atom C =  {{1, 0, 1, 0, 1, 0, 1, 0}, {0,0,0,0,0,0}, 2.48, "C", 0, 0, 6};
Atom N =  {{1, 1, 1, 0, 1, 0, 1, 0}, {0,0,0,0,0,0}, 2.90, "N", 0, 1, 7};
Atom O =  {{1, 1, 1, 1, 1, 0, 1, 0}, {0,0,0,0,0,0}, 3.41, "O", 0, 1, 8};
Atom F =  {{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0,0,0}, 3.91, "F", 0, 1, 9};
Atom Na = {{1, 0, 0, 0, 0, 0, 0, 0}, {0,0,0,0,0,0}, 0.91, "Na", 0, 0, 11};
Atom Mg = {{1, 0, 1, 0, 0, 0, 0, 0}, {0,0,0,0,0,0}, 1.37, "Mg", 0, 0, 12};
Atom Cl = {{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0,0,0}, 3.10, "Cl", 0, 1, 17};
Atom Br = {{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0,0,0}, 2.95, "Br", 0, 1, 35};
Atom I =  {{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0,0,0}, 2.74, "I", 0, 1, 53};


void print_near_atoms()
{
	printf("PRINTING NEAR_ATOMS: \r\n");
	for(uint8_t i = 0; i < 12; i++)  {
		if(near_atoms[i].id==0) break;
		printf("Atom: %c%c  Rng: %hu ID: %04X \r\n", near_atoms[i].atom.name[0],near_atoms[i].atom.name[1], near_atoms[i].range, near_atoms[i].id);
	}
	
	printValence(myID.valence);
	//printf("\r\n");
	//printf("\r\n");
	printf("\tBonded droplets: ");
	uint8_t any_bonded=0;
	for(uint8_t i = 0; i < 6; i++)  {
		if(myID.bonded_atoms[i])  {
			any_bonded=1;
			printf("%04X, ", myID.bonded_atoms[i]);
		}
	}
	if(any_bonded)	printf("\b\b\r\n");
	else			printf("None\r\n");
	printf("My_orbitals: \r\n");
	for(uint8_t i = 0; i < 6; i++) printf("Orbital number %hu,  type %hd, occupied by %04X \r\n", i, my_orbitals[i].type, my_orbitals[i].ID);
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
			return NULL_ATOM;
	}
}

void found_bond_routine(char flag)
{
	uint8_t r = get_red_led();
	uint8_t g = get_green_led();
	uint8_t b = get_blue_led();
	switch(flag)
	{
		case 'i':
			set_rgb(255, 0, 255);
			delay_ms(300);
			set_rgb(255,255,255);
			delay_ms(300);
			set_rgb(255, 0, 255);
			delay_ms(300);
			set_rgb(255,0,0);
			delay_ms(300);
			break;
		case 'd':
			set_rgb(255, 0, 0);
			delay_ms(300);
			setAtomColor(myID);
			delay_ms(300);
			set_rgb(255, 0, 0);
			delay_ms(300);
			set_rgb(0, 0, 255);
			break;
		case 'c':
			set_rgb(255, 0, 255);
			delay_ms(300);
			set_rgb(255,255,255);
			delay_ms(300);
			set_rgb(255, 0, 255);
			delay_ms(300);
			set_rgb(0,0,255);
			delay_ms(300);
			break;
	}
	set_rgb(r, g, b);
	
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
	if(get_time()-last_chem_ID_broadcast<MIN_INTER_CHEM_ID_BROADCAST_DELAY)	return;
	last_chem_ID_broadcast = get_time();	
	//send the character array associated with this atom to all nearby droplets
	//For now, it needs to go to every droplet on the board. Later, possibly change that.
	//global_Atom_str = (char*)(&ID);
	printf("\r\n broadcastChemID called \r\n");

	//uint8_t r=get_red_led(), g=get_green_led(), b=get_blue_led();
	//set_rgb(255,255,255);
	ir_send(ALL_DIRS, (char*)(&ID), sizeof(Atom));
	//delay_ms(100);
	//set_rgb(r,g,b);
	//printf("sending chem ID \r\n");
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

	uint8_t dir_mask = check_collisions();
	//printf("Collisions: %02hX \r\n", dir_mask);
}

void modify_valences_ionic(char* newValence, Atom near_atom, uint16_t senderID)  
{
	newValence[0] = 'i';
	myID.bondType = 1;
	if(myID.chi > near_atom.chi)
	{
		uint8_t zero = 1;
		uint8_t one = 1;

		//Turn one of my electrons and one of my free slots into bonded electrons, and tell near_atom to remove a free electron
		for(uint8_t i = 1; i < 8; i+=2)
		{
			if(myID.valence[i] == 0 && myID.valence[i-1] == 1) {
				myID.valence[i] = 2;
				myID.valence[i-1] = 2;
				break;
			}
		}
		//Fill newValence by copying near_atom's current valence shell into newValence and removing one of its free electrons.
		//newValence starts with the char 'i', so index 0 of near_atom.valence is index 1 of newValence
		for(uint8_t i = 0; i < 8; i++) newValence[i+1] = near_atom.valence[i];
		for(int8_t i = 7; i >= 0; i-=2)
		{
			if(near_atom.valence[i] == 0 && near_atom.valence[i-1] == 1)  {
				newValence[i] = 0;
				break;
			}
		}
	}
	else
	{
		uint8_t zero = 1;
		uint8_t one = 1;
		//Fill newValence with modified shell
		for(uint8_t i = 0; i < 8; i++) newValence[i+1] = near_atom.valence[i];
		for(uint8_t i = 0; i < 7; i+=2)
		{
			if(near_atom.valence[i] == 1 && near_atom.valence[i+1] == 0)  {
				newValence[i+1] = 2;
				newValence[i+2] = 2;
				break;
			}
		}
		//Take away an electron from me
		for(int8_t i = 7; i > 0; i-=2)
		{
			if(myID.valence[i] ==0 && myID.valence[i-1] == 1)
			{
				myID.valence[i-1] = 0;
				break;
			}
		}
	}
	add_to_my_orbitals(senderID, 1);
	newValence[0] = 'i';
}

void modify_valences_covalent(char* newValence, Atom near_atom, uint16_t senderID)
{
	uint8_t my_bonds = 0;
	volatile uint8_t other_bonds = 0;
	volatile uint8_t total_bonds = 0;
	uint8_t type = 0;
	myID.bondType = 2;
	//Figure out how many bonds we can make
	for(uint8_t i = 0; i < 7; i+=2)  {
		if(near_atom.valence[i] == 1 && near_atom.valence[i+1] == 0)	other_bonds++;
		if(myID.valence[i] == 1 && myID.valence[i+1] == 0)				my_bonds++;
	}
	if(my_bonds < other_bonds) total_bonds = my_bonds;
	else total_bonds = other_bonds;
	
	if(total_bonds == 1)		type = 2;
	else if (total_bonds == 2)	type = 3;
	else if (total_bonds == 4)	total_bonds = 3; //even if two atoms have the orbitals to make 4 bonds, they won't.
	else if (total_bonds == 3)	type = 4;
	//Update equal numbers of free electrons and empty slots to be bonded in my valence shell
	for(uint8_t i = 0; i < 7; i+=2)  {
		if(myID.valence[i] == 1 && myID.valence[i+1] == 0 && total_bonds > 0)  {
			myID.valence[i] = type;
			myID.valence[i+1] = type;
			total_bonds--;
		}
	}
	if(my_bonds < other_bonds) total_bonds = my_bonds;
	else total_bonds = other_bonds;
	//Update free electrons and empty slots to be bonded in my partner's valence shell
	for(uint8_t i = 1; i < 9; i++) newValence[i] = near_atom.valence[i-1];
	for(uint8_t i = 0; i < 7; i+=2)  {
		if(near_atom.valence[i] == 1 && near_atom.valence[i+1] == 0 && total_bonds > 0)  {
			printf("newValence[%hu] was %hu", i, newValence[i]);
			newValence[i+1] = type;
			newValence[i+2] = type;
			total_bonds--;
			printf(" and now is %hu \r\n", newValence[i]);
			
		}
	}
	if(my_bonds < other_bonds) total_bonds = my_bonds;
	else total_bonds = other_bonds;
	add_to_my_orbitals(senderID, total_bonds);
	newValence[0] = 'c';
}

void formBond(uint16_t senderID, Atom near_atom, char flag)
{
	if(global_blink_timer > get_droplet_id() || global_blink_timer == 0) global_blink_timer = get_droplet_id();
	printf("\r\n global_blink is %X in a form bond function \r\n", global_blink_timer);
	bonded_atoms_delay = 0;
	uint8_t my_bonds = 0;
	uint8_t other_bonds = 0;
	uint8_t total_bonds = 0;
	//Update near_atom's bonded flag
	for(uint8_t k = 0; k < 12; k++)  {
		if(near_atoms[k].id == senderID)  {
			near_atoms[k].bonded = 1;
			break;
		}
	}
	add_to_bonded_atoms(senderID);
	found_bond_routine('i');
	char newValence[9];	
	if(flag == 'i')			modify_valences_ionic(newValence, near_atom, senderID);
	else if(flag == 'c')	modify_valences_covalent(newValence, near_atom, senderID);
	printValence(newValence);
	ir_targeted_send(ALL_DIRS, newValence, 9, senderID);
}

/* Flag will be either 'i' for ionic bond, 'c' for covalent bond, or 'n' for "not called because of a possible bond message"
*/
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
	printf("\t In makePossibleBonds, myBonds is %u and otherBonds is %u \r\n", myBonds/2, otherBonds/2);
	myBonds/=2;
	otherBonds/=2;
	
	//Check to see if this atom thinks he's bonded to me already, or if I'm already bonded to him
	for(uint8_t i = 0; i < 6; i++) {
		if(near_atom.bonded_atoms[i] == get_droplet_id()) nearAtomBonded = 1;
		//if(myID.bonded_atoms[i] == senderID)  return; //this may not work when bonding gets more complicated
	}
	
	//Check for full or empty valence shell
	//Should this be replaced by a check for an empty orbital?
	if(my_empty == 0 || other_empty == 0 || my_empty == 8 || other_empty == 8) return;
	
	if(flag == 'i') { //Ionic bond
		if(near_atom.bondType == 2 || myID.bondType == 2) {
			printf("near_atom is already bonded covalently, or I am. \r\n");
			return;
		}
		//How do I check if there's more than one type of cation to each anion?
		if(my_empty != 0 && (other_empty != 0 || nearAtomBonded == 1)) {
			formBond(senderID, near_atom, 'i');
			return;
		}
	}
	else if (flag == 'c') {
		if(near_atom.bondType == 1 || myID.bondType == 1) {
			printf("near_atom is already bonded ionically, or I am. \r\n");
			return;
		}
		if(my_empty != 0 && (other_empty != 0 || nearAtomBonded == 1) && myID.chi > 1.70) {
			formBond(senderID, near_atom, 'c');
			return;
		}
	}
	else if(flag == 'n')  {
		if(deltaChi >= 1.5)  {
			//How do I check if there's more than one type of cation to each anion?
			if(near_atom.bondType == 2 || myID.bondType == 2) {
				printf("near_atom is already bonded covalently, or I am. \r\n");
				return;
			}
			if(other_empty != 0 || nearAtomBonded == 1) {
				bondDelay = get_time();
				potentialPartner = senderID;
				char msg[2] = {'p', 'i'};
				ir_targeted_send(ALL_DIRS, msg, sizeof(msg), senderID);
				return;
			}
		}
		else if (deltaChi < 1.5 && myID.chi > 1.70)  {   //Covalent bond
			if(near_atom.bondType == 1 || myID.bondType == 1) {
				printf("near_atom is already bonded ionically, or I am. \r\n");
				return;
			}
			if(other_empty != 0 || nearAtomBonded == 1) {
				bondDelay = get_time();
				potentialPartner = senderID;
				char msg[2] = {'p', 'c'};
				ir_targeted_send(ALL_DIRS, msg, sizeof(msg), senderID);
				return;
			}
		}
	}
}

void repairBondedAtoms()
{
	uint8_t broken = 0;
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
	uint8_t two = 0;
	uint8_t one = 0;
	uint8_t zero = 0;
	uint8_t n_one = 0;
	for(uint8_t i = 0; i < 8; i++)
	{
		if(myID.valence[i] == 2) two++;
		else if(myID.valence[i] == 1) one++;
		else if (myID.valence[i] == 0) zero++;
		else if (myID.valence[i] == -1) n_one ++;
		if(myID.valence[i] > 5 || myID.valence[i] < -1)
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
	
	//This is a horrible hacky fix. I can't find the place where "bonded" isn't being reset properly. This should be removed when that place is found.
	if(two > 0)  {
		if(myID.chi > 1.70) myID.bondType = 2;
		else myID.bondType = 1;
	}
	else myID.bondType = 0;
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

void add_to_my_orbitals(uint16_t ID, uint8_t num_bonds) 
{
	uint8_t i;
	for(i = 0; i < 6; i++)  {
		if(my_orbitals[i].ID == 0 && num_bonds > 0)  {
			my_orbitals[i].ID = ID;
			num_bonds--;
		}
	}
	char msg[2] = {'o', i};
	//ir_targeted_send(ALL_DIRS, msg, 2, ID);
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
	printf("ERROR: getChiFromId failed, I'm not forming the bond I was told to form until that atom is in my near_atoms array \r\n");
	return -1.00;
}

Atom getAtomFromID(uint16_t ID)
{
	for(uint8_t i = 0; i < 12; i++) {
		if(near_atoms[i].id == ID) return near_atoms[i].atom;
	}
	printf("\t\t ERROR: getAtomFromID failed \r\n");
	return NULL_ATOM;
}

void initOrbitals(Atom atom)
{
	uint8_t orbitals = 0;
	uint8_t free_orbitals = 0;  //orbitals w/ unpaired e-s
	uint8_t n_one = 0;
	uint8_t zero = 0;
	uint8_t one = 0;
	for(uint8_t i = 0; i < 8; i++)  {
		if(atom.valence[i] == 0) zero++;
		if(atom.valence[i] == 1) one++;
		if(atom.valence[i] == -1) n_one++;
	}
	for(uint8_t i = 0; i < 7; i+=2)  {
		if(atom.valence[i+1] == 0 && atom.valence[i] == 1)  {
			free_orbitals++;
			orbitals++;
		}
		else if(atom.valence[i+1] == 1 && atom.valence[i] == 1) orbitals++;
	}
	//Fill my_orbitals
	for(uint8_t i = 0; i < 6; i++) my_orbitals[i].ID = 0;
	for(uint8_t i = 4; i < 6; i++) my_orbitals[i].type = -1;
	switch(orbitals)  {
		case 4:
			for(uint8_t i = 0; i < 4; i++) my_orbitals[i].type = 4;
			break;
		case 3:
			for(uint8_t i = 0; i < 3; i++) my_orbitals[i].type = 3;
			my_orbitals[3].type = -1;
			break;
		case 2:
			for(uint8_t i = 0; i < 2; i++) my_orbitals[i].type = 2;
			my_orbitals[2].type = -1;
			my_orbitals[3].type = -1;
			break;
		case 1:
			my_orbitals[0].type = 1;
			for(uint8_t i = 1; i < 4; i++) my_orbitals[i].type = -1;
	}
}

void getOrbitals(Atom atom)
{
	uint8_t orbitals = 0;
	uint8_t free_orbitals = 0;  //orbitals w/ unpaired e-s
	uint8_t n_one = 0;
	uint8_t zero = 0;
	uint8_t one = 0;
	uint8_t two = 0;
	uint8_t three = 0;
	uint8_t four = 0;
	for(uint8_t i = 0; i < 8; i++)  {
		if(atom.valence[i] == 0) zero++;
		if(atom.valence[i] == 1) one++;
		if(atom.valence[i] == 2) two++;
		if(atom.valence[i] == -1) n_one++;
		if(atom.valence[i] == 3) three++;
		if(atom.valence[i] == 4) four++;
	}
	//The following code counts the "electron domains" on the atom.
	//See ch. 7.4, Chemistry Atoms First Vol. 1 (or google "electron domain")
	orbitals+= four/6; //triple bonds (represented as fours in the valence shell) have six electrons per single domain
	orbitals+= three/4; //double bonds (represented by threes) have four electrons per single domain
	orbitals += two/2; //single bonds (twos) have two electrons per domain
	for(uint8_t i = 0; i < 7; i+=2)  {
		if(atom.valence[i+1] == 0 && atom.valence[i] == 1)  {
			free_orbitals++;
			orbitals++;
		}
		else if(atom.valence[i+1] == 1 && atom.valence[i] == 1) orbitals++;
	}
	uint16_t angle;
	switch(orbitals)  {
		case 4:
			angle = 180;
			break;
		case 3:
			angle = 120;
			break;
		case 2:
			angle = 90;
			break;
	}
}

void msgAtom(ir_msg* msg_struct)
{
	printf("\t\t\t Received atom struct from %x \r\n", msg_struct->sender_ID);
	Atom* near_atom;
	near_atom = (Atom*)(msg_struct->msg); //do some kind of check to make sure this is actually an atom.
	for(uint8_t i = 0; i < 8; i++)  {
		if(near_atom->valence[i] < -1 || near_atom->valence[i] > 4)  {
			printf("In msgAtom, msg received that wasn't an atom \r\n");
			return;
		}
	}
	//If this droplet isn't in our list, add it. If it is, update it.
	uint8_t found;
	found = 0;
	uint8_t i;
	for(i = 0; i < 12; i++) {
		if(near_atoms[i].id ==msg_struct->sender_ID) {
			near_atoms[i].last_msg_t = 0;
			found = 1;
			break;
		}
	}		
	if (found == 0) { //add new droplet to near_atoms 
		Near_Atom close_atom = {*near_atom, msg_struct->sender_ID, 0, 0, 0, 0, 0};
		add_to_near_atoms(close_atom);
	}
	else if(found == 1) {
		if(bondDelay == 0 || potentialPartner == msg_struct->sender_ID)		
		for(uint8_t j = 0; j < 6; j++) near_atoms[i].atom.bonded_atoms[j] = near_atom->bonded_atoms[i];
		for(uint8_t j = 0; j < 8; j++) near_atoms[i].atom.valence[j] = near_atom->valence[j];
		near_atoms[i].atom.bondType = near_atom->bondType;
		near_atoms[i].atom.atomicNum = near_atom->atomicNum;
		near_atoms[i].atom.diatomic = near_atom->diatomic;
		near_atoms[i].atom.name[0] = near_atom->name[0];
		near_atoms[i].atom.name[1] = near_atom->name[1];
		near_atoms[i].atom.chi = near_atom->chi;
		near_atoms[i].last_msg_t = 0;
	}
	else
	{
		found = 10;
		printf("ERROR: var found is something weird, %hd \r\n \r\n \r\n", found);
	}
	makePossibleBonds(*near_atom, 'n', msg_struct->sender_ID);
}

void msgBondMade(ir_msg* msg_struct, char flag)
{
	printf("Got 'bond made' message.\r\n");
	uint8_t bondAlreadyExists = 0;
	for(uint8_t i = 0; i < 6; i++)
	{
		if(myID.bonded_atoms[i] == msg_struct->sender_ID) bondAlreadyExists = 1;
	}
	if((flag == 'c' && myID.bondType == 1) || (flag == 'i' && myID.bondType == 2)) return;
	else if (flag == 'i')  {
		if (valenceState() == 2 || valenceState() == 0)  {
			 printf("in msgBondMade, someone tried to bond with me, but I'm already full or empty");
			 return;
		}
		found_bond_routine('i');
		if(msg_struct->msg[0] == 'c') myID.bondType = 2;
		else myID.bondType = 1;
		for(uint8_t i = 0; i < 8; i++)
		{
			if(msg_struct->msg[i+1] > 4 || msg_struct->msg[i+1] < -1) printf("ERROR: In msgBondMade, received corrupted valence shell. Valence[%hu] is %hu \r\n", i, msg_struct->msg[i+1]);
			myID.valence[i] = msg_struct->msg[i+1];
		}
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
		add_to_my_orbitals(msg_struct->sender_ID, 1);
	}
	else if  (flag == 'd')  {
		found_bond_routine('d');
		myID.bondType = 2;
		for(uint8_t i = 0; i < 8; i++)
		{
			if(msg_struct->msg[i+1] > 4 || msg_struct->msg[i+1] < -1)
			printf("ERROR: In msgBondMade, received corrupted valence shell. Valence[%hu] is %hu \r\n", i+1, msg_struct->msg[i+1]);
			delay_ms(10); //this is for printing while debugging only
			myID.valence[i] = msg_struct->msg[i+1];
		}
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
	else if(flag == 'c')  {
		found_bond_routine('c');
		myID.bondType = 2;
		//Count bonds before and after, so I know how many times to add sender to my_orbitals
		uint8_t bonds_before = 0;
		uint8_t bonds_after = 0;
		for(uint8_t i = 0; i < 8; i+=2) if(myID.valence[i] >=2 && myID.valence[i] <= 4) bonds_before++;
		for(uint8_t i = 0; i < 8; i++){
			if(msg_struct->msg[i+1] > 5 || msg_struct->msg[i+1] < -1)  printf("ERROR: In msgBondMade, received corrupted valence shell. Valence[%hu] is %hu \r\n", i+1, msg_struct->msg[i+1]);
			myID.valence[i] = msg_struct->msg[i+1];
		}
		for(uint8_t i = 0; i < 8; i+=2) if(myID.valence[i] >=2 && myID.valence[i] <= 4) bonds_after++;
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
		uint8_t num_bonds = bonds_after - bonds_before;
		add_to_my_orbitals(msg_struct->sender_ID, num_bonds);
	}
	bonded_atoms_delay = 0;
	
}

void msgPossibleBond(ir_msg* msg_struct)
{
	printf("msgPossibleBond called \r\n");
	bonded_atoms_delay = get_time();
	Atom senderAtom = getAtomFromID(msg_struct->sender_ID);
	if(senderAtom.atomicNum == 0)
	{
		printf("Atom who wants to bond with me isn't in my near_atoms array \r\n");
		return;
	}
	if(msg_struct->msg[1] == 'd')  {
		//printf("In msgPossibleBond, bond is d");
		makePossibleBonds(senderAtom, 'd', msg_struct->sender_ID);
	}
	else if (msg_struct->msg[1] == 'i')  {
		//printf("In msgPossibleBond, bond is i");
		makePossibleBonds(senderAtom, 'i', msg_struct->sender_ID);
	}
	else if (msg_struct->msg[1] == 'c')  {
		//printf("In msgPossibleBond, bond is c");
		makePossibleBonds(senderAtom, 'c', msg_struct->sender_ID);
	}
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
			//printf("He's in my bonded_atoms array \r\n");
			senderIDFound = 1;
			break;
		}
	}
		
	//If so, check to see if he's bonded to me. If he isn't, break my bond.
	for(uint8_t j = 0; j < 6; j++)  {
		if(recast_bonded_atoms[j] == get_droplet_id())  {
			//printf("\r\n I'm in his bonded_atoms array \r\n");
			myIdFound = 1;
			break;
		}
	}
	if(myIdFound && senderIDFound)	{
		if(global_blink_timer > recast_bonded_atoms[6] || global_blink_timer == 0)  {
			global_blink_timer = recast_bonded_atoms[6];
		}
	}
	if(myIdFound == 1 && senderIDFound == 0)  {
		printf("Droplet %x thinks he's bonded to me but I don't think I'm bonded to him. I'm sending my bonded_atoms to try to break the bond.\r\n", msg_struct->sender_ID);
		Bonded_Atoms_Msg message;
		for(uint8_t j = 0; j < 6; j++) message.bonded_atoms[j] = myID.bonded_atoms[j];
		message.blink_timer = global_blink_timer;
		ir_targeted_send(ALL_DIRS, (char*)(&message), 14, msg_struct->sender_ID);
	}
	else if(myIdFound == 0 && senderIDFound == 1)
	{
		printf("I think I'm bonded to a droplet who doesn't think he's bonded to me. I'm breaking the bond. \r\n");
		setAtomColor(myID);
		global_blink_timer = 0; //If the atom has multiple bonds, this will be fixed the next time one of its other partners sends its bonded_atoms. Not sure what else to do.
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
				break;
			}
		}
		if (foundBond == 0) myID.bondType = 0;
		//Remove other droplet from bonded_atoms, remove the bonded flag from near_atoms, remove the ID from my_orbitals
		myID.bonded_atoms[i] = 0;
		for(uint8_t k = 0; k < 12; k++)  {
			if(near_atoms[k].id == msg_struct->sender_ID) {
				near_atoms[k].bonded = 0;
				break;
			}
		}
		//Change my valence shell to reflect the absence of that bond.
		//If I'm a cation, I get my electron back:
		if(myID.chi <= 2.0)  {
			for(uint8_t i = 0; i < 8; i++)  {
				if(myID.valence[i] == 0)  {
					myID.valence[i] = 1;
					break;
				}
			}
		}
		/*
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
		}*/
		else  {  //I'm an anion or covalently bonded
			printf("Hit the 'I'm an anion or covalent' statement while removing bond \r\n");
			uint8_t orbital_num = 0;
			uint8_t elec_type = 0;
			for(uint8_t i = 0; i < 6; i++) if(my_orbitals[i].ID == msg_struct->sender_ID) orbital_num++;
			elec_type = orbital_num + 1;
			orbital_num *= 2;
			for(int8_t i = 7; i >=0; i--)  {
				if(myID.valence[i] == elec_type && orbital_num > 0)  {
					if(i%2 == 1) myID.valence[i] = 0;
					else myID.valence[i] = 1;
					orbital_num--;
					printf("removed something \r\n");
				}
			}
		}
		for(uint8_t k = 0; k < 6; k++)  {
			if(my_orbitals[k].ID == msg_struct->sender_ID) {
				my_orbitals[k].ID = 0;
			}
		}
		print_near_atoms();				
	}
}

void msgOrbital(ir_msg* msg_struct)
{
	//TODO: Write this function. Obviously.
}

//uint8_t* orbital_order(uint8_t *valence) 
//{
	//uint8_t n_one = 0;
	//uint8_t zero = 0;
	//uint8_t one = 0;
	//uint8_t two = 0;
	//uint8_t three = 0;
	//uint8_t four = 0;
	//uint8_t new_valence[8];
	//for(uint8_t i = 0; i < 8; i++)  {
		//if(valence[i] == 0)		zero++;
		//if(valence[i] == 1)		one++;
		//if(valence[i] == 2)		two++;
		//if(valence[i] == -1)	n_one++;
		//if(valence[i] == 3)		three++;
		//if(valence[i] == 4)		four++;
		//new_valence[i] = -2;
	//}
	//if(two%2 != 0) printf("ERROR: In orbital_order, number of bonded e-1s not divisible by two. \r\n");
	//for(uint8_t t = 0; t < 8; t++) if(four > 0) new_valence[t] = 4;
	//for(uint8_t t = 0; t < 8; t++) if(three > 0) new_valence[t] = 3;
	//for(uint8_t t = 0; t < 8; t++) if(two > 0) new_valence[t] = 2;
	//for(uint8_t j = 0; j < 8; j+=2) if(one > 0 && new_valence[j] == -2) new_valence[j] = 1;
	//if(one > 0)  {
		//for(uint8_t j = 1; j < 8; j+=2) if(one > 0 && new_valence[j] == -2) new_valence[j] = 1;
	//}
	//for(uint8_t j = 1; j < 8; j+=2) if(zero > 0 && new_valence[j] == -2) new_valence[j] = 0;
	//if(zero > 0)  {
		//for(uint8_t j = 1; j < 8; j+=2) if(zero > 0 && new_valence[j] == -2) new_valence[j] = 0;
	//}
	//for(uint8_t j = 0; j < 8; j++) if(n_one > 0 && new_valence[j] == -2) new_valence[j] = -1;
	//
	//return new_valence;
//}
//
/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	printf("INITIALIZING DROPLET. \r\n");
	switch(get_droplet_id()){
		case 0xC806: MY_CHEM_ID = 1; break;
		case 0x1F08: MY_CHEM_ID = 7; break;
		case 0x4177: MY_CHEM_ID = 3; break;	
		case 0x43BA: MY_CHEM_ID = 7; break;
		default:     MY_CHEM_ID = 7; break;
	}
	
	for(uint8_t i = 0; i < 12; i++)  near_atoms[i] = NULL_NEAR_ATOM;
	myID = getAtomFromAtomicNum(MY_CHEM_ID);
	initOrbitals(myID);
	for(uint8_t i = 0; i < 6; i++)  printf("my_orbitals[i].type = %hd \r\n", my_orbitals[i].type);
	schedule_periodic_task(300, update_near_atoms, NULL);
	enable_leg_status_interrupt();
	bonded_atoms_delay = 0;
	global_blink_timer = 0;
	last_chem_ID_broadcast = 0;
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */

void loop()
{
	delay_ms(LOOP_PERIOD);
	//broadcastChemID(myID);
	if((get_time()-bondDelay) > 1000) {
		bondDelay = 0;
		potentialPartner = 0;	
	}
	if((get_time()-bonded_atoms_delay) > 1000) {
		bonded_atoms_delay = 0;
	}
	uint32_t time_floor = ((get_time()/LOOP_PERIOD));
	if((time_floor%(DETECT_OTHER_DROPLETS_PERIOD/LOOP_PERIOD))==0){
		detectOtherDroplets();
	}	
	if((time_floor%(RNB_BROADCAST_PERIOD/LOOP_PERIOD))==0){
		broadcast_rnb_data();
		Bonded_Atoms_Msg message;
		for(uint8_t j = 0; j < 6; j++) message.bonded_atoms[j] = myID.bonded_atoms[j];
		message.blink_timer = global_blink_timer;
		ir_send(ALL_DIRS, (char*)(&message), sizeof(Bonded_Atoms_Msg)); 
		printf("\r\n\r\n global_blink_timer is %X \r\n \r\n", global_blink_timer);
	}
	//There should be a better way to do this than delay_ms.
	if((global_blink_timer!=0)&&((time_floor%(BLINK_PERIOD/LOOP_PERIOD))==((global_blink_timer/50)%(BLINK_PERIOD/LOOP_PERIOD))))
	//if(time_floor%(BLINK_PERIOD/LOOP_PERIOD)==0)
	{
		uint8_t r = get_red_led();
		uint8_t g = get_green_led();
		uint8_t b = get_blue_led();
		set_rgb(255, 0, 0);
		delay_ms(300);
		set_rgb(r, g, b);
	}
	if(rnb_updated)
	{
		uint16_t received_id = last_good_rnb.id_number;
		float received_range = last_good_rnb.range;
		float received_bearing = last_good_rnb.bearing;
		float received_heading = last_good_rnb.heading;
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
				makePossibleBonds(near_atoms[i].atom, 'n', received_id);
				delay_ms(200); //probably remove this at some point
				//print_near_atoms();
				break;
			}
		}
		broadcastChemID(myID);
		rnb_updated=0;
	}	
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */

void handle_msg(ir_msg* msg_struct)
{
	printf("\nPrinting message of length %u.\r\n\t",msg_struct->length);
	for(uint8_t i=0;i<msg_struct->length;i++)
	{
		printf("%02hX",msg_struct->msg[i]);
	}
	printf("\r\n\n");
	if(msg_struct->length==0)  printf("ERROR: Message length 0.\r\n");
	
	//Message is an Atom struct
	if(msg_struct->length==sizeof(Atom))  
		msgAtom(msg_struct);
	//Message is ionic bond was formed
	else if(msg_struct->msg[0] == 'i') 
		 msgBondMade(msg_struct, 'i');
	//Message is covalent bond formed
	else if(msg_struct->msg[0] == 'c')  
		msgBondMade(msg_struct, 'c');
	//Message is a possible bond
	else if(msg_struct->msg[0] == 'p' && (bondDelay == 0 || potentialPartner == msg_struct->sender_ID))
		msgPossibleBond(msg_struct);
	//Message is another Droplet's bonded_atoms array. T
	else if((msg_struct->length == sizeof(Bonded_Atoms_Msg)) && bonded_atoms_delay == 0) msgBondedAtoms(msg_struct);
	//Message is which orbital on an atom I'm bonded to I should be in (determines my position with relation to the other atom)
	else if(msg_struct->msg[0] == 'o')
		msgOrbital(msg_struct);
	
	printf("Ending handle_msg.\r\n");
	repairValence();
	print_near_atoms();
	repairBondedAtoms();
}

/*
 *	The function below is optional - if it is commented in, and the leg interrupts have been turned on
 *	with enable_leg_status_interrupt(), this function will get called when that interrupt triggers.
 */
void user_leg_status_interrupt()
{
	//printf("user_leg_status_interrupt called\r\n");
	if((get_time() - tap_delay) < 2000 )  {
		uint8_t newNum;
		if(myID.atomicNum == 4) newNum = 6;
		else if(myID.atomicNum == 9) newNum = 17;
		else if(myID.atomicNum == 17) newNum = 35;
		else if(myID.atomicNum == 35) newNum = 53;
		else if(myID.atomicNum == 53) newNum = 1;
		else newNum = myID.atomicNum + 1;
		myID = getAtomFromAtomicNum(newNum);
		setAtomColor(myID);
		for(uint8_t i = 0; i < 12; i++)  {
			if(i < 6) myID.bonded_atoms[i] = 0;
			near_atoms->bonded = 0;
		}
		bonded_atoms_delay = 0;
		global_blink_timer = 0;
		Bonded_Atoms_Msg message;
		for(uint8_t j = 0; j < 6; j++) message.bonded_atoms[j] = myID.bonded_atoms[j];
		message.blink_timer = global_blink_timer;
		initOrbitals(myID);
		ir_send(ALL_DIRS, (char*)(&message), sizeof(Bonded_Atoms_Msg));
	}
	tap_delay = get_time();
}



