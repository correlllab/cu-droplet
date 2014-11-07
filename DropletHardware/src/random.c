#include "random.h"

uint8_t r_position;
uint8_t r_round;

void random_init()
{
	// Advanced Encryption Standard (AES) crypto module [one of two Onboard Crypto Engines]

	// Xmega AU Manual, p. 316:
	// The AES crypto module encrypts and decrypts 128-bit data blocks with the use of a 128-bit key.
    // The key and data must be loaded into the key and state memory in the module before encryption/
    // decryption is started. It takes 375 peripheral clock cycles before the encryption/decryption is
    // done. The encrypted/encrypted data can then be read out, and an optional interrupt can be generated.
    // The AES crypto module also has DMA support with transfer triggers when
    // encryption/decryption is done and optional auto-start of encryption/decryption when the state
    // memory is fully loaded.

	// CTRL – Control Register
	// Bit 5 – RESET: Software Reset
    // Setting this bit will reset the AES crypto module to its initial status on the next positive edge of
    // the peripheral clock. All registers, pointers, and memories in the module are set to their initial
    // value. When written to one, the bit stays high for one clock cycle before it is reset to zero by
    // hardware. (p. 320)

	AES.CTRL = AES_RESET_bm;

    // The KEY register is used to access the key memory. Before encryption/decryption can take
    // place, the key memory must be written sequentially, byte-by-byte, through the KEY register.
    // After encryption/decryption is done, the last subkey can be read sequentially, byte-by-byte,
    // through the KEY register.
    // Loading the initial data to the KEY register should be done after setting the appropriate AES
    // mode and direction. (p. 322)

	AES.KEY = PRODSIGNATURES_LOTNUM0;
	AES.KEY = PRODSIGNATURES_LOTNUM1;
	AES.KEY = PRODSIGNATURES_LOTNUM2;
	AES.KEY = PRODSIGNATURES_LOTNUM3;

	AES.KEY = PRODSIGNATURES_LOTNUM4;
	AES.KEY = PRODSIGNATURES_LOTNUM5;
	AES.KEY = PRODSIGNATURES_WAFNUM;
	AES.KEY = PRODSIGNATURES_COORDX0;

	AES.KEY = PRODSIGNATURES_COORDX1;
	AES.KEY = PRODSIGNATURES_COORDY0;
	AES.KEY = PRODSIGNATURES_COORDY1;
	AES.KEY = ADCA.CH0.RESL;

	AES.KEY = ADCA.CH1.RESL;
	AES.KEY = ADCA.CH2.RESL;
	AES.KEY = ADCB.CH0.RESL;
	AES.KEY = ADCB.CH1.RESL;

	r_round = 1;

	// The STATE register is used to access the state memory. Before encryption/decryption can take
    // place, the state memory must be written sequentially, byte-by-byte, through the STATE register.
    // After encryption/decryption is done, the ciphertext/plaintext can be read sequentially, byte-by-byte,
    // through the STATE register.
    // Loading the initial data to the STATE register should be done after setting the appropriate AES
    // mode and direction. This register can not be accessed during encryption/decryption. (p. 321)

	for (uint8_t i = 0; i < 16; i++)
		AES.STATE = r_round;

	// CTRL – Control Register
	// Bit 2 – XOR: State XOR Load Enable
    // Setting this bit enables a XOR data load to the state memory. When this bit is set, the data
    // loaded to the state memory are bitwise XORed with the data currently in the state memory. Writing
    // this bit to zero disables XOR load mode, and new data written to the state memory will
    // overwrite the current data.
	//Bit 7 – START: Start/Run
    // Setting this bit starts the encryption/decryption procedure, and this bit remains set while the
    // encryption/decryption is ongoing. Writing this bit to zero will stop/abort any ongoing encryption/
    // decryption process. This bit is automatically cleared if the SRIF or the ERROR flags in
    // STATUS are set. (p. 320)

	AES.CTRL = AES_START_bm | AES_XOR_bm;

	r_position = 0;
}

uint8_t rand_byte()
{
	uint8_t r;
	if (r_position == 0) while (!(AES.STATUS & AES_SRIF_bm));
	r = AES.STATE;
	r_position++;
	if (r_position > 15)
	{
		for (uint8_t i = 0; i < 16; i++) AES.STATE = r_round;
		AES.CTRL = AES_START_bm | AES_XOR_bm;
		r_position = 0;
	}
	return r;
}

uint16_t rand_short()
{
	return ((uint16_t)rand_byte()<<8)|((uint16_t)rand_byte());
}

uint32_t rand_quad()
{
	return ((uint32_t)rand_short()<<16)|((uint32_t)rand_short());
}
