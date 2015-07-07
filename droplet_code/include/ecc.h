/** \file *********************************************************************
 * \brief Code for Droplet IR communication message error detection and correction 
 *
 *****************************************************************************/
#pragma once

#include <avr/io.h>
#include <stdint.h>
#include "ir_comm.h"

// Encodes 8 bit data to 16 bit Manchester encoding
uint16_t manchester_encode( uint8_t data );

// Decodes 16 bit Manchester-encoded codeword to 8 bit data
// Does not perform error detection
uint8_t manchester_decode( uint16_t cw ); 

// Detects whether cw is a valid 16 bit Manchester-encoded codeword
// Returns 0 if cw is valid, non-zero if cw is not valid
// The return value specifies the bit position(s) where cw is not valid
uint8_t manchester_verify( uint16_t cw );



// Encodes low-order 12 bits of data into low-order 24 bits of return value
// using the Golay (24,12) error correcting code
uint32_t golay_encode(uint16_t data);

// Returns decoded 12 bit data from codeword cw.  
// Does not perform error detection or correction
uint16_t golay_decode_fast(uint32_t cw);

// Returns best-guess decoded 12 bit data from codeword cw.
// Performs error detection and attempts to correct errors.
// Puts the number of errors corrected in errs and negates that if there
// is an overall parity error
uint16_t golay_decode(uint32_t w, int8_t *errs);

/*
// Detects whether cw is a valid codeword
// Returns 0 if cw is valid, non-zero if cw is invalid
uint8_t golay_verify(uint32_t cw);
*/

// Detects whether cw is a valid codeword
// Returns 0 if cw is valid,
// Returns 1 if parity error
// Returns 2 if syndrome error
uint8_t golay_find_errors(uint32_t cw);