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
 * File    :  $Source: /home/tina/dsk1/cvs/tina-libs/tina/sys/sysGen_swap.h,v $
 * Date    :  $Date: 2003/09/22 16:09:02 $
 * Version :  $Revision: 1.2 $
 * CVS Id  :  $Id: sysGen_swap.h,v 1.2 2003/09/22 16:09:02 tony Exp $
 *
 * Notes :
 *
 *********
*/

#ifndef TINA_SYS_GEN_SWAP_HDR
#define TINA_SYS_GEN_SWAP_HDR

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void short_swap(char *d);
void word_swap(char *d);
void long_swap(char *d);
void longd_swap(char *d);
void set_swapping_ts(int w);
int get_swapping_ts();

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* TINA_SYS_GEN_SWAP_HDR */ 

