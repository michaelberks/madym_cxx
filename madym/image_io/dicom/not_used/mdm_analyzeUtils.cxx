/**
 *  @file    mdm_analyzeUtils.c
 *  @brief   Gio's Analyze file utilities
 *
 *  Original author GA Buonaccorsi 20 May 2004
 *  (c) Copyright ISBE, University of Manchester 2004
 *
 *  Last edited GAB 21 Aug 07
 *  GAB mods:
 *  20 May  2004
 *  - Created
 *  27 Apr 07 - Added set time stamp for dynamic series
 *  11 May 07 - Added handling for CT voxel dimensions
 *  23 May 07 - Added handling for GSK GE timing
 *  21 Aug 07 - Added handling for GSK Siemens timing
 *
 */

#include  "mdm_analyzeUtils.h"

#include <vcl_compiler.h>
#include <iostream>
#include <string>
#include <cassert>
#include <sstream>
#include <time.h>

#include  <tina/sys.h>        /* For Imrect */
#include  <tina/sysfuncs.h>
#include  <tina/math.h>
#include  <tina/mathfuncs.h>

#include  "mdm_utils.h"
#include  "mdm_flip.h"
#include  "mdm_nemaIO.h"
#include  "mdm_dicomIO.h"
#include  "tina_swap.h"

#define VOXELS           450
#define TR_DATA          1000
#define FLIP_ANGLE_DATA  1001
#define IMTIME_DATA      1004

int    mdm_setInputFileType(struct mdm_analyzeInputs *inputs, char newType)
{
  int setError = MDM_YES;
  
  if (inputs->fileType == MDM_FILETYPE_UNSET)
  {
    inputs->fileType = newType;
    setError = MDM_NO;
  }
  
  return setError;
}

/**
 * Returns NULL pointer if file open fails.  Therefore passes responsibility for dealing with file
 * error back up to calling routine.
 *
 * @author   GA Buonaccorsi
 * @brief    Call image file reader appropriate to imgType
 * @version  conv2analyze 2.0
 * @param    imgName   String image file name with full path
 * @param    imgType   Char (i.e. 1-byte integer) image type flag (#defined above)
 * @return   Imrect *   Pointer to Imrect with image data
 */
static Imrect  *mdm_convReadImage(const char *imgName, char imgType)
{
  Imrect *imgPtr = NULL;

  switch (imgType)
  {
    case MDM_NEMA_FILE_R6:
    case MDM_NEMA_FILE_R8:
      imgPtr = (Imrect *) nema_read_image(imgName, imgType);
      break;
    case MDM_DICOM_FILE:
    case MDM_DICOM_FILE_R10:
    case MDM_DICOM_FILE_CT:
    case MDM_DICOM_FILE_CT2HU:
    case MDM_DICOM_FILE_GSKGE:
    case MDM_DICOM_FILE_GSKS:
      imgPtr = (Imrect *) dicom_read_image(imgName, imgType);
      break;
    default:
      mdm_progAbort("mdm_convReadImage", "Image type not supported");
      break;
  }

  return imgPtr;
}

static void   mdm_convReadVoxelDims(char imgType, FILE *imgFilePtr, Imrect *im)
{
  switch (imgType)
  {
    case MDM_NEMA_FILE_R6:
    case MDM_NEMA_FILE_R8:
      nema_hdr_voxelscale_extract(imgFilePtr, im);
      break;
    case MDM_DICOM_FILE:
    case MDM_DICOM_FILE_R10:
    case MDM_DICOM_FILE_GSKGE:
      dicom_hdr_voxelscale_extract(imgFilePtr, im);
      break;
    case MDM_DICOM_FILE_GSKS:
    case MDM_DICOM_FILE_CT:
    case MDM_DICOM_FILE_CT2HU:
      dicom_hdr_voxelscale_extract_CT(imgFilePtr, im);
      break;
    default:
      mdm_progAbort("mdm_convReadVoxelDims", "Image type not supported");
      break;
  }
}

static void   mdm_convReadFlipAngle(char imgType, FILE *imgFilePtr, Imrect *im)
{
  switch (imgType)
  {
    case MDM_NEMA_FILE_R6:
    case MDM_NEMA_FILE_R8:
      nema_hdr_flip_angle_extract(imgFilePtr, im);
      break;
    case MDM_DICOM_FILE:
    case MDM_DICOM_FILE_R10:
    case MDM_DICOM_FILE_CT:
    case MDM_DICOM_FILE_CT2HU:
    case MDM_DICOM_FILE_GSKGE:
    case MDM_DICOM_FILE_GSKS:
      dicom_hdr_flip_angle_extract(imgFilePtr, im);
      break;
    default:
      mdm_progAbort("mdm_convReadFlipAngle", "Image type not supported");
      break;
  }
}

static void   mdm_convReadTR(char imgType, FILE *imgFilePtr, Imrect *im)
{
  switch (imgType)
  {
    case MDM_NEMA_FILE_R6:
    case MDM_NEMA_FILE_R8:
      nema_hdr_TR_extract(imgFilePtr, im);
      break;
    case MDM_DICOM_FILE:
    case MDM_DICOM_FILE_R10:
    case MDM_DICOM_FILE_CT:
    case MDM_DICOM_FILE_CT2HU:
    case MDM_DICOM_FILE_GSKGE:
    case MDM_DICOM_FILE_GSKS:
      dicom_hdr_TR_extract(imgFilePtr, im);
      break;
    default:
      mdm_progAbort("mdm_convReadTR", "Image type not supported");
      break;
  }
}

static void   mdm_convReadTimestamp(char imgType, FILE *imgFilePtr, Imrect *im)
{
  switch (imgType)
  {
    case MDM_NEMA_FILE_R6:
      printf("mdm_convReadTimestamp: Can't do timings for R6 - sorry.\n");
      break;
    case MDM_NEMA_FILE_R8:
      nema_hdr_imagetime_R8_extract(imgFilePtr, im);
      break;
    case MDM_DICOM_FILE:
    case MDM_DICOM_FILE_R10:
    case MDM_DICOM_FILE_CT:
    case MDM_DICOM_FILE_CT2HU:
      dicom_hdr_image_time_extract(imgFilePtr, im);
      break;
    case MDM_DICOM_FILE_GSKGE:
      dicom_hdr_image_time_extract_GSKGE(imgFilePtr, im);
      break;
    case MDM_DICOM_FILE_GSKS:
      dicom_hdr_image_time_extract_GSKS(imgFilePtr, im);
      break;
    default:
      mdm_progAbort("mdm_convReadTimestamp", "Image type not supported");
      break;
  }
}

/**
 * Unused elements also swapped.
 * Should this return a boolean flag in case of error??
 *
 * @author   Gio Buonaccorsi
 * @brief    Swap endian of an Analyze dsr header struct
 * @version  madym 1.0
 * @param    hdr   Pointer to dsr header struct
 */
static void   mdm_anaHdrByteSwap(struct dsr  *hdr)
{
  int i;
  
  set_swapping_ts(1);

  word_swap((char *)&hdr->hk.sizeof_hdr);
  word_swap((char *)&hdr->hk.extents);
  short_swap((char *)&hdr->hk.session_error);
    

  for (i = 0; i < 8; i++)
    short_swap((char *)&hdr->dime.dim[i]);
  short_swap((char *)&hdr->dime.unused1);
  short_swap((char *)&hdr->dime.datatype);
  short_swap((char *)&hdr->dime.bitpix);
  short_swap((char *)&hdr->dime.dim_un0);
  for (i = 0; i < 8; i++)
    word_swap((char *)&hdr->dime.pixdim[i]);
  word_swap((char *)&hdr->dime.vox_offset);
  word_swap((char *)&hdr->dime.roi_scale);
  word_swap((char *)&hdr->dime.funused1);
  word_swap((char *)&hdr->dime.funused2);
  word_swap((char *)&hdr->dime.cal_max);
  word_swap((char *)&hdr->dime.cal_min);
  word_swap((char *)&hdr->dime.compressed);
  word_swap((char *)&hdr->dime.verified);
  word_swap((char *)&hdr->dime.glmax);
  word_swap((char *)&hdr->dime.glmin);

  word_swap((char *)&hdr->hist.views);
  word_swap((char *)&hdr->hist.vols_added);
  word_swap((char *)&hdr->hist.start_field);
  word_swap((char *)&hdr->hist.field_skip);
  word_swap((char *)&hdr->hist.omax);
  word_swap((char *)&hdr->hist.omin);
  word_swap((char *)&hdr->hist.smax);
  word_swap((char *)&hdr->hist.smin);
}

/**
 * Should this return a boolean flag in case of error??
 *
 * @author   Gio Buonaccorsi
 * @brief    Swap endian of an Analyze image array
 * @version  madym 1.0
 * @param    hdr   Pointer to dsr header struct
 */
static void   mdm_anaHdrImgSwap(short *pixelArray, int nPixels)
{
  int i;
  
  set_swapping_ts(1);

  for (i = 0; i < nPixels; i++)
    short_swap((char *)&pixelArray[i]);
}

/**
 * Unused elements also initialised.
 * Should this return a boolean flag in case of error??
 *
 * This is horrific, but mricro decides to read the strings in its own fucked up way
 * so we have to specifically set all the bytes to null.
 *
 * @author   Gio Buonaccorsi
 * @brief    Write blank values to an Analyze dsr header struct
 * @version  madym 1.0
 * @param    hdr   Pointer to dsr header struct
 */
void   mdm_anaHdrSetDefaults(struct dsr  *hdr)
{
  int i;

  /*
   * Initialise Analyze 7.5 header information with blank values
   */
  hdr->hk.sizeof_hdr    = (int) sizeof(struct dsr);               /* Should be 348 */
  if (hdr->hk.sizeof_hdr != 348)
    mdm_progAbort("mdm_anaHdrSetDefaults", "\nmdm_anaHdrSetBlank: Invalid Analyze 7.5 header size\n");
  for (i = 0; i < 10; i++)
    hdr->hk.data_type[i] = '\0';
  for (i = 0; i < 18; i++)
    hdr->hk.db_name[i] = '\0';
  hdr->hk.extents        = (int) 0;
  hdr->hk.session_error  = (short) 0;
  hdr->hk.regular        = 'r';
  hdr->hk.hkey_un0       = ' ';

  for (i = 0; i < 8; i++)
    hdr->dime.dim[i] = (short) 0;
  for (i = 0; i < 4; i++)
    hdr->dime.vox_units[i] = '\0';
  for (i = 0; i < 8; i++)
    hdr->dime.cal_units[i] = '\0';
  hdr->dime.unused1   = (short) 0;
  hdr->dime.datatype  = (short) MDM_DT_NONE;
  hdr->dime.bitpix    = (short) 0;
  hdr->dime.dim_un0   = (short) 0;
  for (i = 0; i < 8; i++)
    hdr->dime.pixdim[i] = (float) 0;
  /* This set as in signa2analyze GJMP 26/4/99 */
  hdr->dime.vox_offset = (float) 0.0;
  hdr->dime.roi_scale  = (float) 1.0;   /* This is where mricro expects to find a scale factor */
  hdr->dime.funused1   = (float) 0.0;
  hdr->dime.funused2   = (float) 0.0;
  hdr->dime.cal_max    = (float) 0.0;
  hdr->dime.cal_min    = (float) 0.0;
  hdr->dime.compressed = (int) 0.0;
  hdr->dime.verified = (int) 0.0;
  hdr->dime.glmax = (int) 0;
  hdr->dime.glmin = (int) 0;

  for (i = 0; i < 80; i++)
    hdr->hist.descrip[i] = '\0';
  for (i = 0; i < 24; i++)
    hdr->hist.aux_file[i] = '\0';
  hdr->hist.orient = (char) 0;
  for (i = 0; i < 10; i++)
    hdr->hist.originator[i] = '\0';
  for (i = 0; i < 10; i++)
    hdr->hist.generated[i] = '\0';
  for (i = 0; i < 10; i++)
    hdr->hist.scannum[i] = '\0';
  for (i = 0; i < 10; i++)
    hdr->hist.patient_id[i] = '\0';
  for (i = 0; i < 10; i++)
    hdr->hist.exp_date[i] = '\0';
  for (i = 0; i < 10; i++)
    hdr->hist.exp_time[i] = '\0';
  for (i = 0; i < 3; i++)
    hdr->hist.hist_un0[i] = '\0';
  hdr->hist.views       = (int) 0;
  hdr->hist.vols_added  = (int) 0;
  hdr->hist.start_field = (int) 0;
  hdr->hist.field_skip  = (int) 0;
  hdr->hist.omax = (int) 0;
  hdr->hist.omin = (int) 0;
  hdr->hist.smax = (int) 0;
  hdr->hist.smin = (int) 0;
}

/**
 * @author   Gio Buonaccorsi
 * @brief    Write blank values to an Analyze dsr header struct
 * @version  madym 1.0
 * @param    inputs   Struct holding inputs from cmd-line args or GUI.
 */
void   mdm_callAnalyzeConverter(struct mdm_analyzeInputs inputs)
{
  const char *ME = "mdm_callAnalyzeConverter";

  char  *hdrName, *imgName, *xtrName;   /* Analyze output file names (*.hdr, *.img, *.xtr) ... */
  FILE  *hdrPtr,  *imgPtr,  *xtrPtr;    /* ... and stream pointers */

  /* Image file and variables */
  struct dsr   hdrStruct;

  int     nDirEntries = 0;
  char  **dirListing;

  char     currentImgName[MAXPATHLEN];
  Imrect  *currentImgPtr = NULL;

  int     xDim, yDim, zDim;
  int     x, y, z;
  int     nPixels;
  short  *pixelArray;
  double  scaledPixel;

  int     dummy;
  char    tmp_str[MAXPATHLEN];

  Vec3    *iscale = NULL;

  FILE    *fp;
  int      hours, minutes;
  float    seconds, timestamp;
  float   *TR = NULL, *FlipAngle = NULL, timings;

  /*
   * Make all required filenames - unnecessarily complicated but I can't be
   * bothered changing it ...
   */
  if ((hdrName = (char *) malloc(strlen(inputs.outputName) + strlen(".hdr") + 1)) == NULL)
    mdm_progAbort(ME, "Can not malloc output hdrPtr file name");
  strcpy(hdrName, inputs.outputName);
  strcat(hdrName, ".hdr");

  if ((imgName = (char *) malloc(strlen(inputs.outputName) + strlen(".img") + 1)) == NULL)
    mdm_progAbort(ME, "Can not malloc output image file name");
  strcpy(imgName, inputs.outputName);
  strcat(imgName, ".img");

  if (inputs.optionsFile == MDM_YES)
  {
    if ((xtrName = (char *) malloc(strlen(inputs.outputName) + strlen(".xtr") + 1)) == NULL)
      mdm_progAbort(ME, "Can not malloc output extras file name");
    strcpy(xtrName, inputs.outputName);
    strcat(xtrName, ".xtr");
  }

  if (inputs.debug)
    printf("%s: Analyze file names: hdr - %s, img - %s, xtr - %s\n", ME, hdrName, imgName, xtrName);

  /* init ana hdr */
  mdm_anaHdrSetDefaults(&hdrStruct);

  /* define list of directory contents */
  if (inputs.debug)
    printf("%s: Input directory: %s\n", ME, inputs.inputDir);
  dirListing = mdm_makeDirList(NULL, &nDirEntries, inputs.inputDir);

  /* get some info from first image in stack */
  sprintf(currentImgName, "%s/%s", inputs.inputDir, dirListing[0]);

  /* read in image */
  if (currentImgPtr)
    currentImgPtr = NULL;
    /* Memory leak the size of Wales - plug it !!! i.e. find tina im_free ... */
  if ((currentImgPtr = mdm_convReadImage(currentImgName, inputs.fileType)) == NULL)
    mdm_progAbort(ME, "Can not read image file");

  xDim = currentImgPtr->width;
  yDim = currentImgPtr->height;
  zDim = nDirEntries;
  nPixels = xDim * yDim * zDim;
  if (inputs.debug)
    printf("%s: hdrName: %s, imgName: %s\n", ME, hdrName, imgName);

  /* Allocate memory for 2-byte pixels */
  pixelArray = (short *) malloc(sizeof(short) * nPixels);

  /* Loop through directory entries ... */
  for (z = 0; z < nDirEntries; z++) 
  {
    printf("%s: List member %d: %s\n", ME, z, dirListing[z]);
    sprintf(currentImgName, "%s/%s", inputs.inputDir, dirListing[z]);

    /* read in image */
    /*
     * Selection of file format and More memory leak ...
     */
    if (currentImgPtr)
      currentImgPtr = NULL;
    currentImgPtr = mdm_convReadImage(currentImgName, inputs.fileType);

    for (x = 0; x < xDim; x++)
    {
      for (y = 0; y < yDim; y++)
      {
        scaledPixel = im_get_pixf(currentImgPtr, y, x) / inputs.scale;
        if (scaledPixel > 32000.0)
        {
          fprintf(stderr, "%s:  scaled pixel value = %g\n", ME, scaledPixel);
          mdm_progAbort(ME, "Output dynamic range exceeded. Use a higher scaling factor (e.g. -s 10000)");
        }
        pixelArray[x + y * xDim + z * xDim * yDim] = (short) (im_get_pixf(currentImgPtr, y, x) / inputs.scale);
      }
    }
  }

  /* Flip all the slices */
  for (z = 0; z < zDim; z++)
    mdm_flip(&pixelArray[z * xDim * yDim], xDim, yDim);

  /* Create new analyze image file */
  if (inputs.debug)
    printf("%s: Creating img file: %s\n", ME, imgName);
  if ((imgPtr = fopen(imgName, "wb")) == NULL)
    mdm_progAbort(ME, "Can not create output image file\n");
  /* Swap image bytes if necessary */
  if (inputs.endianSwap)
    mdm_anaHdrImgSwap(pixelArray, nPixels);
  /* Write pixelArray values to analyze file */
  if ((fwrite(pixelArray, sizeof(short), nPixels, imgPtr)) != nPixels)
    mdm_progAbort(ME, "Can not write to output image file\n");
  fclose(imgPtr);

  /* Initialise analyze header struct */
  hdrStruct.hk.sizeof_hdr = (int) sizeof(hdrStruct);
  hdrStruct.hk.extents    = (int) xDim * yDim;

  hdrStruct.dime.dim[0] = (int) 4;
  hdrStruct.dime.dim[1] = (int) xDim;
  hdrStruct.dime.dim[2] = (int) yDim;
  hdrStruct.dime.dim[3] = (int) zDim;
  hdrStruct.dime.dim[4] = (int) 1;
  hdrStruct.dime.datatype   = MDM_DT_SIGNED_SHORT;
  hdrStruct.dime.bitpix     = (int) sizeof (short) * 8;
  strcpy(hdrStruct.dime.vox_units, "mm");

  /* read voxel dimensions */
  /* Fix to have quit if read fails */
  sprintf(currentImgName, "%s/%s", inputs.inputDir, dirListing[0]);
  if ((fp = fopen(currentImgName, "rb")) == NULL)
    mdm_progAbort(ME, "Can not open image file to read voxel dims");
  mdm_convReadVoxelDims(inputs.fileType, fp, currentImgPtr);
  fclose(fp);

  iscale = vec3_alloc();
  iscale = (Vec3 *) prop_get(currentImgPtr->props, VOXELS);
  sprintf(tmp_str, "%f", iscale->el[0]);
  if (!strcmp(tmp_str, "\0"))
    mdm_progAbort(ME, "pixel_x_size not set");
  hdrStruct.dime.pixdim[1] = (float) iscale->el[0];

  tmp_str[0] = '\0';
  sprintf(tmp_str,"%f",iscale->el[1]);
  if (!strcmp(tmp_str, "\0"))
    mdm_progAbort(ME, "pixel_y_size not set");
  hdrStruct.dime.pixdim[2] = (float) iscale->el[1];

  tmp_str[0] = '\0';
  sprintf(tmp_str,"%f", iscale->el[2]);
  if (!strcmp(tmp_str, "\0"))
    mdm_progAbort(ME, "pixel_z_size not set");
  hdrStruct.dime.pixdim[3] = (float) iscale->el[2];

  /* This set as in signa2analyze GJMP 26/4/99 */
  hdrStruct.dime.pixdim[0] = (float) 4.0;

  /* Create new Analyze header file */
  if (inputs.debug)
    printf("%s: hdrName now: %s\n", ME, hdrName);
  if ((hdrPtr = fopen(hdrName, "wb")) == NULL)
    mdm_progAbort(ME, "Can not create output header file");
  /* Swap bytes if required */
  if (inputs.endianSwap)
    mdm_anaHdrByteSwap(&hdrStruct);
  /* Write hdrPtr values to analyze file */
  if ((dummy = fwrite(&hdrStruct, sizeof(hdrStruct), 1, hdrPtr)) != 1)
    mdm_progAbort(ME, "Can not write to output header file");
  fclose(hdrPtr);

  if (inputs.optionsFile)    /* create exta info for dynamic studies */
  {
    /* read flip angle data */
    if ((fp = fopen(currentImgName, "rb")) == NULL)
      mdm_progAbort(ME, "Can not read flip angle data");
    /* New function for this */
    mdm_convReadFlipAngle(inputs.fileType, fp, currentImgPtr);
    FlipAngle = prop_get(currentImgPtr->props, FLIP_ANGLE_DATA);
    fclose(fp);

    /* read TR data */
    if ((fp = fopen(currentImgName, "rb")) == NULL)
      mdm_progAbort(ME, "Can not read TR data");
    /* New function for this */
    mdm_convReadTR(inputs.fileType, fp, currentImgPtr);
    TR = prop_get(currentImgPtr->props, TR_DATA);
    fclose(fp);

    /* read time info */
    if (inputs.fileType == MDM_NEMA_FILE_R6)
    {
      fprintf(stderr, "mdm_convReadTimestamp: Can't do timings for R6 - sorry.\n");
    }
    else
    {
      /* 
       * 27 Apr 07 - GAB 
       * Added code to deal with fixed time stamp
       */
      if (inputs.timeStamp < 0.0)
      {
        if ((fp = fopen(currentImgName, "rb")) == NULL)
          mdm_progAbort(ME, "Can not read timimg data");
        /* New function for this */
        mdm_convReadTimestamp(inputs.fileType, fp, currentImgPtr);
        fclose(fp);

        timestamp = *((float *) prop_get(currentImgPtr->props, IMTIME_DATA));
      }
      else
      {
        timestamp = inputs.timeStamp;
      }
      hours     = (int) (timestamp / 10000);
      minutes   = (int) ((timestamp - 10000 * hours) / 100);
      seconds   = (float) (timestamp - 10000 * hours - 100 * minutes);
      timings   = hours * 60 * 60 + minutes * 60 + seconds;          /* convert to seconds */
      printf("%s: hours = %d min = %d s = %f timings = %f\n", ME, hours, minutes, seconds, timings);
    }

    if (inputs.debug)
      printf("%s: options file now: %s\n", ME, xtrName);
    if ((xtrPtr = fopen(xtrName, "w")) == NULL)
      mdm_progAbort(ME, "Can not create output info file");

    /* Write values to info file */
    if ((dummy = fprintf(xtrPtr, "voxel dimensions: %f %f %f\n", iscale->el[0], iscale->el[1], iscale->el[2])) < 0)
      mdm_progAbort(ME, "Can not write to output info file");
    if (FlipAngle != NULL)
    {
      if ((dummy = fprintf(xtrPtr, "flip angle: %f\n", *FlipAngle)) < 0)
        mdm_progAbort(ME, "Can not write to output info file");
    }
    else
      if ((dummy = fprintf(xtrPtr, "flip angle: 0.0\n")) < 0)
        mdm_progAbort(ME, "Can not write to output info file");
    if (TR != NULL)
    {
      if ((dummy = fprintf(xtrPtr, "TR: %f\n", *TR)) < 0)
        mdm_progAbort(ME, "Can not write to output info file");
    }
    else
      if ((dummy = fprintf(xtrPtr, "TR: 0.0\n")) < 0)
        mdm_progAbort(ME, "Can not write to output info file");
    if ((dummy = fprintf(xtrPtr, "timestamp: %d %d %f %f\n", hours, minutes, seconds, timestamp)) < 0)
      mdm_progAbort(ME, "Can not write to output info file");

    fclose(hdrPtr);
  }

  free(hdrName);
  free(imgName);
  if (inputs.optionsFile)   
    free(xtrName);
}

