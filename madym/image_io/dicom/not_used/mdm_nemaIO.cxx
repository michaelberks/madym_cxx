/**@(#)Functions to read/write ACR-NEMA image files

modified GJMP from original Tina code

last modified GJMP 6/9/01
*/

#include "mdm_nemaIO.h"
#include "mdm_analyzeUtils.h"

#include <stdio.h>
#include <string.h>

#include <tina/sys.h>
#include <tina/sysfuncs.h>
#include <tina/math.h>
#include <tina/mathfuncs.h>

#define VOXELS          450
#define DYNSTIME        451
#define TE_DATA         453
#define PAT_DATA        454
#define TR_DATA         1000
#define FLIP_ANGLE_DATA 1001
#define IMTIME_DATA     1004

extern Bool  fread_imrect_data(const Imrect * imrect, FILE * stream, const char *pathname);

/*
 *  Check that group value is valid (by what criterion I don't know - GAB).
 */
static int valid_group(short group)
{
  if(group == 0x0000 || group == 0x0008 || group == 0x0010
     || group == 0x0020 || group == 0x0018 || group == 0x0020
     || group == 0x0028 || group == 0x0029 || group == 0x4000 || group == 0x4000
     || group == 0x7FE0 )
  {
    return(1);
  }
  else
    return(0);
} 

/*
 *  Convert 12-bit packed image data to 16-bit integers.
 */
static Imrect *im_nema_conv(Imrect *im)
{
    Imrect *im2;
    Imregion roi;
    unsigned char *row1;
    unsigned char *row2;
    int i,j,k;
    int lx, ux, ly, uy;

    if (im == NULL)
      return(NULL);

    roi = *(im->region);
    roi.ux = roi.ux*4.0/3;
    im2 = im_alloc(im->height,roi.ux,&roi,short_v);
    lx = roi.lx;
    ux = roi.ux;
    ly = roi.ly;
    uy = roi.uy;

    for (i = ly; i < uy; ++i)
    {
        row1 = IM_ROW(im,i);
        row2 = IM_ROW(im2,i);
        for (k = 2*lx, j = 2*lx; k < 2*ux; j+=3)
        {
            row2[k++] = row1[j];
            row2[k++] = row1[j+1]%16;
            row2[k++] = row1[j+1]/16 + (row1[j+2]%16)*16;
            row2[k++] = row1[j+2]/16;
        }
    }

    return(im2);
}

void im_endian_inplace(Imrect *imrect)
{
    unsigned int size = var_size(imrect->vtype);
    Imregion roi;
    int y;
    int lx, ux, ly, uy;

    roi = *(imrect->region);
    lx = roi.lx;
    ux = roi.ux;
    ly = roi.ly;
    uy = roi.uy;


    for(y = ly; y < uy; y++)
    {
        void*  row = NULL;
        char   temp[40];
        int    x;
        unsigned int i;

        row = IM_ROW(imrect,y);

        for(x = lx; x < ux; x++)
        {
            for(i = 0; i < size; i++)
                temp[i] = *(((char *)row) + (x*size+i));
            for(i = 1; i <= size; i++)
                *(((char *)row) + (x*size + size-i)) = temp[i-1];
        }
    }
}


/* added GJMP 6/3/2001 */
Bool   nema_hdr_TR_extract(FILE *fp, Imrect *im)
{
  Bool                swapit = false;
  Bool                success = false;
  float              *TR;
  int                 i;
  char                temp[64];
  unsigned char       junk;
  unsigned short      group, element;
  unsigned short      dblock;
  unsigned short      dblock1;

  set_swapping_ts(0);
  TR = (float *)ralloc(sizeof(float));
  while (fp && fread(&group, sizeof(short), 1, fp) && !success)
    { 
      if(!swapit)
        {
          if(!valid_group(group))
            {
              set_swapping_ts(1);
              short_swap((char *)&group);
              if (valid_group(group)) 
                swapit = true;
              else    
                {
                  short_swap((char *)&group);
                  set_swapping_ts(0);
                }
            }
        }
      else
        short_swap((char *)&group);
 
      fread(&element, sizeof(short), 1, fp);
      short_swap((char *)&element);
      fread(&dblock,  sizeof(short), 1, fp);
      short_swap((char *)&dblock);
      fread(&dblock1, sizeof(short), 1, fp);
      short_swap((char *)&dblock1);

      if (dblock1 != 0)
        {
          fprintf(stderr, "nema_hdr_TR_extract:  Error - non-standard block size\n");
          return(false);
        }
      else if (group == 0x0018 && element == 0x0080)
        {
          fread(temp, sizeof(char), dblock, fp);
          temp[dblock] = '\0';
          sscanf(temp, "%f", TR);
          success = true;
        }
      else 
        for (i = 0; i < dblock; i++) 
          fread(&junk, sizeof(char), 1, fp);

      if (success) break;
    }

  if (success)
    {
      im->props = proplist_rm(im->props, TR_DATA);
      im->props = proplist_addifnp(im->props, (void *)TR, TR_DATA, rfree, false);
    }

/*printf("\nTR = %f",*TR);*/

  return (success);
}

/* added GJMP 6/3/2001 */
Bool   nema_hdr_flip_angle_extract(FILE *fp, Imrect *im)
{
  Bool                swapit = false;
  Bool                success = false;
  float              *flip_angle;
  int                 i;
  char                temp[64];
  unsigned char       junk;
  unsigned short      group, element;
  unsigned short      dblock;
  unsigned short      dblock1;

  set_swapping_ts(0);
  flip_angle = (float *)ralloc(sizeof(float));
  while (fp && fread(&group, sizeof(short), 1, fp) && !success)
    { 
      if(!swapit)
        {
          if(!valid_group(group))
            {
              set_swapping_ts(1);
              short_swap((char *)&group);
              if (valid_group(group)) 
                swapit = true;
              else    
                {
                  short_swap((char *)&group);
                  set_swapping_ts(0);
                }
            }
        }
      else
        short_swap((char *)&group);
 
      fread(&element, sizeof(short), 1, fp);
      short_swap((char *)&element);
      fread(&dblock,  sizeof(short), 1, fp);
      short_swap((char *)&dblock);
      fread(&dblock1, sizeof(short), 1, fp);
      short_swap((char *)&dblock1);

      if (dblock1 != 0)
        {
          fprintf(stderr, "nema_hdr_flip_angle_extract:  Error - non-standard block size\n");
          return(false);
        }
      else if (group == 0x0019 && element == 0x101A)
        {
          fread(temp, sizeof(char), dblock, fp);
          temp[dblock] = '\0';
          sscanf(temp, "%f", flip_angle);
          success = true;
        }
      else 
        for (i = 0; i < dblock; i++) 
          fread(&junk, sizeof(char), 1, fp);

      if (success) break;
    }

  if (success)
    {
      im->props = proplist_rm(im->props, FLIP_ANGLE_DATA);
      im->props = proplist_addifnp(im->props, (void *)flip_angle, FLIP_ANGLE_DATA, rfree, false);
    }

/*printf("\nflip_angle = %f",*flip_angle);*/

  return (success);
}

/* added GJMP 6/9/01 R8 (actually R7 but we never had this) and above */
Bool   nema_hdr_imagetime_R8_extract(FILE *fp, Imrect *im)
{
  Bool                swapit = false;
  Bool                success = false;
  float              *image_time;
  int                 i;
  char                temp[64];
  unsigned char       junk;
  unsigned short      group, element;
  unsigned short      dblock;
  unsigned short      dblock1;

  set_swapping_ts(0);
  image_time = (float *)ralloc(sizeof(float));
  while (fp && fread(&group, sizeof(short), 1, fp) && !success)
    { 
      if(!swapit)
        {
          if(!valid_group(group))
            {
              set_swapping_ts(1);
              short_swap((char *)&group);
              if (valid_group(group)) 
                swapit = true;
              else    
                {
                  short_swap((char *)&group);
                  set_swapping_ts(0);
                }
            }
        }
      else
        short_swap((char *)&group);
 
      fread(&element, sizeof(short), 1, fp);
      short_swap((char *)&element);
      fread(&dblock,  sizeof(short), 1, fp);
      short_swap((char *)&dblock);
      fread(&dblock1, sizeof(short), 1, fp);
      short_swap((char *)&dblock1);

      if (dblock1 != 0)
        {
          fprintf(stderr, "nema_hdr_imagetime_R8_extract:  Error - non-standard block size\n");
          return(false);
        }
      else if (group == 0x0008 && element == 0x0033)
        {
          fread(temp, sizeof(char), dblock, fp);
          temp[dblock] = '\0';
          sscanf(temp, "%f", image_time);
          success = true;
        }
      else 
        for (i = 0; i < dblock; i++) 
          fread(&junk, sizeof(char), 1, fp);

      if (success) break;
    }

  if (success)
    {
      im->props = proplist_rm(im->props, IMTIME_DATA);
      im->props = proplist_addifnp(im->props, (void *)image_time, IMTIME_DATA, rfree, false);
    }

  return (success);
}


Bool  nema_hdr_voxelscale_extract(FILE *fp, Imrect *im)
{
  Bool                swapit = false;
	Vec3               *iscale;
  float               xsize,ysize,zsize;
  float               zgap;
  int                 psuccess = 0;
  int                 i;
  char               *details = NULL;
  char                dimension[64];
  unsigned char       junk;
  unsigned short      group, element;
  unsigned short      dblock;
  unsigned short      dblock1;

  details = (char *)cvector_alloc(0, 256);
  set_swapping_ts(0);
  while (fp && fread(&group, sizeof(short), 1, fp) && psuccess < 3)
    { 
      if(!swapit)
        {
          if(!valid_group(group))
            {
              set_swapping_ts(1);
              short_swap((char *)&group);
              if (valid_group(group)) 
                swapit = true;
              else    
                {
                  short_swap((char *)&group);
                  set_swapping_ts(0);
                }
            }
        }
      else
        short_swap((char *)&group);
 
      fread(&element, sizeof(short), 1, fp);
      short_swap((char *)&element);
      fread(&dblock,  sizeof(short), 1, fp);
      short_swap((char *)&dblock);
      fread(&dblock1, sizeof(short), 1, fp);
      short_swap((char *)&dblock1);

      if (dblock1 != 0)
        {
          fprintf(stderr, "nema_hdr_voxelscale_extract:  Error - non-standard block size\n");
          return(false);
        }
      else if (group == 0x0018 && element == 0x0050)
        {
          fread(dimension, sizeof(char),dblock,fp);
          dimension[dblock] = '\0';
          sscanf(dimension,"%f",&zsize);
          psuccess++;
        }
      else if (group == 0x0021 && element == 0x1221)
        {
          fread(dimension, sizeof(char),dblock,fp);
          dimension[dblock] = '\0';
          sscanf(dimension,"%f",&zgap);
          psuccess++;
        }
      else if(group == 0x0028 && element == 0x0030)
        {
          fread(dimension, sizeof(char),dblock,fp);
          dimension[dblock] = '\0';
          sscanf(dimension,"%f\\%f",&xsize,&ysize);
          psuccess++;
        }
 
      else 
        for (i = 0; i < dblock; i++) 
          fread(&junk, sizeof(char), 1, fp);
    }

  if (psuccess == 3)
  {
      iscale = vec3_alloc();
      iscale->el[0] = xsize;
      iscale->el[1] = ysize;
      iscale->el[2] = zsize + zgap;

      im->props = proplist_rm(im->props, VOXELS);
      im->props = proplist_addifnp(im->props, iscale,
                                   VOXELS, vec3_free, false);
  }

  return ((Bool)(psuccess == 3));
}


Imrect *nema_read_image(const char *pathname, int file_type)
{
  Imrect *imrect = NULL, *imrect2 = NULL, *fim = NULL;
  Imregion imregion;
  FILE                    *fp_img=fopen(pathname, "rb");
  Vartype                new_vtype;
  float        fpmin, fpmax;
  float	       scale_slope=0,scale_intercept=0;	/* from Philips document ID XJR 2466 */
  float                   val, pix;
  int                      i, j, k;
  short                   endian=0;
  unsigned char               junk;
  char               dimension[64];
  unsigned short              abits;
  unsigned short              sbits;
  unsigned short              sign;
  unsigned short              rows;
  unsigned short              cols;
  unsigned short             group;
  unsigned short           element;
  unsigned short            dblock;
  unsigned short           dblock1;
  float          xsize,ysize,zsize;
  float                       zgap;
  Vec3                     *iscale;

  set_swapping_ts(0);
  while (fp_img&&fread(&group, sizeof(short),1,fp_img))
    {
      if(!endian)
        {
          if(valid_group(group)!=1)
            {
              set_swapping_ts(1);
              short_swap((char *)&group);
              if (valid_group(group) == 1) endian = 1;
              else    
                {
                  short_swap((char *)&group);
                  set_swapping_ts(0);
                }
            }
        }
      else
        {
          short_swap((char *)&group);
        } 
      fread(&element, sizeof(short),1,fp_img);
      short_swap((char *)&element);
      fread(&dblock,  sizeof(short),1,fp_img);
      short_swap((char *)&dblock);
      fread(&dblock1, sizeof(short),1,fp_img);
      short_swap((char *)&dblock1);

      if (group == 0x7FE0 && element == 0x0010)
        break; /* found the image data */
      if (dblock1 != 0)
        {
          fprintf(stderr, "nema_read_image:  Error - non-standard block size\n");
          fclose(fp_img); 
          return(NULL);
        }
      else if (group == 0x0018 && element == 0x0050)
        {
          fread(dimension, sizeof(char),dblock,fp_img);
          dimension[dblock] = '\0';
          sscanf(dimension,"%f",&zsize);
        }
      else if (group == 0x0021 && element == 0x1221)
        {
          fread(dimension, sizeof(char),dblock,fp_img);
          dimension[dblock] = '\0';
          sscanf(dimension,"%f",&zgap);
        }

     /* else if (group == 0x0029 && element == 0x1010)*/
      else if (group == 0x0029 && element == 0x1130)
        {
          fread(dimension, sizeof(char), dblock, fp_img);
          dimension[dblock] = '\0';
          sscanf(dimension, "%f", &fpmin);
        }
     /* else if (group == 0x0029 && element == 0x1011)*/
      else if (group == 0x0029 && element == 0x1140)
        {
          fread(dimension, sizeof(char), dblock, fp_img);
          dimension[dblock] = '\0';
          sscanf(dimension, "%f", &fpmax);
        }
      /* added GJMP 6/9/01 */
      else if (group == 0x0029 && element == 0x1053 && file_type == MDM_NEMA_FILE_R8)
        {
          fread(dimension, sizeof(char), dblock, fp_img);
          dimension[dblock] = '\0';
          sscanf(dimension, "%f", &scale_slope);
        }
      else if (group == 0x0029 && element == 0x1052 && file_type == MDM_NEMA_FILE_R8)
        {
          fread(dimension, sizeof(char), dblock, fp_img);
          dimension[dblock] = '\0';
          sscanf(dimension, "%f", &scale_intercept);
        }

      else if (group == 0x0028)
        {
          if(element == 0x0010)
            {
              fread(&rows, sizeof(short),1,fp_img);
              short_swap((char *)&rows);
            }
          else if(element == 0x0011)
            {
              fread(&cols, sizeof(short),1,fp_img);
              short_swap((char *)&cols);
            }
          else if(element == 0x0103)
            {
              fread(&sign, sizeof(short),1,fp_img);
              short_swap((char *)&sign);
            }
          else if(element == 0x0101)
            {
              fread(&sbits, sizeof(short),1,fp_img);
              short_swap((char *)&sbits);
            }
          else if(element == 0x0100)
            {
              fread(&abits, sizeof(short),1,fp_img);
              short_swap((char *)&abits);
            }
          else if(element == 0x0030)
            {
              fread(dimension, sizeof(char),dblock,fp_img);
              dimension[dblock] = '\0';
              sscanf(dimension,"%f\\%f",&xsize,&ysize);
            }
          else for (i=0;i<dblock;i++) 
            fread(&junk, sizeof(char),1,fp_img);
        }   
      else 
        for (i=0;i<dblock;i++) fread(&junk, sizeof(char),1,fp_img);
    }

    
  if (fp_img)
    {
      imregion.lx = 0;
      imregion.ux = cols;
      imregion.ly = 0;
      imregion.uy = rows;
        
      if (sign == 0)
        {
          if (abits == 16 || abits == 12) 
            {
              new_vtype = ushort_v;
            }
          else new_vtype = uchar_v;
        }
      else
        { 
          if (abits == 16 || abits == 12) 
            {
              new_vtype = short_v;
            }
          else new_vtype = char_v;
        }
      if(abits == 12) imregion.ux = 3.0*imregion.ux/4;
      cols = imregion.ux;

      imrect = im_alloc(rows, cols, &imregion, (Vartype)new_vtype);

      if (!fread_imrect_data(imrect, fp_img, pathname))
        {
          im_free(imrect);
          imrect = NULL;
          fclose(fp_img);
          return(NULL);
        }
      fclose(fp_img);
      if (abits == 12)
        {
          imrect2 = im_nema_conv(imrect);
          im_free(imrect);
          imrect = imrect2;
        }
      if (imrect!=NULL&&endian)
        {
          im_endian_inplace(imrect);
        }

      printf("nema_read_image:  scale_slope = %f scale_intercept = %f\n", scale_slope, scale_intercept);

      fim = im_alloc(imrect->height, imrect->width, imrect->region, 
                     (Vartype)float_v);
      for(j = imrect->region->ly; j < imrect->region->uy; j++)
        for(k = imrect->region->lx; k < imrect->region->ux; k++)
          {
            if ((pix = im_get_pixf(imrect, j, k)) != 0.0)
               val = (pix - scale_intercept)/scale_slope;
            else
               val = 0.0; 
            im_put_pixf(val, fim, j, k);   
          }   

      iscale = vec3_alloc();
      iscale->el[0] = xsize;
      iscale->el[1] = ysize;
      iscale->el[2] = zsize + zgap;
      fim->props = proplist_addifnp(fim->props, iscale,
                                    VOXELS, vec3_free, false);

    }
  
  im_free(imrect);
  return fim;
}
