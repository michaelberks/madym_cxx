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
#include <cmath>
#include <madym/mdm_exception.h>

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
		KeyPair flipAngle; //!< Flip-angle
		KeyPair TR; //!< Repetition time in ms
		KeyPair TE; //!< Echo time in ms
		KeyPair B; //!< Magnitude field B-value
		KeyPair TI; //!< Inversion time in ms
		KeyPair TA; //!< TA
		KeyPair ETL; //!< ETL
		KeyPair Xmm; //!< X0
		KeyPair Ymm; //!< Y0
		KeyPair Zmm; //!< Z0
		KeyPair rowDirCosX; //!< rowDirCosX
		KeyPair rowDirCosY; //!< rowDirCosY
		KeyPair rowDirCosZ; //!< rowDirCosZ
		KeyPair colDirCosX; //!< colDirCosX
		KeyPair colDirCosY; //!< colDirCosY
		KeyPair colDirCosZ; //!< colDirCosZ
		KeyPair noiseSigma; //!< Estimate of noise standard deviation

		static const std::string ImageTypeKey; //!< Key used for image type
		static const std::string TimeStampKey; //!< Key used for timestamp
	};

	//! Enum of defined image types
	/*!
	*/
	enum ImageType {
		TYPE_UNDEFINED, //!< Unspecified type
		TYPE_T1WTSPGR, //!< T1 weighted, spoiled gradient echo image
		TYPE_T1BASELINE, //!< Baseline T1 map
		TYPE_T1DYNAMIC, //!< Dynamic T1 map
		TYPE_M0MAP, //!< M0 map
    TYPE_B1MAP, //!< B1 correction map
		TYPE_CAMAP, //!< Contrast-agent concentration map
		TYPE_DEGR, //!< Variable flip-angle map
		TYPE_T2STARMAP, //!< T2* map
		TYPE_DYNMEAN, //!< Temporal mean of dynamic images
		TYPE_DWI, //!< Diffusion weighted-image
		TYPE_ADCMAP, //!< Apparent diffusion coefficient (ADC) map
		TYPE_ERRORMAP, //!< Error map
		TYPE_AIFVOXELMAP, //!< Mask for selecting AIF
		TYPE_KINETICMAP, //!< Tracer-kinetic model parameter map
    TYPE_ROI //!< Region of interest mask
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

  //! Explicit conversion to bool, defined as a non-empty image
  explicit operator bool() const
  {
    return numVoxels() > 0;
  }

  //! Reset the image to empty
  MDM_API void reset();

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
	MDM_API double voxel(size_t idx) const;

	//! Set value at specified voxel index
	/*!
	\param idx index into data array, using C-style ordering. Must be >=0, < numVoxels()
	\param value to set
	*/
	MDM_API void setVoxel(size_t idx, double value);

  //! Return value at specified voxel subscripts
  /*!
  \param x x-axis subscript. Must be >=0, < nX
  \param y y-axis subscript. Must be >=0, < nY
  \param z z-axis subscript. Must be >=0, < nZ
  \return value at voxel subscript
  */
  MDM_API double voxel(size_t x, size_t y, size_t z) const;

  //! Set value at specified voxel subscripts
  /*!
  \param x x-axis subscript. Must be >=0, < nX
  \param y y-axis subscript. Must be >=0, < nY
  \param z z-axis subscript. Must be >=0, < nZ
  \param value to set
  */
  MDM_API void setVoxel(size_t x, size_t y, size_t z, double value);

  //! Set all the values in a slice
  /*!
  \param z slice to insert
  \param values voxel values to insert, must have size n_x *n_z or exception thrown
  */
  MDM_API void setSlice(size_t z, std::vector<double> values);

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
	MDM_API void setDimensions(const size_t nX, const size_t nY, const size_t nZ);

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
	MDM_API void getDimensions(size_t &nX, size_t &nY, size_t &nZ) const;

	//! Return the number of voxels in the data array
	/*!
	\return  integer number of voxels (0 if dimensions not set)
	*/
	MDM_API size_t numVoxels() const;

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
	MDM_API void setTimeStampFromDoubleStr(const double timeStamp);

	//! Set timestamp from the current system clock time
	/*!
	*/
	MDM_API void setTimeStampFromNow();

	//! Set timestamp given a time in minutes
	/*!
	\param   timeInMins assumed to be duration since 00:00:00
	*/
	MDM_API void setTimeStampFromMins(const double timeInMins);

	//! Set timestamp given a time in seconds
	/*!
	\param   timeInSecs assumed to be duration since 00:00:00
	*/
	MDM_API void setTimeStampFromSecs(const double timeInSecs);

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

	//! Return time in accumlated decimalized minutes since midnight
	/*!
	Converts the xtr HHMMSS.SS string saved as a double into total accumulated minutes, as
	decimalized minutes are the standard dynamic time measure for DCE analysis
	\return accumulated minutes since midnight in time stamp
	*/
	MDM_API double minutesFromTimeStamp() const;

  //! Static function to convert time in cumulative seconds since midnight, to a timestamp hhmmss.msecs
  /*!
  \param secs cumulative time in seconds
  \return timestamp as hhmmss.msecs
  */
  MDM_API static double secsToTimestamp(const double secs);

  //! Static function to convert a timestamp to time in cumulative seconds since midnight
  /*!
  \param timestamp in hhmmss.msecs format
  \return secs cumulative time in seconds
  */
  MDM_API static double timestampToSecs(const double timestamp);

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
	\return   true if dimensions are the same 
	*/
	MDM_API bool dimensionsMatch(const mdm_Image3D &img) const;

  //! Check if voxel size match another image 
  /*!
  \param    img image to compare to
  \return   true if all voxel dimensions are within +/- 0.01mm
  */
  MDM_API bool voxelSizesMatch(const mdm_Image3D &img) const;

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
	MDM_API void nonZeroVoxels(std::vector<size_t> &idx, std::vector<double> &vals) const;

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
	template <class T> MDM_API void toBinaryStream(std::ostream &ofs, bool nonZero) const;

	//! Read data array from binary stream
	/*!
	Templated to allow different data formats for the binary stream.
	\param ifs binary stream containing data
	\param nonZero if true, assumes stream contains indices and values of non-zero voxels. If false, assumes stream contains all voxel values. 
	\param swap flag to swap byte order between little/big-endian
	\see toBinaryStream
	*/
	template <class T> MDM_API void fromBinaryStream(std::istream &ifs,
		bool nonZero, bool swap);
	
	//! Helper function to reverse byte order of big/little endian data
	/*!
	\param data pointer to data to swap
	*/
	template <class T> MDM_API static void swapBytes(T& data);

  //!Helper function to convert subscripts x, y, z to voxel index
  /*!
  \param x subscript co-ordinate in x-axis
  \param y subscript co-ordinate in y-axis
  \param z subscript co-ordinate in z-axis
  \return voxel index
  */
	MDM_API size_t sub2ind(size_t x, size_t y, size_t z) const;

  //!Adding an image
  /*!
  \param rhs voxels of rhs are added elementwise to this. Dimensions must match
  */
  MDM_API mdm_Image3D& operator+=(const mdm_Image3D& rhs);

  //!Adding a scalar
  /*!
  \param d added to each voxel value
  */
  MDM_API mdm_Image3D& operator+=(const double d);

  //!Multiplying by an image
  /*!
  \param rhs voxels in this are scaled elementwise by voxels of rhs. Dimensions must match
  */
  MDM_API mdm_Image3D& operator*=(const mdm_Image3D& rhs);

  //!Multiplying by a scalar
  /*!
  \param d added to each voxel value
  */
  MDM_API mdm_Image3D& operator*=(const double d);

  //!Subtracting an image
  /*!
  \param rhs voxels of rhs are subtracted elementwise from this. Dimensions must match
  */
  MDM_API mdm_Image3D& operator-=(const mdm_Image3D& rhs);

  //!Subtracting an image
  /*!
  \param d subtracted from each voxel value
  */
  MDM_API mdm_Image3D& operator-=(const double d);

  //!Dividing by an image
  /*!
  \param rhs voxels in this are scaled elementwise by voxels of rhs. Dimensions must match
  */
  MDM_API mdm_Image3D& operator/=(const mdm_Image3D& rhs);

  //!Dividing by a scalar
  /*!
  \param d added to each voxel value
  */
  MDM_API mdm_Image3D& operator/=(const double d);

private:
	/*!
	*/
	void initDataArray();


	//!   Set meta data using key-value pair 
	void setMetaData(const std::string &key, const double &value);

	ImageType imgType_;

	double timeStamp_;

	
	size_t nX_; //!< Number of voxels in x-axis
  size_t nY_; //!< Number of voxels in y-axis
  size_t nZ_; //!< Number of voxels in z-axis (ie slices)

	//! Array of voxels
	std::vector<double> data_;

	//! Image meta data
	MetaData info_;

  //! Voxel size check tolerance
  static const double voxelSizeTolerance_;
};

//! Custom exception class for specific case when image dimensions don't match
/*!
This should be thrown whenever there is an attempt to add a new image to an analysis
that doesn't match the dimensions of already loaded images.
*/
class mdm_dimension_mismatch : virtual public mdm_exception {

public:
  //! Constructor from standard string message
  /*!
  \param func name of throwing function
  \param ref reference image that dimensions should match
  \param img new image with mis-matched dimensions
  */
  mdm_dimension_mismatch(const char* func,
    const mdm_Image3D &ref, const mdm_Image3D &img) noexcept
    : mdm_exception(func, "Dimension mismatch: ")
  {
    size_t nxr, nyr, nzr, nxi, nyi, nzi;
    const auto &r = ref.info();
    const auto &i = img.info();

    ref.getDimensions(nxr, nyr, nzr);
    img.getDimensions(nxi, nyi, nzi);
    
    append(boost::format(
      "new image (dimensions %1% x %2% x %3%, voxel sizes %4% x %5% x %6% mm3) does not match \n"
      "reference image (dimensions %7% x %8% x %9%, voxel sizes %10% x %11% x %12% mm3)"
      )
      % nxi % nyi %nzi % i.Xmm.value() % i.Ymm.value() % i.Zmm.value() 
      % nxr % nyr %nzr % r.Xmm.value() % r.Ymm.value() % r.Zmm.value());
   
  }

private:

};

//! Custom exception class for specific case when image dimensions don't match
/*!
This should be thrown whenever there is an attempt to add a new image to an analysis
that doesn't match the voxel sizes of already loaded images.
*/
class mdm_voxelsize_mismatch : virtual public mdm_exception {

public:
  //! Constructor from standard string message
  /*!
  \param func name of throwing function
  \param ref reference image that dimensions should match
  \param img new image with mis-matched dimensions
  */
  mdm_voxelsize_mismatch(const char* func,
    const mdm_Image3D &ref, const mdm_Image3D &img) noexcept
    : mdm_exception(func, "Voxel sizes mismatch: ")
  {
    size_t nxr, nyr, nzr, nxi, nyi, nzi;
    const auto &r = ref.info();
    const auto &i = img.info();

    ref.getDimensions(nxr, nyr, nzr);
    img.getDimensions(nxi, nyi, nzi);

    append(boost::format(
      "new image (dimensions %1% x %2% x %3%, voxel sizes %4% x %5% x %6% mm3) does not match \n"
      "reference image (dimensions %7% x %8% x %9%, voxel sizes %10% x %11% x %12% mm3)"
    )
      % nxi % nyi %nzi % i.Xmm.value() % i.Ymm.value() % i.Zmm.value()
      % nxr % nyr %nzr % r.Xmm.value() % r.Ymm.value() % r.Zmm.value());

  }

private:

};
#endif /* MDM_IMAGE3D_H */