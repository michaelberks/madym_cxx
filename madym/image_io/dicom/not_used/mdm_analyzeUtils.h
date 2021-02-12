/**
 *  @file    mdm_analyzeUtils.h
 *  @brief   Gio's Analyze file utilities - header
 *
 *  Original author GA Buonaccorsi 21 May 2004
 *  (c) Copyright ISBE, University of Manchester 2004
 *
 *  Last edited GAB 21 Aug 07
 *  GAB mods:
 *  21 May 2004
 *  - Created
 *  27 Apr 07 - Added timeStamp field for dynamic series time stamp (for when it's missing)
 *  30 Apr 07 - Added endian swap field for dispanal compatibility
 *  11 May 07 - Added MDM_DICOM_FILE_CT for CT images - not a good solution, but we're getting to the point where we need a rethink
 *  21 May 07 - Added MDM_DICOM_FILE_CT2HU for CT images to be scaled to Hounsfield Units
 *  23 May 07 - Added MDM_DICOM_FILE_GSKGE for GE scanner images acquired for the GSK study
 *  21 Aug 07 - Added MDM_DICOM_FILE_GSKS for Siemens scanner images acquired for the GSK study
 *  22 Aug 07 - Added MDM_FILETYPE_UNSET for input control
 *
 */

#ifndef MDM_ANALYZEUTILS_HDR
#define MDM_ANALYZEUTILS_HDR



#include <madym/ana/qbiDbh.h> /* For Analyze 7.5 */
#include <vcl_compiler.h>
#include <string>

/* These should become enumerated types */
#define MDM_FILETYPE_UNSET    -1
#define MDM_NEMA_FILE_R6       0   /* No longer fully supported */
#define MDM_NEMA_FILE_R8       1   /* No longer fully supported */
#define MDM_DICOM_FILE         2
#define MDM_DICOM_FILE_R10     3  
#define MDM_DICOM_FILE_CT      4
#define MDM_DICOM_FILE_CT2HU   5
#define MDM_DICOM_FILE_GSKGE   6
#define MDM_DICOM_FILE_GSKS    7



struct mdm_analyzeInputs
{
  bool   debug;                       /* Y/N flag - show debug info? */
  bool   endianSwap;                  /* Y/N flag - swap bytes? */
  char   fileType;                    /* Byte integer file type flag */
  float  scale;                       /* Scale factor to keep signal in 2-byte int range */
  float  timeStamp;                   /* Time stamp (for when timing info is unavailable) */
  bool   optionsFile;                 /* Y/N flag - create MaDyM info (*.xtr) file? */
  std::string   inputDir;        /* Directory holding input files */
  std::string   outputName;      /* Base file name for output files */
};

int    mdm_setInputFileType(struct mdm_analyzeInputs *inputs, char newType);
void   mdm_anaHdrSetDefaults(struct dsr  *hdr);
void   mdm_callAnalyzeConverter(struct mdm_analyzeInputs inputs);

#endif /* MDM_ANALYZEUTILS_HDR */
