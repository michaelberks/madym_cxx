/*!
 *  @file    mdm_Image3D.h
 *  @brief   Class for storing 3D image data and associated meta-information 
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_IMAGE3D_H
#define MDM_IMAGE3D_H

#include "mdm_api.h"

#include <vector>
#include <string>
#include <fstream>
 /*!
	*  @brief   Storing 3D image data and associated meta-information
	*/
class mdm_Image3D
{
	public:

		//! mdm_image3D nested helper class to store image meta data
		class MetaData{

		public:
			//! Default constructor
			MetaData();

			//! mdm_image3D nested helper class to store image key(string)/value(double) pairs of meta data
			class KeyPair {
			public:
				//! Default constructor
				/*!
				\param key name of meta data field
				*/
				KeyPair(const std::string &key)
					:
					key_(key),
					value_(NAN)
				{};

				//! Return meta data key name
				/*!
				\return key name
				*/
				const std::string &key() const
				{
					return key_;
				};

				//! Return meta data value
				/*!
				\return meta data value
				*/
				double value() const
				{
					return value_;
				};

				//! Set meta data value
				/*!
				\param value
				*/
				void setValue(double value)
				{
					value_ = value;
				}

				//! Check if meta data value is set
				/*!
				\return true if value is set (not NaN)
				*/
				bool isSet() const
				{
					return !std::isnan(value_);
				}
				
			private:
				std::string key_;
				double value_;
			};
			KeyPair flipAngle; ///> Flip-angle
			KeyPair TR; ///> Repetition time in ms
			KeyPair TE; ///> Echo time in ms
			KeyPair B; ///> Magnitude field B-value
			KeyPair TI; ///> T1 in ms
			KeyPair TA; ///> TA
			KeyPair ETL; ///> ETL
			KeyPair Xmm; ///> X0
			KeyPair Ymm; ///> Y0
			KeyPair Zmm; ///> Z0
			KeyPair rowDirCosX; ///> rowDirCosX
			KeyPair rowDirCosY; ///> rowDirCosY
			KeyPair rowDirCosZ; ///> rowDirCosZ
			KeyPair colDirCosX; ///> colDirCosX
			KeyPair colDirCosY; ///> colDirCosY
			KeyPair colDirCosZ; ///> colDirCosZ
			KeyPair noiseSigma; ///> Estimate of noise standard deviation

			static const std::string ImageTypeKey;
			static const std::string TimeStampKey;
		};

		//! Enum of defined image types
		/*!
		*/
		enum ImageType {
			TYPE_UNDEFINED, ///> Unspecified type
			TYPE_T1WTSPGR, ///> T1 weighted, spoiled gradient echo image
			TYPE_T1BASELINE, ///> Baseline T1 map
			TYPE_T1DYNAMIC, ///> Dynamic T1 map
			TYPE_M0MAP, ///> M0 map
			TYPE_CAMAP, ///> Contrast-agent concentration map
			TYPE_DEGR, ///> Variable flip-angle map
			TYPE_T2STARMAP, ///> T2* map
			TYPE_DYNMEAN, ///> Temporal mean of dynamic images
			TYPE_DWI, ///> Diffusion weighted-image
			TYPE_ADCMAP, ///> Apparent diffusion coefficient (ADC) map
			TYPE_ERRORMAP, ///> Error map
			TYPE_AIFVOXELMAP, ///> Mask for selecting AIF
			TYPE_KINETICMAP ///> Tracer-kinetic model parameter map
		};
	
		
	//!    Default constructor
	/*!
	Creates image with TYPE_UNDEFINED and empty data-array
	*/
	MDM_API	mdm_Image3D();

	//!    Default destructor
	/*!
	*/
	MDM_API ~mdm_Image3D();

	//! Read only access to the image data array
	/*!
	\return const reference to data array
	*/
	MDM_API const std::vector<double>& data() const;

	//! Return value at specified voxel index
	/*!
	\param idx index into data array, using C-style ordering. Must be >=0, < numVoxels()
	\return value at voxel index
	*/
	MDM_API double voxel(int idx) const;

	//! Set value at specified voxel index
	/*!
	\param idx index into data array, using C-style ordering. Must be >=0, < numVoxels()
	\param value to set
	*/
	MDM_API void setVoxel(int idx, double value);

	//! Set image type
	/*!
	\param type
	\see ImageType
	*/
	MDM_API void setType(ImageType type);

	//! Return image type
	/*!
	\return type
	\see ImageType
	*/
	MDM_API ImageType type() const;

	//! Set image dimensions
	/*!
	\param   nX  number of voxels in x-axis
	\param   nY  number of voxels in y-axis
	\param   nZ  number of voxels in z-axis (image slices)
	*/
	MDM_API void setDimensions(const int nX, const int nY, const int nZ);

	//! Set image dimensions from existing image
	/*!
	Also sets voxel dimensions
	\param img   image from which to set dimensions
	*/
	MDM_API void setDimensions(const mdm_Image3D &img);

	//! Get image dimensions
	/*!
	\param   nX reference to hold dimension of x-axis
	\param   nY reference to hold dimension of y-axis
	\param   nZ reference to hold dimension of z-axis (image slices)
	*/
	MDM_API void getDimensions(int &nX, int &nY, int &nZ) const;

	//! Return the number of voxels in the data array
	/*!
	\return  integer number of voxels (0 if dimensions not set)
	*/
	MDM_API int numVoxels() const;

	//! Set the voxel dimensions in mm
	/*!
	\param   xmm x-dimension of voxels in mm
	\param   ymm y-dimension of voxels in mm
	\param   zmm z-dimension of voxels (slice thickness) in mm
	*/
	MDM_API void setVoxelDims(const double xmm, const double ymm, const double zmm);

	//! Set timestamp
	/*!
	\param   timeStamp - double representing a valid time stamp
	*/
	MDM_API void setTimeStamp(const double timeStamp);

	//! Return timestamp
	/*!
	For simplicity, store time stamp in the format of Geoff's *.xtr files
	  i.e. as a single double with the format below:
	  -   HHMMSS.SS
	  where HH is the 2-digit hours, MM the 2-digit minutes and SS.SS the seconds
	  of the current time of the day
	\return image timestamp
	*/
	MDM_API double timeStamp() const;

	//! Return image meta data
	/*!
	\return reference to images meta data
	*/
	MDM_API MetaData& info();

	//! Return image meta data
	/*!
	\return const reference to images meta data
	*/
	MDM_API const MetaData& info() const;

	//!   Get list of keys and values that have been set
	/*!
	\param   keys reference to string vector that will be filled with set keys
	\param   values reference to double vector that will be filled with set values
	*/
	MDM_API void getSetKeyValuePairs(std::vector<std::string> &keys,
		std::vector<double> &values) const;

	//! Check if dimensions match another image 
	/*!
	\param    img image to compare to
	\return   true if dimensions are the same and all voxel dimensions are within +/- 0.01mm 
	*/
	MDM_API bool dimensionsMatch(const mdm_Image3D &img);

	//!   Copy meta data (except type and timestamp) and data dimensions from existing image
	/*!
	 Notes:
	 -   We don't copy the type as we're usually copying to a new image type
	 -   We don't copy the time stamp as it doesn't make sense
	 \param   imgToCopy image to copy
	*/
	MDM_API void copy(const mdm_Image3D &imgToCopy);

	//!   Create a string description of a mdm_Image3D struct and store it in imgString
	/*!
	\param   imgString reference to the string array to hold the description
	*/
	MDM_API void toString(std::string &imgString) const;

	//!   Write meta data to output file stream
	/*!
	\param   ofs output filestream
	*/
	MDM_API void metaDataToStream(std::ostream &ofs) const;

	//!   Set meta data from input file stream
	/*!
	\param   ifs input filestream
	*/
	MDM_API void setMetaDataFromStream(std::istream &ifs);

	//!   Legacy function to set meta data from old xtr file format
	/*!
	\param   ifs input filestream
	*/
	MDM_API void setMetaDataFromStreamOld(std::istream &ifs);

	//!Return indices and values of voxels with non-zero value
	/*!
	\param idx reference to hold list of voxels indices with non-zero values
	\param vals reference to hold list of non-zero values
	*/
	MDM_API void nonZeroVoxels(std::vector<int> &idx, std::vector<double> &vals) const;

	//! Write the data array into binary stream   
	/*!
	Templated to allow different data formats for the binary stream. Can be run in sparse mode (set nonZero true) 
	in which case only the indices and values of non-zero voxels are written. This can be significantly smaller 
	than writing all voxel values if the image holds output data for an ROI that is a fraction of the total 
	image volume. If nonZero mode is used, indices are written as ints, values as whatever the template 
	data type specifies.
	\param ofs binary stream to write to
	\param nonZero if true, write indices and values of non-zero voxels. If false, all voxel values written.
	\return  true if write successful
	*/
	template <class T> MDM_API bool toBinaryStream(std::ostream &ofs, bool nonZero) const;

	//! Read data array from binary stream
	/*!
	Templated to allow different data formats for the binary stream.
	\param ifs binary stream containing data
	\param nonZero if true, assumes stream contains indices and values of non-zero voxels. If false, assumes stream contains all voxel values. 
	\param swap flag to swap byte order between little/big-endian
	\see toBinaryStream
	*/
	template <class T> MDM_API bool fromBinaryStream(std::istream &ifs,
		bool nonZero, bool swap);
	
	//! Helper function to reverse byte order of big/little endian data
	/*!
	\param data pointer to data to swap
	\nBytes size of buffer to swap
	*/
	template <class T> MDM_API static void swapBytes(T& data);

	

private:
	/*!
	*/
	bool initDataArray();

	//!   Set meta data using key-value pair 
	void setMetaData(const std::string &key, const double &value);

	ImageType imgType_;

	double timeStamp_;

	
	int nX_; //!< Number of voxels in x-axis
	int nY_; //!< Number of voxels in y-axis
	int nZ_; //!< Number of voxels in z-axis (ie slices)

	//! Array of voxels
	std::vector<double> data_;

	//! Image meta data
	MetaData info_;
};
#endif /* MDM_IMAGE3D_H */