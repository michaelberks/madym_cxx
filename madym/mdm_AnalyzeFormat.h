/**
 *  @file    mdm_AnalyzeFormat.h
 *  @brief   Class for Analyze image format reading and writing
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_ANALYZEFORMAT_H
#define MDM_ANALYZEFORMAT_H

#include "mdm_api.h"
#include <madym/mdm_Image3D.h>

 /**
	* @brief Analyze image format reading and writing
	*/
class mdm_AnalyzeFormat {

public:
	/**
	* @brief    Enum of recognized Analyze data formats
	* @details  Only DT_UNSIGNED_CHAR, DT_SIGNED_SHORT, DT_SIGNED_INT,
	*						DT_FLOAT and DT_DOUBLE supported.
  */
	enum Data_type {
		DT_NONE = 0,
		DT_UNKNOWN = DT_NONE,
		DT_BINARY = 1,
		DT_UNSIGNED_CHAR = 2,
		DT_SIGNED_SHORT = 4,
		DT_SIGNED_INT = 8,
		DT_FLOAT = 16,
		DT_COMPLEX = 32,
		DT_DOUBLE = 64,
		DT_RGB = 128,
		DT_ALL = 255
	};

	/**
	* @brief    Enum of recognized .xtr formats
	*/
	enum XTR_type {
		NO_XTR = -1,
		OLD_XTR = 0,
		NEW_XTR = 1,
	};

	/**
	* @brief    Read Analyze format file(s) to given mdm_Image3D object
	* @param    fileName   string name of file from which to read the data
	* @param    img   reference to mdm_Image3D into which image data is loaded
	* @param		loadXtr			bool flag, if true tries to load .xtr file too
	* @return   bool, true on success, false otherwise
	*/
	MDM_API static bool readImage3D(const std::string& fileName, mdm_Image3D &img,
		bool load_xtr);

	/**
	* @brief    Read Analyze format file(s) and return mdm_Image3D object
	* @param    fileName		string name of file from which to read the data
	* @param		loadXtr			bool flag, if true tries to load .xtr file too
	* @return   mdm_Image3D object containing image read from disk
	*/
	MDM_API static mdm_Image3D readImage3D(const std::string& fileName,
		bool loadXtr);

	
	/**
	* @brief    Write mdm_Image3D to QBI extended Analyze hdr/img/xtr file set
	* @param    baseName      base name for file (gets .hdr/.img/.xtr appended)
	* @param    img           mdm_Image3D holding the data to be written to file
	* @param    dataTypeFlag  integer data type flag; see Data_type enum
	* @param    xtrTypeFlag   integer xtr type flag; 0 for old, 1 for new
	* @param		sparse				bool flag, if true, only non-zero voxels and their indices written
	* @return   bool 0 for success or 1 for failure
	*/
	MDM_API static bool writeImage3D(const std::string & baseName,
		const mdm_Image3D &img,
		const int dataTypeFlag, const int xtrTypeFlag,
		bool sparse = false);

	/**
	 * @brief   Strip Analyze extension from a file name and return in a different string
	 * @param   fileName   string full filename to be stripped
	 * @return  string filename with extension stripped
	 */
	static std::string stripAnalyzeExtension(const std::string & fileName);

	/**
	 * @brief    Test for existence of the file with the specified basename and all Analyze extensions (.img, .hdr)
	 * @param    baseName        string base name of files to test
	 * @param    xtrExistsFlag (output)   reference to bool, stores if .xtr file also exists
	 * @param    warn       bool, triggers warning for program logger if files don't exit
	 * @return   bool true if files exist, false otherwise
	 */
	static bool filesExist(const std::string & baseName, bool &xtrExistsFlag, 
		bool warn = false);

protected:

private:

	//
	static bool writeAnalyzeHdr(const std::string &baseName,
		const struct dsr *const hdr);

	//
	static bool writeAnalyzeImg(const std::string &baseName,
		const mdm_Image3D& img,
		const int typeFlag,
		bool sparse);

	//
	static bool writeNewXtr(std::ofstream *xtrFileStream,
		const mdm_Image3D &img);

	//
	static bool writeOldXtr(std::ofstream *xtrFileStream,
		const mdm_Image3D &img);

	//
	static bool writeAnalyzeXtr(const std::string &baseName,
		const mdm_Image3D &img,
		const int typeFlag);

	//
	static bool readAnalyzeImg(const std::string& imgFileName,
		mdm_Image3D &img,
		const struct dsr *const hdr,
		const bool swapFlag);

	//
	static bool readAnalyzeHdr(const std::string& hdrFileName,
		struct dsr *const hdr);

	//
	static bool readOldXtr(std::ifstream *xtrFileStream,
		mdm_Image3D &img);

	//
	static bool readNewXtr(std::ifstream *xtrFileStream,
		mdm_Image3D &img);

	//
	static bool readAnalyzeXtr(const std::string &xtrFileName,
		mdm_Image3D &img);


	//
	static void  hdrToString(std::string& hdrString, const struct dsr *const hdr);

	//
	static void  setHdrFieldsFromImage3D(struct dsr *const hdr,
		const mdm_Image3D img,
		const int typeFlag,
		bool sparse);
	
	//
	static void  hdrBlankInit(struct dsr *const hdr);

  // Added so we can deal with stuff
	const static int MAX_ANALYZE_DIMS;
	const static int ANALYZE_HDR_SIZE;
	const static int MAX_IMG_DIMS;
};

struct header_key                /*      hk   */
{                                /* off + size*/
	int        sizeof_hdr;         /* 0 + 4     */
	char       data_type[10];      /* 4 + 10    */
	char       db_name[18];        /* 14 + 18   */
	int        extents;            /* 32 + 4    */
	short int  session_error;      /* 36 + 2    */
	char       regular;            /* 38 + 1    */
	char       hkey_un0;           /* 39 + 1    */
};                               /* total=40  */

struct image_dimension           /*      dime      */
{                                /* off + size*/
	short int  dim[8];             /* 0 + 16    */
	char       vox_units[4];       /* 16 + 4    */
	char       cal_units[8];       /* 20 + 4    */
	short int  unused1;            /* 24 + 2    */
	short int  datatype;           /* 30 + 2    */
	short int  bitpix;             /* 32 + 2    */
	short int  dim_un0;            /* 34 + 2    */
	float      pixdim[8];          /* 36 + 32   */
																 /*
																	*  pixdim[] specifies the voxel dimensions:
																	*  pixdim[1] - voxel width
																	*  pixdim[2] - voxel height
																	*  pixdim[3] - interslice distance
																	*  ...etc
																	*/
	float  vox_offset;             /* 68 + 4    */
	float  roi_scale;              /* 72 + 4    */
	float  funused1;               /* 76 + 4    */
	float  funused2;               /* 80 + 4    */
	float  cal_max;                /* 84 + 4    */
	float  cal_min;                /* 88 + 4    */
	int    compressed;             /* 92 + 4    */
	int    verified;               /* 96 + 4    */
	int    glmax, glmin;           /* 100 + 8   */
};                               /* total=108 */

struct data_history              /*      hist     */
{                                /* off + size*/
	char  descrip[80];             /* 0 + 80    */
	char  aux_file[24];            /* 80 + 24   */
	char  orient;                  /* 104 + 1   */
	char  originator[10];          /* 105 + 10  */
	char  generated[10];           /* 115 + 10  */
	char  scannum[10];             /* 125 + 10  */
	char  patient_id[10];          /* 135 + 10  */
	char  exp_date[10];            /* 145 + 10  */
	char  exp_time[10];            /* 155 + 10  */
	char  hist_un0[3];             /* 165 + 3   */
	int   views;                   /* 168 + 4   */
	int   vols_added;              /* 172 + 4   */
	int   start_field;             /* 176 + 4   */
	int   field_skip;              /* 180 + 4   */
	int   omax, omin;              /* 184 + 8   */
	int   smax, smin;              /* 192 + 8   */
};                               /* total=200 */

struct dsr                       /*      dsr              */
{                                /* off + size*/
	struct header_key       hk;    /* 0 + 40    */
	struct image_dimension  dime;  /* 40 + 108  */
	struct data_history     hist;  /* 148 + 200 */
};                               /* total=348 */
#endif /* MDM_ANALYZEFORMAT_H */

/*
 *  Modifications:
 *  14-27 Feb 2006 (GAB)
 *  - Created
 *  29 March 2006 (GAB)
 *  - Fixed DbC commenting and removed static function declarations to c file
 *  ============================    Version 0.2    ============================
 *  23 Aug 2007 (GAB)
 *  - Added qbiWriteAnalyzeHdr() and qbiWriteAnalyzeImg(), i.e. made them non-static for byte-swapper
 *  ============================    Version 0.3    ============================
 *  24 March 2009 (GAB)
 *  - Added new xtr file capability
 *  ============================    Version 0.4    ============================
 *  24 Nov 2009 (GAB)
 *  - Added 4-byt int image writing capability, and no xtr flag for readability
 */
