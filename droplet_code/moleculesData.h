/*
	Credit for the trie algorithm used in these files goes to http://www.google.com/url?q=http%3A%2F%2Fwww.aclweb.org%2Fanthology%2FW%2FW09%2FW09-1505.pdf&sa=D&sntz=1&usg=AFQjCNFrdZl2iVyGIhd0ArX7VhukM4AQRg.
	Data for atoms that are usually diatomic (H, N, O, F, Cl, Br, I) in their monatomic gaseous state came from http://www.google.com/url?q=http%3A%2F%2Fwebbook.nist.gov%2Fchemistry%2F&sa=D&sntz=1&usg=AFQjCNGmDnbV-sNyycBhng6UOxWOS3s5rA
	All other data for delta G came from the DIPPR database found at http://app.knovel.com/web/view/itable/show.v/rcid:kpDIPPRPF7/cid:kt00B0N0L7/viewerType:itble/root_slug:dippr-project-801-full/url_slug:thermodynamic-properties?b-toc-cid=kpDIPPRPF7&b-toc-root-slug=dippr-project-801-full&b-toc-url-slug=thermodynamic-properties&b-toc-title=DIPPR%20Project%20801%20-%20Full%20Version&start=0&columns=1,2,3,24,21,4,5,22,6,7,23,8,9,10,11,12,13,14,15,16,17,20,19,18
	Units for deltaG are in hJ/mol
*/

#pragma once

#include <avr/io.h>
#include <avr/pgmspace.h>

#define MAX_NUM_MOLECULE_RESULTS 50
#define MAX_NUM_ATOMS_RESULTS 255

typedef struct molecule_result_struct{
	int16_t deltaG;
	uint8_t* atoms;
	uint8_t numAtoms;
} molecule_result;

molecule_result results_storage[MAX_NUM_MOLECULE_RESULTS];

//This function will populate bestMoleculeSet with indices into results_storage.
//Make sure to initialize bestMoleculeSet to be the same length as atoms.
//numAtoms should have this length.
uint8_t get_molecules(uint8_t* atoms, uint8_t* bestMoleculeSet, uint8_t numAtoms);
uint8_t molecule_search(uint8_t* atoms, int16_t* deltaG, uint8_t numAtoms);


int comp_func(molecule_result* a, molecule_result* b);