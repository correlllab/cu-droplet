#include "user_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	num_molecule_results=0;
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	uint8_t atoms[5] = {1,1,8,6,53};
	uint32_t time_before=get_time();
	get_molecules(atoms,5);
	printf("dur: %lu\r\n",get_time()-time_before);
	delay_ms(1000);
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{

}

void _get_molecules(uint8_t* molecule, uint8_t moleculeSize, uint8_t* atomIDs, uint8_t atomIDsSize, uint16_t trieIdx, uint8_t hasValFlag)
{
	if(num_molecule_results>=MAX_NUM_MOLECULE_RESULTS) return;
	uint16_t childStartIdx;
	if(hasValFlag){
		for(uint8_t i=0;i<moleculeSize;i++)
		{
			results_storage[num_molecule_results].atoms[i] = molecule[i];
		}
		results_storage[num_molecule_results].numAtoms = moleculeSize;
		results_storage[num_molecule_results].deltaG = pgm_read_word(&(MOLECULE_LOOKUP[trieIdx]));
		num_molecule_results++;
		childStartIdx=trieIdx+2;
	}else{
		childStartIdx=trieIdx;
	}
	uint8_t numAtomsLeft=0;
	for(uint8_t i=0;i<atomIDsSize;i++) if(atomIDs[i]!=0) numAtomsLeft++;
	if(numAtomsLeft==0) return;
	uint8_t childDataSize = pgm_read_byte(&(MOLECULE_LOOKUP[childStartIdx]));
	uint8_t i=1;
	while(i<childDataSize){
		uint8_t childKeyByte = pgm_read_byte(&(MOLECULE_LOOKUP[childStartIdx+i]));
		i+=1;
		uint16_t childIdxOffset;
		if(childKeyByte&0x80){
			childIdxOffset = pgm_read_word(&(MOLECULE_LOOKUP[childStartIdx+i]));
			i+=2;
		}else{
			childIdxOffset = pgm_read_byte(&(MOLECULE_LOOKUP[childStartIdx+i]));
			i+=1;
		}
		uint8_t childValFlag;
		if(childKeyByte&0x40){
			childValFlag=0;
		}else{
			childValFlag=1;
		}
		uint8_t childKey = childKeyByte&0x3F;
		uint16_t childIdx = trieIdx-childIdxOffset;
		for(uint8_t j=0;j<atomIDsSize;j++)
		{
			if(atomIDs[j]==childKey){
				//printf("Found match. %hu:%hu\r\n",atomIDs[j],childKey);
				atomIDs[j]=0;
				molecule[moleculeSize]=childKey;
				_get_molecules(molecule, moleculeSize+1, atomIDs, atomIDsSize, childIdx, childValFlag);
				atomIDs[j]=childKey;
				break;
			}
		}
	}
}

int comp_func(molecule_result* a, molecule_result* b)
{
	int16_t aScore = (a->deltaG)/(a->numAtoms);
	int16_t bScore = (b->deltaG)/(b->numAtoms);
	if(aScore<bScore) return -1;
	else if(aScore==bScore) return 0;
	else return 1;
}

uint8_t checkMake(uint8_t* atomSet, uint8_t atomSetSize, uint8_t* molecule, uint8_t moleculeSize)
{
	//printf("Checking make for: (");
	//for(uint8_t i=0;i<moleculeSize;i++) printf("%hu ", molecule[i]);
	//printf(") with atomSet (");	
	//for(uint8_t i=0;i<atomSetSize;i++) if(atomSet[i]!=0) printf("%hu ", atomSet[i]);
	//printf(")\r\n");
	//
	int8_t i;
	for(i=0;i<moleculeSize;i++){
		uint8_t found=0;
		for(uint8_t j=0;j<atomSetSize;j++){
			if(atomSet[j]==molecule[i]){
				found=1;
				atomSet[j]=0;
				break;
			}
		}
		if(!found){
			return 0;
		}
	}
	return 1;
}

uint8_t getBest(uint8_t* atoms, uint8_t numAtoms, uint8_t* bestMoleculeSet)
{
	uint8_t curNumAtoms=0;
	for(uint8_t i=0;i<numAtoms;i++){
		if(atoms[i]!=0) curNumAtoms++;
	}
	if(curNumAtoms==0){
		return 0;
	}
	uint8_t atoms_copy[numAtoms];
	for(uint8_t i=0;i<numAtoms;i++) atoms_copy[i]=atoms[i];
	for(uint8_t i=0;i<num_molecule_results;i++)
	{
		uint8_t numAtomsInMolecule = results_storage[i].numAtoms;
		if(checkMake(atoms, numAtoms, results_storage[i].atoms, numAtomsInMolecule)){
			uint8_t idx = getBest(atoms, numAtoms, bestMoleculeSet);
			bestMoleculeSet[idx] = i;
			return idx+1;
		}else{
			for(uint8_t j=0;j<numAtoms;j++) atoms[j]=atoms_copy[j];
		}
	}
}

void get_molecules(uint8_t* atoms, uint8_t numAtoms)
{
	uint8_t molecule_arr[12];
	uint16_t rootIdx = pgm_read_word(&(MOLECULE_LOOKUP[0]));
	num_molecule_results=0;
	_get_molecules(molecule_arr, 0, atoms, numAtoms, rootIdx, 0);
	qsort(results_storage,num_molecule_results,sizeof(molecule_result),comp_func);
	//for(uint8_t i=0;i<num_molecule_results;i++)
	//{
		//uint8_t numMoleculeAtoms = results_storage[i].numAtoms;
		//printf("(");
		//for(uint8_t j=0;j<numMoleculeAtoms;j++) printf("%hu ", results_storage[i].atoms[j]);
		//printf("): %d\r\n", results_storage[i].deltaG);
	//}	
	uint8_t bestMoleculeSet[numAtoms];
	uint8_t numBest = getBest(atoms, numAtoms, bestMoleculeSet);
	int16_t totalScore=0;
	for(uint8_t i=0;i<numBest;i++)
	{
		uint8_t numMoleculeAtoms = results_storage[bestMoleculeSet[i]].numAtoms;
		totalScore+=results_storage[bestMoleculeSet[i]].deltaG;
		printf("(");
		for(uint8_t j=0;j<numMoleculeAtoms;j++) printf("%hu ", results_storage[bestMoleculeSet[i]].atoms[j]);
		printf(")\r\n");	
	}
	printf("deltaG: %d\r\n", totalScore);	
}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args)
//{
	//return 0;
//}

///*
 //*	The function below is optional - if it is commented in, and the leg interrupts have been turned on
 //*	with enable_leg_status_interrupt(), this function will get called when that interrupt triggers.
 //*/	
//void	user_leg_status_interrupt()
//{
	//
//}
