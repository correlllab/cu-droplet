/*
 * chem_sim.c
 *
 * Created: 5/10/2016 
 *  Author: Audrey Randall
 */ 
#include "chem_sim_refactor.h"

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
		case 1: return 0;
		case 2: return 1;
		case 3: return 2;
		case 4: return 3;
		case 6: return 4;
		case 7: return 5;
		case 8: return 6;
		case 9: return 7;
		case 11: return 8;
		case 12: return 9;
		case 17: return 10;
		case 35: return 11;
		case 53: return 12;
	}
	return 0xFF;
}

uint8_t get_atomic_num_from_ord(uint8_t ord) {
	switch(ord){
		case 0: return 1;
		case 1:	 return 2;
		case 2:	 return 3;
		case 3:	 return 4;
		case 4:	 return 6;
		case 5:	 return 7;
		case 6:	 return 8;
		case 7:	 return 9;
		case 8:	 return 11;
		case 9:	 return 12;
		case 10: return 17;
		case 11: return 35;
		case 12: return 53;
	}
	return 0xFF;
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

void packMolecule(uint8_t packed_mc[21], MC_Component mc[15]) {
	uint8_t ords[15];
	uint8_t atomicOrds[15];
	for(uint8_t i = 0; i < 15; i++) ords[i] = get_droplet_ord(mc[i].ID); //105 bits
	for(uint8_t i = 0; i < 15; i++) atomicOrds[i] = get_atomic_ord(mc[i].atomicNum); //60 bits
	packed_mc[0] = (ords[0]<<1)|(ords[1]>>6);
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
}

void unpackMolecule(uint8_t packed_mc[21], MC_Component mc[15]) {
	mc[0].ID = (packed_mc[0]>>1)&0x7F;
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
	}
}

void initAtomState() {
	for(uint8_t i = 0; i < MAX_NEAR_ATOMS; i++)  { near_atoms[i] = NULL_NEAR_ATOM; }
	for(uint8_t i = 0; i < MAX_ATOMS_IN_MC; i++) {  
		my_molecule[i].atomicNum = i;  
		my_molecule[i].ID = i;
	}
	
	Atom* baseAtom = getAtomFromAtomicNum(MY_CHEM_ID);
	for(uint8_t i=0;i<8;i++)  {  myID.valence[i] = baseAtom->valence[i];  }
	myID.chi = baseAtom->chi;
	myID.name[0] = baseAtom->name[0];
	myID.name[1] = baseAtom->name[1];
	myID.atomicNum = baseAtom->atomicNum;
	
	setAtomColor(&myID);
	initBondedAtoms(myID);
}

void createStateMessage(State_Msg* msg, char flag)
{
	msg->atomicNum = myID.atomicNum;
	globalBlinkTimer = get_droplet_id();
	uint8_t numBonds = 0;
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		if(myID.bonded_atoms[i] != 0 && myID.bonded_atoms[i] != 0xFFFF) {
			if(myID.bonded_atoms[i] < globalBlinkTimer) {
				globalBlinkTimer = myID.bonded_atoms[i];
			}
			numBonds++;
		}
	}
	if(numBonds == 0){
		disable_sync_blink();
	}else{
		enable_sync_blink(globalBlinkTimer); 
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
	uint16_t myDropletID = get_droplet_id();
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		if(other->bonded_atoms[i] == myDropletID) {
			return 1;
		}
	}
	return 0;
}

uint8_t isInMyMolecule(Atom* other) {
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

uint8_t attemptToBond(Atom* other, int bondType, uint16_t other_ID) {
	/*Determine the number of bonds to form: double and triple atomic bonds are treated like multiple single bonds to the same atom
	  If sender is bonded to me, their valence shell will reflect that and I might think they have no room to make another bond, 
	  since I don't realize one of their bonds is already to me. Soln: count the number of times my name appears 
	  in their bonded_atoms and add it to maxOtherBonds.*/
	printf("\r\n\tAttempting to bond, sender ID is %04X \r\n", other_ID);
	int maxBondsSelf = 0;
	int maxBondsOther = 0;
	for(uint8_t i = 0; i < 8; i+=2) {
		if(myID.valence[i] == 1 && myID.valence[i+1] == 0) {
			maxBondsSelf++;
		}
		if(other->valence[i] == 1 && other->valence[i+1] == 0) {
			maxBondsOther++;
		}
	}
	printf("Other's bonded atoms: ");
	for(uint8_t i = 0; i < MAX_BONDS; i++) {
		printf("%04X, ", other->bonded_atoms[i]);
		if(other->bonded_atoms[i] == get_droplet_id()) {
			maxBondsOther++;
		}
	}
	printf("\r\n");
	if(bondType == BOND_TYPE_ION) {
		if(maxBondsSelf > 0) {  maxBondsSelf = 1;  }
		if(maxBondsOther > 0) {  maxBondsOther = 1;  }
	}
	int numBonds = (int)(fminf((float)maxBondsSelf, (float)maxBondsOther));
	printf("\t\t NumBonds = %d, maxBondsSelf = %d, maxBondsOther = %d \r\n", numBonds, maxBondsSelf, maxBondsOther);
	printMyValence();
	printMyBondedAtoms();
	if(numBonds == 0) {
		return 0;
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
				printf("Before adding sender to bonded_atoms, sender ID is %04X", other_ID);
				myID.bonded_atoms[i/2] = other_ID;
				break;
			}
			else {
				if(numBonds > 0) {
					myID.valence[i] = type;
					myID.valence[i+1] = type;
					printf("Before adding sender to bonded_atoms, sender ID is %04X \r\n", other_ID);
					myID.bonded_atoms[i/2] = other_ID;
					numBonds--;
				}
			}
		}
	}
	printMyValence();
	printMyBondedAtoms();
	return 1;
}

uint8_t breakBond(Atom* other, uint16_t senderID, uint8_t bondType) {
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

void msgState(ir_msg* msg_struct) {
	State_Msg state;
	state = *((State_Msg*)(msg_struct->msg));
	MC_Component sender_mc[15];
	unpackMolecule(state.molecule, sender_mc);
	printf("State message received. Molecule list is as follows: \r\n");
	for(uint8_t i = 0; i < 15; i++) {
		printf("%04X, ", sender_mc[i].ID);
	}
	printf("\r\n");
	for(uint8_t i = 0; i < 15; i++) {
		printf("%hu, ", sender_mc[i].atomicNum);
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
	if(!selfBondedToOther(msg_struct->sender_ID)) {
		printf("\tI'm not bonded to sender %X \r\n", msg_struct->sender_ID);
		uint8_t shouldBond = 1;
		if(isInMyMolecule(&near_atom)) {
			shouldBond = 0;
		}
		printf("\tBond type is %hu \r\n", bondType);
		if(shouldBond) {
			printf("Before attemptToBond, sender ID is %04X", msg_struct->sender_ID);
			uint8_t success = attemptToBond(&near_atom, bondType, msg_struct->sender_ID);
			if(success) {
				printf("\t Bond successful \r\n");
			}
			else printf("\tBond unsuccessful \r\n");
		}
	}
	else {
		//If self is bonded to other, and other is not bonded to self, break the half bond. 
		if (!otherBondedToSelf(&near_atom)) {
			printf("\t Breaking bond to sender \r\n");
			breakBond(&near_atom, msg_struct->sender_ID, bondType);
		}
	}
}

void init()  {
	printf_P(PSTR("INITIALIZING DROPLET. \r\n"));
	switch(get_droplet_id()){
		case 0xD0AE: MY_CHEM_ID = 6; break;
		case 0x5264: MY_CHEM_ID = 6; break;
		case 0x73AF: MY_CHEM_ID = 6; break;
		case 0x2B4E: MY_CHEM_ID = 17; break;
		case 0x7d78: MY_CHEM_ID = 8; break;
		case 0x43BA: MY_CHEM_ID = 6; break;
		case 0xB944: MY_CHEM_ID = 6; break;
		case 0xC24B: MY_CHEM_ID = 17; break;
		case 0xEEB0: MY_CHEM_ID = 17; break;
		case 0x8B46: MY_CHEM_ID = 17; break;
		case 0x3B49: MY_CHEM_ID = 53; break;
		case 0xF60A: MY_CHEM_ID = 17; break;
		case 0x4177: MY_CHEM_ID = 53; break;
		default:     MY_CHEM_ID = 1; break;
	}
	
	initAtomState();
	frameCount=0;
	frameStart=get_time();
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
					//calculateTarget(&(near_atoms[i]), near_atoms[i].range, near_atoms[i].bearing, near_atoms[i].heading);  //Comment to disable movement
				}
			}
		}
		rnb_updated = 0;
	}
	delay_ms(LOOP_PERIOD);
}

void handle_msg(ir_msg* msg_struct)  {
	printf("Got message from: %04X.\r\n", msg_struct->sender_ID);
	if(msg_struct->length==0)  printf("ERROR: Message length 0.\r\n");
	
	//State message
	if(msg_struct->length == sizeof(State_Msg))  {
		msgState(msg_struct);
	}
}