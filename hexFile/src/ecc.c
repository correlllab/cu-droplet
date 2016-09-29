#include "ecc.h"


uint16_t manchester_encode( uint8_t data )
{
	uint16_t cw = (0x4000 & (data << 7)) | (0x1000 & (data << 6)) | (0x0400 & (data << 5)) | (0x0100 & (data << 4))
	| (0x0040 & (data << 3)) | (0x0010 & (data << 2)) | (0x0004 & (data << 1)) | (0x0001 & data);
	return (cw & 0x5555) | ((~((cw & 0x5555) << 1)) & 0xAAAA);
}

uint8_t manchester_decode( uint16_t cw )
{
	return ((cw & 0x4000) >> 7) | ((cw & 0x1000) >> 6) | ((cw & 0x0400) >> 5) | ((cw & 0x0100) >> 4)
	| ((cw & 0x0040) >> 3) | ((cw & 0x0010) >> 2) | ((cw & 0x0004) >> 1) | (cw & 0x0001);
}

uint8_t manchester_verify( uint16_t cw )
{
	cw = (~(cw & 0xAAAA) >> 1) ^ (cw & 0x5555);
	return manchester_decode(cw);
}


#define POLY  0xAE3  /* or use the other polynomial, 0xC75 */

/* ====================================================== */

uint8_t parity(uint32_t cw)
/* This function checks the overall parity of 24 bit codeword cw.
If parity is even, 0 is returned, else 1. */
{
	unsigned char p;

	/* XOR the bytes of the codeword */
	p=*(unsigned char*)&cw;
	p^=*((unsigned char*)&cw+1);
	p^=*((unsigned char*)&cw+2);

	/* XOR the halves of the intermediate result */
	p=p ^ (p>>4);
	p=p ^ (p>>2);
	p=p ^ (p>>1);

	/* return the parity result */
	return(p & 1);
}

/* ====================================================== */

uint32_t syndrome(uint32_t cw)
/* This function calculates and returns the syndrome
of a [23,12] Golay codeword. */
{
	int i;
	cw&=0x7fffffl;
	for (i=1; i<=12; i++)  /* examine each data bit */
	{
		if (cw & 1)        /* test data bit */
		cw^=POLY;        /* XOR polynomial */
		cw>>=1;            /* shift intermediate result */
	}
	return(cw<<12);        /* value pairs with upper bits of cw */
}

/* ====================================================== */

uint8_t weight(uint32_t cw)
/* This function calculates the weight of
23 bit codeword cw. */
{
	uint8_t bits,k;

	/* nibble weight table */
	const char wgt[16] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};

	bits=0; /* bit counter */
	k=0;
	/* do all bits, six nibbles max */
	while ((k<6) && (cw))
	{
		bits=bits+wgt[cw & 0xf];
		cw>>=4;
		k++;
	}

	return(bits);
}

/* ====================================================== */

uint32_t rotate_left(uint32_t cw, uint8_t n)
/* This function rotates 23 bit codeword cw left by n bits. */
{
	int i;

	if (n != 0)
	{
		for (i=1; i<=n; i++)
		{
			if ((cw & 0x400000l) != 0)
			cw=(cw << 1) | 1;
			else
			cw<<=1;
		}
	}

	return(cw & 0x7fffffl);
}

/* ====================================================== */

uint32_t rotate_right(uint32_t cw, uint8_t n)
/* This function rotates 23 bit codeword cw right by n bits. */
{
	int i;

	if (n != 0)
	{
		for (i=1; i<=n; i++)
		{
			if ((cw & 1) != 0)
			cw=(cw >> 1) | 0x400000l;
			else
			cw>>=1;
		}
	}

	return(cw & 0x7fffffl);
}

uint32_t golay_encode(uint16_t data)
/* This function calculates [23,12] Golay codewords.
The format of the returned longint is
[parity(1),checkbits(11),data(12)]. */
{
	uint8_t i;
	uint32_t c;
	data &= 0xfffl;
	c = data;						/* save original codeword */
	for (i = 1; i <= 12; i++)  /* examine each data bit */
	{
		if (c & 1)        /* test data bit */
		c^=POLY;        /* XOR polynomial */
		c >>= 1;            /* shift intermediate result */
	}
	c = ((c << 12) | data);
	return ((((uint32_t)parity(c) << 23) | c) ^ 0x7FF000);    /* assemble codeword */
}

/* ====================================================== */

uint32_t correct(uint32_t cw, int8_t *errs)
/* This function corrects Golay [23,12] codeword cw, returning the
corrected codeword. This function will produce the corrected codeword
for three or fewer errors. It will produce some other valid Golay
codeword for four or more errors, possibly not the intended
one. *errs is set to the number of bit errors corrected. */
{
	uint8_t	w;                /* current syndrome limit weight, 2 or 3 */
	uint32_t mask;             /* mask for bit flipping */
	int8_t i,j;              /* index */
	uint32_t s,                /* calculated syndrome */
			 cwsaver;          /* saves initial value of cw */

	cwsaver=cw;         /* save */
	*errs=0;
	w=3;                /* initial syndrome weight threshold */
	j=-1;               /* -1 = no trial bit flipping on first pass */
	mask=1;
	while (j<23) /* flip each trial bit */
	{
		if (j != -1) /* toggle a trial bit */
		{
			if (j>0) /* restore last trial bit */
			{
				cw=cwsaver ^ mask;
				mask+=mask; /* point to next bit */
			}
			cw=cwsaver ^ mask; /* flip next trial bit */
			w=2; /* lower the threshold while bit diddling */
		}

		s=syndrome(cw); /* look for errors */
		if (s) /* errors exist */
		{
			for (i=0; i<23; i++) /* check syndrome of each cyclic shift */
			{
				if ((*errs=weight(s)) <= w) /* syndrome matches error pattern */
				{
					cw=cw ^ s;              /* remove errors */
					cw=rotate_right(cw,i);  /* unrotate data */
					//*errs |= IR_RX_STATUS_ECCERR_bm;
					return(s=cw);
				}
				else
				{
					cw=rotate_left(cw,1);   /* rotate to next pattern */
					s=syndrome(cw);         /* calc new syndrome */
				}
			}
			j++; /* toggle next trial bit */
		}
		else
		return(cw); /* return corrected codeword */
	}

	return(cwsaver); /* return original if no corrections */
} /* correct */

/*
uint8_t golay_verify(uint32_t cw)
{
	uint8_t errs = 0;
	
	cw ^= 0x7FF000;

	if (parity(cw))
		errs |= IR_RX_STATUS_ECCERR_bm;

	cw &= ~0x800000l; 

	if (syndrome(cw))
		errs |= IR_RX_STATUS_ECCERR_bm;

	return(errs);
}
*/

uint8_t golay_find_errors(uint32_t cw)
{
	cw ^= 0x7FF000;

	if (parity(cw))
		return 1;

	cw &= ~0x800000l;

	if (syndrome(cw))
		return 2;

	return 0;
}

uint16_t golay_decode_fast(uint32_t cw)
{
	return (cw & 0xFFF);
}

// SOURCE: www.aqdi.com/golay.htm

uint16_t golay_decode(uint32_t cw, int8_t *errs)
{
	uint32_t parity_bit;

	cw ^= 0x7FF000;		// THERE IS NO JUSTIFICATION FOR THIS LINE, WHAT IS IT DOING HERE??? ref: www.aqdi.com/golay.htm

	parity_bit = cw & 0x800000l; /* save parity bit */
	cw &= ~0x800000l;            /* remove parity bit for correction */

	cw = correct(cw, errs);     /* correct up to three bits */		//dont put the star next to errs
	cw |= parity_bit;            /* restore parity bit */

	/* check for 4 bit errors */
	if (parity(cw))            /* odd parity is an error */
		//*errs |= IR_RX_STATUS_ECCERR_bm;			// NDF: WHAT?? according to correct(), *errs is set to the number of bit errors corrected
		*errs = 1;
	return (cw & 0xFFF);
}
