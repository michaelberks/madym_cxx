/**********
 * 
 * Copyright (c) 2003, Division of Imaging Science and Biomedical Engineering,
 * University of Manchester, UK.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 *   . Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer.
 *    
 *   . Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation 
 *     and/or other materials provided with the distribution.
 * 
 *   . Neither the name of the University of Manchester nor the names of its
 *     contributors may be used to endorse or promote products derived from this 
 *     software without specific prior written permission.
 * 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 **********
 * 
 * Program :	TINA
 * File    :  $Source: /home/tina/dsk1/cvs/tina-libs/tina/sys/sysGen_swap.c,v $
 * Date    :  $Date: 2003/09/22 16:09:02 $
 * Version :  $Revision: 1.5 $
 * CVS Id  :  $Id: sysGen_swap.c,v 1.5 2003/09/22 16:09:02 tony Exp $
 *
 * Author  :  Legacy TINA
*/
/**
 * @file Endian reversal routines.
 * @brief A flag is set to indicate the need for endian reversal on input of all binary
 * data from file. The routines below will then automatically handle byte swapping
 * as the data is read.
 *
 *********
*/

#include "sysGen_swap.h"

#if HAVE_CONFIG_H
  #include <config.h>
#endif


static int swapping_flag = 0;		/* static data! */

int get_swapping_ts()
{
	return swapping_flag;
}

void set_swapping_ts(int w)
{
	swapping_flag = w;
}


void longd_swap(char *d)
{
	union swap
	{
		char in[16];
		// a.lacey@man.ac.uk 5.9.03
		// removed long as unnecessary
		// long double out
		double out;
	}
	Swap;

	if (!swapping_flag)
		return;
	Swap.in[0] = *(d + 15);
	Swap.in[1] = *(d + 14);
	Swap.in[2] = *(d + 13);
	Swap.in[3] = *(d + 12);
	Swap.in[4] = *(d + 11);
	Swap.in[5] = *(d + 10);
	Swap.in[6] = *(d + 9);
	Swap.in[7] = *(d + 8);
	Swap.in[8] = *(d + 7);
	Swap.in[9] = *(d + 6);
	Swap.in[10] = *(d + 5);
	Swap.in[11] = *(d + 4);
	Swap.in[12] = *(d + 3);
	Swap.in[13] = *(d + 2);
	Swap.in[14] = *(d + 1);
	Swap.in[15] = *d;
	// a.lacey@man.ac.uk 5.9.03
	// removed long as unnecessary
	// *long (double *) d = Swap.out;
	*(double *) d = Swap.out;
}


void long_swap(char *d)
{
	union swap
	{
		char in[8];
		double out;
	}
	Swap;

	if (!swapping_flag)
		return;
	Swap.in[0] = *(d + 7);
	Swap.in[1] = *(d + 6);
	Swap.in[2] = *(d + 5);
	Swap.in[3] = *(d + 4);
	Swap.in[4] = *(d + 3);
	Swap.in[5] = *(d + 2);
	Swap.in[6] = *(d + 1);
	Swap.in[7] = *d;
	*(double *) d = Swap.out;
}


void word_swap(char *d)
{
	union swap
	{
		char in[4];
		int out;
	}
	Swap;

	if (!swapping_flag)
		return;
	Swap.in[0] = *(d + 3);
	Swap.in[1] = *(d + 2);
	Swap.in[2] = *(d + 1);
	Swap.in[3] = *d;
	*(int *) d = Swap.out;
}


void short_swap(char *d)
{
	union swap
	{
		char in[4];
		short out;
	}
	Swap;

	if (!swapping_flag)
		return;
	Swap.in[0] = *(d + 1);
	Swap.in[1] = *d;
	*(short *) d = Swap.out;
}
