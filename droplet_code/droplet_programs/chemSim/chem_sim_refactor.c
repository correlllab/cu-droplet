/*
 * chem_sim.c
 *
 * Created: 5/10/2016 
 *  Author: Audrey Randall
 */ 
#include "chem_sim_refactor.h"
#include "droplet_programs/chemSim/moleculesData.h"

enum Atoms_enum{
	H, He, Li, Be, C, N, O, F, Na, Mg, Cl, Br, I
};

Atom PeriodicTable[NUM_ELEMENTS]=
{
	{{1, 0, -1, -1, -1, -1, -1, -1}, {0,0,0,0}, 2.25, "H", 1}	,
	{{1, 1, -1, -1, -1, -1, -1, -1}, {0,0,0,0}, 3.49, "He", 2}	,
	{{1, 0, 0, 0, 0, 0, 0, 0},{0,0,0,0}, 0.97, "Li", 3}			,
	{{1, 0, 1, 0, 0, 0, 0, 0}, {0,0,0,0}, 1.54, "Be", 4}		,
	{{1, 0, 1, 0, 1, 0, 1, 0}, {0,0,0,0}, 2.48, "C", 6}			,
	{{1, 1, 1, 0, 1, 0, 1, 0}, {0,0,0,0}, 2.90, "N", 7}			,
	{{1, 1, 1, 1, 1, 0, 1, 0}, {0,0,0,0}, 3.41, "O", 8}			,
	{{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0}, 3.91, "F", 9}			,
	{{1, 0, 0, 0, 0, 0, 0, 0}, {0,0,0,0}, 0.91, "Na", 11}		,
	{{1, 0, 1, 0, 0, 0, 0, 0}, {0,0,0,0}, 1.37, "Mg", 12}		,
	{{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0}, 3.10, "Cl", 17}		,
	{{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0}, 2.95, "Br", 35}		,
	{{1, 1, 1, 1, 1, 1, 1, 0}, {0,0,0,0}, 2.74, "I", 53}
};

uint8_t addToNearAtoms(Near_Atom* near_atom)
{
	for(uint8_t i = 0; i < MAX_NEAR_ATOMS; i++)  {
		if(near_atoms[i].id == 0)  {
			near_atoms[i].atom.atomicNum = near_atom->atom.atomicNum; 
			for(uint8_t j = 0; j < MAX_BONDS; j++) near_atoms[i].atom.bonded_atoms[j] = near_atom->atom.bonded_atoms[j];
			near_atoms[i].atom.chi = near_atom->atom.chi;
			near_atoms[i].atom.name[0] = near_atom->atom.name[0];
			near_atoms[i].atom.name[1] = near_atom->atom.name[1];
			for(uint8_t j = 0; j < 8; j++) near_atoms[i].atom.valence[j]= near_atom->atom.valence[j];
			near_atoms[i].bearing = near_atom->bearing;
			near_atoms[i].heading = near_atom->heading;
			near_atoms[i].range = near_atom->range;
			near_atoms[i].id = near_atom->id;
			return i;
		}
	}
	printf_P(PSTR("ERROR: No space to add another Droplet \r\n"));
	return 0;
}

uint8_t updateNearAtoms(Atom* near_atom, ir_msg* msg_struct)
{
	uint8_t index = 0;
	//If this droplet isn't in our list, add it. If it is, update it.
	uint16_t sender_ID = msg_struct->sender_ID;
	uint8_t found;
	found = 0;
	uint8_t i;
	for(i = 0; i < MAX_NEAR_ATOMS; i++) {
		if(near_atoms[i].id == sender_ID) {
			found = 1;
			index = i;
			break;
		}
	}
	if (found == 0) { //add new droplet to near_atoms
		uint16_t range = 0xFFFF;
		//if(msg_struct->range==0xFF) range = 150;
		//else if(msg_struct->range==0x01) range = 50;
		//else if(msg_struct->range==0)	range = 25;
		/*
		 * Changing this so we initialize with NAN. I don't trust the on-message rnb.
		 */
		Near_Atom close_atom = {*near_atom, NAN, NAN, sender_ID, range};
		return addToNearAtoms(&close_atom);
	}
	else if(found == 1) {
		//if(bondDelay == 0 || potentialPartner == sender_ID) why was this here?
		for(uint8_t j = 0; j < MAX_BONDS; j++) near_atoms[i].atom.bonded_atoms[j] = near_atom->bonded_atoms[j];
		for(uint8_t j = 0; j < 8; j++) near_atoms[i].atom.valence[j] = near_atom->valence[j];
		near_atoms[i].atom.atomicNum = near_atom->atomicNum;
		near_atoms[i].atom.name[0] = near_atom->name[0];
		near_atoms[i].atom.name[1] = near_atom->name[1];
		near_atoms[i].atom.chi = near_atom->chi;
		return index;
	}
	else {
		return 0;
	}
}

void getAtomColor(Atom* ID, uint8_t* r, uint8_t* g, uint8_t* b)
{
	uint8_t atomicNum = ID->atomicNum;
	switch(atomicNum){
		case 1:
		*r = 255;
		*g = 200;
		*b = 0;
		return;
		case 2:
		*r = 255;
		*g = 50;
		*b = 0;
		return;
		case 3:
		*r = 100;
		*g = 100;
		*b = 255;
		return;
		case 4:
		*r = 100;
		*g = 255;
		*b = 100;
		return;
		case 6:
		*r = 100;
		*g = 0;
		*b = 255;
		return;
		case 7:
		*r = 200;
		*g = 10;
		*b = 10;
		return;
		case 8:
		*r = 0;
		*g = 0;
		*b = 255;
		return;
		//All halogens are green
		case 9:
		*r = 0;
		*g = 255;
		*b = 0;
		return;
		case 17:
		*r = 0;
		*g = 255;
		*b = 0;
		return;
		case 35:
		*r = 0;
		*g = 255;
		*b = 0;
		return;
		case 53:
		*r = 0;
		*g = 255;
		*b = 0;
		return;
		default: printf_P(PSTR("No such element in setAtomColor \r\n"));
	}
}

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

uint8_t get_atomic_ord(uint8_t num){
	switch(num){
		case 0: return 0;
		case 1: return 1;
		case 2: return 2;
		case 3: return 3;
		case 4: return 4;
		case 6: return 5;
		case 7: return 6;
		case 8: return 7;
		case 9: return 8;
		case 11: return 9;
		case 12: return 10;
		case 17: return 11;
		case 35: return 12;
		case 53: return 13;
	}
	return 0xFF;
}

uint8_t get_atomic_num_from_ord(uint8_t ord) {
	switch(ord){
		case 0:  return 0;
		case 1:	 return 1;
		case 2:	 return 2;
		case 3:	 return 3;
		case 4:	 return 4;
		case 5:	 return 6;
		case 6:	 return 7;
		case 7:	 return 8;
		case 8:	 return 9;
		case 9:	 return 11;
		case 10: return 12;
		case 11: return 17;
		case 12: return 35;
		case 13: return 53;
	}
	return 0xFF;
}

void getNameFromAtomicNum(char* name, uint8_t atomicNum) {
	
	switch(atomicNum) {
		case 1:
			name[0] = 'H';
			name[1] = ' ';
			break;
		case 2:
			name[0] = 'H';
			name[1] = 'e';
			break;
		case 3:
			name[0] = 'L';
			name[1] = 'i';
			break;
		case 4:
			name[0] = 'B';
			name[1] = 'e';
			break;
		case 6:
			name[0] = 'C';
			name[1] = ' ';
			break;
		case 7:
			name[0] = 'N';
			name[1] = ' ';
			break;
		case 8:
			name[0] = 'O';
			name[1] = ' ';
			break;
		case 9:
			name[0] = 'F';
			name[1] = ' ';
			break;
		case 17:
			name[0] = 'C';
			name[1] = 'l';
			break;
		case 35:
			name[0] = 'B';
			name[1] = 'r';
			break;
		case 53:
			name[0] = 'I';
			name[1] = ' ';
			break;
		default:
			name[0] = ' ';
			name[1] = ' ';
			break;
	}
}

void setAtomColor(Atom* ID)
{
	uint8_t r,g,b;
	getAtomColor(ID, &r, &g, &b);
	set_rgb(r,g,b);
}

/*Pre-condition: myID.valence has been initialized */
void initBondedAtoms(Atom atom)
{
	for(uint8_t i = 0; i < 8; i+=2)  {
		if(myID.valence[i] == 1 && myID.valence[i+1] == 0) myID.bonded_atoms[i/2] = 0;
		else if (myID.valence[i] == 1 && myID.valence[i+1] == 1) myID.bonded_atoms[i/2] = 0;
		else if(myID.valence[i] == -1 && myID.valence[i+1] == -1) myID.bonded_atoms[i/2] = -1;
	}
}

void packValences(uint8_t* packed_shells, int8_t* shells)
{
	uint8_t v_plus[8];
	for(uint8_t i = 0; i < 8; i++) v_plus[i] = (uint8_t)(shells[i]+1);
	packed_shells[0] =   v_plus[0]|(v_plus[1]<<3)|((v_plus[2]<<6)&0xC0);
	packed_shells[1] =   (v_plus[2]>>2)|(v_plus[3]<<1)|(v_plus[4]<<4)|((v_plus[5]<<7)&0x80);
	packed_shells[2] =   (v_plus[5]>>1)|(v_plus[6]<<2)|(v_plus[7]<<5);
}

void unpackValences(uint8_t* packed_shells, int8_t* shells)
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
}

void packMolecule(uint8_t packed_mc[23], MC_Component mc[15]) {
	uint8_t ords[15];
	uint8_t atomicOrds[15];
	for(uint8_t i = 0; i < 15; i++) ords[i] = get_droplet_ord(mc[i].ID); //105 bits
	for(uint8_t i = 0; i < 15; i++) atomicOrds[i] = get_atomic_ord(mc[i].atomicNum); //60 bits
	/*packed_mc[0] = (ords[0]<<1)|(ords[1]>>6);
	packed_mc[1] = (ords[1]<<2)|(ords[2]>>5);
	packed_mc[2] = (ords[2]<<3)|(ords[3]>>4);
	packed_mc[3] = (ords[3]<<4)|(ords[4]>>3);
	packed_mc[4] = (ords[4]<<5)|(ords[5]>>2);
	packed_mc[5] = (ords[5]<<6)|(ords[6]>>1);
	packed_mc[6] = (ords[6]<<7)|(ords[7]);

	packed_mc[7] = (ords[8]<<1)|(ords[9]>>6);
	packed_mc[8] = (ords[9]<<2)|(ords[10]>>5);
	packed_mc[9] = (ords[10]<<3)|(ords[11]>>4);
	packed_mc[10] = (ords[11]<<4)|(ords[12]>>3);
	packed_mc[11] = (ords[12]<<5)|(ords[13]>>2);
	packed_mc[12] = (ords[13]<<6)|(ords[14]>>1);
	packed_mc[13] = (ords[14]<<7)|((atomicOrds[0])<<3)|(atomicOrds[1]>>1);
	packed_mc[14] = (atomicOrds[1]<<7)|(atomicOrds[2]<<3)|(atomicOrds[3]>>1);
	packed_mc[15] = (atomicOrds[3]<<7)|(atomicOrds[4]<<3)|(atomicOrds[5]>>1);
	packed_mc[16] = (atomicOrds[5]<<7)|(atomicOrds[6]<<3)|(atomicOrds[7]>>1);
	packed_mc[17] = (atomicOrds[7]<<7)|(atomicOrds[8]<<3)|(atomicOrds[9]>>1);
	packed_mc[18] = (atomicOrds[9]<<7)|(atomicOrds[10]<<3)|(atomicOrds[11]>>1);
	packed_mc[19] = (atomicOrds[11]<<7)|(atomicOrds[12]<<3)|(atomicOrds[13]>>1);
	packed_mc[20] = ((atomicOrds[13]<<7)|(atomicOrds[14]<<3))&0xF0;
	*/
	for(uint8_t i = 0; i < 15; i++) {
		packed_mc[i] = ords[i];
	}
	uint8_t atmIdx = 0;
	for(uint8_t i = 15; i < 23; i++) {
		if(atmIdx < 14) packed_mc[i] = (atomicOrds[atmIdx]<<4)|(atomicOrds[atmIdx+1]);
		else packed_mc[i] = (atomicOrds[atmIdx]<<4)&0xF0;
		atmIdx+=2;
	}
}

void unpackMolecule(uint8_t packed_mc[23], MC_Component mc[15]) {
	/*mc[0].ID = (packed_mc[0]>>1)&0x7F;
	mc[1].ID = ((packed_mc[0]&0x1)<<6)|((packed_mc[1]>>2)&0x3F);
	mc[2].ID = ((packed_mc[1]&0x3)<<5)|((packed_mc[2]>>3)&0x1F);
	mc[3].ID = ((packed_mc[2]&0x7)<<4)|((packed_mc[3]>>4)&0xF);
	mc[4].ID = ((packed_mc[3]&0xF)<<3)|((packed_mc[4]>>5)&0x7);
	mc[5].ID = ((packed_mc[4]&0x1F)<<2)|((packed_mc[5]>>6)&0x3);
	mc[6].ID = ((packed_mc[5]&0x3F)<<1)|((packed_mc[6]>>7)&0x1);
	mc[7].ID = packed_mc[6]&0x7F;
	mc[8].ID = (packed_mc[7]>>1)&0x7F;
	mc[9].ID = ((packed_mc[7]&0x1)<<6)|((packed_mc[8]>>2)&0x3F);//
	mc[10].ID = ((packed_mc[8]&0x3)<<5)|((packed_mc[9]>>3)&0x1F);
	mc[11].ID = ((packed_mc[9]&0x7)<<4)|((packed_mc[10]>>4)&0xF);
	mc[12].ID = ((packed_mc[10]&0xF)<<3)|((packed_mc[11]>>5)&0x7);
	mc[13].ID = ((packed_mc[11]&0x1F)<<2)|((packed_mc[12]>>6)&0x3);
	mc[14].ID = ((packed_mc[12]&0x3F)<<1)|((packed_mc[13]>>7)&0x1);
	mc[0].atomicNum = (packed_mc[13]>>3)&0xF;
	mc[1].atomicNum = ((packed_mc[13]<<1)|(packed_mc[14]>>7))&0xF;
	mc[2].atomicNum = (packed_mc[14]>>3)&0xF;
	mc[3].atomicNum = ((packed_mc[14]<<1)|(packed_mc[15]>>7))&0xF;
	mc[4].atomicNum = (packed_mc[15]>>3)&0xF;
	mc[5].atomicNum = ((packed_mc[15]<<1)|(packed_mc[16]>>7))&0xF;
	mc[6].atomicNum = (packed_mc[16]>>3)&0xF;
	mc[7].atomicNum = ((packed_mc[16]<<1)|(packed_mc[17]>>7))&0xF;
	mc[8].atomicNum = (packed_mc[17]>>3)&0xF;
	mc[9].atomicNum = ((packed_mc[17]<<1)|(packed_mc[18]>>7))&0xF;
	mc[10].atomicNum = (packed_mc[18]>>3)&0xF;
	mc[11].atomicNum = ((packed_mc[18]<<1)|(packed_mc[19]>>7))&0xF;
	mc[12].atomicNum = (packed_mc[19]>>3)&0xF;
	mc[13].atomicNum = ((packed_mc[19]<<1)|(packed_mc[20]>>7))&0xF;
	mc[14].atomicNum = (packed_mc[20]>>3)&0xF;
	for(uint8_t i = 0; i < MAX_ATOMS_IN_MC; i++) {
		mc[i].atomicNum = get_atomic_num_from_ord(mc[i].atomicNum);
		mc[i].ID = get_id_from_ord(mc[i].ID);
	}*/
	for(uint8_t i = 0; i < MAX_ATOMS_IN_MC; i++) {
		mc[i].ID = get_id_from_ord(packed_mc[i]);
	}
	uint8_t idx = 0;
	for(uint8_t i = 15; i < 23; i++) {
		mc[idx].atomicNum = packed_mc[i]>>4;
		if((idx+1) != 15) mc[idx+1].atomicNum = packed_mc[i]&0xF;
		idx+=2;
	}
	//Extra loop probably not necessary except to debug
	for(uint8_t i = 0; i < 15; i++) {
		mc[i].atomicNum = get_atomic_num_from_ord(mc[i].atomicNum);
	}
}

void initAtomState() {
	for(uint8_t i = 0; i < MAX_NEAR_ATOMS; i++)  { near_atoms[i] = NULL_NEAR_ATOM; }
	for(uint8_t i = 1; i < MAX_ATOMS_IN_MC; i++) {  
		my_molecule[i].atomicNum = 0;  
		my_molecule[i].ID = 0;
	}
	Atom* baseAtom = getAtomFromAtomicNum(MY_CHEM_ID);
	for(uint8_t i=0;i<8;i++)  {  myID.valence[i] = baseAtom->valence[i];  }
	myID.chi = baseAtom->chi;
	myID.name[0] = baseAtom->name[0];
	myID.name[1] = baseAtom->name[1];
	myID.atomicNum = baseAtom->atomicNum;
	my_molecule[0].ID = get_droplet_id();
	my_molecule[0].atomicNum = myID.atomicNum;
	setAtomColor(&myID);
	initBondedAtoms(myID);
	timeLastMoved=0;
}

void moveToTarget(uint16_t rng, float bearing)
{
	int16_t degree_bearing = (int16_t)rad_to_deg(bearing);
	printf_P(PSTR("\tMoving to target. range = %u, bearing = %d\r\n"), rng, degree_bearing);
	if (rng<(DROPLET_RADIUS*25)){
		//printf_P(PSTR("Range is too small. Not moving.\r\n"));
	}else{
		if (abs(degree_bearing)<=30)  {
			//printf_P(PSTR("moving forward.\r\n"));
			walk(0, rng>>2);
		}
		else if (abs(degree_bearing)>=150)  {
			//printf_P(PSTR("moving backward.\r\n"));
			walk(3, rng>>2);
		}
		else if (degree_bearing>0)  {
			//printf_P(PSTR("moving ccw.\r\n"));
			walk(7, abs(degree_bearing));
		}
		else if (degree_bearing<0)  {
			//printf_P(PSTR("moving cw.\r\n"));
			walk(6, abs(degree_bearing));
		}
		timeLastMoved = get_time();
	}
}

void calculatePath(float target, uint16_t range, float bearing)
{
	//Target is the angle of the spot that I need to occupy, from the center of the Droplet I'm moving toward. I'm aiming to be right next to that Droplet at that angle to him.
	float o_x, o_y;
	o_x = cos(target-M_PI_2)*(DROPLET_RADIUS*25);
	o_y = sin(target-M_PI_2)*(DROPLET_RADIUS*25);
	float s_x, s_y;
	s_x = -cos(bearing-M_PI_2)*range;
	s_y = -sin(bearing-M_PI_2)*range;
	printf_P(PSTR("\trange: %u bearing: %f o_x: %f o_y: %f s_x: %f s_y: %f \r\n"), range, rad_to_deg(bearing), o_x, o_y, s_x, s_y);
	float new_x = o_x + s_x;
	float new_y = o_y + s_y;
	float new_bearing = atan2(new_y, new_x)/*+M_PI_2*/;
	uint16_t new_rng = (uint16_t)hypotf(new_x, new_y);
	printf_P(PSTR("\tnew_rng: %u, new_bearing: %f\r\n"), new_rng, rad_to_deg(new_bearing));
	moveToTarget(new_rng, new_bearing);
}

void calculateTarget(Atom* other, uint16_t range, float bearing, float heading)
{
	uint8_t bondingRegions;
	uint8_t myBondingRegion = -1;
	for(uint8_t i=0;i<8;i+=2){
		if((other->valence[i]==1)&&(other->valence[i+1]==0)){
			bondingRegions++;
		}else{
			if(other->valence[i]!= other->valence[i+1]) printf("ERROR: HORRIBLE ERRORS.\r\n");
			if(other->bonded_atoms[i/2]==get_droplet_id()) myBondingRegion = i/2;
			if(other->valence[i]==1){
				bondingRegions++;
			}else if(other->valence[i]==2){
				bondingRegions++;
			}else if(other->valence[i]==3){
				bondingRegions++;
				i+=2;
			}else if(other->valence[i]==4){
				bondingRegions++;
				i+=4;
			}
		}
	}
	if(myBondingRegion == -1) {
		printf_P(PSTR("ERROR: Attempted to calculate a target orbital on an atom who isn't bonded to me \r\n"));
		return;
	}
	float sizeOfRegion = ((2*M_PI)/bondingRegions);
	float angle = sizeOfRegion*myBondingRegion;
	printf("\tIn calculate_target, numBondingRegions: %hu, myBondingRegion: %hu, angle: %f, heading: %f\r\n", bondingRegions, myBondingRegion, angle, rad_to_deg(heading));
	calculatePath(angle+heading, range, bearing);
}

void createStateMessage(State_Msg* msg, char flag)
{
	msg->atomicNum = myID.atomicNum;
	uint8_t numBonds = 0;
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		if(myID.bonded_atoms[i] != 0 && myID.bonded_atoms[i] != 0xFFFF) {
			numBonds++;
		}
	}
	globalBlinkTimer = get_droplet_id();
	if(numBonds == 0){
		disable_sync_blink();
		printf("\tBlink disabled.\r\n");
	}else{
		for(uint8_t i=0;i<MAX_ATOMS_IN_MC;i++){
			if(my_molecule[i].ID < globalBlinkTimer && my_molecule[i].ID != 0){
				globalBlinkTimer = my_molecule[i].ID;
				target_id = my_molecule[i].ID;
			} else if (my_molecule[i].ID == 0) break;
		}
		enable_sync_blink(globalBlinkTimer);
		printf("\tBlink timer: %u\r\n", globalBlinkTimer);
	}
	msg->blink_timer = globalBlinkTimer;
	for(uint8_t i = 0; i < MAX_BONDS; i++) {  msg->bonded_atoms[i] = myID.bonded_atoms[i]; }
	packValences(msg->valence, myID.valence);
	msg->msgFlag = flag;
	packMolecule(msg->molecule, my_molecule);
}

/* Checks if self is currently bonded to other */
uint8_t selfBondedToOther(uint16_t other_ID) {
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		if(myID.bonded_atoms[i] == other_ID) {
			return 1;
		}
	}
	return 0;
}

uint8_t otherBondedToSelf(Atom* other) {
	//printf_P(PSTR("Sender's bonded atoms: "));
	uint16_t myDropletID = get_droplet_id();
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		//printf_P(PSTR("%04X, "), other->bonded_atoms[i]);
		if(other->bonded_atoms[i] == myDropletID) {
			//printf_P(PSTR("\r\n"));
			return 1;
		}
	}
	//printf_P(PSTR("\r\n"));
	return 0;
}

uint8_t isInMyMolecule(uint16_t ID) {
	for(uint8_t i = 0; i < MAX_ATOMS_IN_MC; i++){
		if(my_molecule[i].ID == ID) return 1;
	}
	return 0;	
}

float getChiFromAtomicNum(uint8_t atomicNum) {
	for(uint8_t i = 0; i < NUM_ELEMENTS; i++) {
		if(PeriodicTable[i].atomicNum == atomicNum) {
			return PeriodicTable[i].chi;
		}
	}
	return -1;
}

uint8_t chiCheck(Atom* other) {
	float myChi = getChiFromAtomicNum(myID.atomicNum);
	if(myChi == -1) {
		printf_P(PSTR("ERROR: in chiCheck, my atomic number is incorrect \r\n"));
	}
	float otherChi = getChiFromAtomicNum(other->atomicNum);
	if(otherChi == -1) {
		printf_P(PSTR("ERROR: in chiCheck, the atom I'm talking to has an incorrect atomic number \r\n"));
	}
	if(fabsf(myChi - otherChi) < 1.70) {
		return BOND_TYPE_COV;
	}
	else {
		return BOND_TYPE_ION;
	}
	
	return 0;
}

/*
	Returns the number of overlapping atoms if the molecules overlap but are not identical, indicating that no bond should be formed.
	Returns 0 if the molecules don't overlap.

	Molecules A and B might overlap if an atom in A (called a) has calculated that it should steal an atom from B (b),
	but the atom from B hasn't bonded to the one from A yet. If two OTHER atoms, x from A and y from B are then talking,
	x could think its molecule contains atom b because its atom a thinks it bonded to b, while y also thinks
	its molecule contains b because b hasn't left yet. If this occurs, the delta-G calculations will be incorrect,
	so no attempt to bond should be made until the half-formed bond between a and b resolves itself.
*/
uint8_t moleculesOverlap(MC_Component sender_mc[MAX_ATOMS_IN_MC]) {
	//Neither of us is in a molecule yet
	if(molecule_length(my_molecule) == 1 && molecule_length(sender_mc) == 1) {
		return 0;
	}
	uint8_t numOverlapped = 0;
	for(uint8_t i = 0; i < MAX_ATOMS_IN_MC; i++) {
		if(sender_mc[i].ID != 0) {
			if(isInMyMolecule(sender_mc[i].ID)) numOverlapped++;
		} else {
			break;
		}
	}
	return numOverlapped;
}

/*
	Determines if a reaction would occur spontaneously or not by using the concept of Gibbs Free Energy.
	Calculates the difference in Gibbs Free Energies between the products and reactants of two reactions:
	the first being where I bond to sender, stealing sender from its molecule, and the second being where 
	sender steals me from my molecule. In both cases I should bond to sender, but in the second I should also leave my molecule. 
*/
uint8_t energyCheck(MC_Component sender_mc[MAX_ATOMS_IN_MC], uint8_t otherHalfBond, uint8_t overlap) {
	/*Gibbs Free Energy values for the other molecule(gO), other mc plus me(gO_p), and other mc minus sender gO_m) */
	int16_t gO;
	int16_t gO_p;
	int16_t gO_m;

	/*Gibbs Free Energy values for my molecule(gS), my molecule plus sender(gS_p), and my molecule minus me(gS_m) */
	int16_t gS;
	int16_t gS_p;
	int16_t gS_m;

	/*Gibbs free energy values that only need to be calculated if otherHalfBond is true*/
	int16_t gOther_alone;
	int16_t gOther_without_me;

	uint8_t other_len = molecule_length(sender_mc);
	uint8_t my_len = molecule_length(my_molecule);

	/*Arrays of the atomic numbers of the atoms that compose these molecules, named similarly to above */
	uint8_t O_mc[other_len];
	uint8_t O_p_mc[other_len+1];
	uint8_t O_m_mc[other_len-1];
	uint8_t S_mc[my_len];
	uint8_t S_p_mc[my_len+1];
	uint8_t S_m_mc[my_len-1];
	uint8_t sender_alone[1];
	uint8_t other_without_me[other_len-1];

	/*Delta G values*/
	int32_t deltaGrxn_1;
	int32_t deltaGrxn_2;

	for(uint8_t i = 0; i < other_len+1; i++) {
		if(i < other_len-1) {
			O_mc[i] = sender_mc[i].atomicNum;
			O_m_mc[i] = sender_mc[i+1].atomicNum;
			O_p_mc[i] = sender_mc[i].atomicNum;
		}
		else if(i < other_len) {
			O_mc[i] = sender_mc[i].atomicNum;
			O_p_mc[i] = sender_mc[i].atomicNum;
		}
		else if (i < other_len +1) {
			O_p_mc[i] = myID.atomicNum;
		}
	}
	for(uint8_t i = 0; i < my_len+1; i++) {
		if(i < my_len-1) {
			S_mc[i] =   my_molecule[i].atomicNum;
			S_m_mc[i] = my_molecule[i+1].atomicNum;
			S_p_mc[i] = my_molecule[i].atomicNum;
		}
		else if(i < my_len) {
			S_mc[i] =   my_molecule[i].atomicNum;
			S_p_mc[i] = my_molecule[i].atomicNum;
		}
		else if (i < my_len +1) {
			S_p_mc[i] = sender_mc[0].atomicNum;
		}
	}
	sender_alone[0] = sender_mc[0].atomicNum;
	if(otherHalfBond) {
		uint8_t foundMe = 0;
		uint8_t idx = 0;
		for(uint8_t i = 0; i < other_len-1; i++) {
			if(O_mc[i] == myID.atomicNum && !foundMe) {
				foundMe = 1;
				idx++;
			}
			other_without_me[i] = O_mc[idx];
			idx++;
		}
	}
	
	printf("\tOtherHalfBond is %hu \r\n", otherHalfBond);
	if(!otherHalfBond) {
		printf_P(PSTR("\tReaction one: \r\n\t"));
		printMolecularReaction(S_mc, O_mc, S_p_mc, O_m_mc, my_len, other_len, my_len+1, other_len-1);
		printf_P(PSTR("\tReaction two: \r\n\t"));
		printMolecularReaction(S_mc, O_mc, S_m_mc, O_p_mc, my_len, other_len, my_len-1, other_len+1);
	} else {
		printf_P(PSTR("\tReaction one: \r\n\t"));
		printMolecularReaction(S_mc, sender_alone, S_p_mc, NULL, my_len, 1, my_len+1, 0);
		printf_P(PSTR("\tReaction two: \r\n\t"));
		printMolecularReaction(S_mc, other_without_me, S_m_mc, O_mc, my_len, other_len-1, my_len-1, other_len);
	}
	//Fill in energy values
	if(!molecule_search(O_mc, &gO, other_len))  {
		printf_P(PSTR("\t\tGibbs free energy value for other molecule not found. \r\n"));
		//for(uint8_t i = 0; i < other_len; i++) {
			//printf_P(PSTR("%hu, "), O_mc[i]);
		//}
		//printf_P(PSTR("\r\n"));
		gO = 32767;
	}
	if(!molecule_search(O_p_mc, &gO_p, other_len+1))  {
		printf_P(PSTR("\t\tGibbs free energy value for other molecule plus not found. \r\n"));
		//for(uint8_t i = 0; i < other_len+1; i++) {
			//printf_P(PSTR("%hu, "), O_p_mc[i]);
		//}
		//printf_P(PSTR("\r\n"));
		gO_p = 32767;
	}
	if(!molecule_search(O_m_mc, &gO_m, other_len-1))  {
		printf_P(PSTR("\t\tGibbs free energy value for other molecule minus not found. \r\n"));
		//for(uint8_t i = 0; i < other_len-1; i++) {
			//printf_P(PSTR("%hu, "), O_m_mc[i]);
		//}
		//printf_P(PSTR("\r\n"));
		if(other_len-1 == 0) {
			printf_P(PSTR("\t\tSetting G for other molecule minus to zero. \r\n"));
			gO_m = 0;
		}
		else {
			gO_m = 32767;
		}
	}
	if(!molecule_search(S_mc, &gS, my_len))  {
		printf_P(PSTR("\t\tGibbs free energy value for my molecule not found. \r\n"));
		//for(uint8_t i = 0; i < my_len; i++) {
			//printf_P(PSTR("%hu, "), S_mc[i]);
		//}
		//printf_P(PSTR("\r\n"));
		gS = 32767;
	}
	if(!molecule_search(S_p_mc, &gS_p, my_len+1))  {
		printf_P(PSTR("\t\tGibbs free energy value for my molecule plus not found. \r\n"));
		//for(uint8_t i = 0; i < my_len+1; i++) {
			//printf_P(PSTR("%hu, "), S_p_mc[i]);
		//}
		//printf_P(PSTR("\r\n"));
		gS_p = 32767;
	}
	if(!molecule_search(S_m_mc, &gS_m, my_len-1))  {
		printf_P(PSTR("\t\tGibbs free energy value for my molecule minus not found.\r\n"));
		//for(uint8_t i = 0; i < my_len-1; i++) {
			//printf_P(PSTR("%hu, "), S_m_mc[i]);
		//}
		//printf_P(PSTR("\r\n"));
		if(my_len-1 == 0) {
			printf_P(PSTR("\t\tSetting G for my molecule minus to zero. \r\n"));
			gS_m = 0;
		}
		else {
			gS_m = 32767;
		}
	}
	//test
	if(!molecule_search(sender_alone, &gOther_alone, 1)) {
		printf_P(PSTR("\t\tGibbs free energy value for sender alone not found.\r\n"));
		gOther_alone = 32767;
	}
	if(!molecule_search(other_without_me, &gOther_without_me, other_len-1)) {
		printf_P(PSTR("\t\tGibbs free energy value for other_without_me not found. \r\n"));
		//for(uint8_t i = 0; i < other_len-1; i++) {
			//printf_P(PSTR("%hu, "), other_without_me[i]);
		//}
		//printf_P(PSTR("\r\n"));
		if(other_len-1 == 0) {
			printf_P(PSTR("\t\tSetting G for other without me to zero. \r\n"));
			gOther_without_me = 0;
		} else {
			gOther_without_me = 32767;
		}
	}
	/*Calculate delta G values. Take into account the one remaining discrepancy in states: sender can be bonded to me, and if so, 
	it either thinks I've joined its molecule or it has joined mine. */
	if(!otherHalfBond) {
		printf_P(PSTR("Reaction one: (%d + %d) - (%d + %d) \r\n"), (int)gS_p, (int)gO_m, (int)gS, (int)gO);
		printf_P(PSTR("Reaction two: (%d + %d) - (%d + %d) \r\n"), (int)gS_m, (int)gO_p, (int)gS, (int)gO);
		deltaGrxn_1 = ((int32_t)gS_p + (int32_t)gO_m) - ((int32_t)gS + (int32_t)gO);
		deltaGrxn_2 = ((int32_t)gS_m + (int32_t)gO_p) - ((int32_t)gS + (int32_t)gO);
	} else {
		//if sender joined my molecule:
		//my_mc plus sender_alone -> sender's current molecule which is my mc-plus-sender plus nothing
		printf_P(PSTR("Reaction one: (%d) - (%d + %d) \r\n"), (int)gS_p, (int)gS, (int)gOther_alone);
		deltaGrxn_1 = ((int32_t)gS_p) - ((int32_t)gS + (int32_t)gOther_alone);
		//if sender thinks I joined its molecule:
		//my_mc plus sender's-mc-minus-me -> my_mc_minus_me + sender's-current-mc
		printf_P(PSTR("Reaction two: (%d + %d) - (%d + %d) \r\n"), (int)gS_m, (int)gO, (int)gOther_without_me, (int)gS);
		deltaGrxn_2 = ((int32_t)gS_m + (int32_t)gO) - ((int32_t)gOther_without_me + (int32_t)gS); 
	}

	int32_t lowestEnergyRxn = (int32_t)fmin((float)deltaGrxn_2, (float)deltaGrxn_1);
	printf("Delta G rxn 1 = %ld, Delta G rxn 2 = %ld \r\n", deltaGrxn_1, deltaGrxn_2);
	printf("Lowest energy reaction: ");
	if(deltaGrxn_2 == deltaGrxn_1) {
		if(!otherHalfBond) {
			printf("reaction one, but they were equal \r\n");
			return 1;
		} else {
			printf("reaction two, but they were equal \r\n");
			return 2;
		}
	}
	if(lowestEnergyRxn == deltaGrxn_1 && deltaGrxn_1 < 0) {
		printf("reaction 1 \r\n");
		return 1;
	}
	else if(lowestEnergyRxn == deltaGrxn_2 && deltaGrxn_2 < 0) {
		printf("reaction 2 \r\n");
		return 2;
	}
	else {
	printf("None \r\n");
		return 0;
	}
	
}

void printMolecularReaction(uint8_t* reactant1, uint8_t* reactant2, uint8_t* product1, uint8_t* product2, uint8_t lenR1, uint8_t lenR2, uint8_t lenP1, uint8_t lenP2) {
	uint8_t found_atom = 0;
	for(uint8_t i = 0; i < lenR1; i++) {
		char* myAtom = malloc(2);
		getNameFromAtomicNum(myAtom, reactant1[i]);
		if(!(myAtom[0] == ' ' && myAtom[1] == ' '))  {
			printf_P(PSTR("%c"), myAtom[0]);
			if(myAtom[1] != ' ') printf_P(PSTR("%c"), myAtom[1]);
			free(myAtom);
		} else {
			free(myAtom);
			break;
		}
	}
	printf_P(PSTR(" + "));
	for(uint8_t i = 0; i < lenR2; i++) {
		char* otherAtom = malloc(2);
		getNameFromAtomicNum(otherAtom, reactant2[i]);
		if(!(otherAtom[0] == ' ' && otherAtom[1] == ' ')) {
			printf_P(PSTR("%c"), otherAtom[0]);
			if(otherAtom[1] != ' ')printf_P(PSTR("%c"), otherAtom[1]);
			free(otherAtom);
		} else {
			free(otherAtom);
			break;
		}
	}
	printf_P(PSTR(" -> "));
	for(uint8_t i = 0; i < lenP1; i++) {
		char* myAtom = malloc(2);
		getNameFromAtomicNum(myAtom, product1[i]);
		if(!(myAtom[0] == ' ' && myAtom[1] == ' '))  {
			printf_P(PSTR("%c"), myAtom[0]);
			if(myAtom[1] != ' ') printf_P(PSTR("%c"), myAtom[1]);
			free(myAtom);
		} else {
			free(myAtom);
			break;
		}
	}
	if(lenP2 > 0 && lenP1 > 0) printf_P(PSTR(" + "));
	if(product2 != NULL) {
		for(uint8_t i = 0; i < lenP2; i++) {
			char* otherAtom = malloc(2);
			getNameFromAtomicNum(otherAtom, product2[i]);
			if(!(otherAtom[0] == ' ' && otherAtom[1] == ' ')) {
				printf_P(PSTR("%c"), otherAtom[0]);
				if(otherAtom[1] != ' ')printf_P(PSTR("%c"), otherAtom[1]);
				free(otherAtom);
			} else {
				free(otherAtom);
				break;
			}
		}
	}
	printf_P(PSTR("\r\n"));	
}

void printMyValence() {
	printf_P(PSTR("Valence shell: "));
	for(uint8_t i = 0; i < 8; i++) {
		printf_P(PSTR("%hu, "), myID.valence[i]);
	}
	printf_P(PSTR("\r\n"));
}

void printMyBondedAtoms() {
	printf_P(PSTR("My bonded atoms: "));
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		printf_P(PSTR("%X, "), myID.bonded_atoms[i]);
	}
	printf_P(PSTR("\r\n"));
}

uint8_t addToMolecule(uint16_t ID, uint8_t atomicNum) {
	if(ID == 0) {
		return 1;
	}
	printf_P(PSTR("*****Adding atom %04X to molecule \r\n"), ID);
	uint8_t idx = molecule_length(my_molecule);
	if(idx == MAX_ATOMS_IN_MC) {
		printf("ALERT: Tried to add atom to molecule but there was no room \r\n");
		return 0;
	}
	for(uint8_t i = 0; i < MAX_ATOMS_IN_MC; i++) {
		if(my_molecule[i].ID == ID) {
			printf_P(PSTR("ALERT: Tried to add atom to molecule but it was already there \r\n"));
			return 0;
		}
	}
	my_molecule[idx].ID = ID;
	my_molecule[idx].atomicNum = atomicNum;
	return 1;
}

void removeFromMolecule(uint16_t ID) {
	printf_P(PSTR("*****Removing atom %04X from molecule \r\n"), ID);
	if(ID == get_droplet_id()) {
		set_rgb(0,0,255);
		delay_ms(1000);
		printf("\r\n ERROR \r\n");
	}
	for(uint8_t i = 0; i < MAX_ATOMS_IN_MC; i++) {
		if(my_molecule[i].ID == ID) {
			for(uint8_t j = i; j < MAX_ATOMS_IN_MC-1; j++) {
				my_molecule[j].ID = my_molecule[j+1].ID;
				my_molecule[j].atomicNum = my_molecule[j+1].atomicNum;
			}
			my_molecule[MAX_ATOMS_IN_MC-1].ID = 0;
			my_molecule[MAX_ATOMS_IN_MC-1].atomicNum = 0;
		}
	}
}

void joinNewMolecule(MC_Component newMC[MAX_ATOMS_IN_MC]) {
	//Leave my current molecule
	for(uint8_t i = 1; i < MAX_ATOMS_IN_MC; i++) {
		my_molecule[i].ID = 0;
		my_molecule[i].atomicNum = 0;
	}

	//Join new molecule
	for(uint8_t i = 0; i < MAX_ATOMS_IN_MC; i++) {
		addToMolecule(newMC[i].ID, newMC[i].atomicNum);
	}
}

uint8_t attemptToBond(Atom* other, int bondType, uint16_t other_ID, uint8_t rxnType) {
	/*Determine the number of bonds to form: double and triple atomic bonds are treated like multiple single bonds to the same atom
	  If sender is bonded to me, their valence shell will reflect that and I might think they have no room to make another bond, 
	  since I don't realize one of their bonds is already to me. Soln: count the number of times my name appears 
	  in their bonded_atoms and add it to maxOtherBonds.*/
	//printf("\r\n\tAttempting to bond, sender ID is %04X \r\n", other_ID);
	int maxBondsSelf = 0;
	int maxBondsOther = 0;
	//Calculate the possible bonds other can make
	printf("\tOther's bonded atoms: ");
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		printf("%04X, ", other->bonded_atoms[i]);
		if(other->bonded_atoms[i] == get_droplet_id()) {
			maxBondsOther++;
		}
	}
	printf("\r\n");
	for(uint8_t i = 0; i < 8; i+=2) {
		if(other->valence[i] == 1 && other->valence[i+1] == 0) {
			maxBondsOther++;
		}
	}
	//If the correct reaction is for me to leave my molecule, break all my current bonds so that happens, as long as other has room to bond to me.
	if(maxBondsOther == 0) {
		printf("Other has no room to bond. \r\n");
		return 0;
	}else if(maxBondsOther > 0 && rxnType == 2) {
		for(uint8_t i = 0; i < MAX_BONDS; i++) {
			breakBond(myID.bonded_atoms[i], BOND_TYPE_COV);
		}
	} 
	//Calculate the possible bonds I can make
	for(uint8_t i = 0; i < 8; i+=2) {
		if(myID.valence[i] == 1 && myID.valence[i+1] == 0) {
			maxBondsSelf++;
		}
	}
	
	printf("\r\n");
	if(bondType == BOND_TYPE_ION) {
		if(maxBondsSelf > 0) {  maxBondsSelf = 1;  }
		if(maxBondsOther > 0) {  maxBondsOther = 1;  }
	}
	int numBonds = (int)(fminf((float)maxBondsSelf, (float)maxBondsOther));
	//printf("\t\t NumBonds = %d, maxBondsSelf = %d, maxBondsOther = %d \r\n", numBonds, maxBondsSelf, maxBondsOther);
	//printMyValence();
	//printMyBondedAtoms();
	if(numBonds == 0) {
		printf("\tI have no room to bond. \r\n");
		return 0;
	}
	//Last check: room in mc?
	if(rxnType == 1) {
		if(!addToMolecule(other_ID, other->atomicNum)) {
			printf("\tSQUIIIIID \r\n");
			return 0;
		}
	} 
	if(numBonds == 4) {
		numBonds = 3;
	}
	//Set electron code
	uint8_t type = numBonds+1;
	printf("\t\tForming bond: \r\n");
	//Change my valence shell and bonded atoms
	for(uint8_t i = 0; i < 8; i+=2) {
		if(myID.valence[i] == 1 && myID.valence[i+1] == 0) {
			if(bondType == BOND_TYPE_ION) {
				//If I'm the less electronegative of an ionic pair, I lose my free electron
				if(myID.chi < 1.6) {
					myID.valence[i] = 0;
				}
				else {
				//I'm the more electronegative, I gain an electron and both of the electrons in that orbital become bonded
					myID.valence[i] = 2;
					myID.valence[i+1] = 2;
				}
				myID.bonded_atoms[i/2] = other_ID;
				break;
			}
			else {
				if(numBonds > 0) {
					myID.valence[i] = type;
					myID.valence[i+1] = type;
					myID.bonded_atoms[i/2] = other_ID;
					numBonds--;
				}
			}
		}
	}
	//printMyValence();
	//printMyBondedAtoms();
	return 1;
}

uint8_t breakBond(uint16_t senderID, uint8_t bondType) {
	if(senderID == 0 || senderID == -1) return 0;
	removeFromMolecule(senderID); //disallows cycles in atomic structure
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		if(myID.bonded_atoms[i] == senderID) {
			myID.bonded_atoms[i] = 0;
			if(bondType == BOND_TYPE_ION) {
				if(myID.chi < 1.6) {
					myID.valence[i*2+1] = 0;
				}
				else {
					myID.valence[i*2] = 1;
					myID.valence[i*2+1] = 0;
				}
				break;
			}
			else {
				myID.valence[i*2] = 1;
				myID.valence[i*2+1] = 0;
			}
		}
	}
	uint8_t numBonds = 0;
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		if(myID.bonded_atoms[i] != 0 && myID.bonded_atoms[i] != 0xFFFF) numBonds++;
	}
	if(numBonds == 0) disable_sync_blink();
	return 0;
}

void updateMolecule(MC_Component sender_mc[MAX_ATOMS_IN_MC]) {
	MC_Component keep[MAX_BONDS];
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		keep[i].ID = 0;
		keep[i].atomicNum = 0;
	}
	uint8_t idx = 0;
	for(uint8_t i = 1; i < MAX_ATOMS_IN_MC; i++) {
		uint8_t found = 0;
		for(uint8_t j  = 0; j < MAX_BONDS; j++) {
			if(my_molecule[i].ID && (myID.bonded_atoms[j] == my_molecule[i].ID)) found = 1;
		}
		if(found) {
			keep[idx].ID = my_molecule[i].ID;
			keep[idx].atomicNum = my_molecule[i].atomicNum;
			idx++;
		}
		my_molecule[i].ID = 0;
		my_molecule[i].atomicNum = 0;
	}
	for(uint8_t i = 0; i < MAX_ATOMS_IN_MC; i++) {
		if(!isInMyMolecule(sender_mc[i].ID)) addToMolecule(sender_mc[i].ID, sender_mc[i].atomicNum);
	}
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		if(!isInMyMolecule(keep[i].ID)) addToMolecule(keep[i].ID, keep[i].atomicNum);
	}
}

void msgState(ir_msg* msg_struct) {
	State_Msg state;
	state = *((State_Msg*)(msg_struct->msg));
	MC_Component sender_mc[MAX_ATOMS_IN_MC];
	unpackMolecule(state.molecule, sender_mc);
	printf("State message received. Molecule list is as follows: \r\n");
	for(uint8_t i = 0; i < 15; i++) {
		if(sender_mc[i].ID == 0) break;
		printf("%04X, ", sender_mc[i].ID);
	}
	printf("\r\n");
	printf("My molecule: \r\n");
	for(uint8_t i = 0; i < 15; i++) {
		if(my_molecule[i].ID == 0) break;
		printf("%04X, ", my_molecule[i].ID);
	}
	printf("\r\nMy bonded atoms:");
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		printf("%04X, ", myID.bonded_atoms[i]);
	}
	Atom near_atom;
	near_atom.atomicNum = state.atomicNum;
	unpackValences(state.valence, near_atom.valence);
	for(uint8_t i = 0; i < MAX_BONDS; i++) near_atom.bonded_atoms[i] = state.bonded_atoms[i]; 
	Atom* base_atom = getAtomFromAtomicNum(near_atom.atomicNum);
	near_atom.name[0] = base_atom->name[0];
	near_atom.name[1] = base_atom->name[1];
	near_atom.chi = base_atom->chi;
	updateNearAtoms(&near_atom, msg_struct);
	//setAtomColor(&near_atom);
	//delay_ms(500);
	//setAtomColor(&myID);
	uint8_t bondType = chiCheck(&near_atom);
	printf("Beginning bond check \r\n");
	uint8_t otherHalfBond = 0;
	uint8_t shouldUpdateMc = 1;
	uint8_t rxnType = 0;
	if(!selfBondedToOther(msg_struct->sender_ID)) {
		printf("\tI'm not bonded to sender %X \r\n", msg_struct->sender_ID);
		uint8_t shouldBond = 1;
		//Fixes a heisenbug caused by half-complete reactions, which lead to inconsistent states.
		//See moleculesOverlap for more details.
		uint8_t overlap = moleculesOverlap(sender_mc);
		printf("\tOverlap: %hu Other bonded to me? %hu\r\n", overlap, otherBondedToSelf(&near_atom));
		if(overlap > 1 || (overlap > 0 /*&& overlap == molecule_length(my_molecule)-1*/ && !otherBondedToSelf(&near_atom))) {
			printf("\tOverlap check caught \r\n");
			shouldBond = 0;
		}   
		if(otherBondedToSelf(&near_atom)) {
			otherHalfBond = 1;
		}
		if(!isInMyMolecule(msg_struct->sender_ID) && shouldBond) {
			rxnType = energyCheck(sender_mc, otherHalfBond, overlap);
			if(!rxnType) {
				shouldBond = 0;
			}
		} if(isInMyMolecule(msg_struct->sender_ID)) {
			printf("\tSender is in my molecule \r\n");
			shouldBond = 0;
		}
		if(shouldBond) {
			uint8_t success = attemptToBond(&near_atom, bondType, msg_struct->sender_ID, rxnType);
			if(success) {
				printf("\t Bond successful \r\n");
				if(rxnType == 2) joinNewMolecule(sender_mc);
				shouldUpdateMc = 0; //If rxn type was 1, sender's mc isn't relevant anymore because it joined my mc. If rxn type is 2, I just dealt with updating my molecule.0
			}
			else printf("\tBond unsuccessful \r\n");
		}
	}
	else {
		//If self is bonded to other, and other is not bonded to self, break the half bond. 
		if (!otherBondedToSelf(&near_atom)) {
			printf("\t Breaking bond to sender \r\n");
			breakBond(msg_struct->sender_ID, bondType);
		}
	}

	if(otherBondedToSelf(&near_atom) && selfBondedToOther(msg_struct->sender_ID) &&shouldUpdateMc) {
		updateMolecule(sender_mc);
	}
	updateTargetId();

}

void init()  {
	printf_P(PSTR("INITIALIZING DROPLET. \r\n"));
	switch(get_droplet_id()){
		case 0xD0AE: MY_CHEM_ID = 6; break;
		case 0x5264: MY_CHEM_ID = 6; break;
		case 0x73AF: MY_CHEM_ID = 6; break;
		case 0x2B4E: MY_CHEM_ID = 1; break;
		case 0x7d78: MY_CHEM_ID = 8; break;
		case 0x43BA: MY_CHEM_ID = 6; break;
		case 0xB944: MY_CHEM_ID = 6; break;
		case 0xC24B: MY_CHEM_ID = 17; break;
		case 0xEEB0: MY_CHEM_ID = 17; break;
		case 0x8B46: MY_CHEM_ID = 17; break;
		case 0x3B49: MY_CHEM_ID = 53; break;
		case 0xF60A: MY_CHEM_ID = 17; break;
		case 0x4177: MY_CHEM_ID = 53; break;
		case 0xC806: MY_CHEM_ID = 8; break;
		default:     MY_CHEM_ID = 1; break;
	}
	
	initAtomState();
	frameCount=0;
	frameStart=get_time();
	uint8_t slot = get_droplet_ord(get_droplet_id());
	if(slot<101){
		printf_P(PSTR("I am not an audio droplet! I'm not really supposed to be running the chem code, but I'll do it anyway. For you.\r\n\tYou probably want more than 23 SLOTS_PER_FRAME, though.\r\n"));
		mySlot = (slot+get_droplet_id())%SLOTS_PER_FRAME;
	}else{
		mySlot = slot-100;
	}
	lastLoop=1; //just needs to be not 0.
}

void loop()  {
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime>FRAME_LENGTH_MS){
		frameTime -= FRAME_LENGTH_MS;
		frameStart = get_time()-frameTime;
		frameCount++;
	}
	uint16_t loopID = (SLOTS_PER_FRAME*frameTime)/FRAME_LENGTH_MS;
	
	if(loopID!=lastLoop){
		if(loopID==mySlot){
			delay_ms(8+(rand_byte()>>3));
			broadcast_rnb_data();
			delay_ms(8+(rand_byte()>>3));
			State_Msg message;
			createStateMessage(&message, 'p');
			ir_send(ALL_DIRS, (char*)(&message), sizeof(State_Msg));
			//printf("message->bonded_atoms[0] is %x, message->atomicNum is %hu \r\n", message.bonded_atoms[0], message.atomicNum);
			//if(rand_byte()%3==0)		ir_send(DIR0|DIR3, (char*)(&message), sizeof(State_Msg));
			//else if(rand_byte()%2==0)	ir_send(DIR1|DIR4, (char*)(&message), sizeof(State_Msg));
			//else						ir_send(DIR2|DIR5, (char*)(&message), sizeof(State_Msg));
		}
		setAtomColor(&myID); 
		lastLoop = loopID;
	}

	if(rnb_updated){
		if(last_good_rnb.conf > 0.5){
			uint16_t id = last_good_rnb.id_number;
			uint8_t i;
			for(i=0; i<MAX_NEAR_ATOMS; i++){
				if(near_atoms[i].id==id) break;
			}
			if(i<12){
				near_atoms[i].range = (uint16_t)(10.0*last_good_rnb.range);
				near_atoms[i].bearing = last_good_rnb.bearing;
				near_atoms[i].heading = last_good_rnb.heading;
				//printf("New RNB Data from %04X:\t%u\t%f\t%f\t%f\r\n", id, near_atoms[i].range, rad_to_deg(near_atoms[i].bearing), rad_to_deg(near_atoms[i].heading), last_good_rnb.conf);
				if(target_id==last_good_rnb.id_number){
					calculateTarget(&(near_atoms[i]), near_atoms[i].range, near_atoms[i].bearing, near_atoms[i].heading);  //Comment to disable movement
				}
			}
		}
		rnb_updated = 0;
	}
	delay_ms(LOOP_PERIOD);
}

void handle_msg(ir_msg* msg_struct)  {
	printf("\r\n\r\n\r\nGot message from: %04X.\r\n", msg_struct->sender_ID);
	if(msg_struct->length==0)  printf("ERROR: Message length 0.\r\n");
	
	//State message
	if(msg_struct->length == sizeof(State_Msg))  {
		msgState(msg_struct);
	}
}