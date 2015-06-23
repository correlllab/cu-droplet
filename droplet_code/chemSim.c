#include "main.h"

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
	printf("PRINTING NEAR_ATOMS: \r\n");
	for(uint8_t i = 0; i < 12; i++)
	{
		if(near_atoms[i].id==0) break;
		printf("Atom: %s  Rng: %hu ID: %04X \r\n", near_atoms[i].atom.name, near_atoms[i].range, near_atoms[i].id);
	}
	
	printf("\r\n");
	printf("\r\n");
	printf("\t Bonded droplets: \r\n");
	for(uint8_t i = 0; i < 6; i++)
	{
		printf("\t %04X \r\n", myID.bonded_atoms[i]);
		//if(bonded_atoms[i] == 0) break;
	}
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
			printf("Adding %s \r\n", near_atom.atom.name);
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
			printf("Removing %c \r\n", near_atoms[i].atom.name);
		}
		else if(near_atoms[i].id != 0)
		{
			near_atoms->last_msg_t += UPDATE_ATOMS_PERIOD; //assuming update is called every 100 ms
		}
	}
}

//Returns a 3-char array containing [deltaH of formation, deltaG of formation, and S naught].
//In other words, [enthalpy, gibbs free energy, entropy]
//Note that this is for individual atoms, not diatomics. Case 1 returns for H, not H2
//Units are kJ/mol, except for S, which is in kJ/mol*K
float* getThermoInfo(uint8_t atomicNum, uint8_t phase, uint8_t diatomic)
{
	
	if(diatomic == 0) //get the monotomic numbers
	{
		if(phase == 1) //element is a gas
		{
			switch(atomicNum){
				case 1:
				{
					float thermo[3] = {218.0, 203.30, 0.11460};
					return thermo;
				}
				case 2:
				{
					//This is oversimplified, aka, wrong. Pending more research on Gibbs free energy and noble gases, it'll have to do.
					float thermo[3] = {0, -1.000, 0.1262};
					return thermo;
				}
				case 3:
				{
					float thermo[3] = {161.0, 128.0, 0.13867};
					return thermo;
				}
				case 4:
				{
					float thermo[3] = {-1.000, -1.000, -1.000};
					return thermo;
				}
				case 6:
				{
					float thermo[3] = {715.0, 669.6, 0.1580};
					return thermo;
				}
				case 7:
				{
					float thermo[3] = {473.0, 456.0, 0.1532};
					return thermo;
				}
				case 8:
				{
					float thermo[3] = {249.2, 231.7, 0.16095};
					return thermo;
				}
				case 9:
				{
					float thermo[3] = {78.9, 61.8, 0.15864};
					return thermo;
				}
				case 17:
				{
					float thermo[3] = {121.0, 105.0, 0.1651};
					return thermo;
				}
				case 35:
				{
					float thermo[3] = {111.9, 82.40, 0.17490};
					return thermo;	
				}
				case 53:
				{
					float thermo[3] = {106.8, 70.21, 0.18067};
					return thermo;
				}
				default:
					printf("No such element");
					float thermo[3] = {-1.0, -1.0, -1.0};
					return thermo;
			}
		}
	
		else if (phase == 2) //element is aqueous, which means it probably has a formal charge != 0. The most common formal charge has been assumed.
		{
			switch(atomicNum){
				case 1: //H+
				{
					float thermo[3] = {0, 0, 0};
					return thermo;
				}
				case 3: //Li+
				{
					float thermo[3] = {-278.46, -293.8, 0.014};
					return thermo;
				}
				case 9: //F-
				{
					float thermo[3] = {-329.1, -276.5, -0.0096};
					return thermo;
				}
				case 17: //Cl-
				{
					float thermo[3] = {-167.46, -131.17, 0.05510};
					return thermo;
				}
				case 35: //Br-
				{
					float thermo[3] = {-120.9,  -102.82, 0.08071};
					return thermo;	
				}
				case 53: //I-
				{
					float thermo[3] = {-55.94, -51.67, 0.1094};
					return thermo;
				}
				default:
				{
					printf("No such element in that state");
					float thermo[3] = {-1.0, -1.0, -1.0};
					return thermo;
				}
			}
		}
		else if(phase == 3) //element is solid
		{
			switch(atomicNum){
				case 3: 
				{
					float thermo[3] = {0, 0, 0.02910};
					return thermo;
				}
				default:
				{
					printf("No such element in that state");
					float thermo[3] = {-1.0, -1.0, -1.0};
					return thermo;
				}
		}
	}
	else //get the diatomic numbers
	{
		switch(atomicNum){
			case 1: //H2
			{
				float thermo[3] = {0, 0, 0.1306};
				return thermo;
			}
			case 7: //N2
			{
				float thermo[3] = {0, 0, 0.1915};
				return thermo;
			}
			case 8: //O2
			{
				float thermo[3] = {0, 0, 0.2050};
				return thermo;
			}
			case 9: //F2
			{
				float thermo[3] = {0, 0, 0.2027};
				return thermo;
			}
			case 17: //Cl2
			{
				float thermo[3] = {0, 0, 0.2230};
				return thermo;
			}
			case 35: //Br2
			{
				if(phase == 2) {
					float thermo[3] = {0, 0, 0.15223};
					return thermo;
				}
				else {
					float thermo[3] = {30.91, 3.13, 0.24538};
					return thermo;
				}
			}
			case 53: //I2
			{
				if(phase == 3) {
					float thermo[3] = {0, 0, 0.11614};
					return thermo;
				}
				else {
					float thermo[3] = {62.442, 19.38, 0.26058};
					return thermo;
				}
			}
			default:
			{
				printf("No such element in that state");
				float thermo[3] = {-1.0, -1.0, -1.0};
				return thermo;
			}
		}
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
	printf("\r\n broadcastChemID called \r\n");
	uint8_t r=get_red_led(), g=get_green_led(), b=get_blue_led();
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

uint8_t valenceFull()
{
	for(uint8_t i = 0; i < 8; i++)
	{
		if(myID.valence[i] == 0) return 0; //false
	}
	return 1; //true
}

void detectOtherDroplets()
{
	//printf("detectOtherDroplets called");
	uint16_t received_id;
	float received_range;
	float received_bearing;
	float received_heading;
	uint8_t dir_mask = check_collisions();
	//printf("Collisions: %02hX \r\n", dir_mask);
	
	if(rnb_updated)
	{
		received_id = last_good_rnb.id_number;
		received_range = last_good_rnb.range;
		received_bearing = last_good_rnb.bearing;
		received_heading = last_good_rnb.heading;
		//convert to degrees from radians
		received_bearing = rad_to_deg(received_bearing);
		received_heading = rad_to_deg(received_heading);
		//scaling the range to mm.
		received_range = received_range*10;
		printf("range: %f\r\n", received_range);
		
		for(uint8_t i = 0; i < 12; i++)
		{
			if(near_atoms[i].id == received_id)
			{
				near_atoms[i].bearing = (int16_t)received_bearing;
				near_atoms[i].heading = (int16_t)received_heading;
				near_atoms[i].range = (uint8_t)received_range;
			}
		}
		print_near_atoms();
		broadcastChemID(myID);
		rnb_updated=0;
	}
}

//Checks to see what kind of bonds the newly discovered near_atom can make with me. Returns 1 if a bond was formed, 0 otherwise.
uint8_t checkPossibleBonds(Atom near_atom, uint16_t senderID)
{
	unsigned char diatomic[9];
	unsigned char newValence[9];
	uint8_t myBonds = 0;
	uint8_t otherBonds;
	uint8_t my_empty = 0; //number of empty electron slots in my valence shell
	uint8_t other_empty = 0; //number of empty electron slots in near_atom's valence shell
	
	float deltaChi;
	if(myID.chi > near_atom.chi) deltaChi = myID.chi - near_atom.chi;
	else deltaChi = near_atom.chi - myID.chi;
	printf("*********************Ionic bond forming? deltaChi is %f", deltaChi);
	
	for(uint8_t i = 0; i < 8; i++)
	{
		if(near_atom.valence[i] == 0) other_empty++;
		if(myID.valence[i] == 0) my_empty++;
		if(near_atom.valence[i] == 2) otherBonds++;
		if(myID.valence[i] == 2) myBonds++;
	}
	myBonds/=2;
	otherBonds/=2;
	
	//Diatomic bond?
	if(near_atom.diatomic == 1 && near_atom.atomicNum == myID.atomicNum && other_empty != 0 && my_empty != 0 && otherBonds == 0 && myBonds == 0) 
	{
		printf("\tEntered diatomic if statement.\r\n");
		diatomic[0] = 'd';
		myID.bondType = 2;
		uint8_t empty_slot_counter = other_empty;
		uint8_t free_electron_counter = other_empty;
		for(uint8_t i = 1; i < 9; i++)
		{
			printf("near_atom.valence[i] = %hd \r\n ", near_atom.valence[i-1]);
			if(near_atom.valence[i-1] == 0 && empty_slot_counter > 0)
			{
				diatomic[i] = 2;
				empty_slot_counter--;
			}
			else if(near_atom.valence[i-1] == 1 && free_electron_counter > 0)
			{
				diatomic[i] = 2;
				free_electron_counter = free_electron_counter - 1;
			}
			else diatomic[i] = near_atom.valence[i-1];
		}
		//printf("At the end of the diatomic statement in checkPossibleBonds, valence is: ");
		//printValence(diatomic);
		//for(uint8_t i=0; i<9; i++) printf(" %hd ", (int8_t)diatomic[i]); 
		//printf("\r\n");
		found_diatomic_routine();
		for(uint8_t i = 0; i < 6; i++)
		{
			if(myID.bonded_atoms[i] == 0)
			{
				myID.bonded_atoms[i] = senderID;
				break;
			}
		}
		ir_targeted_send(ALL_DIRS, diatomic, 9, senderID);
		return 1;
	}
	
	//Ionic bond?
	else if(deltaChi > 1.5)
	{
		newValence[0] = 'i';
		if(near_atom.bondType == 2 || myID.bondType == 2) return -1; //near_atom is already bonded covalently, or I am
		//How do I check if there's more than one type of cation to each anion?
		
		if(otherBonds == 0 && myBonds == 0) //CHANGE THIS CONDITION
		{
			add_to_bonded_atoms(senderID);
			myID.bondType = 1;
			uint8_t zero = 1;
			uint8_t one = 1;
			if(myID.chi > near_atom.chi)
			{
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
				newValence[1] = near_atom.valence[0];
				for(uint8_t i = 1; i < 8; i++)
				{
					if(near_atom.valence[i] == 0 && near_atom.valence[i-1] == 1) newValence[i-1] = 0;
					else newValence[i+1] = near_atom.valence[i];
				}
				found_bond_routine();
				ir_targeted_send(ALL_DIRS, newValence, 9, senderID);
			}
			else
			{
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
				for(uint8_t i = 1; i < 8; i++)
				{
					if(myID.valence[i+1] ==0 && myID.valence[i] == 1) 
					{
						myID.valence[i] = 0;
						break;
					}
				}
				ir_targeted_send(ALL_DIRS, newValence, 9, senderID);
			}
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
		else if(myID.bonded_atoms[i] == ID) printf("ERROR: Tried to add ID to bonded_atoms while it was already there.");
	}
	if(slotFound == 0) printf("ERROR: Tried to add an ID to bonded_atoms but the array was full.");
}

void printValence(int8_t valence[])
{
	for(uint8_t i = 0; i < 8; i++)
	{
		printf(" %hd ", valence[i]);
	}
	printf("\r\n");
}

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	switch(get_droplet_id()){
		case 0xAFD8: MY_CHEM_ID = 3; break;
		case 0x3062: MY_CHEM_ID = 17; break;
		case 0xFA6F: MY_CHEM_ID = 6; break;
		case 0x6C6F: MY_CHEM_ID = 3; break;
		case 0xD86C: MY_CHEM_ID = 17; break;
		case 0xB36F: MY_CHEM_ID = 17; break;
		case 0x6B6F: MY_CHEM_ID = 17; break;
		case 0xBC6E: MY_CHEM_ID = 17; break;
		case 0x46A1: MY_CHEM_ID = 3; break;
		default:     MY_CHEM_ID = 3; break;
	}
	

	for(uint8_t i = 0; i < 12; i++)
	{
		near_atoms[i] = NULL_NEAR_ATOM;
	}
	//set_rgb(10, 255, 255); //this is a test line
	myID = getAtomFromAtomicNum(MY_CHEM_ID);
	schedule_periodic_task(300, update_near_atoms, NULL);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	delay_ms(500);
	//broadcastChemID(myID);
	detectOtherDroplets();
	uint32_t time_floor = ((get_time()/500)*500);
	if(time_floor%4000==0){
		printf("\r\n sent bonded_atoms\r\n");
		broadcast_rnb_data();
		ir_send(ALL_DIRS, myID.bonded_atoms, 12); //Should this be here or inside the 5 second loop? Also, do I have the last parameter right? 12 bytes?
	}
}

////this recursively schedules itself to happen every RNB_BROADCAST_PERIOD, which is defined in main.h. It broadcasts my range and bearing (rnb) data.
//void periodic_rnb_broadcast()
//{
	////printf("periodic_rnb_broadcast \r\n");
	//broadcast_rnb_data();
	//schedule_task(RNB_BROADCAST_PERIOD, periodic_rnb_broadcast, NULL);
//}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{
	uint8_t r=get_red_led(), g=get_green_led(), b=get_blue_led();
	//set_rgb(0,0,0);
	delay_ms(50);
	Atom* near_atom;
	printf("message length: %u and atom length: %u\r\n", msg_struct->length, sizeof(Atom));
	
	if(msg_struct->length==0)
	{
		printf("ERROR: Message length 0.\r\n");
	}
	//Message is an Atom struct
	if(msg_struct->length==sizeof(Atom))
	{
		printf("RECEIVED ATOM STRUCT \r\n");
		near_atom = (Atom*)(msg_struct->msg); //do some kind of check to make sure this is actually an atom.
		
		//If this droplet isn't in our list, add it. If it is, update its last_msg_t to 0.
		uint8_t found = 0;
		for(uint8_t i = 0; i < 12; i++)
		{
			//printf("near_atoms[i].id = %u and msg_struct.sender_id is %u \r\n", near_atoms[i].id, msg_struct->sender_ID);
			if(near_atoms[i].id ==msg_struct->sender_ID)
			{
				near_atoms[i].last_msg_t = 0;
				found = 1;
				break;
			}
		}
		printf("Check_possible_bonds called");
		uint8_t isBonded = checkPossibleBonds(*near_atom, msg_struct->sender_ID);
		if (found == 0) //add new droplet to near_atoms
		{
			Near_Atom close_atom = {*near_atom, msg_struct->sender_ID, 0, 0, 0, 0, 0};
			if(isBonded) close_atom.bonded = 1;
			add_to_near_atoms(close_atom);
		}
	}
	
	//Message is that a diatomic bond was formed
	else if(msg_struct->msg[0] == 'd')
	{
		printf("Got 'diatomic bond made' message.\r\n");
		found_diatomic_routine();
		myID.bondType = 2;
		for(uint8_t i = 0; i < 8; i++)
		{
			myID.valence[i] = msg_struct->msg[i+1];
		}
		printf("After receiving the 'bond found' message, valence is: ");
		printValence(myID.valence);
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
	
	//Message is that a bond was formed
	else if(msg_struct->msg[0] == 'c' || msg_struct->msg[0] == 'i')
	{
		printf("Got 'bond made' message.\r\n");
		found_bond_routine();
		if(msg_struct->msg[0] == 'c') myID.bondType = 2;
		else myID.bondType = 1;
		for(uint8_t i = 0; i < 8; i++)
		{
			myID.valence[i] = msg_struct->msg[i+1];
		}
		printf("After receiving the 'bond found' message, valence is: ");
		printValence(myID.valence);
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
	
	//Message is another Droplet's bonded_atoms array
	else if(msg_struct->length == sizeof(myID.bonded_atoms))
	{
		//printf("Received bonded_atoms \r\n");
		//How does the code know this is an array of uint16_ts? Can I iterate through it like usual?
		//Check to see if I'm bonded to the droplet who just sent me his bonded_atoms array
		uint8_t senderIDFound = 0;
		uint8_t i;
		for(i = 0; i < 6; i++)
		{
			if(myID.bonded_atoms[i] == msg_struct->sender_ID)
			{
				senderIDFound = 1;
				break;
			}
		}
		//If so, check to see if he's bonded to me. If he isn't, break my bond.
		if(senderIDFound == 1)
		{
			uint8_t myIdFound = 0;
			for(uint8_t j = 0; j < 6; j++)
			{
				if(msg_struct->msg[j] == get_droplet_id())
				{
					myIdFound = 1;
					break;
				}
			}
			if(myIdFound == 0)
			{
				printf("I think I'm bonded to a droplet who doesn't think he's bonded to me. I'm telling him to make that bond. \r\n");
				//setAtomColor(myID);
				char make_bond[2] = {'m', myID.bondType};
				ir_targeted_send(ALL_DIRS, make_bond, 2, msg_struct->sender_ID);
				//Remove other droplet from bonded_atoms and remove the bonded flag from near_atoms
				/*myID.bonded_atoms[i] = 0;
				for(uint8_t k = 0; k < 12; k++)
				{
					if(near_atoms[k].id == msg_struct->sender_ID) 
					{
						near_atoms[k].bonded = 0;
						break;
					}
				}
				*/
			}
		}
		
	}
	//set_rgb(r,g,b);
	else if(msg_struct->msg[0] == 'm')
	{
		add_to_bonded_atoms(msg_struct->sender_ID);
		uint8_t zero = 1;
		uint8_t one = 1;
		switch(msg_struct->msg[1])
		{case 0:
			printf("Something's wrong. Probably bondType didn't get set on something somewhere.");
			break;
		case 1:
			//Make an ionic bond. For now, we'll say if your chi < 2.00, you're the cation, if not, anion.
			set_rgb(255, 0, 0);
			if(myID.chi < 2.00)
			{
				for(uint8_t i = 1; i < 8; i++)
				{
					if(myID.valence[i] == 0 && myID.valence[i-1] == 1) myID.valence[i-1] = 0;
				}
			}
			else
			{
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
			}
		case 2:
			set_rgb(255, 255, 0);
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
		}
		
	}
}



