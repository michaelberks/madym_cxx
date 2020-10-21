/**
 *  @file    mdm_Image3D.h
 *  @brief   Class for storing 3D image data and associated meta-information 
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_IMAGE3D_H
#define MDM_IMAGE3D_H

#include "mdm_api.h"

#include <vector>
#include <string>
#include <fstream>
#include <cmath>
 /**
	*  @brief   Storing 3D image data and associated meta-information
	*  @details More info...
	*/
class mdm_Image3D
{
	public:

		class Info{

		public:
			Info();

			class key_pair {
			public:
				key_pair(const std::string &key)
					:
					key_(key),
					value_(NAN)
				{};

				const std::string &key() const
				{
					return key_;
				};
				double value() const
				{
					return value_;
				};
				void setValue(double v)
				{
					value_ = v;
				}
				bool isSet() const
				{
					return !std::isnan(value_);
				}
				
			private:
				std::string key_;
				double value_;
			};

			key_pair TimeStamp;
			key_pair flipAngle;
			key_pair TR;
			key_pair TE;
			key_pair B;
			key_pair TI;
			key_pair TA;
			key_pair ETL;
			key_pair X0;
			key_pair Y0;
			key_pair Z0;
			key_pair rowDirCosX;
			key_pair rowDirCosY;
			key_pair rowDirCosZ;
			key_pair colDirCosX;
			key_pair colDirCosY;
			key_pair colDirCosZ;
			key_pair noiseSigma;
		};

		//I'm not massively keen on this design... but make sure
		//we let this auto number, so INFO_NTYPES is the last item
		//we can use as a counter
		enum imageType {
			TYPE_UNDEFINED,
			TYPE_T1WTSPGR,
			TYPE_T1BASELINE,
			TYPE_T1DYNAMIC,
			TYPE_M0MAP,
			TYPE_CAMAP,
			TYPE_DEGR,
			TYPE_T2STARMAP,
			TYPE_DYNMEAN,
			TYPE_DWI,
			TYPE_ADCMAP,
			TYPE_ERRORMAP,
			TYPE_AIFVOXELMAP,
			TYPE_KINETICMAP,
			INFO_NTYPES
		};
	/* Function prototypes */
	/* CONSTRUCTOR & DESTRUCTOR */
	/**
	 * @brief    Create a new mdm_Image3D struct, initialise it with NULL member data and return its pointer
	 * @return   Pointer to the new, blanked, mdm_Image3D struct (NULL indicates failure)
	 * DbC stuff (deal with return values in @return above) ...
	 * @pre    none
	 * @post   a mdm_Image3D struct has been created on the heap and initialised with blank member data
	 */
		MDM_API	mdm_Image3D();

	/**
	 * @brief    Delete an existing mdm_Image3D struct, free its memory and return a NULL pointer
	 * @param    img - Pointer to the mdm_Image3D struct to be deleted
	 * @return   NULL pointer to be assigned to img
	 * DbC stuff ...
	 * @pre    img is a valid mdm_Image3D struct pointer
	 * @post   img data array memory will be freed and its own storage will be freed
	 * @post   return == NULL
	 *
	 * Usage:
	 * -   img = deleteQbiImage3D(img);
	 * -   Not nice but I found I couldn't set the passed img to NULL within the function
	 */
		MDM_API ~mdm_Image3D();

	/* FIELD ACCESSORS AND MODIFIERS */
	/**
	 * @brief   Set the image type of a mdm_Image3D struct
	 * @param   img     - pointer to a valid mdm_Image3D struct
	 * @param   newType - integer representing an image type
	 * DbC stuff ...
	 * @pre    img is a valid mdm_Image3D struct pointer
	 * @pre    newType is an integer image type
	 * @post   mdm_Image3DgetType(img) == newType, if newType is a valid image type (defined in mdm_Image.h)
	 * @post   mdm_Image3DgetType(img) undefined, if newType is not a valid image type
	*/
		MDM_API const std::vector<double>& getData() const;

	/* Function to access the actual image date
	 * When I understand more about the way this all works, reconsider the best way
	 * to do this, as not hugely happy returning a pointer to internal data member */
		MDM_API double getVoxel(int i) const;

			/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setVoxel(int i, double value);

			/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setType(const int newType);

	/**
	 * @brief   Return the image type of a mdm_Image3D struct
	 * @param   img - pointer to a valid mdm_Image3D struct
	 * @return  Integer representing the image type of img (see mdm_Image.h)
	 * DbC stuff ...
	 * @pre    img is a valid mdm_Image3D struct pointer
	 * @post   return == mdm_Image3DgetType(img)
	*/
		MDM_API int getType() const;

	/**
	 * @brief   Set the voxel matrix dimensions of a mdm_Image3D struct
	 * @param   img - pointer to a valid mdm_Image3D struct
	 * @param   nX  - integer number of x-voxels
	 * @param   nY  - integer number of y-voxels
	 * @param   nZ  - integer number of z-voxels (image slices)
	 * DbC stuff ...
	 * @pre    img is a pointer to a valid mdm_Image3D struct
	 * @pre    nX, nY and nZ are valid integer numbers of voxels for the image matrix
	 * @post   mdm_Image3DgetMatrixDims(img) == {nX nY nZ}
	 */
		MDM_API void setMatrixDims(const int nX, const int nY, const int nZ);

	/**
	 * @brief   Return the voxel matrix dimensions of a mdm_Image3D struct
	 * @param   img - pointer to a valid mdm_Image3D struct
	 * @param   nX  - pointer to hold integer number of x-voxels
	 * @param   nY  - pointer to hold integer number of y-voxels
	 * @param   nZ  - pointer to hold integer number of z-voxels (image slices)
	 * DbC stuff ...
	 * @pre    img is a pointer to a valid mdm_Image3D struct
	 * @pre    nX, nY and nZ are valid integer pointers
	 * @post   {nX nY nZ} == mdm_Image3DgetMatrixDims(img)
	 */
		MDM_API void getMatrixDims(int &nX, int &nY, int &nZ) const;

	/**
	 * @brief   Return the number of voxels in the image matrix of a mdm_Image3D struct
	 * @param   img - pointer to a valid mdm_Image3D struct
	 * @return  integer number of voxels (0 if dimensions not set)
	 * DbC stuff ...
	 * @pre    img is a pointer to a valid mdm_Image3D struct
	 * @pre    voxel matrix dimensions of img have been set to valid values
	 */
		MDM_API int getNvoxels() const;

	/**
	 * @brief   Set the voxel dimensions fields of a mdm_Image3D struct
	 * @param   img - pointer to a valid mdm_Image3D struct
	 * @param   xmm - double x-dimension of voxels in mm
	 * @param   ymm - double y-dimension of voxels in mm
	 * @param   zmm - double z-dimension of voxels (slice thickness) in mm
	 * DbC stuff ...
	 * @pre    img is a pointer to a valid mdm_Image3D struct
	 * @pre    xmm, ymm and zmm are valid double voxel-dimensions for the image matrix
	 * @post   mdm_Image3DgetVoxelDims(img) == {xmm ymm zmm}
	 */
		MDM_API void setVoxelDims(const double xmm, const double ymm, const double zmm);

	/**
	 * @brief   Return the voxel dimensions fields of a mdm_Image3D struct
	 * @param   img - pointer to a valid mdm_Image3D struct
	 * @param   xmm - pointer to hold double x-dimension of voxels in mm
	 * @param   ymm - pointer to hold double y-dimension of voxels in mm
	 * @param   zmm - pointer to hold double z-dimension of voxels (slice thickness) in mm
	 * DbC stuff ...
	 * @pre    img is a pointer to a valid mdm_Image3D struct
	 * @pre    xmm, ymm and zmm are valid double pointers
	 * @post   {xmm ymm zmm} == mdm_Image3DgetVoxelDims(img)
	 */
		MDM_API void getVoxelDims(double &xmm, double &ymm, double &zmm) const;

	/**
	 * @brief   Set the time stamp of a mdm_Image3D struct
	 * @param   img       - pointer to a mdm_Image3D struct
	 * @param   timeStamp - double representing a valid time stamp
	 * DbC stuff ...
	 * @pre    img is a pointer to a valid mdm_Image3D struct
	 * @pre    timeStamp is a valid double time stamp
	 * @post   mdm_Image3DgetTimeStamp(img) == timeStamp
	 */
		MDM_API void setTimeStamp(const double timeStamp);

	/**
	 * @brief   Return the time stamp of a mdm_Image3D struct
	 * @param   img  - pointer to a mdm_Image3D struct
	 * @return  double time stamp
	 * DbC stuff ...
	 * @pre    img is a pointer to a valid mdm_Image3D struct
	 */
		MDM_API double getTimeStamp() const;

	/**
	 * @brief   Set info values in a mdm_Image3D struct based on key-value pair arrays
	 * @param   keys   - string vector keys (matching values)
	 * @param   values - double vector of values (matching keys)
	 */
		MDM_API void decodeKeyValuePairs(const std::vector<std::string> &keys,
																 const std::vector<double> &values);

		/**
		 * @brief   Get list of keys and values that have been set
		 * @param   keys   - reference to string vector that will be filled with set keys
		 * @param   values - reference to double vector that will be filled with set values
		 */
		MDM_API void getSetKeyValuePairs(std::vector<std::string> &keys,
			std::vector<double> &values) const;

	
			/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool initDataArray();

	/**
	 * @brief   Y-flip 2D frame of image data in a mdm_Image3D struct
	 * @param   img      pointer to mdm_Image3D struct holding the data to be flipped
	 * @param   sliceNo  integer number of slice to be flipped
	 * DbC stuff ...
	 * @pre    img is a valid pointer to a mdm_Image3D struct
	 * @pre    img holds valid voxel data that can be flipped
	 * @pre    sliceNo is a valid slice in the data held in img
	 * @post   img data at sliceNo have been flipped in Y
	 */
		MDM_API void flipSlice(const int sliceNo);

	/**
	 * @brief    Cross-check the voxel and matrix dimensions of 2 mdm_Image3Ds
	 * @param    img1 - pointer to one mdm_Image3D struct for comparison
	 * @param    img2 - pointer to the second mdm_Image3D struct for comparison
	 * @return   Integer 1 if voxel matrices and dimensions match, otherwise 0
	 * DbC stuff (deal with return values in @return above) ...
	 * @pre    img1 and img 2 are pointers to valid, initialised mdm_Image3D structs
	 */
		MDM_API bool voxelMatsMatch(const mdm_Image3D &img2);

	/**
	 * @brief   Copy the data fields (except type) of imgToCopy to img and allocate a new data array
	 * @param   img       - pointer to the mdm_Image3D struct to be "copy-initialised"
	 * @param   imgToCopy - pointer to the mdm_Image3D struct to be "copied"
	 * DbC stuff ...
	 * @pre    img is a pointer to a valid mdm_Image3D struct
	 * @pre    imgToCopy is a pointer to a valid, initialised mdm_Image3D struct
	 * @post   the img data fields have values copied from imgToCopy
	 * @post   the img data data array has been allocated on the heap and cleared
	 * @post   the type and time stamp data are not copied so must be set if required
	 *
	 * Notes:
	 * -   We don't copy the type as we're usually copying to a new image type
	 * -   We don't copy the time stamp as it doesn't make sense
	 *     This means it remains set at -1.0, as long as img was properly created
	 *     using newQbiImage3D().
	 */
		MDM_API void copyFields(const mdm_Image3D &imgToCopy);

	/**
	 * @brief   Copy the data matrix and voxel dimensions and allocate a new data array
	 * @param   img       - pointer to the mdm_Image3D struct to be "copy-initialised"
	 * @param   imgToCopy - pointer to the mdm_Image3D struct to be "copied"
	 * DbC stuff ...
	 * @pre    img is a pointer to a valid mdm_Image3D struct
	 * @pre    imgToCopy is a pointer to a valid, initialised mdm_Image3D struct
	 * @post   the img data matrix and mattrix dimension fields have values copied from imgToCopy
	 * @post   the img data data array has been allocated on the heap and cleared
	 *
	 * Notes:
	 * -   We don't copy the type as we're usually copying to a new image type
	 */
		MDM_API void copyMatrix(const mdm_Image3D &imgToCopy);

	/**
	 * @brief   Create a string description of a mdm_Image3D struct and store it in imgString
	 * @param   imgString - pointer to the string array to hold the description
	 * @param   img       - pointer to the mdm_Image3D struct to be "described"
	 * DbC stuff ...
	 * @pre    img is a pointer to a valid mdm_Image3D struct
	 * @pre    imgString is a valid char pointer
	 * @post   imgString contains a text description of img
	 */
		MDM_API void toString(std::string &imgString) const;

		/*
		*
		*
		*/
		MDM_API void nonZero(std::vector<int> &idx, std::vector<double> &vals) const;

		/*
		* @brief    
		* @return  
		*/
		template <class T> MDM_API bool toBinaryStream(std::ostream &ofs, bool nonZero) const;

		/*
		* @brief
		* @return 
		*
		* Note:   Don't forget to free the new int array ...
		*/
		template <class T> MDM_API bool fromBinaryStream(std::istream &ifs,
			bool nonZero, bool swapBytes);
	

		/*Struct of info - it's nothing but a list of doubles we allowed public write 
		//access to anyway, so may as well just make this pubic*/
		Info info_;

private:
			int imgType_;   /* Allowed values listed above as QBITYPE_* */

			int nX_;
			int nY_;
			int nZ_;

			double xmm_;     /* From Analyze header so not really necessary ... */
			double ymm_;
			double zmm_;

			/**
			*  For simplicity, store time stamp in the format of Geoff's *.xtr files
			*  i.e. as a single double with the format below:
			*  -   HHMMSS.SS
			*  where HH is the 2-digit hours, MM the 2-digit minutes and SS.SS the seconds
			*  of the current time of the day
			* timStamp is now stored in info
			*/  
			std::vector<double> data_;
};
#endif /* MDM_IMAGE3D_H */