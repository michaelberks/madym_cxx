/*!
 *  @file    mdm_ImageDatatypes.h
 *  @brief   Defines allowed datatype for Analyze/NIFTI images
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_IMAGEDATATYPES_H
#define MDM_IMAGEDATATYPES_H

#include "mdm_api.h"
#include <madym/mdm_Image3D.h>
#include <madym/image_io/xtr/mdm_XtrFormat.h>

 //! Analyze image format reading and writing
	/*!
	*/
class mdm_ImageDatatypes {

public:
	//!    Enum of recognized Analyze data formats
	/*!
	* Only DT_UNSIGNED_CHAR, DT_SIGNED_SHORT, DT_SIGNED_INT, DT_FLOAT and DT_DOUBLE supported.
  */
	enum DataType {
    DT_NONE = 0, ///< No data supplied, not expected to be used
    DT_UNKNOWN = DT_NONE, ///< Data-type not recognised, not expected to be used
    DT_BINARY = 1, ///< 8-bit data, cast to binary true/false
    DT_UNSIGNED_CHAR = 2, ///< 8-bit data, integers [0,255] 
    DT_SIGNED_SHORT = 4, ///< 16-bit data, integers [-32,768,32,767]
    DT_SIGNED_INT = 8, ///< 32-bit data, integers [-2,147,483,648, 2,147,483,647]
    DT_FLOAT = 16, ///< 32-bit data, floating point numbers
    DT_COMPLEX = 32, ///< Not supported
    DT_DOUBLE = 64, ///< 64-bit data, floating point numbers
    DT_RGB = 128, ///< Not supported in mdm_AnalyzeFormat
    DT_ALL = 255, ///< Not supported
                                /*----- another set of names for the same ---*/
    DT_UINT8 = 2, ///< 8-bit data, integers [0,255] 
    DT_INT16 = 4, ///< 16-bit data, integers [-32,768,32,767]
    DT_INT32 = 8, ///< 32-bit data, integers [-2,147,483,648, 2,147,483,647]
    DT_FLOAT32 = 16, ///< 32-bit data, floating point numbers
    DT_COMPLEX64 = 32, ///< Not supported
    DT_FLOAT64 = 64, ///< 64-bit data, floating point numbers
    DT_RGB24 = 128, ///< Not supported in mdm_AnalyzeFormat
                              /*------------------- new codes for NIFTI ---*/
    DT_INT8 = 256, ///< signed char (8 bits)
    DT_UINT16 = 512, ///< unsigned short (16 bits)
    DT_UINT32 = 768, ///< unsigned int (32 bits)
    DT_INT64 = 1024, ///< long long (64 bits)
    DT_UINT64 = 1280, ///< unsigned long long (64 bits)
    DT_FLOAT128 = 1536 , ///< long double (128 bits)
    DT_COMPLEX128 = 1792, ///< double pair (128 bits)
    DT_COMPLEX256 = 2048,  ///< long double pair (256 bits) 
    DT_RGBA32 = 2304,  ///< 4 byte RGBA (32 bits/voxel)  
	};
};


#endif /* MDM_ANALYZEFORMAT_H */
