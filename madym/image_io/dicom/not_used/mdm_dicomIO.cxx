/**@(#)Functions to read/write DICOM image files
       These functions are a modified version of the 
       ACR-NEMA ones
*/

#include "mdm_DicomFormat.h"
#include "mdm_nemaIO.h"
#include "mdm_analyzeUtils.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>

#define VOXELS    450
#define DYNSTIME  451
#define TE_DATA   453
#define PAT_DATA  454

#define TR_DATA           1000
#define FLIP_ANGLE_DATA   1001
#define IMTIME_DATA       1004

#define HEADERMAXCOUNT 100

#define MDM_DCMFILE_PART10     1
#define MDM_DCMFILE_NONPART10  0
#define MDM_DCMFILE_INVALID   -1

#define PREAMBLE_LENGTH 128

#define I_LITTLE_ENDIAN "1.2.840.10008.1.2"
#define E_LITTLE_ENDIAN "1.2.840.10008.1.2.1"
#define E_BIG_ENDIAN    "1.2.840.10008.1.2.2"

int mdm_DicomFormat::dicom_format = MDM_DCMFILE_INVALID;

extern bool fclose_2(FILE * stream, const std::string &pathname);
extern FILE *fopen_2(const std::string &pathname, const std::string &mode);
extern bool fread_imrect_data(const mdm_Image3D & imrect, FILE * stream, const std::string &pathname);
extern bool fwrite_imrect_data(const mdm_Image3D & imrect, FILE * stream, const std::string &pathname);




void mdm_DicomFormat::set_dicom_format(int format)
{
	dicom_format = format;
}


int mdm_DicomFormat::dblock_vr_conv(unsigned int *nbytes, FILE * fp)
{
	size_t readerr;
	unsigned int dtemp;
	unsigned char *pvr;

	if (dicom_format == MDM_DCMFILE_INVALID)
		return (-1);

	pvr = (char *) nbytes;

	/* 
	 * VR is a nested sequence (SQ)
	 */

	if ((pvr[0] == 'S' && pvr[1] == 'Q') || (pvr[3] == 'S' && pvr[2] == 'Q'))
	{
		readerr = fread(nbytes, sizeof(int), 1, fp);
		word_swap((char *) nbytes);
		*nbytes = 0;
		return (1);
	}

	/* Try for a quick solution to the UN problem - GAB 5 Sep 2006 */
        
        if ((pvr[0] == 'U' && pvr[1] == 'N'))
	{
		readerr = fread(nbytes, sizeof(int), 1, fp);
		word_swap((char *) nbytes);
		return (1);
	}

	if ((pvr[3] == 'U' && pvr[2] == 'N'))
	{
		readerr = fread(nbytes, sizeof(int), 1, fp);
		word_swap((char *) nbytes);
		return (1);
	}

        if ((pvr[0] == 'O' && pvr[1] == 'B') || (pvr[0] == 'O' && pvr[1] == 'W'))
	{
		readerr = fread(nbytes, sizeof(int), 1, fp);
		word_swap((char *) nbytes);
		return (1);
	}

	if ((pvr[3] == 'O' && pvr[2] == 'B') || (pvr[3] == 'O' && pvr[2] == 'W'))
	{
		readerr = fread(nbytes, sizeof(int), 1, fp);
		word_swap((char *) nbytes);
		return (1);
	}

	if (*nbytes == 0xffffffff)
	{
		*nbytes = 0;
		return (1);
	}

	if ((pvr[0] == 'A' && pvr[1] == 'E') ||
			(pvr[0] == 'A' && pvr[1] == 'S') ||
			(pvr[0] == 'A' && pvr[1] == 'T') ||
			(pvr[0] == 'C' && pvr[1] == 'S') ||
			(pvr[0] == 'D' && pvr[1] == 'A') ||
			(pvr[0] == 'D' && pvr[1] == 'S') ||
			(pvr[0] == 'D' && pvr[1] == 'T') ||
			(pvr[0] == 'F' && pvr[1] == 'L') ||
			(pvr[0] == 'F' && pvr[1] == 'D') ||
			(pvr[0] == 'I' && pvr[1] == 'S') ||
			(pvr[0] == 'L' && pvr[1] == 'O') ||
			(pvr[0] == 'L' && pvr[1] == 'T') ||
			(pvr[0] == 'P' && pvr[1] == 'N') ||
			(pvr[0] == 'S' && pvr[1] == 'H') ||
			(pvr[0] == 'S' && pvr[1] == 'L') ||
			(pvr[0] == 'S' && pvr[1] == 'S') ||
			(pvr[0] == 'S' && pvr[1] == 'T') ||
			(pvr[0] == 'T' && pvr[1] == 'M') ||
			(pvr[0] == 'U' && pvr[1] == 'I') ||
			(pvr[0] == 'U' && pvr[1] == 'L') || (pvr[0] == 'U' && pvr[1] == 'S'))
	{
		/*
		 * (*nbytes) &= 0xffff0000;
		 * (*nbytes) >>= 16;
		 */

#ifdef LITTLE_ENDIAN_ARCHITECTURE
		dtemp = (unsigned int) (256 * pvr[3] + pvr[2]);
#else
		dtemp = (unsigned int) (256 * pvr[2] + pvr[3]);
#endif

		(*nbytes) = dtemp;
		return (1);
	}

	if ((pvr[3] == 'A' && pvr[2] == 'E') ||
			(pvr[3] == 'A' && pvr[2] == 'S') ||
			(pvr[3] == 'A' && pvr[2] == 'T') ||
			(pvr[3] == 'C' && pvr[2] == 'S') ||
			(pvr[3] == 'D' && pvr[2] == 'A') ||
			(pvr[3] == 'D' && pvr[2] == 'S') ||
			(pvr[3] == 'D' && pvr[2] == 'T') ||
			(pvr[3] == 'F' && pvr[2] == 'L') ||
			(pvr[3] == 'F' && pvr[2] == 'D') ||
			(pvr[3] == 'I' && pvr[2] == 'S') ||
			(pvr[3] == 'L' && pvr[2] == 'O') ||
			(pvr[3] == 'L' && pvr[2] == 'T') ||
			(pvr[3] == 'P' && pvr[2] == 'N') ||
			(pvr[3] == 'S' && pvr[2] == 'H') ||
			(pvr[3] == 'S' && pvr[2] == 'L') ||
			(pvr[3] == 'S' && pvr[2] == 'S') ||
			(pvr[3] == 'S' && pvr[2] == 'T') ||
			(pvr[3] == 'T' && pvr[2] == 'M') ||
			(pvr[3] == 'U' && pvr[2] == 'I') ||
			(pvr[3] == 'U' && pvr[2] == 'L') || (pvr[3] == 'U' && pvr[2] == 'S'))
	{
		/*
		 * (*nbytes) &= 0x0000ffff;
		 */

#ifdef LITTLE_ENDIAN_ARCHITECTURE
		dtemp = (unsigned int) (256 * pvr[1] + pvr[0]);
#else
		dtemp = (unsigned int) (256 * pvr[0] + pvr[1]);
#endif

		(*nbytes) = dtemp;
		return (1);
	}

	/*
	 * must be implicit VR
	 */
	return (-1);
}

/**
 * Post-conditions:
 * - Leave file ptr after last DCM_GROUPFILEMETA tag
 *
 * @author   TINA legacy code - mods by GA Buonaccorsi
 * @brief    Check whether DICOM Part 10 file needs endian swap (finds endian UID in part10)
 * @version  dicom2analyze 2.0
 * @param    fp     Pointer to input image file stream
 * @return   bool   Retruns TINA boolean indicating endian - but it's complicated ...
 */
bool mdm_DicomFormat::dicom_part10_endian_swap(FILE * fp)
{
  bool  success = false;
  char  endian[32];
  
  unsigned char   junk;
  unsigned short  group, element;
  unsigned int    i, nbytes;
  
  long int where = ftell(fp);

/* part10 meta dicom are implicit little endian by default */
#ifdef LITTLE_ENDIAN_ARCHITECTURE
  bool little = false;
  bool big    = true;
  set_swapping_ts(0);
#else
  bool little = true;
  bool big    = false;
  set_swapping_ts(1);
#endif

  fread(&group, sizeof(short), 1, fp);
  short_swap((char *) &group);
  while (fp && group <= DCM_GROUPFILEMETA)         /* Assumes numerical tag ordering - not guaranteed */
  {
    fread(&element, sizeof(short), 1, fp);
    short_swap((char *) &element);
    fread(&nbytes, sizeof(int), 1, fp);
    word_swap((char *) &nbytes);
    dblock_vr_conv(&nbytes, fp);
    switch (DCM_MAKETAG(group, element))
    {
      case DCM_METATRANSFERSYNTAX:
        fread(endian, sizeof(char), nbytes, fp);
        endian[nbytes] = '\0';
        success = true;
        break;
      case DCM_DLMITEM:
      case DCM_DLMITEMDELIMITATIONITEM:
      case DCM_DLMSEQUENCEDELIMITATIONITEM:
        break;
      default:
        for (i = 0; i < nbytes; i++)
          fread(&junk, sizeof(char), 1, fp);
        break;
    }

    where = ftell(fp);
    fread(&group, sizeof(short), 1, fp);
    short_swap((char *) &group);
  }

  /* put fptr back before last group read */
  fseek(fp, where, SEEK_SET);
  if (success && !strcmp(endian, E_BIG_ENDIAN))
    return (big);
  /* else */
  return (little);
}


/**
 * Post-conditions:
 * -  PART10     - Leave fp after last DCM_GROUPFILEMETA tag (see dicom_part10_endian_swap())
 * -  NON PART10 - Leave fp at strt of stream (i.e. rewind())
 * -  ALL        - Set byte-swapping flag
 *
 * @author   TINA legacy code - mods by GA Buonaccorsi
 * @brief    Do some preread tests on a dicom image header  
 * @version  dicom2analyze 2.0
 * @param    fp     Pointer to input image file stream
 * @return   int    One of {MDM_DCMFILE_PART10 MDM_DCMFILE_NONPART10 MDM_DCMFILE_INVALID}
 */
int mdm_DicomFormat::dicom_preread_tests(FILE *fp)
{
  size_t  readerr;
  short   group, element, temp;
  char    td, ti, tc, tm;
  int     i, nbytes, repeat, count;

  if (fp == NULL)
    return (MDM_DCMFILE_INVALID);

  /* Option 1:  part10 DICM header */
  /* Part 10 dicom has a preamble of 128 bytes followed by "DICM" .... */
  fseek(fp, 128, SEEK_SET);
  readerr = fread(&td, sizeof(char), 1, fp);
  readerr = fread(&ti, sizeof(char), 1, fp);
  readerr = fread(&tc, sizeof(char), 1, fp);
  readerr = fread(&tm, sizeof(char), 1, fp);
  if (td == 'D' && ti == 'I' && tc == 'C' && tm == 'M')
  {
    set_dicom_format(MDM_DCMFILE_PART10);

    if (dicom_part10_endian_swap(fp))
      set_swapping_ts(1);
    else
      set_swapping_ts(0);

    return (MDM_DCMFILE_PART10);           
  }

  /* Option 2:  no pre-amble part10 DICM header */
  /* ... although preamble may be missing */
  rewind(fp);
  readerr = fread(&td, sizeof(char), 1, fp);
  readerr = fread(&ti, sizeof(char), 1, fp);
  readerr = fread(&tc, sizeof(char), 1, fp);
  readerr = fread(&tm, sizeof(char), 1, fp);
  if (td == 'D' && ti == 'I' && tc == 'C' && tm == 'M')
  {
    set_dicom_format(MDM_DCMFILE_PART10);

    if (dicom_part10_endian_swap(fp))
      set_swapping_ts(1);
    else
      set_swapping_ts(0);

    return (MDM_DCMFILE_PART10);
  }

  /* Option 3:  something else ... */
  repeat = 0;
  set_swapping_ts(0);   /* Just to be sure ... */
  while (repeat < 2)
  {
    rewind(fp);
    readerr = fread(&group, sizeof(short), 1, fp);
    short_swap((char *) &group);
    readerr = fread(&element, sizeof(short), 1, fp);
    short_swap((char *) &element);
    readerr = fread(&nbytes, sizeof(int), 1, fp);
    word_swap((char *) &nbytes);

    count = 0;
    while (group < DCM_GROUPIDENTIFYING && count < HEADERMAXCOUNT)
    {
      for (i = 0; i < nbytes; i++)
        readerr = fread(&temp, sizeof(char), 1, fp);

      readerr = fread(&group, sizeof(short), 1, fp);
      short_swap((char *) &group);
      readerr = fread(&element, sizeof(short), 1, fp);
      short_swap((char *) &element);
      readerr = fread(&nbytes, sizeof(int), 1, fp);
      word_swap((char *) &nbytes);
      count++;
    }

    /* standard non-part10 header start */
    if (DCM_MAKETAG(group, element) == DCM_IDGROUPLENGTH && nbytes == 4)
    {
      rewind(fp);
      set_dicom_format(MDM_DCMFILE_NONPART10);
      return (MDM_DCMFILE_NONPART10);
    }

    /* inferior although common non-part10 header starts */
    if ((DCM_MAKETAG(group, element) == DCM_IDLENGTHTOEND) 
        || (DCM_MAKETAG(group, element) == DCM_IDIMAGETYPE) 
        || (DCM_MAKETAG(group, element) == DCM_IDSPECIFICCHARACTER))
    {
      rewind(fp);
      set_dicom_format(MDM_DCMFILE_NONPART10);
      return (MDM_DCMFILE_NONPART10); 
    }

    set_swapping_ts(1);
    repeat ++;
  }

  /* If we got here, it ain't likely to be DICOM */
  return (MDM_DCMFILE_INVALID);   
}


/**
 * Pre-conditions
 * -  im is a valid Imrect, with packed 12-bit data
 *
 * DICOM allows 12-bit data to be packed into 2-byte parcels with an overlap.
 * This function unpacks the data into 2-byte parcels with 4 blank bits at the front.
 *
 * @author   TINA legacy code - mods by GA Buonaccorsi
 * @brief    Unpack 12-bit data in an Imrect
 * @version  dicom2analyze 2.0
 * @param    im     Pointer to input Imrect with packed 12-bit data
 * @return   mdm_Image3D &    Pointer to new Imrect with unpacked data
 */
mdm_Image3D & mdm_DicomFormat::im_dicom_conv(mdm_Image3D &im)
{
  Imrect   *im2;
  Imregion  roi;
  
  unsigned char *row1;
  unsigned char *row2;
  
  int i, j, k;
  int lx, ux, ly, uy;

  /* Change to assert(), when I can be bothered */
  if (im == NULL)
    return (NULL);

  roi = *(im->region);
  roi.ux = roi.ux * 4.0 / 3;
  im2 = im_alloc(im->height, roi.ux, &roi, short_v);
  lx = roi.lx;
  ux = roi.ux;
  ly = roi.ly;
  uy = roi.uy;

  for (i = ly; i < uy; ++i)
  {
    row1 = IM_ROW(im, i);
    row2 = IM_ROW(im2, i);
    for (k = 2 * lx, j = 2 * lx; k < 2 * ux; j += 3)
    {
      row2[k++] = row1[j];
      row2[k++] = row1[j + 1] % 16;
      row2[k++] = row1[j + 1] / 16 + (row1[j + 2] % 16) * 16;
      row2[k++] = row1[j + 2] / 16;
    }
  }

  return (im2);
}


/*
static bool dicom_hdr_TE_extract(FILE * fp, mdm_Image3D & im)
{
	bool success = false;
	float *TE;
	int type;
	int i;
	char temp[64];
	unsigned char junk;
	unsigned short group, element;
	unsigned int nbytes;

	if ((type = dicom_preread_tests(fp)) == MDM_DCMFILE_INVALID)
	{
		format("error: file failed pre checks\n");
		return (false);
	}

	TE = (float *) ralloc(sizeof(float));
	fread(&group, sizeof(short), 1, fp);
	short_swap((char *) &group);

	while (fp && !success && group != DCM_GROUPPIXEL)
	{
		fread(&element, sizeof(short), 1, fp);
		short_swap((char *) &element);
		fread(&nbytes, sizeof(int), 1, fp);
		word_swap((char *) &nbytes);
		dblock_vr_conv(&nbytes, fp);

		switch (DCM_MAKETAG(group, element))
		{
			case DCM_ACQECHOTIME:
				fread(temp, sizeof(char), nbytes, fp);
				temp[nbytes] = '\0';
				sscanf(temp, "%f", TE);
				*TE = *TE / 100.0;
				success = true;
				break;

			case DCM_DLMITEM:
			case DCM_DLMITEMDELIMITATIONITEM:
			case DCM_DLMSEQUENCEDELIMITATIONITEM:
				break;

			default:
				for (i = 0; i < nbytes; i++)
					fread(&junk, sizeof(char), 1, fp);
				break;
		}
		fread(&group, sizeof(short), 1, fp);
		short_swap((char *) &group);
	}

	if (success)
	{
		im->props = proplist_rm(im->props, TE_DATA);
		im->props =
				proplist_addifnp(im->props, (void *) TE, TE_DATA, rfree, false);
	}

	return (success);
}
 */
 
bool mdm_DicomFormat::dicom_hdr_TR_extract(FILE * fp, mdm_Image3D & im)
{
	bool success = false;
	float *TR;
	int type;
	int i;
	char temp[64];
	unsigned char junk;
	unsigned short group, element;
	unsigned int nbytes;

	if ((type = dicom_preread_tests(fp)) == MDM_DCMFILE_INVALID)
	{
		format("error: file failed pre checks\n");
		return (false);
	}

	TR = (float *) ralloc(sizeof(float));
	fread(&group, sizeof(short), 1, fp);
	short_swap((char *) &group);

	while (fp && !success && group != DCM_GROUPPIXEL)
	{
		fread(&element, sizeof(short), 1, fp);
		short_swap((char *) &element);
		fread(&nbytes, sizeof(int), 1, fp);
		word_swap((char *) &nbytes);
		dblock_vr_conv(&nbytes, fp);

		switch (DCM_MAKETAG(group, element))
		{
			case DCM_ACQREPETITIONTIME:
				fread(temp, sizeof(char), nbytes, fp);
				temp[nbytes] = '\0';
				sscanf(temp, "%f", TR);
				success = true;
				break;

			case DCM_DLMITEM:
			        for (i = 0; i < nbytes; i++)
			        fread(&junk, sizeof(char), 1, fp);
        			break;
			case DCM_DLMITEMDELIMITATIONITEM:
			case DCM_DLMSEQUENCEDELIMITATIONITEM:
				break;

			default:
				for (i = 0; i < nbytes; i++)
					fread(&junk, sizeof(char), 1, fp);
				break;
		}
		fread(&group, sizeof(short), 1, fp);
		short_swap((char *) &group);
	}

	if (success)
	{
		im->props = proplist_rm(im->props, TR_DATA);
		im->props =
				proplist_addifnp(im->props, (void *) TR, TR_DATA, rfree, false);
	}

	return (success);
}

bool mdm_DicomFormat::dicom_hdr_flip_angle_extract(FILE * fp, mdm_Image3D & im)
{
	bool success = false;
	float *flip_angle;
	int type;
	int i;
	char temp[64];
	unsigned char junk;
	unsigned short group, element;
	unsigned int nbytes;

	if ((type = dicom_preread_tests(fp)) == MDM_DCMFILE_INVALID)
	{
		format("error: file failed pre checks\n");
		return (false);
	}

	flip_angle = (float *) ralloc(sizeof(float));
	fread(&group, sizeof(short), 1, fp);
	short_swap((char *) &group);

	while (fp && !success && group != DCM_GROUPPIXEL)
	{
		fread(&element, sizeof(short), 1, fp);
		short_swap((char *) &element);
		fread(&nbytes, sizeof(int), 1, fp);
		word_swap((char *) &nbytes);
		dblock_vr_conv(&nbytes, fp);

		switch (DCM_MAKETAG(group, element))
		{
			case DCM_ACQFLIPANGLE:
				fread(temp, sizeof(char), nbytes, fp);
				temp[nbytes] = '\0';
				sscanf(temp, "%f", flip_angle);
				success = true;
				break;

			case DCM_DLMITEM:
			        for (i = 0; i < nbytes; i++)
			        fread(&junk, sizeof(char), 1, fp);
        			break;
			case DCM_DLMITEMDELIMITATIONITEM:
			case DCM_DLMSEQUENCEDELIMITATIONITEM:
				break;

			default:
				for (i = 0; i < nbytes; i++)
					fread(&junk, sizeof(char), 1, fp);
				break;
		}
		fread(&group, sizeof(short), 1, fp);
		short_swap((char *) &group);
	}

	if (success)
	{
		im->props = proplist_rm(im->props, FLIP_ANGLE_DATA);
		im->props =
				proplist_addifnp(im->props, (void *) flip_angle, FLIP_ANGLE_DATA, rfree, false);
	}

	return (success);
}

bool mdm_DicomFormat::dicom_hdr_image_time_extract(FILE * fp, mdm_Image3D & im)
{
	bool success = false;
	float *image_time;
	int type;
	int i;
	char temp[64];
	unsigned char junk;
	unsigned short group, element;
	unsigned int nbytes;

	if ((type = dicom_preread_tests(fp)) == MDM_DCMFILE_INVALID)
	{
		format("error: file failed pre checks\n");
		return (false);
	}

	image_time = (float *) ralloc(sizeof(float));
	fread(&group, sizeof(short), 1, fp);
	short_swap((char *) &group);

	while (fp && !success && group != DCM_GROUPPIXEL)
	{
		fread(&element, sizeof(short), 1, fp);
		short_swap((char *) &element);
		fread(&nbytes, sizeof(int), 1, fp);
		word_swap((char *) &nbytes);
		dblock_vr_conv(&nbytes, fp);

		switch (DCM_MAKETAG(group, element))
		{
			case DCM_IDIMAGETIME:
				fread(temp, sizeof(char), nbytes, fp);
				temp[nbytes] = '\0';
				sscanf(temp, "%f", image_time);
				success = true;
				break;

			case DCM_DLMITEM:
			        for (i = 0; i < nbytes; i++)
			        fread(&junk, sizeof(char), 1, fp);
        			break;
			case DCM_DLMITEMDELIMITATIONITEM:
			case DCM_DLMSEQUENCEDELIMITATIONITEM:
				break;

			default:
				for (i = 0; i < nbytes; i++)
					fread(&junk, sizeof(char), 1, fp);
				break;
		}
		fread(&group, sizeof(short), 1, fp);
		short_swap((char *) &group);
	}

	if (success)
	{
		im->props = proplist_rm(im->props, IMTIME_DATA);
		im->props =
				proplist_addifnp(im->props, (void *) image_time, IMTIME_DATA, rfree, false);
	}

	return (success);
}

bool mdm_DicomFormat::dicom_hdr_image_time_extract_GSKS(FILE * fp, mdm_Image3D & im)
{
	bool success = false;
	float *image_time;
	int type;
	int i;
	char temp[64];
	unsigned char junk;
	unsigned short group, element;
	unsigned int nbytes;

	if ((type = dicom_preread_tests(fp)) == MDM_DCMFILE_INVALID)
	{
		format("error: file failed pre checks\n");
		return (false);
	}

	image_time = (float *) ralloc(sizeof(float));
	fread(&group, sizeof(short), 1, fp);
	short_swap((char *) &group);

	while (fp && !success && group != DCM_GROUPPIXEL)
	{
		fread(&element, sizeof(short), 1, fp);
		short_swap((char *) &element);
		fread(&nbytes, sizeof(int), 1, fp);
		word_swap((char *) &nbytes);
		dblock_vr_conv(&nbytes, fp);

		switch (DCM_MAKETAG(group, element))
		{
			case DCM_IDACQUISITIONTIME:
				fread(temp, sizeof(char), nbytes, fp);
				temp[nbytes] = '\0';
				sscanf(temp, "%f", image_time);
				success = true;
				break;

			case DCM_DLMITEM:
			        for (i = 0; i < nbytes; i++)
			        fread(&junk, sizeof(char), 1, fp);
        			break;
			case DCM_DLMITEMDELIMITATIONITEM:
			case DCM_DLMSEQUENCEDELIMITATIONITEM:
				break;

			default:
				for (i = 0; i < nbytes; i++)
					fread(&junk, sizeof(char), 1, fp);
				break;
		}
		fread(&group, sizeof(short), 1, fp);
		short_swap((char *) &group);
	}

	if (success)
	{
		im->props = proplist_rm(im->props, IMTIME_DATA);
		im->props =
				proplist_addifnp(im->props, (void *) image_time, IMTIME_DATA, rfree, false);
	}

	return (success);
}

bool mdm_DicomFormat::dicom_hdr_image_time_extract_GSKGE(FILE * fp, mdm_Image3D & im)
{
	bool success = false;
        int    got = 0;
	float *image_time;
	float *series_time;
	float *trigger_time;
        int    hours, minutes;
        float  seconds, milliseconds;
	int type;
	int i;
	char temp[64];
	unsigned char junk;
	unsigned short group, element;
	unsigned int nbytes;

	if ((type = dicom_preread_tests(fp)) == MDM_DCMFILE_INVALID)
	{
		format("error: file failed pre checks\n");
		return (false);
	}

	image_time   = (float *) ralloc(sizeof(float));
	series_time  = (float *) ralloc(sizeof(float));
	trigger_time = (float *) ralloc(sizeof(float));
	fread(&group, sizeof(short), 1, fp);
	short_swap((char *) &group);

	while (fp && (got != 2) && group != DCM_GROUPPIXEL)
	{
		fread(&element, sizeof(short), 1, fp);
		short_swap((char *) &element);
		fread(&nbytes, sizeof(int), 1, fp);
		word_swap((char *) &nbytes);
		dblock_vr_conv(&nbytes, fp);

		switch (DCM_MAKETAG(group, element))
		{
			case DCM_IDSERIESTIME:
				fread(temp, sizeof(char), nbytes, fp);
				temp[nbytes] = '\0';
				sscanf(temp, "%f", series_time);
				got++;
				break;
			case DCM_ACQTRIGGERTIME:
				fread(temp, sizeof(char), nbytes, fp);
				temp[nbytes] = '\0';
				sscanf(temp, "%f", trigger_time);
				got++;
				break;

			case DCM_DLMITEM:
			        for (i = 0; i < nbytes; i++)
			        fread(&junk, sizeof(char), 1, fp);
        			break;
			case DCM_DLMITEMDELIMITATIONITEM:
			case DCM_DLMSEQUENCEDELIMITATIONITEM:
				break;

			default:
				for (i = 0; i < nbytes; i++)
					fread(&junk, sizeof(char), 1, fp);
				break;
		}
		fread(&group, sizeof(short), 1, fp);
		short_swap((char *) &group);
	}
        success = true;
        /* Now convert series time to hrs, mins, secs ... */
        hours     = (int) (*series_time / 10000);
        minutes   = (int) ((*series_time - 10000 * hours) / 100);
        seconds   = (float) (*series_time - 10000 * hours - 100 * minutes);
        /* ... then add trigger time to seconds and minutes ... */
        milliseconds = (seconds * 1000.0) + *trigger_time;
        minutes   = minutes + (int) (milliseconds / (60.0 * 1000.0));
        seconds   = (float) ((((int) milliseconds) % 60000) / 1000.0);
        /* ... and convert back to a timestamp */
        *image_time = hours * 10000 + minutes * 100 + seconds;
          
	if (success)
	{
		im->props = proplist_rm(im->props, IMTIME_DATA);
		im->props =
				proplist_addifnp(im->props, (void *) image_time, IMTIME_DATA, rfree, false);
	}

	return (success);
}
/*
bool dicom_hdr_dynstimes_extract(FILE * fp, mdm_Image3D & im)
{
	bool success = false;
	float *times;
	int type;
	int i, j, k, stime;
	char *buffer = NULL;
	char temp[64];
	unsigned char junk;
	unsigned short group, element;
	unsigned int nbytes;

	if ((type = dicom_preread_tests(fp)) == MDM_DCMFILE_INVALID)
	{
		format("error: file failed pre checks\n");
		return (false);
	}

	fread(&group, sizeof(short), 1, fp);
	short_swap((char *) &group);

	while (fp && !success && group != DCM_GROUPPIXEL)
	{
		fread(&element, sizeof(short), 1, fp);
		short_swap((char *) &element);
		fread(&nbytes, sizeof(int), 1, fp);
		word_swap((char *) &nbytes);
		dblock_vr_conv(&nbytes, fp);

		switch (DCM_MAKETAG(group, element))
		{
			case DCM_RELNUMBERTEMPORALPOSITIONS:
				fread(temp, sizeof(char), nbytes, fp);
				temp[nbytes] = '\0';
				sscanf(temp, "%d", &stime);
				break;

			case DCM_MAKETAG(0x0019, 0x1021):
				buffer = (char *) ralloc(nbytes * sizeof(char));
				times = fvector_alloc(0, stime);
				fread(buffer, sizeof(char), nbytes, fp);

				j = k = 0;
				for (i = 0; i < stime; i++)
				{
					while (buffer[j] != '\\')
						temp[k++] = buffer[j++];
					temp[k] = '\0';
					sscanf(temp, "%f", &(times[i]));
					k = 0;
					j++;
				}
				rfree(buffer);
				success = true;
				break;

			case DCM_DLMITEM:
			case DCM_DLMITEMDELIMITATIONITEM:
			case DCM_DLMSEQUENCEDELIMITATIONITEM:
				break;


			default:
				for (i = 0; i < nbytes; i++)
					fread(&junk, sizeof(char), 1, fp);
				break;
		}
		fread(&group, sizeof(short), 1, fp);
		short_swap((char *) &group);
	}

	if (success)
	{
		im->props = proplist_rm(im->props, DYNSTIME);
		im->props =
				proplist_addifnp(im->props, (void *) times, DYNSTIME, dynstimes_free, false);
	}

	return (success);
}
 */

/*
bool dicom_hdr_heartrate_extract(FILE * fp, mdm_Image3D & im)
{
	bool success = false;
	float *times, btime, phases, heartrate;
	float beattime;
	int type;
	int i;
	int psuccess = 0;
	char temp[64];
	unsigned char junk;
	unsigned short group, element;
	unsigned int nbytes;

	if ((type = dicom_preread_tests(fp)) == MDM_DCMFILE_INVALID)
	{
		format("error: file failed pre checks\n");
		return (false);
	}

	fread(&group, sizeof(short), 1, fp);
	short_swap((char *) &group);

	while (fp && !success && group != DCM_GROUPPIXEL)
	{
		fread(&element, sizeof(short), 1, fp);
		short_swap((char *) &element);
		fread(&nbytes, sizeof(int), 1, fp);
		word_swap((char *) &nbytes);
		dblock_vr_conv(&nbytes, fp);

		switch (DCM_MAKETAG(group, element))
		{
			case DCM_MAKETAG(0x0019, 0x1022):
				fread(temp, sizeof(char), nbytes, fp);
				temp[nbytes] = '\0';
				sscanf(temp, "%f", &btime);
				if (psuccess == 2)
					success = true;
				else
					psuccess++;
				break;

			case DCM_MAKETAG(0x0019, 0x1069):
				fread(temp, sizeof(char), nbytes, fp);
				temp[nbytes] = '\0';
				sscanf(temp, "%f", &phases);
				if (psuccess == 2)
					success = true;
				else
					psuccess++;
				break;

			case DCM_MAKETAG(0x0019, 0x106A):
				fread(temp, sizeof(char), nbytes, fp);
				temp[nbytes] = '\0';
				sscanf(temp, "%f", &heartrate);
				if (psuccess == 2)
					success = true;
				else
					psuccess++;
				break;

			case DCM_DLMITEM:
			case DCM_DLMITEMDELIMITATIONITEM:
			case DCM_DLMSEQUENCEDELIMITATIONITEM:
				break;


			default:
				for (i = 0; i < nbytes; i++)
					fread(&junk, sizeof(char), 1, fp);
				break;
		}
		fread(&group, sizeof(short), 1, fp);
		short_swap((char *) &group);
	}

	if (phases == 1)
		success = false;
	if (success)
	{
		times = fvector_alloc(0, phases);
		beattime = 1.0 / (heartrate / 60);
		for (i = 0; i < phases; i++)
			times[i] = btime + (float) i *(beattime / (phases - 1));
		im->props = proplist_rm(im->props, DYNSTIME);
		im->props =
				proplist_addifnp(im->props, (void *) times, DYNSTIME, rfree, false);
	}

	return (success);
}
 */

/*
bool dicom_hdr_patientdetails_extract(FILE * fp, mdm_Image3D & im)
{
	bool psuccess = false;
	int i, type;
	char *details = NULL;
	unsigned char junk;
	unsigned short group, element;
	unsigned int nbytes;

	if ((type = dicom_preread_tests(fp)) == MDM_DCMFILE_INVALID)
	{
		format("error: file failed pre checks\n");
		return (false);
	}

	fread(&group, sizeof(short), 1, fp);
	short_swap((char *) &group);

	details = (char *) cvector_alloc(0, 256);
	while (fp && !psuccess && group != DCM_GROUPPIXEL)
	{
		fread(&element, sizeof(short), 1, fp);
		short_swap((char *) &element);
		fread(&nbytes, sizeof(int), 1, fp);
		word_swap((char *) &nbytes);
		dblock_vr_conv(&nbytes, fp);

		switch (DCM_MAKETAG(group, element))
		{
			case DCM_MAKETAG(0x0008, 0x0020):
				fread(details, sizeof(char), nbytes, fp);
				details[nbytes] = '\0';
				psuccess = true;
				break;

			case DCM_DLMITEM:
			case DCM_DLMITEMDELIMITATIONITEM:
			case DCM_DLMSEQUENCEDELIMITATIONITEM:
				break;


			default:
				for (i = 0; i < nbytes; i++)
					fread(&junk, sizeof(char), 1, fp);
				break;
		}
		fread(&group, sizeof(short), 1, fp);
		short_swap((char *) &group);
	}

	if (psuccess != true)
		cvector_free(details, 0);
	else
	{
		im->props = proplist_rm(im->props, PAT_DATA);
		im->props = proplist_addifnp(im->props, (void *) details,
																 PAT_DATA, rfree, false);
	}

	return (psuccess);
}
 */

bool mdm_DicomFormat::dicom_hdr_voxelscale_extract(FILE * fp, mdm_Image3D & im)
{
	Vec3 *iscale;
	float xsize, ysize, zsize;
	int type;
	int psuccess = 0;
	int i;
	char value[64];
	unsigned char junk;
	unsigned short group, element;
	unsigned int nbytes;

	xsize = ysize = zsize = 1.0;
	if ((type = dicom_preread_tests(fp)) == MDM_DCMFILE_INVALID)
	{
		format("error: file failed pre checks\n");
		return (false);
	}

	fread(&group, sizeof(short), 1, fp);
	short_swap((char *) &group);

	while (fp && group != DCM_GROUPPIXEL && psuccess < 3)
	{

		fread(&element, sizeof(short), 1, fp);
		short_swap((char *) &element);
		fread(&nbytes, sizeof(int), 1, fp);
		word_swap((char *) &nbytes);
		dblock_vr_conv(&nbytes, fp);

		switch (DCM_MAKETAG(group, element))
		{
			case DCM_ACQSLICESPACING:
				fread(value, sizeof(char), nbytes, fp);
				value[nbytes] = '\0';
				sscanf(value, "%f", &zsize);
				psuccess++;
				break;

			case DCM_IMGPIXELSPACING:
				fread(value, sizeof(char), nbytes, fp);
				value[nbytes] = '\0';
				sscanf(value, "%f\\%f", &xsize, &ysize);
				psuccess++;
				break;

			case DCM_DLMITEM:
			        for (i = 0; i < nbytes; i++)
			        fread(&junk, sizeof(char), 1, fp);
        			break;
			case DCM_DLMITEMDELIMITATIONITEM:
			case DCM_DLMSEQUENCEDELIMITATIONITEM:
				break;


			default:
				for (i = 0; i < nbytes; i++)
					fread(&junk, sizeof(char), 1, fp);
				break;
		}
		fread(&group, sizeof(short), 1, fp);
		short_swap((char *) &group);
	}

	if (psuccess == 2)
	{
		iscale = vec3_alloc();
		iscale->el[0] = xsize;
		iscale->el[1] = ysize;
		iscale->el[2] = zsize;

		im->props = proplist_rm(im->props, VOXELS);
		im->props = proplist_addifnp(im->props, iscale,
																 VOXELS, vec3_free, false);
	}

	return ((bool) (psuccess == 3));
}

bool mdm_DicomFormat::dicom_hdr_voxelscale_extract_CT(FILE * fp, mdm_Image3D & im)
{
	Vec3 *iscale;
	float xsize, ysize, zsize;
	int type;
	int psuccess = 0;
	int i;
	char value[64];
	unsigned char junk;
	unsigned short group, element;
	unsigned int nbytes;

	xsize = ysize = zsize = 1.0;
	if ((type = dicom_preread_tests(fp)) == MDM_DCMFILE_INVALID)
	{
		format("error: file failed pre checks\n");
		return (false);
	}

	fread(&group, sizeof(short), 1, fp);
	short_swap((char *) &group);

	while (fp && group != DCM_GROUPPIXEL && psuccess < 3)
	{

		fread(&element, sizeof(short), 1, fp);
		short_swap((char *) &element);
		fread(&nbytes, sizeof(int), 1, fp);
		word_swap((char *) &nbytes);
		dblock_vr_conv(&nbytes, fp);

		switch (DCM_MAKETAG(group, element))
		{
			case DCM_ACQSLICETHICKNESS:
				fread(value, sizeof(char), nbytes, fp);
				value[nbytes] = '\0';
				sscanf(value, "%f", &zsize);
				psuccess++;
				break;

			case DCM_IMGPIXELSPACING:
				fread(value, sizeof(char), nbytes, fp);
				value[nbytes] = '\0';
				sscanf(value, "%f\\%f", &xsize, &ysize);
				psuccess++;
				break;

			case DCM_DLMITEM:
			        for (i = 0; i < nbytes; i++)
			        fread(&junk, sizeof(char), 1, fp);
        			break;
			case DCM_DLMITEMDELIMITATIONITEM:
			case DCM_DLMSEQUENCEDELIMITATIONITEM:
				break;


			default:
				for (i = 0; i < nbytes; i++)
					fread(&junk, sizeof(char), 1, fp);
				break;
		}
		fread(&group, sizeof(short), 1, fp);
		short_swap((char *) &group);
	}

	if (psuccess == 2)
	{
		iscale = vec3_alloc();
		iscale->el[0] = xsize;
		iscale->el[1] = ysize;
		iscale->el[2] = zsize;

		im->props = proplist_rm(im->props, VOXELS);
		im->props = proplist_addifnp(im->props, iscale,
																 VOXELS, vec3_free, false);
	}

	return ((bool) (psuccess == 3));
}

/**
 * Pre-conditions
 * -  fp_img is a valid stream to a dicom image file
 * -  stream pointer for fp_img is at the start of the header data
 *
 * Stores voxel dimensions in Imrect props list
 *
 * @author   TINA legacy code - mods by GA Buonaccorsi
 * @brief    Read a dicom image file and store as an Imrect, with dimensions
 * @version  dicom2analyze 2.0
 * @param    pathname   String dicom image file name
 * @param    fp_img     Pointer to input image file stream
 * @return   mdm_Image3D &   Pointer to Imrect with the image data and dimensions
 */
mdm_Image3D mdm_DicomFormat::dicom_read_multiformat_image(const std::string &pathname, FILE *fp_img, int file_type)
{
  int get_swapping_ts();    /* Missing header.  Get away with set ... because it returns nowt */
  
  Imrect   *imrect  = NULL;
  Imrect   *imrect2 = NULL;
  Imregion  imregion;
  Vartype   new_vtype;
  
  unsigned short  group;
  unsigned short  element;
  unsigned int    nbytes;
  char            value[64];
  unsigned short  abits;
  unsigned short  sign;
  unsigned short  rows;
  unsigned short  cols;
  unsigned char   junk;
  
  float  xsize = 1.0, ysize = 1.0, zsize = 1.0;   /* "Sensible" default values for voxel dimensions ... */
  Vec3  *iscale;                                  /* To store voxel dimensions in an Imrect props list ...*/
  float  scale_slope = 1.0, scale_intercept = 0.0;  
  
  int    i, j, k;
  float  pixelValue;
  
  /* USe assert() for fp_img is not NULL */

  fread(&group, sizeof(short), 1, fp_img);
  short_swap((char *) &group);
  fread(&element, sizeof(short), 1, fp_img);
  short_swap((char *) &element);
  
  /* 
   * Loop through the header elements, picking up the ones we want and ignoring the rest. 
   * Stop when we get to the pixel data tag.
   */



  while (fp_img && DCM_MAKETAG(group, element) != DCM_PXLPIXELDATA)
  {
    fread(&nbytes, sizeof(int), 1, fp_img);
    word_swap((char *) &nbytes);
    dblock_vr_conv(&nbytes, fp_img);
    
    if (DCM_MAKETAG(group, element) == DCM_PXLPIXELDATA)
      break;

    switch (DCM_MAKETAG(group, element))
    {
      case DCM_ACQSLICESPACING:
        fread(value, sizeof(char), nbytes, fp_img);
        value[nbytes] = '\0';
        sscanf(value, "%f", &zsize);
        break;
      case DCM_IMGPIXELSPACING:
        fread(value, sizeof(char), nbytes, fp_img);
        value[nbytes] = '\0';
        sscanf(value, "%f\\%f", &xsize, &ysize);
        break;
      case DCM_IMGROWS:
        fread(&rows, sizeof(short), 1, fp_img);
        short_swap((char *) &rows);

        /* printf("there are %d rows\n", rows); */

        break;
      case DCM_IMGCOLUMNS:
        fread(&cols, sizeof(short), 1, fp_img);
        short_swap((char *) &cols);

        /* printf("there are %d columns\n", cols); */

        break;
      case DCM_IMGPIXELREPRESENTATION:
        fread(&sign, sizeof(short), 1, fp_img);
        short_swap((char *) &sign);
        break;
      case DCM_IMGBITSALLOCATED:
        fread(&abits, sizeof(short), 1, fp_img);
        short_swap((char *) &abits);
        break;
      case DCM_IMGRESCALESLOPE:
        if (file_type == MDM_DICOM_FILE_CT2HU)
        {
          fread(value, sizeof(char), nbytes, fp_img);
          value[nbytes] = '\0';
          sscanf(value, "%f", &scale_slope);
        }
        else
        {
          for (i = 0; i < nbytes; i++)
            fread(&junk, sizeof(char), 1, fp_img);
        }
        break;
      case DCM_IMGRESCALEINTERCEPT:
        if (file_type == MDM_DICOM_FILE_CT2HU)
        {
          fread(value, sizeof(char), nbytes, fp_img);
          value[nbytes] = '\0';
          sscanf(value, "%f", &scale_intercept);
        }
        else
        {
          for (i = 0; i < nbytes; i++)
            fread(&junk, sizeof(char), 1, fp_img);
        }
        break;
      case DCM_MAKETAG(0x2005, 0x100e):
        if (file_type == MDM_DICOM_FILE_R10)
        {
          fread(&scale_slope, sizeof(float), 1, fp_img);
          word_swap((char *) &scale_slope);
        }
        else
        {
          for (i = 0; i < nbytes; i++)
            fread(&junk, sizeof(char), 1, fp_img);
        }
        break;
      case DCM_MAKETAG(0x2005, 0x100d):
        if (file_type == MDM_DICOM_FILE_R10)
        {
          fread(&scale_intercept, sizeof(float), 1, fp_img);
          word_swap((char *) &scale_intercept);
        }
        else
        {
          for (i = 0; i < nbytes; i++)
            fread(&junk, sizeof(char), 1, fp_img);
        }
        break;
      case DCM_MAKETAG(0x0029, 0x1053):         /* Phillips Philips document ID XJR 2466 - slope */
        if (file_type == MDM_DICOM_FILE)
        {
          fread(value, sizeof(char), nbytes, fp_img);
          value[nbytes] = '\0';
          sscanf(value, "%f", &scale_slope);
        }
        else
        {
          for (i = 0; i < nbytes; i++)
            fread(&junk, sizeof(char), 1, fp_img);
        }
        break;
      case DCM_MAKETAG(0x0029, 0x1052):         /* Phillips Philips document ID XJR 2466 - intercept */
        if (file_type == MDM_DICOM_FILE)
        {
          fread(value, sizeof(char), nbytes, fp_img);
          value[nbytes] = '\0';
          sscanf(value, "%f", &scale_intercept);
        }
        else
        {
          for (i = 0; i < nbytes; i++)
            fread(&junk, sizeof(char), 1, fp_img);
        }
        break;


      case DCM_DLMITEM:


/* embedded items ie tag of FFFE, E000 are skipped */


        /* printf("SKIP EMBEDDED STUFF\n"); */


        for (i = 0; i < nbytes; i++)
          fread(&junk, sizeof(char), 1, fp_img);
        break;

       

      case DCM_DLMITEMDELIMITATIONITEM:

       break;

      case DCM_DLMSEQUENCEDELIMITATIONITEM:
        break;
      default:
        for (i = 0; i < nbytes; i++)
          fread(&junk, sizeof(char), 1, fp_img);
        break;
    }
    
    fread(&group, sizeof(short), 1, fp_img);
    short_swap((char *) &group);
    fread(&element, sizeof(short), 1, fp_img);
    short_swap((char *) &element);
  } /* End of while loop - now we're at the pixel data ... */

  if (fp_img)
  {
    /* Move to start of image data depending on VR mode */
    fread(&nbytes, sizeof(int), 1, fp_img);
    word_swap((char *) &nbytes);
    dblock_vr_conv(&nbytes, fp_img);

    /* Set ROI to whole image */
    imregion.lx = 0;
    imregion.ux = cols;
    imregion.ly = 0;
    imregion.uy = rows;

    /* 
     * Set Imrect var type, depending on allocated bits (should only be 16, 12 or 8)
     * and whether data is signed or not.
     */
    if (sign == 0)
    {
      if (abits == 16 || abits == 12)    
        new_vtype = ushort_v;
      else
        new_vtype = uchar_v;
    } 
    else
    {
      if (abits == 16 || abits == 12)
        new_vtype = short_v;
      else
        new_vtype = char_v;
    }
                                
    /*
     * Packed 12-bit data will take up less space than 16-bit, so adjust the number of cols to read.
     * We need to do this beacause fread_imrect_data() only wants to know how many bytes to read,
     * so we'll go off the end of the file.
     */ 
    if (abits == 12)
    {
      imregion.ux = 3.0 * imregion.ux / 4;
      cols = imregion.ux;
    }
    /* Malloc the Imrect and read the pixel data */
    imrect = im_alloc(rows, cols, &imregion, (Vartype) new_vtype);
    if (!fread_imrect_data(imrect, fp_img, pathname))
    {
      im_free(imrect);
      imrect = NULL;
      return (NULL);
    }
    /* Now re-adjust if we had 12-bit packed data */
    if (abits == 12)
    {
      imrect2 = im_dicom_conv(imrect);
      im_free(imrect);
      imrect = imrect2;
    }
    
    /* Next do byte-swapping if required */
    if (imrect != NULL && get_swapping_ts())
    {
      im_endian_inplace(imrect);
    }

    /* Apply scaling to pixel data, if required */
    printf("dicom_read_multiformat_image:  scale_intercept = %g;  scale_slope = %g\n", scale_intercept, scale_slope);
    imrect2 = im_cast(imrect, float_v);
    im_free(imrect);
    imrect = imrect2;
    if (scale_slope != 0.0)
    {
      for (j = imrect->region->ly; j < imrect->region->uy; j++)
      {
        for (k = imrect->region->lx; k < imrect->region->ux; k++)
        {
          pixelValue = im_get_pixf(imrect, j, k);
          if (file_type == MDM_DICOM_FILE_CT2HU)
            pixelValue = pixelValue * scale_slope + scale_intercept;
          else
            pixelValue = (pixelValue - scale_intercept) / scale_slope;
          im_put_pixf(pixelValue, imrect, j, k);
        }
      }
    }
    else
      printf("dicom_read_multiformat_image:  Warning - Zero scale factor - scaling ignored!!!!!\n");

    /* Add voxel dimensions to Imrect props list */
    iscale = vec3_alloc();
    iscale->el[0] = xsize;
    iscale->el[1] = ysize;
    iscale->el[2] = zsize;
    imrect->props = proplist_addifnp(imrect->props, iscale, VOXELS, vec3_free, false);
  }

  return imrect;
}

/**
 * Returns NULL pointer if file open fails.  Therefore passes responsibility for dealing with file
 * error back up to calling routine.
 *
 * @author   TINA legacy code - mods by GA Buonaccorsi
 * @brief    Wrapper for dicom_read_multiformat_image 
 * @version  dicom2analyze 2.0
 * @param    pathname   String dicom image file name with full path
 * @return   mdm_Image3D &   Pointer to Imrect with image data
 */
mdm_Image3D mdm_DicomFormat::dicom_read_image(const std::string &pathname, int file_type)
{
  FILE    *fp_img;
  Imrect  *im;

  if ((fp_img = fopen_2(pathname, "rb")) == NULL)
    return (NULL);

  switch (dicom_preread_tests(fp_img))
  {
    case MDM_DCMFILE_PART10:
    case MDM_DCMFILE_NONPART10:
      im = dicom_read_multiformat_image(pathname, fp_img, file_type);
      fclose_2(fp_img, pathname);
      break;
    default:
      printf("\ndicom_read_image:  error - file failed pre checks\n");
      fclose_2(fp_img, pathname);
      im = NULL;
      break;
  }

  return (im);
}


/*
 *  Adding dicom writer; GAB Aug 02
 */


mdm_Image3D & mdm_DicomFormat::imrect_to_ushort(mdm_Image3D &imrect1)
{
	mdm_Image3D &imrect2;
	double min;
	float immin, immax;
	FILE *debug = fopen("dcm_debug.txt", "w");

	switch (imrect1->vtype)
	{
		case ushort_v:	
			imrect2 = imrect1;
			break;

		case uchar_v:	
			imrect2 = im_cast(imrect1, ushort_v);
			break;

		case short_v:
		case char_v:	
			min = imf_min(imrect1, NULL, NULL);
			if (floor(min + 0.5) < 0)
			{
				imf_add_inplace(-min, imrect1);
			}
			imrect2 = im_cast(imrect1, ushort_v);
			break;

		case int_v:
		case uint_v:
		case float_v:
		case double_v:
			imf_minmax(imrect1, &immin, &immax);
			fprintf(debug, "USHRT_MAX = %d\n", USHRT_MAX);
			fprintf(debug, "immin = %-6.2g\t immax = %-6.2g\n", immin, immax);
			if (ceil(immax - immin) > USHRT_MAX)
			{
				imrect2 = imf_scale(imrect1, 0, (float) USHRT_MAX);
				imrect1 = imrect2;
			}
			imrect2 = im_cast(imrect1, ushort_v);
			break;

		default:	
			imrect2 = NULL;
			break;
	}
	fclose(debug);
	return imrect2;
}

bool mdm_DicomFormat::dicom_write_preamble(FILE * stream, char *err_msg)
{
	bool write_ok = true;
	char preamble[PREAMBLE_LENGTH + 4];
	int i;

	if (stream == NULL)
	{
		strcpy(err_msg, "No stream to write dicom preamble\n");
		write_ok = false;
		return write_ok;
	}

	for (i = 0; i <= PREAMBLE_LENGTH - 1; i++)
	{
		preamble[i] = 0x00;
	}
	preamble[PREAMBLE_LENGTH]     = 'D';
	preamble[PREAMBLE_LENGTH + 1] = 'I';
	preamble[PREAMBLE_LENGTH + 2] = 'C';
	preamble[PREAMBLE_LENGTH + 3] = 'M';

	if (fwrite(preamble, sizeof(preamble), 1, stream) != 1)
	{
		strcpy(err_msg, "Error writing dicom preamble\n");
		write_ok = false;
		return write_ok;
	} 

	return write_ok;
}

bool mdm_DicomFormat::dicom_write_att(DCM_TAG tag, char *vr, unsigned int vl, void *vf, FILE *stream)
{
	unsigned short group, element;
	bool write_ok = false;
	int i;
	unsigned int vlsave;
	char *vfstr;

	vlsave = vl;

	group = DCM_TAG_GROUP(tag);
	short_swap((char *) &group);
	fwrite(&group, sizeof(short), 1, stream);

	element = DCM_TAG_ELEMENT(tag);
	short_swap((char *) &element);
	fwrite(&element, sizeof(short), 1, stream);

	word_swap((char *) &vl);
	fwrite(&vl, sizeof(int), 1, stream);

	if ((vr == "OW") || (vr == "US") || (vr == "SS"))
	{
		short_swap((char *) vf);
		if (fwrite(vf, sizeof(short), 1, stream) == 1)
			write_ok = true;
		return write_ok;
	}

	if ((vr == "SL") || (vr == "UL") || (vr == "FL"))
	{
		word_swap((char *) vf);
		if (fwrite(vf, sizeof(int), 1, stream) == 1)
			write_ok = true;
		return write_ok;
	}

	if (vr == "FD")
	{
		long_swap((char *) vf);
		if (fwrite(vf, sizeof(long), 1, stream) == 1)
			write_ok = true;
		return write_ok;
	}

	vfstr = (char *) vf;

	for(i = 0; i <= vlsave - 1; i++)
	{
		if (fwrite(&vfstr[i], sizeof(char), 1, stream) == 1)
			write_ok = true;
	}

	return write_ok;
}

bool mdm_DicomFormat::dicom_write_header(FILE * stream, mdm_Image3D &imrect, char *err_msg)
{
	bool write_ok = true;
	char *sopclassuid = "1.2.840.10008.5.1.4.1.1.4";
	unsigned short sperpix = 1, abits = 16, sbits = 16, pixrep = 0, highbit = 15, rows = 256, cols = 256;

/*	write_ok = dicom_write_att(DCM_METAGROUPLENGTH, "OB", 2, &sperpix, stream);             */
/*	write_ok = dicom_write_att(DCM_METAIMPLEMENTATIONCLASS, "UI", 8, otheruid, stream);  	*/
	write_ok = dicom_write_att(DCM_IDIMAGETYPE, "CS", 16, "ORIGINAL/PRIMARY", stream);      
	write_ok = dicom_write_att(DCM_IDSOPCLASSUID, "UI", 26, sopclassuid, stream);
	write_ok = dicom_write_att(DCM_IDSOPINSTANCEUID, "UI", 32, "999.999.2.19960619.163000.1.111", stream);
	write_ok = dicom_write_att(DCM_IDSTUDYDATE, "DA", 8, "19950626", stream);
	write_ok = dicom_write_att(DCM_IDIMAGEDATE, "DA", 8, "19950626", stream);
	write_ok = dicom_write_att(DCM_IDSTUDYTIME, "TM", 6, "112000", stream);
	write_ok = dicom_write_att(DCM_IDMODALITY, "CS", 2, "MR", stream);
	write_ok = dicom_write_att(DCM_IDMANUFACTURER, "LO", 8, "Philips", stream);
	write_ok = dicom_write_att(DCM_IDINSTITUTIONNAME, "LO", 18, "Community Hospital", stream);
	write_ok = dicom_write_att(DCM_IDINSTITUTIONADDR, "ST", 18, "Anytown, Anywhere", stream); 
	write_ok = dicom_write_att(DCM_PATNAME, "PN", 8, "Doe John", stream);
	write_ok = dicom_write_att(DCM_PATID, "LO", 12, "123-45-6789", stream);	
	write_ok = dicom_write_att(DCM_PATBIRTHDATE, "DA", 8, "19000101", stream);
	write_ok = dicom_write_att(DCM_PATSEX, "CS", 2, "M", stream);
	write_ok = dicom_write_att(DCM_ACQSLICETHICKNESS, "DS", 6, "10.00", stream);
	write_ok = dicom_write_att(DCM_ACQREPETITIONTIME, "DS", 8, "1333.33", stream);
	write_ok = dicom_write_att(DCM_ACQECHOTIME, "DS", 6, "11.98", stream);
	write_ok = dicom_write_att(DCM_ACQFIELDOFVIEWDIMENSION, "IS", 4, "350", stream);
	write_ok = dicom_write_att(DCM_ACQFLIPANGLE, "DS", 2, "50", stream);
	write_ok = dicom_write_att(DCM_IMGSAMPLESPERPIXEL, "US", 2, &sperpix, stream);
	write_ok = dicom_write_att(DCM_IMGPHOTOMETRICINTERP, "CS", 12, DCM_IMGPHOTOINTERPMONOCHROME2, stream);
	write_ok = dicom_write_att(DCM_IMGROWS, "US", 2, &rows, stream);
	write_ok = dicom_write_att(DCM_IMGCOLUMNS, "US", 2, &cols, stream);
	write_ok = dicom_write_att(DCM_IMGBITSALLOCATED, "US", 2, &abits, stream);
	write_ok = dicom_write_att(DCM_IMGBITSSTORED, "US", 2, &sbits, stream);
	write_ok = dicom_write_att(DCM_IMGHIGHBIT, "US", 2, &highbit, stream);
	write_ok = dicom_write_att(DCM_IMGPIXELREPRESENTATION, "US", 2, &pixrep, stream);
/*	write_ok = dicom_write_att(DCM_IMGRESCALEINTERCEPT, "DS", 6, "-0.055", stream);         */
/*	write_ok = dicom_write_att(DCM_IMGRESCALESLOPE, "DS", 4, "2.01", stream);               */

	return write_ok;          /*  NEED TO FIX THIS TO GIVE PROPER AND OF WRITES (ALSO SWAP) */
}

bool mdm_DicomFormat::dicom_write_pixeldata(FILE * stream, mdm_Image3D &imrect, const std::string & pathname)
{
	bool write_ok = true;
	int vl;
	short group, element;

	vl = (imrect->region->uy - imrect->region->ly) *
		 (imrect->region->ux - imrect->region->lx) *
		 sizeof(short);	

	group = DCM_TAG_GROUP(DCM_PXLPIXELDATA);
	short_swap((char *) &group);
	fwrite(&group, sizeof(short), 1, stream);

	element = DCM_TAG_ELEMENT(DCM_PXLPIXELDATA);
	short_swap((char *) &element);
	fwrite(&element, sizeof(short), 1, stream);

	word_swap((char *) &vl);
	fwrite(&vl, sizeof(int), 1, stream);

	im_endian_inplace(imrect);
	write_ok = fwrite_imrect_data(imrect, stream, pathname);

	return write_ok;
}                                                                            

bool mdm_DicomFormat::dicom_write_image(mdm_Image3D &imrect, const std::string &pathname)
{
	mdm_Image3D &imrect2;
	bool write_ok = true;
	FILE *stream;
	int swap = 1;

	set_swapping_ts(swap);

	if (imrect == NULL)
	{
		strcpy(err_msg, "No image for dicom writer to play with\n");
		write_ok = false;
		return write_ok;
	}

	if ((stream = fopen(pathname, "wb")) == NULL) 
	{
		strcpy(err_msg, strcat("Dicom writer failed to open stream to ", pathname));
		write_ok = false;
		return write_ok;
	}

	if ((imrect2 = imrect_to_ushort(imrect)) == NULL)
	{
		if (fclose(stream) == EOF)
		{
			strcpy(err_msg, "Error writing to dicom file\n");
			write_ok = false;
			return write_ok;
		}
	}
	else
	{
		imrect = im_copy(imrect2);
		im_free(imrect2);
	}

	if (dicom_write_preamble(stream, err_msg) == false)
	{
		write_ok = false;
		return write_ok;
	}

	if (dicom_write_header(stream, imrect, err_msg) == false)
	{
		write_ok = false;
		return write_ok;
	}

	if (dicom_write_pixeldata(stream, imrect, pathname, err_msg) == false)
	{
		strcpy(err_msg, "Error writing pixel data to dicom file\n");
		write_ok = false;
		return write_ok;
	}
/*
 *   Leaves open the question of who closes the stream - RESOLVE
 */
	if (fclose(stream) == EOF)
	{
		strcpy(err_msg, "Error closing dicom file\n");
		write_ok = false;
		return write_ok;
	}
	
	return write_ok;	
}

/*
 *   End of Dicom writer additions GAB Aug 02
 */                                                               

