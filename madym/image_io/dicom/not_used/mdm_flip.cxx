#include  "mdm_flip.h"

#include <stdlib.h>

void mdm_flip(short *slice, int xdim, int ydim)
{
  short  *slice_tmp, *tmp, *tmp_start;
  int     i, j;
  int     npixels;

  npixels   = xdim * ydim;
  tmp_start = (short *) malloc(sizeof(short) * npixels);

  /* first copy slice into temp buffer */
  tmp       = tmp_start;
  slice_tmp = slice;
  for(i = 0; i < npixels; i++)
    *tmp++ = *slice_tmp++;

  /* Now flip the slice back into the original buffer */
  tmp = tmp_start;

  /* *** Corrected offset bug 23/5/91 AMKS *** */
  /* was slice_tmp = slice + (npixels-1-xdim); */
  slice_tmp = slice + (npixels - xdim);
  for(j = 0; j < ydim; j++)
  {
    for(i = 0; i < xdim; i++)
    {
      *slice_tmp++ = *tmp++;
    }
    slice_tmp = slice_tmp - 2 * xdim;
  }
  free(tmp_start);
}
