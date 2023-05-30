/*!
 *  @file    mdm_ImageDatatypes.h
 *  @brief   Defines allowed datatype for Analyze/NIFTI images
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_IMAGEDATATYPES_H
#define MDM_IMAGEDATATYPES_H

#include <madym/utils/mdm_api.h>
#include <madym/utils/mdm_Image3D.h>
#include <madym/image_io/meta/mdm_XtrFormat.h>

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


  MDM_API static std::string toString(DataType type)
  {
    switch (type)
    {
    case DT_UNKNOWN: return "DT_UNKNOWN (0)";
    case DT_BINARY: return "DT_BINARY (1)";
    case DT_UNSIGNED_CHAR: return "DT_UNSIGNED_CHAR (2)";
    case DT_SIGNED_SHORT: return "DT_SIGNED_SHORT (4)";
    case DT_SIGNED_INT: return "DT_SIGNED_INT (8)";
    case DT_FLOAT: return "DT_FLOAT (16)";
    case DT_COMPLEX: return "DT_COMPLEX (32)";
    case DT_DOUBLE: return "DT_DOUBLE (64)";
    case DT_RGB: return "DT_RGB24 (128)";
    case DT_ALL: return "DT_ALL (255)";
    case DT_INT8: return "DT_INT8 (256)";
    case DT_UINT16: return "DT_UINT16 (512)";
    case DT_UINT32: return "DT_UINT32 (768)";
    case DT_INT64: return "DT_INT64 (1024)";
    case DT_UINT64: return "DT_UINT64 (1280)";
    case DT_FLOAT128: return "DT_FLOAT128 (1536)";
    case DT_COMPLEX128: return "DT_COMPLEX128 (1792)";
    case DT_COMPLEX256: return "DT_COMPLEX256 (2048)";
    case DT_RGBA32: return "DT_RGBA32 (2304)";
    default:
      throw mdm_exception(__func__, "Unknown format option " + type);
    }
  }

  MDM_API static const std::vector<std::string> validTypes()
  {
    return {
      toString(DT_BINARY),
      toString(DT_UNSIGNED_CHAR),
      toString(DT_SIGNED_SHORT),
      toString(DT_SIGNED_INT),
      toString(DT_FLOAT),
      toString(DT_DOUBLE)
    };
  }

  //
  MDM_API static DataType typeFromString(const std::string& type)
  {
    if (type == toString(DT_UNKNOWN))
      return DT_UNKNOWN;
    if (type == toString(DT_BINARY))
      return DT_BINARY;
    else if (type == toString(DT_UNSIGNED_CHAR))
      return DT_UNSIGNED_CHAR;
    else if (type == toString(DT_SIGNED_SHORT))
      return DT_SIGNED_SHORT;
    else if (type == toString(DT_SIGNED_INT))
      return DT_SIGNED_INT;
    if (type == toString(DT_FLOAT))
      return DT_FLOAT;
    if (type == toString(DT_COMPLEX))
      return DT_COMPLEX;
    else if (type == toString(DT_DOUBLE))
      return DT_DOUBLE;
    else if (type == toString(DT_RGB))
      return DT_RGB;
    else if (type == toString(DT_ALL))
      return DT_ALL;
    if (type == toString(DT_INT8))
      return DT_INT8;
    if (type == toString(DT_UINT16))
      return DT_UINT16;
    else if (type == toString(DT_UINT32))
      return DT_UINT32;
    else if (type == toString(DT_INT64))
      return DT_INT64;
    else if (type == toString(DT_UINT64))
      return DT_UINT64;
    if (type == toString(DT_FLOAT128))
      return DT_FLOAT128;
    if (type == toString(DT_COMPLEX128))
      return DT_COMPLEX128;
    else if (type == toString(DT_COMPLEX256))
      return DT_COMPLEX256;
    else if (type == toString(DT_RGBA32))
      return DT_RGBA32;
    else
      throw mdm_exception(__func__, "Unknown data type option " + type);
  }
};


#endif /* MDM_IMAGEDATATYPES_H */
