/*!
 *  @file    nifti_swaps.h
 *  @brief   Helper functions for NIFTI IO
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef NIFTI_SWAPS_H
#define NIFTI_SWAPS_H

/*----------------------------------------------------------------------*/
/*! swap each byte pair from the given list of n pairs
 *
 *  Due to alignment of structures at some architectures (e.g. on ARM),
 *  stick to char varaibles.
 *  Fixes http://bugs.debian.org/446893   Yaroslav <debian@onerussian.com>
 *
*//*--------------------------------------------------------------------*/
void nifti_swap_2bytes(int64_t n, void *ar)    /* 2 bytes at a time */
{
  int64_t ii;
  unsigned char * cp1 = (unsigned char *)ar, *cp2;
  unsigned char   tval;

  for (ii = 0; ii < n; ii++) {
    cp2 = cp1 + 1;
    tval = *cp1;  *cp1 = *cp2;  *cp2 = tval;
    cp1 += 2;
  }
  return;
}

/*----------------------------------------------------------------------*/
/*! swap 4 bytes at a time from the given list of n sets of 4 bytes
*//*--------------------------------------------------------------------*/
void nifti_swap_4bytes(int64_t n, void *ar)    /* 4 bytes at a time */
{
  int64_t ii;
  unsigned char * cp0 = (unsigned char *)ar, *cp1, *cp2;
  unsigned char tval;

  for (ii = 0; ii < n; ii++) {
    cp1 = cp0; cp2 = cp0 + 3;
    tval = *cp1;  *cp1 = *cp2;  *cp2 = tval;
    cp1++;  cp2--;
    tval = *cp1;  *cp1 = *cp2;  *cp2 = tval;
    cp0 += 4;
  }
  return;
}

/*----------------------------------------------------------------------*/
/*! swap 8 bytes at a time from the given list of n sets of 8 bytes
 *
 *  perhaps use this style for the general Nbytes, as Yaroslav suggests
*//*--------------------------------------------------------------------*/
void nifti_swap_8bytes(int64_t n, void *ar)    /* 8 bytes at a time */
{
  int64_t ii;
  unsigned char * cp0 = (unsigned char *)ar, *cp1, *cp2;
  unsigned char tval;

  for (ii = 0; ii < n; ii++) {
    cp1 = cp0;  cp2 = cp0 + 7;
    while (cp2 > cp1)      /* unroll? */
    {
      tval = *cp1; *cp1 = *cp2; *cp2 = tval;
      cp1++; cp2--;
    }
    cp0 += 8;
  }
  return;
}

/*----------------------------------------------------------------------*/
/*! swap 16 bytes at a time from the given list of n sets of 16 bytes
*//*--------------------------------------------------------------------*/
void nifti_swap_16bytes(int64_t n, void *ar)    /* 16 bytes at a time */
{
  int64_t ii;
  unsigned char * cp0 = (unsigned char *)ar, *cp1, *cp2;
  unsigned char tval;

  for (ii = 0; ii < n; ii++) {
    cp1 = cp0;  cp2 = cp0 + 15;
    while (cp2 > cp1)
    {
      tval = *cp1; *cp1 = *cp2; *cp2 = tval;
      cp1++; cp2--;
    }
    cp0 += 16;
  }
  return;
}

/*----------------------------------------------------------------------*/
/*! based on siz, call the appropriate nifti_swap_Nbytes() function
*//*--------------------------------------------------------------------*/
void nifti_swap_Nbytes(int64_t n, int siz, void *ar)  /* subsuming case */
{
  switch (siz) {
  case 2:  nifti_swap_2bytes(n, ar); break;
  case 4:  nifti_swap_4bytes(n, ar); break;
  case 8:  nifti_swap_8bytes(n, ar); break;
  case 16: nifti_swap_16bytes(n, ar); break;
  default:    /* nifti_swap_bytes  ( n , siz, ar ) ; */
    fprintf(stderr, "** NIfTI: cannot swap in %d byte blocks\n", siz);
    break;
  }
  return;
}

#endif //NIFTI_SWAPS_H