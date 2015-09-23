/*
*  Written by Audrey Randall
*  Begun 5/25/15
*/
#include "droplet_programs/chemSim/chemSim.h"
#include "droplet_programs/chemSim/moleculesData.h"

uint8_t MY_CHEM_ID; 
char* global_Atom_str;

enum Atoms_enum{
	H, He, Li, Be, C, N, O, F, Na, Mg, Cl, Br, I
	};

void enable_sync_blink(uint16_t phase_offset_ms){
	main_cca = ((uint16_t)(phase_offset_ms*FFSYNC_MS_CONVERSION_FACTOR)%FFSYNC_FULL_PERIOD);
	TCE0.CCA = main_cca;
	TCE0.INTCTRLB = TC_CCAINTLVL_MED_gc;
	turning_on = 1;
}

void disable_sync_blink(){
	TCE0.INTCTRLB = TC_CCAINTLVL_OFF_gc;
	TCE0.CCA = 0;
}

ISR(TCE0_CCA_vect){
	if(turning_on){
		set_rgb(255,0,0);
		TCE0.CCA = (TCE0.CCA+((uint16_t)(200*FFSYNC_MS_CONVERSION_FACTOR)))%FFSYNC_FULL_PERIOD;
		turning_on = 0;
	}else{
		setAtomColor(&myID);
		TCE0.CCA = main_cca;
		turning_on = 1;
	}
}

//This is a list of Atoms. Should they be static or constant or something?
//Variables are valence shell, current bonded atoms, Mulliken electronegativity, symbol, diatomic, atomic number
Atom PeriodicTable[13]=
{
/*Atom H = */ {{1, 0, -1, -1, -1, -1, -1, -1}, {0,0,0,0,0,0}, 2.25, "H", 0, 1, 1}	,
/*Atom He =*/ {{1, 1, -1, -1, -1, -1, -1, -1}, {0,0,0,0,0,0}, 3.49, "He", 0, 0, 2}	,
/*Atom Li =*/ {{1, 0, 0, 0, 0, 0, 0, 0},{0,0,0,0,0,0}, 0.97, "Li", 0, 0, 3}			,
/*Atom Be =*/ {{1, 0, 1, 0, 0, 0, 0, 0}, {0,0,0,0,0,0}, 1.54, "Be", 0, 0, 4}		,
/*Atom C = */ {{1, 0, 1, 0, 1, 0, 1, 0}, {0,0,0,0,0,0}, 2.48, "C", 0, 0, 6}			,
/*Atom N = */ {{1, 1, 1, 0, 1, 0, 1, 0}, {0,0,0,0,0,0}, 2.90, "N", 0, 1, 7}			,
/*Atom O = */ {{1, 1, 1, 1, 1, 0, 1, 0}, {0,0,0,0,0,0}, 3.41, "O", 0, 1, 8}			,
/*Atom F = */ {{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0,0,0}, 3.91, "F", 0, 1, 9}			,
/*Atom Na =*/ {{1, 0, 0, 0, 0, 0, 0, 0}, {0,0,0,0,0,0}, 0.91, "Na", 0, 0, 11}		,
/*Atom Mg =*/ {{1, 0, 1, 0, 0, 0, 0, 0}, {0,0,0,0,0,0}, 1.37, "Mg", 0, 0, 12}		,
/*Atom Cl =*/ {{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0,0,0}, 3.10, "Cl", 0, 1, 17}		,
/*Atom Br =*/ {{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0,0,0}, 2.95, "Br", 0, 1, 35}		,
/*Atom I = */ {{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0,0,0}, 2.74, "I", 0, 1, 53}	   
};

void pack_valences(uint8_t* packed_shells, int8_t* shells)
{
	uint8_t v_plus[8];
	for(uint8_t i = 0; i < 8; i++) v_plus[i] = (uint8_t)(shells[i]+1);
	packed_shells[0] =   v_plus[0]|(v_plus[1]<<3)|((v_plus[2]<<6)&0xC0);
	packed_shells[1] =   (v_plus[2]>>2)|(v_plus[3]<<1)|(v_plus[4]<<4)|((v_plus[5]<<7)&0x80);
	packed_shells[2] =   (v_plus[5]>>1)|(v_plus[6]<<2)|(v_plus[7]<<5);
	//printf("In pack_valences \r\n");
	//for(uint8_t i = 0; i < 8; i++) printf("shells[%hu] = %hd \r\n", i, shells[i]);
	//for(uint8_t i = 0; i < 3; i++) printf("packed_shells[%hu] = %hu \r\n", i, packed_shells[i]);
}

void unpack_valences(uint8_t* packed_shells, int8_t* shells)
{
	shells[0]= packed_shells[0]&0x7;									
	shells[1]= (packed_shells[0]>>3)&0x7;								
	shells[2]= ((packed_shells[0]>>6)&0x3)|((packed_shells[1]&0x1)<<2);
	shells[3]= (packed_shells[1]>>1)&0x7;								
	shells[4]= (packed_shells[1]>>4)&0x7;								
	shells[5]= ((packed_shells[1]>>7)&0x1)|((packed_shells[2]&0x3)<<1);
	shells[6]= (packed_shells[2]>>2)&0x7;								
	shells[7]= (packed_shells[2]>>5)&0x7;								
	for(uint8_t i=0;i<8;i++)
		shells[i]--;
	//printf("In unpack_valences \r\n");
	//for(uint8_t i = 0; i < 3; i++) printf("packed_shells[%hu] = %hu \r\n", i, packed_shells[i]);
	//for(uint8_t i = 0; i < 8; i++) printf("shells[%hu] = %hd \r\n", i, shells[i]);
}

void init_random_move(uint16_t direc) 
{
	
	uint8_t r = rand_byte();
	if(direc == 0 && r > 150) r/=2;
	else if(direc == 6 || direc == 7) r/=5;
	uint16_t t;
	if(direc == 0) t = rand_byte()*40;
	else t = rand_byte()*10;
	uint16_t newDirec;
	if(direc == 0 && t%2) newDirec = 6;
	else if (direc == 0) newDirec = 7;
	else newDirec = 0;
	uint8_t bonded = 0;
	for(uint8_t i = 0; i < 6; i++) if(myID.bonded_atoms[i] != 0 && myID.bonded_atoms[i] != -1) bonded = 1;
	if(is_moving() == -1 && !bonded)  {
		schedule_task(t+10, init_random_move, (void*)newDirec);
		walk(direc, r);
	}
	else if(!bonded) schedule_task(15, init_random_move, (void*)direc);
	
}

void print_bonded_atoms()
{
	printf_P(PSTR("\tBonded Droplets:\t"));
	uint8_t any_bonded=1;
	for(uint8_t i = 0; i < 6; i++)  {
		printf("%04X, ", myID.bonded_atoms[i]);		
		//if(myID.bonded_atoms[i]!= -1 && myID.bonded_atoms[i] != 0)  {
			//any_bonded=1;
			//printf("%04X, ", myID.bonded_atoms[i]);
		//}
	}
	if(any_bonded)	printf("\b\b\r\n");
	else			printf("None\r\n");
}

//for(uint16_t i=0;i<(6*6*6*6*6-1);i++){
	//printf(%d%d%d%d, i%(6*6*6*6), i%(6*6*6), i%(6*6), i%6);
//}

void add_to_near_atoms(Near_Atom near_atom)
{
	uint8_t isSpace = 0;
	for(uint8_t i = 0; i < 12; i++)
	{
		if(near_atoms[i].id == 0) 
		{
			near_atoms[i] = near_atom; //This is probably okay but if the data structure is getting corrupted take a look at what exactly changes when you make this assignment.
			isSpace = 1;
			break;
		}
	}	
	if (isSpace == 0) printf_P(PSTR("ERROR: No space to add another Droplet \r\n"));
}

//void update_near_atoms()
//{
	//for(uint8_t i = 0; i < 12; i++)
	//{
		//if(near_atoms[i].id != 0 && near_atoms[i].last_msg_t > 10000) //assuming this is in ms
		//{
			//near_atoms[i] = NULL_NEAR_ATOM;
			//printf("Removing %c%c \r\n", near_atoms[i].atom.name[0], near_atoms[i].atom.name[1]);
		//}
		//else if(near_atoms[i].id != 0)
		//{
			//near_atoms->last_msg_t += UPDATE_ATOMS_PERIOD; //assuming update is called every 100 ms
		//}
	//}
//}

Atom* getAtomFromAtomicNum(uint8_t atomicNum)
{
	switch(atomicNum){
		case 1:
		{
			return &PeriodicTable[H];
		}
		case 2:
		{
			return &PeriodicTable[He];
		}
		case 3:
		{
			return &PeriodicTable[Li];
		}
		case 4:
		{
			return &PeriodicTable[Be];
		}
		case 6:
		{
			return &PeriodicTable[C];
		}
		case 7:
		{
			return &PeriodicTable[N];
		}
		case 8:
		{
			return &PeriodicTable[O];
		}
		//All halogens are green
		case 9:
		{
			return &PeriodicTable[F];
		}
		case 17:
		{
			return &PeriodicTable[Cl];
		}
		case 35:
		{
			return &PeriodicTable[Br];
		}
		case 53:
		{
			return &PeriodicTable[I];
		}
		default:
			//printf_P(PSTR("No such element in getAtomFromAtomicNum \r\n"));
			return &NULL_ATOM;
	}
}

void found_bond_routine(char flag)
{
	//uint8_t r = get_red_led();
	//uint8_t g = get_green_led();
	//uint8_t b = get_blue_led();
	set_rgb(0,255,255);
	schedule_task(300, setAtomColor, (void*)&myID);
	//switch(flag)
	//{
		//case 'i':
			//set_rgb(255, 0, 255);
			//delay_ms(300);
			//set_rgb(255,255,255);
			//delay_ms(300);
			//set_rgb(255, 0, 255);
			//delay_ms(300);
			//set_rgb(255,0,0);
			//delay_ms(300);
			//break;
		//case 'd':
			//set_rgb(255, 0, 0);
			//delay_ms(300);
			//setAtomColor(&myID);
			//delay_ms(300);
			//set_rgb(255, 0, 0);
			//delay_ms(300);
			//set_rgb(0, 0, 255);
			//break;
		//case 'c':
			//set_rgb(255, 0, 255);
			//delay_ms(300);
			//set_rgb(255,255,255);
			//delay_ms(300);
			//set_rgb(255, 0, 255);
			//delay_ms(300);
			//set_rgb(0,0,255);
			//delay_ms(300);
			//break;
	//}
	//set_rgb(r, g, b);
	
}

void setAtomColor(Atom* ID)
{
	uint8_t atomicNum = ID->atomicNum;
	switch(atomicNum){
		case 1:
		{
			set_rgb(255, 200, 0); //yellow
			//set_rgb(255, 0, 0);
			break;
		}
		case 2:
		{
			set_rgb(255, 50, 0); //orange
			break;
		}
		case 3:
		{
			set_rgb(100, 100, 255); //purple
			break;
		}
		case 4:
		{
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
		default: printf_P(PSTR("No such element in setAtomColor \r\n"));
	}
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
	if(dir_mask > 0)  {
		//broadcast_rnb_data();
		//char msg = 'h'; //for "hello, are you a Droplet?"
		delay_ms(10);
		//ir_send(dir_mask, msg, 1);
		collided = 1;
		sent_atom_delay = get_time();
	}
	else collided = 0;
}

void modify_valences_ionic(char* newValence, Atom* near_atom_ptr, uint16_t senderID)  
{
	Atom near_atom = *near_atom_ptr;
	newValence[0] = 'i';
	myID.bondType = 1;
	uint8_t found = 0;
	uint8_t index = -1;
	if(myID.chi > near_atom.chi)  {

		//Turn one of my electrons and one of my free slots into bonded electrons, and tell near_atom to remove a free electron
		for(uint8_t i = 1; i < 8; i+=2)  {
			if(myID.valence[i] == 0 && myID.valence[i-1] == 1) {
				myID.valence[i] = 2;
				myID.valence[i-1] = 2;
				if(found == 0) {
					found = 1;
					index = i-1;
				}
				break;
			}
		}
		//Fill newValence by copying near_atom's current valence shell into newValence and removing one of its free electrons.
		//newValence starts with the char 'i', so index 0 of near_atom.valence is index 1 of newValence
		for(uint8_t i = 0; i < 8; i++) newValence[i+1] = near_atom.valence[i];
		for(int8_t i = 7; i >= 0; i-=2)  {
			if(near_atom.valence[i] == 0 && near_atom.valence[i-1] == 1)  {
				newValence[i] = 0;
				break;
			}
		}
	}
	else
	{
		//Fill newValence with modified shell
		for(uint8_t i = 0; i < 8; i++) newValence[i+1] = near_atom.valence[i];
		for(uint8_t i = 0; i < 7; i+=2)  {
			if(near_atom.valence[i] == 1 && near_atom.valence[i+1] == 0)  {
				newValence[i+1] = 2;
				newValence[i+2] = 2;
				break;
			}
		}
		//Take away an electron from me
		for(int8_t i = 7; i > 0; i-=2)  {
			if(myID.valence[i] ==0 && myID.valence[i-1] == 1)  {
				myID.valence[i-1] = 0;
				if(found == 0) {
					found = 1;
					index = i-1;
				}
				break;
			}
		}
	}
	if(!found){
		printf("index not found in modify_valences_ionic.\r\n");
	}
	index/=2;
	add_to_bonded_atoms(senderID, index, 1);
	//add_to_my_orbitals(senderID, 1);
}

void modify_valences_covalent(char* newValence, Atom* near_atom_ptr, uint16_t senderID)
{
	Atom near_atom = *near_atom_ptr;
	//printf("My Valence:\r\n");
	printValence(myID.valence);
	//printf("Their Valence:\r\n");
	printValence(near_atom.valence);
	newValence[0] = 'c';
	uint8_t my_bonds = 0;
	volatile uint8_t other_bonds = 0;
	volatile uint8_t total_bonds = 0;
	//delay_ms(10);
	//printf("Total Bonds: %hu\r\n", total_bonds);
	uint8_t type = 0;
	myID.bondType = 2;
	//Figure out how many bonds we can make
	for(uint8_t i = 0; i < 7; i+=2)  {
		if(near_atom.valence[i] == 1 && near_atom.valence[i+1] == 0)	other_bonds++;
		if(myID.valence[i] == 1 && myID.valence[i+1] == 0)				my_bonds++;
	}
	//for(uint8_t i = 0; i < 4; i++) {
		//if(near_atom.bonded_atoms[i] == get_droplet_id()) other_bonds++;
	//}
	if(my_bonds < other_bonds) total_bonds = my_bonds;
	else total_bonds = other_bonds;
	//total_bonds = 1;
	//delay_ms(10);
	printf("\tTotal Bonds: %hu\r\n", total_bonds);	
	volatile uint8_t total_b_temp = total_bonds;
	
	if(total_bonds == 1)		type = 2;
	else if (total_bonds == 2)	type = 3;
	else if (total_bonds == 4)	total_bonds = 3; //even if two atoms have the orbitals to make 4 bonds, they won't.
	if (total_bonds == 3)	type = 4;
	//delay_ms(10);
	//printf("Total Bonds: %hu\r\n", total_bonds);	
	//printf("For loop (filling my valence shell)\r\n");
	//Update equal numbers of free electrons and empty slots to be bonded in my valence shell
	uint8_t found = 0;
	uint8_t index = -1;
	for(uint8_t i = 0; i < 7; i+=2)  {
		//delay_ms(10);
		//printf("\tTotal Bonds: %hu\r\n", total_bonds);		
		if((myID.valence[i] == 1) && (myID.valence[i+1] == 0) && (total_bonds > 0))  {
			myID.valence[i] = type;
			myID.valence[i+1] = type;
			total_bonds--;
			if(found == 0) {
				found = 1;
				index = i;
			}
			//delay_ms(10);
			//printf("Decrementing total_bonds");
		}
	}
	printValence(myID.valence);
	//if(index == -1) printf_P(PSTR("ERROR: modify_valences_covalent called with no room in my valence shell \r\n"));
	total_bonds = total_b_temp;
	//Update free electrons and empty slots to be bonded in my partner's valence shell
	for(uint8_t i = 1; i < 9; i++) newValence[i] = near_atom.valence[i-1];
	//delay_ms(10);
	//printf("\r\n total_b_temp: %hu\r\n", total_b_temp);	
	printf("\tFor loop (filling newValence)\r\n");	
	for(uint8_t i = 0; i < 7; i+=2)  {
		//delay_ms(10);
		//printf("\tTotal Bonds: %hu\r\n", total_bonds);
		if((near_atom.valence[i] == 1) && (near_atom.valence[i+1] == 0) && (total_bonds > 0))  {
			newValence[i+1] = type;
			newValence[i+2] = type;
			total_bonds--;
			//delay_ms(10);
			printf("\t\tnewValence [i+1] = %hd\r\n", newValence[i+1]);
			printf("\t\tnewValence [i+2] = %hd\r\n", newValence[i+2]);
		}
	}
	total_bonds = total_b_temp;
	//delay_ms(10);
	//printf("\r\nTotal Bonds: %hu\r\n", total_bonds);
	if(!found){ 
		printf("index not found in modify_valences_covalent.\r\n");
	}	
	index /= 2;
	//printf("index = %hu \r\n", index);
	add_to_bonded_atoms(senderID, index, total_bonds);
	if(total_b_temp > 0) add_atom_to_molecule(senderID);	
	//add_to_my_orbitals(senderID, total_bonds);
}

uint8_t add_atom_to_molecule(uint16_t atom_id)
{
	uint8_t i;
	for(i=0;i<my_molecule_length;i++)
	{
		if(my_molecule[i]==atom_id)
			return 0;
	}
	my_molecule[my_molecule_length] = atom_id;
	my_molecule_length++;
	if(my_molecule_length>=15){
		//printf_P(PSTR("ERROR: Molecule too long!"));
	}
	return 1;
}

void remove_atom_from_molecule(uint16_t atom_id)
{
	uint8_t found = 0;
	for(uint8_t i=0;i<my_molecule_length-1;i++){
		if(my_molecule[i]==atom_id||found){
			my_molecule[i]=my_molecule[i+1];
			found=1;
		}
	}
	if(my_molecule[my_molecule_length-1]==atom_id||found){
		my_molecule[my_molecule_length-1]=0;
		found=1;
	}
	if(!found){
		//printf_P(PSTR("Possible error? Tried to remove an atom from my_molecule, but it wasn't there.\r\n"));
	}else{
		my_molecule_length--;
	}
}

void sendTargetedMsg(char* msg, uint8_t msgLength, uint16_t target)
{
	float bearing=0.0;
	for(uint8_t i=0;i<12;i++){
		if(near_atoms[i].id==target){
			bearing = near_atoms[i].bearing;
			break;
		}
	}	
	int8_t bear = (int8_t)ceilf((3.0*bearing)/M_PI);
	uint8_t newDir = ((6-bear)%6);
	//printf("targeted to %04X in dir %hu.\r\n", target, newDir);
	while(!ir_is_available(1<<newDir));
	uint8_t send_successful = ir_targeted_send(1<<newDir, msg, msgLength, target);
	if(!send_successful) printf("Failed to send a targeted message of length %hu.\r\n", msgLength);
}

void formBond(uint16_t senderID, Atom* near_atom, char flag)
{
	printf_P(PSTR("\tCalled formBond \r\n"));
	//Update near_atom's bonded flag
	bonded_atoms_delay = get_time();
	for(uint8_t k = 0; k < 12; k++)  {
		if(near_atoms[k].id == senderID){
			near_atoms[k].bonded = 1;
			break;
		}
	}
	char newValence[9];
	if(flag == 'i')	{
		modify_valences_ionic(newValence, near_atom, senderID);
		found_bond_routine('i');
	}
	else if(flag == 'c')  {
		modify_valences_covalent(newValence, near_atom, senderID);
		found_bond_routine('c');
	}

	for(uint8_t i=0;i<8;i++){
		near_atom->valence[i]=newValence[i+1];
	}
	//stability = calculate_my_stability();
	//update_stability(stability);

	//printValence(newValence);
	
	//printf("About to send formBond message.\r\n");
	//sendTargetedMsg(newValence, 9, senderID);
	//delay_ms(200);
	//transmit_molecule_struct(0, 'm');	
	for(uint8_t i = 0; i < 4; i++)  {
		if(near_atom->bonded_atoms[i] != -1 && near_atom->bonded_atoms[i] != 0 && near_atom->bonded_atoms[i] != get_droplet_id()) add_atom_to_molecule(near_atom->bonded_atoms[i]);
	}
}

/*  Flag will be either 'i' for ionic bond or 'c' for covalent bond
	deltaGother, deltaGother_p, and deltaGother_m will be unpacked from the possible bond message that calls this function.
*/
void makePossibleBonds(Atom* near_atom_ptr, char flag, int16_t deltaGother, int16_t deltaGother_p, int16_t deltaGother_m, uint16_t senderID)
{
	//printf_P(PSTR("makePossibleBonds \r\n"));
	Atom near_atom = *near_atom_ptr;
	uint8_t my_orbs = 0; //number of orbitals that can form bonds in my valence shell
	uint8_t other_orbs = 0; //number of orbitals that can form bonds in near_atom's valence shell
	uint8_t nearAtomBonded = 0; //Flag that determines if near_atom is already bonded to me, and therefore a bond should form regardless of other checks
	//float deltaChi;
	//if(myID.chi > near_atom.chi) deltaChi = myID.chi - near_atom.chi;
	//else deltaChi = near_atom.chi - myID.chi;
	for(uint8_t i = 0; i < 4; i++)  {
		if(near_atom.bonded_atoms[i] == get_droplet_id()) nearAtomBonded = 1;
	}
	
	for(uint8_t i = 0; i < 8; i+=2) {
		if(near_atom.valence[i] == 1 && near_atom.valence[i+1] == 0) other_orbs++;
		if(myID.valence[i] == 1 && myID.valence[i+1] == 0) my_orbs++;
	}

	char IMR_flag;
	uint8_t self_monatomic = 1;
	uint8_t other_monatomic = 1;
	for(uint8_t i = 0; i < 4; i++)  {
		if(myID.bonded_atoms[i] != 0 && myID.bonded_atoms[i] != -1) self_monatomic = 0;	
		if(near_atom.bonded_atoms[i] != 0 && near_atom.bonded_atoms[i] != -1 && near_atom.bonded_atoms[i] != get_droplet_id()) {
			other_monatomic = 0;
			//printf("i = %hu, %X ", i, near_atom.bonded_atoms[i]);
		}
	}
	//printf("self_monatomic: %hu other_monatomic: %hu \r\n", self_monatomic, other_monatomic);
	IMR_flag = IMR_test(&near_atom, deltaGother, deltaGother_p, deltaGother_m, senderID);
	printf_P(PSTR("\tIMR_flag is %c \r\n"), IMR_flag);
	if (IMR_flag == 'x')  {
		printf_P(PSTR("\tBOND NOT FORMED: Delta G was unfavorable. \r\n"));
		return;
	}
	else if(IMR_flag == 'n')  { //If other checks hold, I need to break my current bonds, then bond to sender.
		for(uint8_t i = 0; i < 6; i++) {
			//printf("%x ", myID.bonded_atoms[i]);
			//if(near_atom.bonded_atoms[i] == get_droplet_id()) nearAtomBonded = 1;
			if(myID.bonded_atoms[i] == senderID)  {
				printf_P(PSTR("\tBOND NOT FORMED: sender of atom struct is already in my bonded_atoms. myID.bonded_atoms[i] = %x. senderID is %x. makePossibleBonds. \r\n"), myID.bonded_atoms[i], senderID);
				return; //this may not work when bonding gets more complicated
			}
		}
		//for(uint8_t i = 0; i < 6; i++) {
			//if(near_atom.bonded_atoms[i] == get_droplet_id())  {
				////printf_P(PSTR("BOND NOT FORMED: self is in sender's bonded_atoms but sender is not in self's. A half-formed bond hasn't been corrected yet. \r\n"), myID.bonded_atoms[i], senderID);
				//return;
			//}
		//}
		my_orbs = 0;
		Atom base_state = *(getAtomFromAtomicNum(myID.atomicNum));
		for(uint8_t i = 0; i < 8; i+=2)  {
			if(base_state.valence[i] == 1 && base_state.valence[i+1] == 0) my_orbs++;
		}
		
		//Check for no available orbitals
		if(my_orbs == 0 && (other_orbs == 0 && !nearAtomBonded)) {
			if(my_orbs == 0) printf_P(PSTR("\tBOND NOT FORMED: I have no empty orbitals. makePossibleBonds. \r\n"));
			else printf_P(PSTR("\tBOND NOT FORMED 1: Sender has no empty orbitals. makePossibleBonds. \r\n"));
			return;
		}
		//If neither of these checks are hit, break all my bonds and bond to sender.
		//printf("Breaking all bonds and bonding to sender %04X because IMR_flag = n and orbitals work \r\n", senderID);
		for(uint8_t i = 0; i < 6; i++)  {
			if(myID.bonded_atoms[i] == senderID) {
				printf("SOMETHING VERY WRONG. \r\n");
				return;
			}
			if(myID.bonded_atoms[i] != 0 && myID.bonded_atoms[i] != -1)  {
				break_bond(&near_atom, myID.bonded_atoms[i]);
			}
		}
		
		my_orbs = 0;
		for(uint8_t i = 0; i < 8; i+=2)  {
			if(base_state.valence[i] == 1 && base_state.valence[i+1] == 0) my_orbs++;
		}		
		
		if(flag == 'i') { //Ionic bond
			if(near_atom.bondType == 2 || myID.bondType == 2) {
				if(near_atom.bondType == 2) printf_P(PSTR("\tBOND NOT FORMED: sender is already bonded covalently and I'm ionic. makePossibleBonds. \r\n"));
				else printf_P(PSTR("\tBOND NOT FORMED: I am already bonded covalently and sender is ionic. makePossibleBonds. \r\n"));
				return;
			}
			//How do I check if there's more than one type of cation to each anion?
			if(my_orbs != 0 && (other_orbs != 0 || nearAtomBonded == 1)) {
				formBond(senderID, &near_atom, 'i');
				return;
			}
			else printf("Blargh! \r\n");
		}
		else if (flag == 'c') {
			if(near_atom.bondType == 1 || myID.bondType == 1) {
				if(near_atom.bondType == 1) printf_P(PSTR("\tBOND NOT FORMED: sender is already bonded ionically and I'm covalent. makePossibleBonds. \r\n"));
				else printf_P(PSTR("\tBOND NOT FORMED: I am already bonded ionically and sender is covalent. makePossibleBonds. \r\n"));
				return;
			}
			if(my_orbs != 0 && (other_orbs != 0 || nearAtomBonded == 1)) {
				formBond(senderID, &near_atom, 'c');
				return;
			}
			else printf("Blargh! \r\n");
		}
	}
	else if(IMR_flag == 'f')  {
		for(uint8_t i = 0; i < 6; i++) {
			//printf("%x ", myID.bonded_atoms[i]);
			//if(near_atom.bonded_atoms[i] == get_droplet_id()) nearAtomBonded = 1;
			if(myID.bonded_atoms[i] == senderID)  {
				printf_P(PSTR("\tBOND NOT FORMED: sender of atom struct is already in my bonded_atoms. myID.bonded_atoms[i] = %x. senderID is %x. makePossibleBonds. \r\n"), myID.bonded_atoms[i], senderID);
				return; 
			}
		}
		//for(uint8_t i = 0; i < 6; i++) {
			//if(near_atom.bonded_atoms[i] == get_droplet_id())  {
				////printf_P(PSTR("BOND NOT FORMED: self is in sender's bonded_atoms but sender is not in self's. A half-formed bond hasn't been corrected yet. \r\n"), myID.bonded_atoms[i], senderID);
				//return;
			//}
		//}
		other_orbs = 0;
		Atom o_base_state = *(getAtomFromAtomicNum(near_atom.atomicNum));
		for(uint8_t i = 0; i < 8; i+=2)  {
			if(o_base_state.valence[i] == 1 && o_base_state.valence[i+1] == 0) other_orbs++;
		}
		
		for(uint8_t i=0;i<8;i++){
			near_atom.valence[i] = o_base_state.valence[i];
		}
		
		//Check for no available orbitals
		if(my_orbs == 0 && (other_orbs == 0 || nearAtomBonded == 1)) {
			if(my_orbs == 0) printf_P(PSTR("\tBOND NOT FORMED: I have no empty orbitals. makePossibleBonds. \r\n"));
			else printf_P(PSTR("\tBOND NOT FORMED 2: Sender has no empty orbitals. makePossibleBonds. nearAtomBonded = %hu \r\n"),nearAtomBonded);
			return;
		}
		//Tell other to break all bonds then bond to me.
		//State_Msg msg;
		//create_state_message(&msg, 'm');
		//sendTargetedMsg((char*)(&msg), sizeof(State_Msg), senderID);
		formBond(senderID, &near_atom, flag);
	}
}

void break_and_form_bonds(Atom* near_atom_ptr, uint16_t senderID)
{
	//printf_P(PSTR("break_and_form_bonds \r\n"));
	Atom near_atom = *near_atom_ptr;
	uint8_t my_orbs = 0; //number of orbitals that can form bonds in my valence shell
	uint8_t other_orbs = 0; //number of orbitals that can form bonds in near_atom's valence shell
	uint8_t nearAtomBonded = 0; //Flag that determines if near_atom is already bonded to me, and therefore a bond should form regardless of other checks
	float deltaChi;
	if(myID.chi > near_atom.chi) deltaChi = myID.chi - near_atom.chi;
	else deltaChi = near_atom.chi - myID.chi;
	char flag;
	if(deltaChi > 1.70) flag = 'i';
	else flag = 'c';
	for(uint8_t i = 0; i < 6; i++) {
		//printf("%x ", myID.bonded_atoms[i]);
		//if(near_atom.bonded_atoms[i] == get_droplet_id()) nearAtomBonded = 1;
		if(myID.bonded_atoms[i] == senderID)  {
			//printf_P(PSTR("BOND NOT FORMED: sender of atom struct is already in my bonded_atoms. myID.bonded_atoms[i] = %x. senderID is %x. break_and_form_bonds. \r\n"), myID.bonded_atoms[i], senderID);
			return; //this may not work when bonding gets more complicated
		}
	}
	Atom base_state = *(getAtomFromAtomicNum(myID.atomicNum));
	for(uint8_t i = 0; i < 8; i+=2)  {
		if(base_state.valence[i] == 1 && base_state.valence[i+1] == 0) my_orbs++;
		if(near_atom.valence[i] == 1 && near_atom.valence[i+1] == 0) other_orbs++;
	}
	
	//Check for no available orbitals
	if(my_orbs == 0 || other_orbs == 0) {
		//if(my_orbs == 0) printf_P(PSTR("BOND NOT FORMED: I have no empty orbitals. break_and_form_bonds. \r\n"));
		//else printf_P(PSTR("BOND NOT FORMED: Sender has no empty orbitals. break_and_form_bonds. \r\n"));
		return;
	}
	//If neither of these checks are hit, break all my bonds and bond to sender.
	printf("Breaking all bonds and bonding to sender %04X because break_and_form was called and orbitals work \r\n", senderID);
	for(uint8_t i = 0; i < 6; i++) if(myID.bonded_atoms[i] != 0 && myID.bonded_atoms[i] != -1) break_bond(&near_atom, myID.bonded_atoms[i]);
	
	if(flag == 'i') { //Ionic bond
		if(near_atom.bondType == 2 || myID.bondType == 2) {
			//if(near_atom.bondType == 2) printf_P(PSTR("BOND NOT FORMED: sender is already bonded covalently and I'm ionic. break_and_form_bonds. \r\n"));
			//else printf_P(PSTR("BOND NOT FORMED: I am already bonded covalently and sender is ionic. break_and_form_bonds. \r\n"));
			return;
		}
		//How do I check if there's more than one type of cation to each anion?
		if(my_orbs != 0 && (other_orbs != 0 || nearAtomBonded == 1)) {
			formBond(senderID, &near_atom, 'i');
			return;
		}
	}
	else if (flag == 'c') {
		if(near_atom.bondType == 1 || myID.bondType == 1) {
			//if(near_atom.bondType == 1) printf_P(PSTR("BOND NOT FORMED: sender is already bonded ionically and I'm covalent. break_and_form_bonds. \r\n"));
			//else printf_P(PSTR("BOND NOT FORMED: I am already bonded ionically and sender is covalent. break_and_form_bonds. \r\n"));
			return;
		}
		if(my_orbs != 0 && (other_orbs != 0 || nearAtomBonded == 1) && myID.chi > 1.70) {
			formBond(senderID, &near_atom, 'c');
			return;
		}
	}
}

void sendPossibleBondMsg(Atom* near_atom_ptr, uint16_t senderID)
{
	char one = deltaGself>>8;
	char two = deltaGself&0xFF;
	char three = deltaGself_p>>8;
	char four = deltaGself_p&0xFF;
	char five = deltaGself_m>>8;
	char six = deltaGself_m&0xFF;
	Atom near_atom = *near_atom_ptr;
	float deltaChi = fabsf(myID.chi - near_atom.chi);
	if(deltaChi >= 1.5)  {
		//How do I check if there's more than one type of cation to each anion?
		if(near_atom.bondType == 2 || myID.bondType == 2) {
			//if(near_atom.bondType == 2) //printf_P(PSTR("POSSIBLE BOND MSG NOT SENT: sender %04X is already bonded covalently and I'm ionic. \r\n"), senderID);
			//else //printf_P(PSTR("POSSIBLE BOND MSG NOT SENT: I am already bonded covalently and sender %04X is ionic. \r\n"), senderID);
			return;
		}
		//if(other_orbs != 0 || nearAtomBonded == 1) {
		bondDelay = get_time();
		potentialPartner = senderID;
		char msg[8] = {'p', 'i', one, two, three, four, five, six};
		sendTargetedMsg(msg, 8, senderID);
		return;
		//}
	}
	else if (deltaChi < 1.5 && myID.chi > 1.70)  {   //Covalent bond
		if(near_atom.bondType == 1 || myID.bondType == 1) {
			//if(near_atom.bondType == 1) //printf_P(PSTR("POSSIBLE BOND MSG NOT SENT: sender %04X is already bonded ionically and I'm covalent. \r\n"), senderID);
			//else //printf_P(PSTR("POSSIBLE BOND MSG NOT SENT: I am already bonded ionically and sender %04X is covalent. \r\n"), senderID);
			return;
		}
		//if(other_orbs != 0 || nearAtomBonded == 1) {
		bondDelay = get_time();
		potentialPartner = senderID;
		char msg[8] = {'p', 'c', one, two, three, four, five, six};
		sendTargetedMsg(msg, 8, senderID);
		return;
		//}
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
				//printf_P(PSTR("ERROR: Bonded atoms has a repeated atom. \r\n"));
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
			//printf_P(PSTR("ERROR: Corrupted valence shell."));
			printValence(myID.valence);
			//printf("\r\n");
			break;
		}
		if(myID.atomicNum == 3 && i >=1 &&myID.valence[i] != 0)
		{
			//printf_P(PSTR("ERROR: Too many valence electrons for lithium. \r\n"));
		}
	}
	
	//This is a horrible hacky fix. I can't find the place where "bonded" isn't being reset properly. This should be removed when that place is found.
	if(two > 0)  {
		if(myID.chi > 1.70) myID.bondType = 2;
		else myID.bondType = 1;
	}
	else myID.bondType = 0;
}

void add_to_bonded_atoms(uint16_t ID, uint8_t index, uint8_t num_bonds)
{
	uint8_t slotFound = 0;
	for(uint8_t i = index; i < (num_bonds+index); i++)  {
		if(myID.bonded_atoms[i] == 0)
		{
			myID.bonded_atoms[i] = ID;
			slotFound = 1;
		}
	}
	if(slotFound == 0) 
	{
		delay_ms(10);
		printf_P(PSTR("ERROR: Tried to add an ID to bonded_atoms %hu times at index %hu.  \r\n"), num_bonds, index);
		print_bonded_atoms();
		set_rgb(255, 0, 255);
	}
}

//void add_to_my_orbitals(uint16_t ID, uint8_t num_bonds) 
//{
	//uint8_t i;
	//for(i = 0; i < 6; i++)  {
		//if(my_orbitals[i].ID == 0 && num_bonds > 0)  {
			//my_orbitals[i].ID = ID;
			//num_bonds--;
		//}
	//}
//}

void match_molecule(uint16_t* other_molecule, uint8_t length, uint16_t exclude_id)
{
	//printf_P(PSTR("match_molecule \r\n"));
	for(uint8_t i = 0; i < my_molecule_length; i++)  {
		uint8_t found = 0;
		uint8_t found_2 = 0;
		for(uint8_t j = 0; j < length; j++)  {
			if(my_molecule[i] == other_molecule[j]) found = 1;
			if(my_molecule_stability[i].ID == other_molecule[j]) found_2 = 1;
		}
		if(!found)  {
			for(uint8_t k = i; k < my_molecule_length; k++)  {
				my_molecule[i] = my_molecule[i+1];
			}
			my_molecule[my_molecule_length-1]=0;
			my_molecule_length--;
		}
		if(!found_2)  {
			my_molecule_stability[i].ID = 0;
			my_molecule_stability[i].stability = 0;
			update_stability();
		}
	}
	transmit_molecule_struct(exclude_id, 'r');
}

void update_molecule_two(Atom* near_atom, uint16_t senderID)
{
	//printf("update_molecule_two \r\n");
	uint16_t senderAtomIdx=0;
	for(uint8_t i=0;i<12;i++){
		if(near_atoms[i].id==senderID){
			senderAtomIdx=i;
			break;
		}
	}
	Atom senderAtomBefore = near_atoms[senderAtomIdx].atom;
	for(uint8_t i=0;i<4;i++){
		if(senderAtomBefore.bonded_atoms[i]==0||senderAtomBefore.bonded_atoms[i]==-1 || senderAtomBefore.bonded_atoms[i] == get_droplet_id()) continue;
		uint8_t bondedAtomFound = 0;
		for(uint8_t j=0;j<4;j++){
			if(senderAtomBefore.bonded_atoms[i]==near_atom->bonded_atoms[j]) {
				bondedAtomFound = 1;
				break;
			}
		}
		if(!bondedAtomFound) {
			remove_atom_from_molecule(senderAtomBefore.bonded_atoms[i]); //This atom used to be in the other atom's bonded_atoms, but wasn't.
		}
	}
	
	for(uint8_t i=0;i<4;i++){
		if(near_atom->bonded_atoms[i]!=0 && near_atom->bonded_atoms[i]!=-1) add_atom_to_molecule(near_atom->bonded_atoms[i]);
	}
	
}

void update_molecule(uint16_t* atoms, uint8_t length, uint16_t sender)
{
	//printf("update_molecule. Sender: %x", sender);
	
	for (uint8_t i = 0; i <length; i++){
		add_atom_to_molecule(atoms[i]);
	}
	//uint8_t my_s = calculate_my_stability();
	//update_stability(my_s);
	transmit_molecule_struct(sender, 'm');
}		

void transmit_molecule_struct(uint16_t exclude_id, char flag)
{
	uint8_t msg_length = my_molecule_length*2+1;
	char msg[msg_length];
	msg[0] = flag;
	memcpy((msg+1), my_molecule, msg_length-1);
	//for(uint8_t i = 0; i < my_length; i++) msg[i+1] = my_molecule[i];
	//send new molecule struct to all atoms bonded to me EXCEPT the one who sent me this message. This stops the update from bouncing around infinitely, right?
	//uint32_t pre_time = get_time();
	for(uint8_t i = 0; i < 6; i++) {
		if(myID.bonded_atoms[i] != 0 && myID.bonded_atoms[i] != exclude_id && myID.bonded_atoms[i] != -1)  {
			sendTargetedMsg(msg, msg_length, myID.bonded_atoms[i]);
		}
	}
	//printf("We sent the thing after %lu ms.\r\n", get_time()-pre_time);
}

void print_molecule() 
{
	printf_P(PSTR("\tMy molecule contains:\t"));
	for(uint8_t i = 0; i < my_molecule_length; i++)  {
		printf("%04X ", my_molecule[i]);
	}
	printf("\r\n");
}

void printValence(int8_t valence[])
{
	printf_P(PSTR("\tValence is:\t\t"));
	for(uint8_t i = 0; i < 8; i++)
	{
		printf("%hd ", valence[i]);
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
	//printf_P(PSTR("ERROR: getChiFromId failed, I'm not forming the bond I was told to form until that atom is in my near_atoms array \r\n"));
	return -1.00;
}

Atom* getAtomFromID(uint16_t ID)
{
	for(uint8_t i = 0; i < 12; i++) {
		if(near_atoms[i].id == ID) return &(near_atoms[i].atom);
	}
	//printf_P(PSTR("\t\t ERROR: getAtomFromID failed \r\n"));
	return &NULL_ATOM;
}

void init_bonded_atoms(Atom atom)
{
	//uint8_t orbitals = 0;
	//uint8_t free_orbitals = 0;  //orbitals w/ unpaired e-s
	//uint8_t n_one = 0;
	//uint8_t zero = 0;
	//uint8_t one = 0;
	//for(uint8_t i = 0; i < 8; i++)  {
		//if(atom.valence[i] == 0) zero++;
		//if(atom.valence[i] == 1) one++;
		//if(atom.valence[i] == -1) n_one++;
	//}
	//for(uint8_t i = 0; i < 7; i+=2)  {
		//if(atom.valence[i+1] == 0 && atom.valence[i] == 1)  {
			//free_orbitals++;
			//orbitals++;
		//}
		//else if(atom.valence[i+1] == 1 && atom.valence[i] == 1) orbitals++;
	//}
	//uint8_t null_orbs = 6-orbitals;
	////Fill my_orbitals
	//for(int8_t i = 5; i >= 0; i--)  {
		//if(null_orbs > 0)  {
			//myID.bonded_atoms[i] = -1;
			//null_orbs--;
		//}
		//else myID.bonded_atoms[i] = 0;
	//}
	for(uint8_t i = 0; i < 8; i+=2)  {
		if(myID.valence[i] == 1 && myID.valence[i+1] == 0) myID.bonded_atoms[i/2] = 0;
		else if (myID.valence[i] == 1 && myID.valence[i+1] == 1) myID.bonded_atoms[i/2] = 0;
		else if(myID.valence[i] == -1 && myID.valence[i+1] == -1) myID.bonded_atoms[i/2] = -1;
	}
	myID.bonded_atoms[4] = -1;
	myID.bonded_atoms[5] = -1;
}

//Pre-condition: near_atom.bondType is comprised of the bitmask of bondType (2 least significant bits) and stability (6 most significant bits) as it's transmitted in the state msg
void update_near_atoms(Atom* near_atom, ir_msg* msg_struct)
{
	//printf_P(PSTR("update_near_atoms \r\n"));
	//If this droplet isn't in our list, add it. If it is, update it.
	uint16_t sender_ID = msg_struct->sender_ID;
	uint8_t found;
	found = 0;
	uint8_t i;
	for(i = 0; i < 12; i++) {
		if(near_atoms[i].id == sender_ID) {
			near_atoms[i].last_msg_t = 0;
			found = 1;
			break;
		}
	}		
	if (found == 0) { //add new droplet to near_atoms 
		uint16_t range = 0xFFFF;
		if(msg_struct->range==0xFF) range = 150;
		else if(msg_struct->range==0x01) range = 50;
		else if(msg_struct->range==0)	range = 25;
		Near_Atom close_atom = {*near_atom, sender_ID, range, msg_struct->bearing, msg_struct->heading, 0, 0, 0};
		
		add_to_near_atoms(close_atom);
	}
	else if(found == 1) {
		if(bondDelay == 0 || potentialPartner == sender_ID)		
		for(uint8_t j = 0; j < 6; j++) near_atoms[i].atom.bonded_atoms[j] = near_atom->bonded_atoms[j];
		for(uint8_t j = 0; j < 8; j++) near_atoms[i].atom.valence[j] = near_atom->valence[j];
		near_atoms[i].atom.bondType = (near_atom->bondType)&3;
		near_atoms[i].atom.atomicNum = near_atom->atomicNum;
		near_atoms[i].atom.name[0] = near_atom->name[0];
		near_atoms[i].atom.name[1] = near_atom->name[1];
		near_atoms[i].atom.chi = near_atom->chi;
		near_atoms[i].last_msg_t = 0;
		near_atoms[i].stability = (near_atom->bondType)>>2;
	}
	else
	{
		found = 10;
		//printf_P(PSTR("ERROR: var found is something weird, %hd \r\n \r\n \r\n"), found);
	}
}

void msgBondMade(ir_msg* msg_struct, char flag)
{
	//printf_P(PSTR("msgBondMade. Partner: %x"), msg_struct->sender_ID);
	char* msg = msg_struct->msg;
	uint8_t bondAlreadyExists = 0;
	for(uint8_t i = 0; i < 6; i++)
	{
		if(myID.bonded_atoms[i] == msg_struct->sender_ID) bondAlreadyExists = 1;
	}
	if(bondAlreadyExists)  {
		//printf_P(PSTR("ERROR: In msgBondMade, bond already exists. Calling return. \r\n"));
		return;
	}
	if((flag == 'c' && myID.bondType == 1) || (flag == 'i' && myID.bondType == 2)) {
		//if(flag == 'c' && myID.bondType == 1) //printf_P(PSTR("BOND NOT FORMED: My bond type is ionic  and my partner wants to make a covalent bond. msgBondMade \r\n"));
		//else //printf_P(PSTR("BOND NOT FORMED: My bond type is covalent  and my partner wants to make an ionic bond. msgBondMade \r\n"));
		return;
	}
	else if (flag == 'i')  {
		if (valenceState() == 2 || valenceState() == 0)  {
			 //printf_P(PSTR("BOND NOT FORMED: someone tried to bond with me, but I have no empty orbitals. msgBondMade \r\n"));
			 return;
		}
		found_bond_routine('i');
		myID.bondType = 1;
		uint8_t indSet = 0;
		uint8_t ind = 0;
		uint8_t numBonds = 0;
		for(uint8_t i = 0; i < 8; i++)
		{
			//if((msg[i+1] > 4  && msg[i+1] != 255) || msg[i+1] < -1) printf_P(PSTR("ERROR: In msgBondMade, received corrupted valence shell. Valence[%hu] is %hu \r\n"), i, msg[i+1]);
			if(myID.valence[i] != msg[i+1] && !indSet) {
				ind = i;
				indSet = 1;
				numBonds = msg[i+1]-1;
			}
			myID.valence[i] = msg[i+1];
		}
		ind/=2;
		add_to_bonded_atoms(msg_struct->sender_ID, ind, numBonds);
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
		if(found == 0) printf_P(PSTR("ERROR: Someone tried to bond with me who isn't in my near_atoms array."));
		//add_to_my_orbitals(msg_struct->sender_ID, 1);
	}
	else if(flag == 'c')  {
		found_bond_routine('c');
		myID.bondType = 2;
		//Count bonds before and after, so I know how many times to add sender to my_orbitals
		uint8_t bonds_before = 0;
		uint8_t bonds_after = 0;
		for(uint8_t i = 0; i < 8; i+=2) if(myID.valence[i] >=2 && myID.valence[i] <= 4) bonds_before++;
		uint8_t indSet = 0;
		uint8_t ind = 0;
		uint8_t numBonds = 0;
		for(uint8_t i = 0; i < 8; i++){
			//if((msg[i+1] > 5 && msg[i+1] != 255) || msg[i+1] < -1)  printf_P(PSTR("ERROR: In msgBondMade, received corrupted valence shell. Valence[%hu] is %hu \r\n"), i+1, msg[i+1]);
			if(myID.valence[i] != msg[i+1] && !indSet) {
				ind = i;
				indSet = 1;
				numBonds = msg[i+1]-1;
			}
			myID.valence[i] = msg[i+1];
		}
		ind/=2;
		for(uint8_t i = 0; i < 8; i+=2) if(myID.valence[i] >=2 && myID.valence[i] <= 4) bonds_after++;
		add_to_bonded_atoms(msg_struct->sender_ID, ind, numBonds);
		uint8_t found = 0;
		for(uint8_t i = 0; i < 12; i++) {
			if(near_atoms[i].id == msg_struct->sender_ID) {
				found = 1;
				near_atoms[i].bonded = 1;
				break;
			}
		}
		if(found == 0) printf_P(PSTR("ERROR: Someone tried to bond with me who isn't in my near_atoms array."));
		//uint8_t num_bonds = bonds_after - bonds_before;
		//add_to_my_orbitals(msg_struct->sender_ID, num_bonds);
	}

	//add_atom_to_molecule(msg_struct->sender_ID);
	//transmit_molecule_struct(0, 'm');
	//stability = calculate_my_stability();
	//update_stability(stability);
	if(target_id == 0) target_id = msg_struct->sender_ID;   //Commenting this line can disable movement
	bonded_atoms_delay = 0;
	
}

uint8_t getAtomicNumFromID(uint16_t ID)
{
	if(ID==get_droplet_id()) return myID.atomicNum;
	for(uint8_t i = 0; i < 12; i++)  {
		if(near_atoms[i].id == ID) return near_atoms[i].atom.atomicNum;
	}
	return 0;
}

uint8_t get_filled_orbs()
{
	uint8_t filled_orbs = 0;
	if(myID.atomicNum <= 4 || myID.atomicNum == 11 || myID.atomicNum == 12)  {
		if(myID.valence[1] > 0) filled_orbs++;
	}
	else  {
		for(uint8_t i = 1; i < 8; i+=2)  {
			if(myID.valence[i] > 0) filled_orbs++;
		}
	}
	//printf("In get_filled_orbs, filled_orbs is %hu \r\n", filled_orbs);
	return filled_orbs;
}

uint8_t calculate_my_stability()
{
	uint8_t filled = get_filled_orbs();
	uint8_t total = 0;
	for(uint8_t i = 0; i < 6; i++)  {
		//printf("myID.bonded_atoms[i] = %hu \r\n", myID.bonded_atoms[i]);
		if(myID.bonded_atoms[i] != 0xFFFF) total++;
	}
	uint8_t my_stability = (uint8_t)(((float)(filled))/((float)(total))*64);
	//printf("In calculate_my_stability, filled_orbs is %hu and total is %hu, so MY_STABILITY IS %hu \r\n", filled, total, my_stability);
	if(my_stability == 64) my_stability = 63;
	return my_stability;
}

void update_stability()
{
	//printf_P(PSTR("update_stability \r\n"));
	uint16_t my_s = (uint16_t)calculate_my_stability();
	for(uint8_t i = 1; i < my_molecule_length; i++)  {
		uint8_t current_s = -1;
		for(uint8_t j = 0; j < 12; j++)  {
			if(near_atoms[j].id == my_molecule[i])  {
				current_s = near_atoms[j].stability;
			}
		}
		//printf("\t\t Adding %hu to %u and", current_s, my_s);
		//if(current_s == -1) printf_P(PSTR("ERROR: update_stability couldn't get stability from near_atoms \r\n"));
		my_s+=current_s;
		//printf(" my_s is now %u \r\n", my_s);
	}
	my_s/= my_molecule_length;
	stability = (uint8_t)my_s;
}

int16_t add_atom_to_stability(uint16_t ID, int16_t cur_s, uint8_t cur_s_size)
{
	uint8_t add_s = -1;
	for(uint8_t i = 0; i < 12; i++)  {
		if(near_atoms[i].id == ID) add_s = near_atoms[i].stability;
	}
	int16_t new_s = cur_s*cur_s_size/(cur_s_size+1)+(add_s/cur_s_size+1);
	return new_s;
}

int16_t remove_atom_from_stability(uint16_t ID, int16_t cur_s, uint8_t cur_s_size)
{
	uint8_t remove = -1;
	for(uint8_t i = 0; i < 12; i++)  {
		if(near_atoms[i].id == ID) remove = near_atoms[i].stability;
	}
	int16_t new_s = (cur_s*cur_s_size - remove)/ (cur_s_size-1);
	return new_s;
}

void create_state_message(State_Msg* msg, char flag)
{
	//printf_P(PSTR("create_state_msg \r\n"));
	msg->atomicNum = myID.atomicNum;
	global_blink_timer=0xFFFF;
	uint8_t i;
	for(i = 0; i < 15; i++) {
		if(my_molecule[i]==0) break;
		if(my_molecule[i] < global_blink_timer) {
			global_blink_timer = my_molecule[i];
		}
	}
	if(i==1){ 
		global_blink_timer=0;
		disable_sync_blink();
	}else{
		enable_sync_blink(global_blink_timer);
	}
	msg->blink_timer = global_blink_timer;
	for(uint8_t i = 0; i < 4; i++) msg->bonded_atoms[i] = myID.bonded_atoms[i];
	pack_valences(msg->valence, myID.valence);
	uint8_t my_stability = calculate_my_stability();
	msg->bondType = (myID.bondType&3) | (my_stability<<2);
	msg->msgFlag = flag;	
	msg->molecule_nums[0] = myID.atomicNum;
	for(uint8_t x = 1; x < 15; x++) {
		msg->molecule_nums[x] = 0;
	}
	for(uint8_t k = 1; k < my_molecule_length; k++)  {
		for(int8_t j = 0; j < 12; j++)  {
			//printf("csm: k = %hu && near_atoms[j].atomicNum = %hu \r\n", k, near_atoms[j].atom.atomicNum);
			if(near_atoms[j].id == my_molecule[k])  {
				msg->molecule_nums[k] = near_atoms[j].atom.atomicNum;
				//printf("csm: msg->molecule_nums[k] = %hu && near_atoms[j].atomicNum = %hu \r\n", msg->molecule_nums[k], near_atoms[j].atom.atomicNum);
			}
		}
	}
}

void msgPossibleBond(ir_msg* msg_struct)
{
	for(uint8_t i = 0; i < 4; i++) if(msg_struct->sender_ID == myID.bonded_atoms[i]) return;
	//printf_P(PSTR("msgPossibleBond\r\n"));
	bonded_atoms_delay = get_time();
	Atom* senderAtom = getAtomFromID(msg_struct->sender_ID);
	if(senderAtom->atomicNum == 0)
	{
		//printf_P(PSTR("BOND NOT FORMED: Atom who wants to bond with me isn't in my near_atoms array. msgPossibleBond. \r\n"));
		return;
	}
	int16_t deltaGother = (msg_struct->msg[2])<<8 | msg_struct->msg[3];
	int16_t deltaGother_p = (msg_struct->msg[4])<<8 | msg_struct->msg[5];
	int16_t deltaGother_m = (msg_struct->msg[6])<<8 | msg_struct->msg[7];
	if (msg_struct->msg[1] == 'i')  {
		makePossibleBonds(senderAtom, 'i', deltaGother, deltaGother_p, deltaGother_m, msg_struct->sender_ID);
	}
	else if (msg_struct->msg[1] == 'c')  {
		makePossibleBonds(senderAtom, 'c', deltaGother, deltaGother_p, deltaGother_m, msg_struct->sender_ID);
	}
}

void msgBondedAtoms(Atom* near_atom, uint16_t new_blink, uint16_t sender_ID)
{
	printf_P(PSTR("\tIn msgBondedAtoms. Sender: %04x \r\n"), sender_ID);
	//Check to see if I'm bonded to the droplet who just sent me his bonded_atoms array
	uint8_t senderIDFound = 0;
	uint8_t myIdFound = 0;
	uint8_t i;
	uint8_t monotomic = 1;
	//uint16_t* recast_bonded_atoms = (uint16_t*)(msg_struct->msg);
	//printf("msgBondedAtoms: starting check for sender in myID.bonded_atoms. \r\n");
	for(i = 0; i < 4; i++)  {
		//printf("\tmyID.bonded_atoms[i] = %04X \r\n", myID.bonded_atoms[i]);
		if(myID.bonded_atoms[i] != 0 && myID.bonded_atoms[i] != -1) monotomic = 0;
		if(myID.bonded_atoms[i] == sender_ID) {
			senderIDFound = 1;
			break;
		}
	}
	if(monotomic) {
		for(uint8_t j = 1; j < 15; j++)	my_molecule[j] = 0;
		my_molecule_length=1;
	}
		
	//If so, check to see if he's bonded to me. If he isn't, break my bond.
	//printf("msgBondedAtoms: starting check for self in recast_bonded_atoms. \r\n");
	for(uint8_t j = 0; j < 4; j++)  {
		//printf("\trecast_bonded_atoms[j] = %04X \r\n", recast_bonded_atoms[j]);
		if(near_atom->bonded_atoms[j] == get_droplet_id())  {
			//printf("\t myID found \r\n");
			myIdFound = 1;
			break;
		}
	}
	if(!(myIdFound || senderIDFound)) {
		printf_P(PSTR("\tI'm not bonded to sender and sender isn't bonded to me.\r\n"));
	}
	else if(myIdFound == 1 && senderIDFound == 0 && bonded_atoms_delay == 0)  {
		printf_P(PSTR("BOND ERROR: %x is bonded to me but I'm not bonded to him. Sending bonded_atoms. \r\n"), sender_ID);
		State_Msg message;
		create_state_message(&message, 'b');
		sendTargetedMsg((char*)(&message), sizeof(State_Msg), sender_ID);
	}
	else if(myIdFound == 0 && senderIDFound == 1 && bonded_atoms_delay == 0)
	{
		printf("Breaking bond because I'm bonded to a droplet who isn't bonded to me \r\n");
		break_bond(near_atom, sender_ID);
	}
}

void break_bond(Atom* near_atom, uint16_t sender_ID)
{
	//printf_P(PSTR("break_bond. \r\n"));
	setAtomColor(&myID);
	//global_blink_timer = 0; //If the atom has multiple bonds, this will be fixed the next time one of its other partners sends its bonded_atoms. Not sure what else to do.
	//Set my bondType based on what bonds I have remaining
	//Possible error here if a droplet I'm bonded to isn't in near_atoms, but unlikely.
	uint8_t foundBond = 0;
	for(uint8_t k = 0; k < 6; k++)  {
		if(myID.bonded_atoms[k] != 0 && myID.bonded_atoms[k] != -1)  {
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
	//Remove other droplet from bonded_atoms, remove the bonded flag from near_atoms, remove the ID from my_orbitals, remove from my_molecule
	for(uint8_t k = 0; k < 12; k++)  {
		if(near_atoms[k].id == sender_ID) {
			near_atoms[k].bonded = 0;
			break;
		}
	}
	//Change my valence shell to reflect the absence of that bond.
	uint8_t bond_num = 0;
	uint8_t elec_type = 0;
	uint8_t found = 0;
	uint8_t index = 0;
	for(uint8_t i = 0; i < 6; i++)  {
		if(myID.bonded_atoms[i] == sender_ID)  {
			bond_num++;
			myID.bonded_atoms[i] = 0;
			if(!found) {
				found = 1;
				index = i;
			}
		}
	}
	elec_type = bond_num + 1;
	bond_num *= 2;
	index *= 2;
	for(int8_t i = index; i < 8; i++)  {
		if(myID.valence[i] == elec_type && bond_num > 0)  {
			if(i%2 == 1) myID.valence[i] = 0;
			else myID.valence[i] = 1;
			bond_num--;
		}
	}
	
	if(foundBond==0){
		for(uint8_t i=1;i<my_molecule_length;i++){
			remove_atom_from_molecule(my_molecule[i]);
		}
	}else{
		remove_atom_from_molecule(sender_ID);
		printf("\tIn break_bond, removed sender_ID. Also removing:\t");
		for(uint8_t i = 0; i < 4; i++)  {
			printf("%04X\t", near_atom->bonded_atoms[i]);
			if(near_atom->bonded_atoms[i] != 0 && near_atom->bonded_atoms[i] != -1 && near_atom->bonded_atoms[i] != get_droplet_id()){
				remove_atom_from_molecule(near_atom->bonded_atoms[i]);
			}
		}
		printf("\r\n");
	}	
	//transmit_molecule_struct(0, 'r');
}

void move_to_target(uint16_t rng, float bearing)
{
	int16_t degree_bearing = (int16_t)rad_to_deg(bearing);
	printf_P(PSTR("\tMoving to target. range = %u, bearing = %d\r\n"), rng, degree_bearing);	
	if (rng<(DROPLET_RADIUS*5))  {
		//printf_P(PSTR("Range is too small. Not moving.\r\n"));
	}else{
		if (abs(degree_bearing)<=30)  {
			//printf_P(PSTR("moving forward.\r\n"));
			walk(0, rng);
		}
		else if (abs(degree_bearing)>=150)  {
			//printf_P(PSTR("moving backward.\r\n"));
			walk(3, rng);
		}
		else if (degree_bearing>0)  {
			//printf_P(PSTR("moving ccw.\r\n"));
			walk(7, abs(degree_bearing));
		}
		else if (degree_bearing<0)  {
			//printf_P(PSTR("moving cw.\r\n"));
			walk(6, abs(degree_bearing));
		}
	}
}

void calculate_path(float target, uint16_t range, float bearing)
{
	//Target is the angle of the spot that I need to occupy, from the center of the Droplet I'm moving toward. I'm aiming to be right next to that Droplet at that angle to him. 
	float o_x, o_y;
	o_x = cos(target-M_PI_2)*(DROPLET_RADIUS*20);
	o_y = sin(target-M_PI_2)*(DROPLET_RADIUS*20);
	float s_x, s_y;
	s_x = -cos(bearing-M_PI_2)*range;
	s_y = -sin(bearing-M_PI_2)*range;
	printf_P(PSTR("\trange: %u bearing: %f o_x: %f o_y: %f s_x: %f s_y: %f \r\n"),range, rad_to_deg(bearing), o_x, o_y, s_x, s_y);
	float new_x = o_x + s_x;
	float new_y = o_y + s_y;
	float new_bearing = atan2(new_y, new_x)/*+M_PI_2*/;
	uint16_t new_rng = (uint16_t)hypotf(new_x, new_y);
	printf_P(PSTR("\tnew_rng: %u, new_bearing: %f\r\n"), new_rng, rad_to_deg(new_bearing));	
	move_to_target(new_rng, new_bearing);
}

int16_t convert_stability_to_deltaG(uint8_t stability)  {
	int16_t new_s = stability*3+4500;
	return new_s;
}

void update_delta_Gs(Atom* near_atom, uint16_t sender) {
	uint8_t mc_self[my_molecule_length];
	uint8_t mc_self_p[my_molecule_length+1];
	uint8_t mc_self_m[my_molecule_length-1];
	mc_self[0] = myID.atomicNum;
	mc_self_p[0] = myID.atomicNum;	
	for(uint8_t i = 1; i < my_molecule_length; i++)  {
		if(getAtomicNumFromID(my_molecule[i]) == 0)  {
			//printf_P(PSTR("getAtomicNumFromID failed in update_delta_Gs. DeltaG values not computed. \r\n"));
			return;
		}
		mc_self_p[i] = getAtomicNumFromID(my_molecule[i]);
		mc_self[i] = getAtomicNumFromID(my_molecule[i]);
		mc_self_m[i-1] = getAtomicNumFromID(my_molecule[i]);
	}
	mc_self_p[my_molecule_length] = near_atom->atomicNum;	
	printf("\tIn update_delta_Gs, printing molecule: ");
	for(uint8_t i=0;i<my_molecule_length+1;i++){
		printf("%2hu ", mc_self_p[i]);
	}
	printf("\r\n");	
	
	if(!molecule_search(mc_self, &deltaGself, my_molecule_length))  {
		printf_P(PSTR("\t\tdeltaGself not found.\r\n"));
		//deltaGself = convert_stability_to_deltaG(stability);
		deltaGself = 32767;
	}
	if(!molecule_search(mc_self_p, &deltaGself_p, my_molecule_length+1))	{
		printf_P(PSTR("\t\tdeltaGself_p not found.\r\n"));
		//int16_t s = add_atom_to_stability(sender, stability, my_molecule_length);
		//deltaGself_p = convert_stability_to_deltaG(s);
		deltaGself_p = 32767;
	}
	if(my_molecule_length==1){
		deltaGself_m = 0;
	}else if(!molecule_search(mc_self_m, &deltaGself_m, my_molecule_length-1))  {
		printf_P(PSTR("\t\tdeltaGself_m not found.\r\n"));
		//int16_t n_s = remove_atom_from_stability(sender, stability, my_molecule_length);
		//deltaGself_m = convert_stability_to_deltaG(n_s);
		deltaGself_m = 32767;
	}
}

char IMR_test(Atom* near_atom, int16_t deltaGother, int16_t deltaGother_p, int16_t deltaGother_m, uint16_t sender)
{
	update_delta_Gs(near_atom, sender);
	int32_t deltaGrxn_1;
	int32_t deltaGrxn_2;
	int32_t reactionCalled=0;
	uint8_t flag;
	//uint8_t other_monatomic = 1;
	//uint8_t self_monatomic = 1;
	//for(uint8_t i = 0; i < 4; i++)  {
		//if(near_atom->bonded_atoms[i] != 0 && near_atom->bonded_atoms[i] != -1) other_monatomic = 0;
		//if(myID.bonded_atoms[i] != 0 && myID.bonded_atoms[i] != -1) self_monatomic = 0;
	//}
	//printf("\tReaction one: \r\n\tDeltaGself_p: %d deltaGother_m: %d deltaGself: %d deltaGother: %d \r\n", deltaGself_p, deltaGother_m, deltaGself, deltaGother);
	//printf("\tReaction two: \r\n\tDeltaGself_m: %d deltaGother_p: %d deltaGself: %d deltaGother: %d \r\n", deltaGself_m, deltaGother_p, deltaGself, deltaGother);
	deltaGrxn_1 = ((int32_t)deltaGself_p + (int32_t)deltaGother_m) - ((int32_t)deltaGself + (int32_t)deltaGother);
	//else deltaGrxn_1 = (int32_t)deltaGself_p - ((int32_t)deltaGself + (int32_t)deltaGother);
	deltaGrxn_2 = ((int32_t)deltaGself_m + (int32_t)deltaGother_p) - ((int32_t)deltaGself + (int32_t)deltaGother);
	//else deltaGrxn_2 = (int32_t)deltaGother_p - ((int32_t)deltaGself + (int32_t)deltaGother);
	
	if(deltaGrxn_1<=deltaGrxn_2){
		reactionCalled = deltaGrxn_1;
		flag = 'f';
	}else if(deltaGrxn_2<deltaGrxn_1){
		reactionCalled = deltaGrxn_2;
		flag = 'n';
	}
	
	printf_P(PSTR("\tIn IMR_test: DeltaGrxn_1 is %ld, deltaGrxn_2 is %ld. \r\n"), deltaGrxn_1, deltaGrxn_2);
	if(reactionCalled<0){
		return flag;
	}else{
		return 'x';
	}
	
	//if((deltaGrxn_1 < 0 && deltaGrxn_2 > 0) || (deltaGrxn_1 <= deltaGrxn_2 && deltaGrxn_2 < 0)) return 'f'; //for "force reaction"
	//else if (deltaGrxn_2 < 0)  {
		//return 'n'; //for "no forced rxn message should be sent"
	//}
	//else return 'x';  //for "no reaction at all"
}

void get_other_delta_Gs(uint8_t *mc, uint8_t mc_length, int16_t* deltaGother, int16_t* deltaGother_p, int16_t* deltaGother_m)
{
	uint8_t mc_plus[mc_length+1];
	uint8_t mc_minus[mc_length-1];
	for(uint8_t i = 0; i < mc_length; i++) {
		//printf("mc[i] is %hu \r\n", mc[i]);
		mc_plus[i] = mc[i];
		if(i != mc_length-1) mc_minus[i] = mc[i+1];
	}
	mc_plus[mc_length] = myID.atomicNum;
	printf("\tIn get_other_delta_Gs, printing molecule: ");
	for(uint8_t i=0;i<(mc_length+1);i++){
		printf("%2hu ", mc_plus[i]);
	}
	printf("\r\n");
	if(!molecule_search(mc, deltaGother, mc_length))  {
		printf_P(PSTR("\t\tdeltaGother not found. \r\n"));
		*deltaGother = 32767;
		//deltaGother = convert_stability_to_deltaG(stability);
	}
	if(!molecule_search(mc_plus, deltaGother_p, mc_length+1))	{
		printf_P(PSTR("\t\tdeltaGother_p not found. \r\n"));
		//int16_t s = add_atom_to_stability(sender, stability, my_molecule_length);
		//deltaGother_p = convert_stability_to_deltaG(s);
		*deltaGother_p = 32767;
	}
	if(mc_length==1){
		*deltaGother_m = 0;
	}else if(!molecule_search(mc_minus, deltaGother_m, mc_length-1)) {
		printf_P(PSTR("\t\tdeltaGother_m not found. \r\n"));
		//int16_t n_s = remove_atom_from_stability(sender, stability, my_molecule_length);
		//deltaGother_m = convert_stability_to_deltaG(n_s);
		*deltaGother_m = 32767;
	}
}

int comparison(uint8_t* aPtr, uint8_t* bPtr){
	uint8_t a = *aPtr;
	uint8_t b = *bPtr;
	if(a<b){
		return 1;
	}else if(a==b){
		return 0;
	}else if(a>b){
		return -1;
	}
}

uint8_t cleanOtherMolecule(Atom* near_atom, uint8_t* dirtyMolecule, uint8_t* cleanMolecule, uint8_t count){
	uint16_t otherID;
	uint8_t otherFound;
	uint8_t numAtomsRemoved=0;
	for(uint8_t i=0;i<4;i++){
		otherID = near_atom->bonded_atoms[i];
		otherFound=0;
		if(otherID == get_droplet_id()){
			otherFound=1;
		}else if(otherID==0||otherID==-1){
			continue;
		}else{
			for(uint8_t j=0;j<my_molecule_length;j++){
				if(my_molecule[j]==otherID){
					otherFound=1;
					break;
				}
			}
		}
		if(otherFound){
			for(uint8_t j=0;j<count;j++){
				if(dirtyMolecule[j]==getAtomicNumFromID(otherID)){
					cleanMolecule[j]=0;
					numAtomsRemoved++;
					break;
				}
			}
		}
	}
	qsort(cleanMolecule, 15, 1, comparison);
	return numAtomsRemoved;
}

void msgState(ir_msg* msg_struct)
{
	State_Msg state;
	state = *((State_Msg*)(msg_struct->msg));
	Atom near_atom;
	near_atom.atomicNum = state.atomicNum;
	near_atom.bondType = state.bondType; //this still has the stability value in it so that update_near_atoms works as expected
	uint8_t received_stability = (state.bondType)>>2;
	////printf_P(PSTR("*********Received_stability: %hu \r\n"), received_stability);
	unpack_valences(state.valence, near_atom.valence);
	for(uint8_t i = 0; i < 4; i++) near_atom.bonded_atoms[i] = state.bonded_atoms[i];
	Atom* base_atom = getAtomFromAtomicNum(near_atom.atomicNum);
	near_atom.name[0] = base_atom->name[0];
	near_atom.name[1] = base_atom->name[1];
	near_atom.chi = base_atom->chi;
	uint8_t bonded = 0;
	for(uint8_t i = 0; i < 4; i++) {
		if(myID.bonded_atoms[i] == msg_struct->sender_ID) {
			printf("\tIn msgState, bonded to sender, not calling makePoss \r\n");
			bonded = 1;
			break;
		}
	}
	uint8_t count = 0;
	for(uint8_t i = 0; i < 15; i++) if(state.molecule_nums[i] != 0) count++;	
	uint8_t moddedMoleculeNums[15];
	for(uint8_t i=0;i<15;i++){
		moddedMoleculeNums[i]=state.molecule_nums[i];
	}
	uint8_t numAtomsRemovedFromMolecule=0;
	if(bonded){
		update_molecule_two(&near_atom, msg_struct->sender_ID);	
	}else {
		uint8_t other_bonded = 0;
		for(uint8_t i = 0; i < 4; i++) {
			if(near_atom.bonded_atoms[i] == get_droplet_id()) other_bonded = 1;
		}
		if(other_bonded){
			numAtomsRemovedFromMolecule = cleanOtherMolecule(&near_atom, state.molecule_nums, moddedMoleculeNums, count);
			//printf("Finished cleaning. Count: %hu, numRemoved: %hu.\r\n",count, numAtomsRemovedFromMolecule);
		}
	}
	update_near_atoms(&near_atom, msg_struct);

	if(state.msgFlag == 'm') break_and_form_bonds(&near_atom, msg_struct->sender_ID);
	float deltaChi = fabsf(myID.chi - near_atom.chi);
	char flag;
	if(deltaChi > 1.70) flag = 'i';
	else flag = 'c';
	if(!bonded){
		int16_t dGo;
		int16_t dGo_p;
		int16_t dGo_m;
		get_other_delta_Gs(moddedMoleculeNums, count-numAtomsRemovedFromMolecule, &dGo, &dGo_p, &dGo_m);
		makePossibleBonds(&near_atom, flag, dGo, dGo_p, dGo_m, msg_struct->sender_ID);  //sendPossibleBondMsg(&near_atom,  msg_struct->sender_ID);
	}
	uint8_t found = 0;
	for (uint8_t i = 0; i < 15; i++) if(my_molecule[i] == msg_struct->sender_ID) found = 1;
	if(found)  update_stability();
	//printf("****************************************************msgState: bonded_atoms_delay = %lu \r\n", bonded_atoms_delay);
	if(bonded_atoms_delay == 0)  {
		msgBondedAtoms(&near_atom, state.blink_timer, msg_struct->sender_ID);
	}
	update_target_id();
	if(target_id==msg_struct->sender_ID){
		uint16_t mmRange;
		if(msg_struct->range==0xFF)			mmRange = 100;
		else if(msg_struct->range==0x01)	mmRange = 40;
		else								mmRange = 25;
		calculate_target(&near_atom, mmRange, msg_struct->bearing);
	}
}

void update_target_id(){
	uint16_t mostBondsID=0;
	uint8_t mostBondsCount=0;
	uint8_t myBondsCount=0;
	uint8_t otherBondsCount;
	uint8_t myIDFound;
	uint16_t* otherBondedAtoms;
	for(uint8_t i=0;i<4;i++){
		if(myID.bonded_atoms[i]==0||myID.bonded_atoms[i]==-1) continue;
		myBondsCount++;
		otherBondsCount=0;
		for(uint8_t j=0;j<12;j++){
			if(near_atoms[j].id==myID.bonded_atoms[i]){
				otherBondedAtoms = near_atoms[j].atom.bonded_atoms;
				break;
			}
		}
		myIDFound=0;		
		for(uint8_t j=0;j<4;j++){
			if(otherBondedAtoms[j]==0||otherBondedAtoms[j]==-1) continue;
			if(otherBondedAtoms[j]==get_droplet_id()) myIDFound=1;
			otherBondsCount++;
		}
		if(!myIDFound) continue;
		if(otherBondsCount>mostBondsCount){
			mostBondsCount=otherBondsCount;
			mostBondsID=myID.bonded_atoms[i];
		}
	}
	if(myBondsCount>mostBondsCount){
		mostBondsCount=myBondsCount;
		mostBondsID=get_droplet_id();
	}
	if(target_id!=mostBondsID){
		target_bearing_to_me=0;
	}
	target_id = mostBondsID;
}

void calculate_target(Atom* nearAtom, uint16_t range, float bearing)
{
	uint8_t bondingRegions;
	uint8_t myBondingRegion;
	for(uint8_t i=0;i<8;i+=2){
		if((nearAtom->valence[i]==1)&&(nearAtom->valence[i+1]==0)){
			bondingRegions++;
		}else{
			if(nearAtom->valence[i]!=nearAtom->valence[i+1]) printf("ERROR: HORRIBLE ERRORS.\r\n");
			if(nearAtom->bonded_atoms[i/2]==get_droplet_id()) myBondingRegion = i/2;			
			if(nearAtom->valence[i]==1){
				bondingRegions++;
			}else if(nearAtom->valence[i]==2){
				bondingRegions++;
			}else if(nearAtom->valence[i]==3){
				bondingRegions++;
				i+=2;
			}else if(nearAtom->valence[i]==4){
				bondingRegions++;
				i+=4;
			}
		}		
	}
	float anglePerRegion = ((2*M_PI)/bondingRegions);
	float angle = anglePerRegion*myBondingRegion;	
	float heading = pretty_angle(bearing+target_bearing_to_me);
	printf("\tIn calculate_target, numBondingRegions: %hu, myBondingRegion: %hu, angle: %f, heading: %f\r\n",bondingRegions,myBondingRegion,angle, rad_to_deg(heading));	
	calculate_path(angle+heading, range, bearing);
}

uint8_t convert_bearing_to_IR_dir(float bearing)
{
	uint8_t dir_1 = rad_to_deg(bearing)/60;
	uint8_t dir_2 = dir_1+1;
	return dir_1+dir_2;
}

void convert_IR_dir_to_array(uint8_t dirs, uint8_t* bits)
{
	for(uint8_t i = 0; i < 8; i++)  {
		bits[i] = dirs&1;
		dirs = dirs >> 1;
	}
}

uint8_t is_good_rnb(float n_rng, float n_bearing, uint16_t ID)
{
	float p_rng;
	float p_bearing;
	for(uint8_t i = 0; i < 12; i++)  {
		if(near_atoms[i].id == ID)  {
			p_rng = near_atoms[i].range;
			p_bearing = near_atoms[i].bearing;
		}
	}
	uint32_t time_elapsed = get_time()-last_rnb;
	float velocity = 1000*(float)(abs(n_rng-p_rng))/(float)(time_elapsed); //in mm/s 
	float delta_brng = 1000*abs(n_bearing - p_bearing)/time_elapsed; //in rad/s
	if(velocity > 20)  {
		//printf_P(PSTR("Inaccurate RNB data received, judged based on range. Function is_good_rnb returning false. \r\n"));
		return 0;
	}
	else if (delta_brng > 0.3)  {
		//printf_P(PSTR("Inaccurate RNB data received, judged based on bearing (but not range). Function is_good_rnb returning false. \r\n"));
		return 0;
	}
	return 1;
}

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	printf_P(PSTR("INITIALIZING DROPLET. \r\n"));
	switch(get_droplet_id()){
		case 0xC24B: MY_CHEM_ID = 6; break;
		case 0xC806: MY_CHEM_ID = 1; break;
		case 0x0A0B: MY_CHEM_ID = 8; break;
		case 0x1F08: MY_CHEM_ID = 7; break;
		case 0x4177: MY_CHEM_ID = 8; break;
		case 0xB944: MY_CHEM_ID = 8; break;
		case 0x5161: MY_CHEM_ID = 8; break;
		case 0x1B4B: MY_CHEM_ID = 6; break;
		default:     MY_CHEM_ID = 1; break;
	}
	//uint16_t temp = get_droplet_id()%10;
	//if(temp < 6) MY_CHEM_ID = 1;
	//else MY_CHEM_ID = temp;
	//printf("%hu \r\n", sizeof(State_Msg));
	init_atom_state();
	
	//int16_t deltaG;
	//uint8_t atoms[12] = {6,1,1,1,1,8,8,8,8,8,8,8};
	//molecule_search(atoms, &deltaG, 12);
	//printf("Search result: %d\r\n",deltaG);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	delay_ms(LOOP_PERIOD);
	uint32_t time_floor = ((get_time()/LOOP_PERIOD));
	//if(global_blink_timer!=0)
	//{
		//if( (time_floor%(BLINK_PERIOD/LOOP_PERIOD)) == ((global_blink_timer/LOOP_PERIOD)%(BLINK_PERIOD/LOOP_PERIOD)) )
		//{
			//set_rgb(255, 0, 0);
			//schedule_task(300, returnLightToDefault, NULL);
		//}
	//}
	if((get_time()-bondDelay) > 1000) {
		bondDelay = 0;
		potentialPartner = 0;	
	}
	if((get_time()-sent_atom_delay) > 1000) {
		sent_atom_delay = 0;
	}
	if((get_time()-bonded_atoms_delay) > 4000) {
		bonded_atoms_delay = 0;
	}
	if(((time_floor+get_droplet_id())%(CHEM_ID_BROADCAST_PERIOD/LOOP_PERIOD))==0){
		State_Msg message;	
		create_state_message(&message, 'p');
		//printf("message->bonded_atoms[0] is %x, message->atomicNum is %hu \r\n", message.bonded_atoms[0], message.atomicNum);
		if(rand_byte()%3==0)		ir_send(DIR0|DIR3, (char*)(&message), sizeof(State_Msg));
		else if(rand_byte()%2==0)	ir_send(DIR1|DIR4, (char*)(&message), sizeof(State_Msg));
		else						ir_send(DIR2|DIR5, (char*)(&message), sizeof(State_Msg));
	}
	for(uint8_t i=0;i<4;i++){
		uint16_t otherID = myID.bonded_atoms[i];
		if(otherID==0||otherID==-1) continue;
		if(((time_floor+get_droplet_id()+otherID)%(CHEM_ID_BROADCAST_PERIOD/LOOP_PERIOD))==0){
			float bearing;
			for(uint8_t i=0;i<12;i++){
				if(near_atoms[i].id==otherID){
					bearing = near_atoms[i].bearing;
					break;
				}
			}
			int8_t bear = (int8_t)ceilf((3.0*bearing)/M_PI);
			uint8_t newDir = ((6-bear)%6);
			char msg[4] = {'<','3','0','0'};
			int16_t* bearingSpot = (int16_t*)(&msg[2]);
			bearingSpot = (int16_t)rad_to_deg(bearing);
			ir_targeted_send(newDir, msg, 4, otherID);
			break;
		}
	}
	if(((time_floor+get_droplet_id())%(MOLECULE_BROADCAST_PERIOD/LOOP_PERIOD))==0){
		//transmit_molecule_struct(0, 'm');
	}
	//if((global_blink_timer!=0)&&((time_floor%(BLINK_PERIOD/LOOP_PERIOD))==((global_blink_timer/50)%(BLINK_PERIOD/LOOP_PERIOD))))
	////if(time_floor%(BLINK_PERIOD/LOOP_PERIOD)==0)
	//{
		////uint8_t r = get_red_led();
		////uint8_t g = get_green_led();
		////uint8_t b = get_blue_led();
		//set_rgb(255, 0, 0);
		//schedule_task(300, returnLightToDefault, NULL);
		////set_rgb(r, g, b);
	//}
	//if(rnb_updated)
	//{
		//uint16_t received_id = last_good_rnb.id_number;
		//float received_range = last_good_rnb.range;
		//float received_bearing = last_good_rnb.bearing;
		//float received_heading = last_good_rnb.heading;
		////scaling the range to mm.
		//received_range = received_range*10;
		////printf("*****************************************************************range: %f\r\n", received_range);
		//
		//uint8_t i;
		//for(i = 0; i < 12; i++)
		//{
			//if(near_atoms[i].id == received_id /*&&(is_good_rnb(received_range, received_bearing, received_id) || (near_atoms[i].bearing == 0 && near_atoms[i].heading == 0 && near_atoms[i].range == 0))*/)
			//{
				//near_atoms[i].bearing = received_bearing;
				//near_atoms[i].heading = received_heading;
				//near_atoms[i].range = (uint16_t)received_range;
				//if(target_spot != -1) calculate_path(target_spot, target_id);
				////delay_ms(200); //probably remove this at some point
				////print_near_atoms();
				//
				//break;
			//}
		//}
		////broadcastChemID(myID);
		//rnb_updated=0;
		//last_rnb = get_time();
	//}
	//mc_self[0] = myID.atomicNum;
	//for(uint8_t i = 0; i < 6; i++) if(myID.bonded_atoms[i] != 0 && myID.bonded_atoms[i] != -1) bonded = 1;
	//if(is_moving == -1 && !bonded)	init_random_move(0);
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */

void handle_msg(ir_msg* msg_struct)
{
	//printf_P(PSTR("\nHandle_msg: Printing message of length %hu from %04X.\r\n\t"/* at time%%10000: %u.\r\n\t")*/),msg_struct->length, msg_struct->sender_ID/*, (uint16_t)(get_time()%10000)*/);
	//for(uint8_t i=0;i<msg_struct->length;i++)
	//{
		//printf("%02hX",msg_struct->msg[i]);
	//}
	//printf("\r\n\n");
	//printf("********************************global_blink_timer: %04X \r\n", global_blink_timer);
	printf("Got message from: %04X.\r\n", msg_struct->sender_ID);	
	if(msg_struct->length==0)  printf("ERROR: Message length 0.\r\n");
	if(msg_struct->wasTargeted){
		for(uint8_t i=0;i<12;i++){
			if(near_atoms[i].id==msg_struct->sender_ID){
				near_atoms[i].range = msg_struct->range;
				near_atoms[i].bearing = msg_struct->bearing;
				near_atoms[i].heading = msg_struct->heading;
				break;
			}
		}
	}
	
	//Message is the state of another atom
	if(msg_struct->length == sizeof(State_Msg))  {
		msgState(msg_struct);
	}else if(msg_struct->length==4){
		if(msg_struct->msg[0]=='<'&&msg_struct->msg[1]=='3'){		
			if(msg_struct->sender_ID==target_id){
				int16_t intTargetBearingToMe = *((int16_t*)(&(msg_struct->msg[2])));
				target_bearing_to_me = deg_to_rad((float)intTargetBearingToMe);
			}
		}

	}

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
		MY_CHEM_ID = newNum;
		init_atom_state();
		State_Msg message;
		create_state_message(&message, 't'); //t is for "tap code called"		
		ir_send(ALL_DIRS, (char*)(&message), sizeof(State_Msg));
	}
	tap_delay = get_time();
}

void init_atom_state()
{
	for(uint8_t i = 0; i < 12; i++)  near_atoms[i] = NULL_NEAR_ATOM;
	Atom* baseAtom = getAtomFromAtomicNum(MY_CHEM_ID);
	for(uint8_t i=0;i<8;i++){
		myID.valence[i] = baseAtom->valence[i];
	}
	for(uint8_t i=0;i<6;i++){
		myID.bonded_atoms[i] = 0;
	}
	myID.chi = baseAtom->chi;
	myID.name[0] = baseAtom->name[0];
	myID.name[1] = baseAtom->name[1];
	myID.bondType = baseAtom->bondType;	
	myID.diatomic = baseAtom->diatomic;
	myID.atomicNum = baseAtom->atomicNum;	
	//
	//myID = *getAtomFromAtomicNum(MY_CHEM_ID);
	setAtomColor(&myID);
	disable_sync_blink();
	for(uint8_t i = 0; i < 15; i++) my_molecule[i] = 0;
	//schedule_periodic_task(300, update_near_atoms, NULL);
	//enable_leg_status_interrupt();
	bonded_atoms_delay = 0;
	global_blink_timer = 0;
	sent_atom_delay = 0;
	last_rnb = 0;
	last_chem_ID_broadcast = 0;
	target_id = 0;
	target_bearing_to_me=0;
	target_spot = -1;
	my_molecule[0] = get_droplet_id();
	my_molecule_length = 1;	
	collided = 0;
	for(uint8_t i = 0; i < 15; i++)  {
		my_molecule_stability[i].ID = 0;
		my_molecule_stability[i].stability = 0;
		used_in_stability[i] = 0;
	}
	init_bonded_atoms(myID);
	stability = calculate_my_stability();
	//printf_P(PSTR("My stability is: %hu"), stability);	
	//init_random_move(0);
}
