/**
 *  @file    mdm_Image.cxx
 *  @brief   Functions for use with the QBI internal image format
 *
 *  Original author GA Buonaccorsi 9-24 February 2006
 *  (c) Copyright ISBE, University of Manchester 2006
*
 * @author   Mike Berks
 * @version  2.0 (26 Feb 2012)
 *
 *
 *  Documentation comments are in *.h, except for static methods
 *  Version info and list of modifications in comment at end of file
 */

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif
#include "mdm_api.h"

#include "mdm_Image3D.h"

#include <string>
#include <cassert>
#include <sstream>
#include <iostream>

/*
 * Create a new mdm_Image3D object
 * and initialise its fields with default member data.
 *
 * Notes: 1.  The voxel data pointer is set to NULL as it needs matrix dimensions
 *        2.  The image types are #defined in the *.h file, as they may be required by users
 *        3.  The negative double values allow simple tests for "not set", as testing floats
 *            for equality to 0.0 is dangerous
 */

mdm_Image3D::Info::Info()
	: /* Defaulting to NaN llows test isnan() for unset */
	TimeStamp("TimeStamp"),
	flipAngle("FlipAngle"),
	TR("TR"),
	TE("TE"),
	B("B"),
	TI("TI"),
	TA("TA"),
	ETL("ETL"),
	X0("X0"),
	Y0("Y0"),
	Z0("Z0"),
	rowDirCosX("RowDirCosX"),
	rowDirCosY("RowDirCosY"),
	rowDirCosZ("RowDirCosZ"),
	colDirCosX("ColDirCosX"),
	colDirCosY("ColDirCosY"),
	colDirCosZ("ColDirCosZ"),
	noiseSigma("NoiseSigma")
{}

MDM_API mdm_Image3D::mdm_Image3D()
	:
	imgType_(TYPE_UNDEFINED),
	nX_(0),
	nY_(0),
	nZ_(0),
	xmm_(-1.0),
	ymm_(-1.0),
	zmm_(-1.0),
	info_(),
	data_(0)

{   
}

//Destructor
MDM_API mdm_Image3D::~mdm_Image3D()
{

}

//Return const ref to data
MDM_API const std::vector<double>& mdm_Image3D::getData() const
{
	return data_;
}

//
MDM_API double mdm_Image3D::getVoxel(int i) const
{
	//Should we check the input is in range - possibly, although adds overhead?
	//Maybe have a separate safe method?
	assert(i >= 0 && i < getNvoxels());
	return data_[i];
}

//
MDM_API void mdm_Image3D::setVoxel(int i, double value)
{
	//Should we check the input is in range - possibly, although adds overhead?
	//Maybe have a separate safe method?
	assert(i >= 0 && i < getNvoxels());
	data_[i] = value;
}

//
MDM_API void mdm_Image3D::setType(const int newType)
{
  if ((newType >= 0) && (newType < INFO_NTYPES))
    imgType_ = newType;
  else
  {
    /* LOG WARNING MESSAGE */
    imgType_ = TYPE_UNDEFINED;
  }
}

//
MDM_API int mdm_Image3D::getType() const
{
  return imgType_;
}

//
MDM_API void mdm_Image3D::setMatrixDims(const int nX, const int nY, const int nZ)
{
  assert((nX >= 0) && (nY >= 0) && (nZ >= 0));

  nX_ = nX;
  nY_ = nY;
  nZ_ = nZ;

	//To me it's just a bad API to be able to set the dimension without resizing the data
	//array - leaves potentials for inconsistency, I'm not going to allow. So if
	// we set the dimensions, lets resize the data array
	initDataArray();
}

//
MDM_API void mdm_Image3D::getMatrixDims(int &nX, int &nY, int &nZ) const
{
  nX = nX_;
  nY = nY_;
  nZ = nZ_;
}

//
MDM_API int mdm_Image3D::getNvoxels() const
{
  return (int) (nX_ * nY_ * nZ_);
}

//
MDM_API void mdm_Image3D::setVoxelDims(const double xmm, const double ymm, const double zmm)
{
  assert((xmm >= 0.0) && (ymm >= 0.0) && (zmm >= 0.0));

  xmm_ = xmm;
  ymm_ = ymm;
  zmm_ = zmm;
}

//
MDM_API void mdm_Image3D::getVoxelDims(double &xmm, double &ymm, double &zmm) const
{
  xmm = xmm_;
  ymm = ymm_;
  zmm = zmm_;
}

//
MDM_API void mdm_Image3D::setTimeStamp(const double timeStamp)
{
  info_.TimeStamp.setValue(timeStamp);
}

//
MDM_API double mdm_Image3D::getTimeStamp() const
{
  return info_.TimeStamp.value();
}

//
MDM_API void mdm_Image3D::decodeKeyValuePairs(const std::vector<std::string> &keys,
	const std::vector<double> &values)
{
	int nKeys = keys.size();
  assert(nKeys == values.size());

	for (int i = 0; i < nKeys; i++)
	{
		if (keys[i].compare(info_.TimeStamp.key()) == 0)
			setTimeStamp( values[i]);
		else if (keys[i].compare(info_.flipAngle.key()) == 0)
			info_.flipAngle.setValue(values[i]);
		else if (keys[i].compare(info_.TR.key()) == 0)
			info_.TR.setValue(values[i]);
		else if (keys[i].compare(info_.TE.key()) == 0)
			info_.TE.setValue(values[i]);
		else if (keys[i].compare(info_.B.key()) == 0)
			info_.B.setValue(values[i]);
		else if (keys[i].compare(info_.TI.key()) == 0)
			info_.TI.setValue(values[i]);
		else if (keys[i].compare(info_.TA.key()) == 0)
			info_.TA.setValue(values[i]);
		else if (keys[i].compare(info_.ETL.key()) == 0)
			info_.ETL.setValue(values[i]);
		else if (keys[i].compare(info_.X0.key()) == 0)
			info_.X0.setValue(values[i]);
		else if (keys[i].compare(info_.Y0.key()) == 0)
			info_.Y0.setValue(values[i]);
		else if (keys[i].compare(info_.Z0.key()) == 0)
			info_.Z0.setValue(values[i]);
		else if (keys[i].compare(info_.rowDirCosX.key()) == 0)
			info_.rowDirCosX.setValue(values[i]);
		else if (keys[i].compare(info_.rowDirCosY.key()) == 0)
			info_.rowDirCosY.setValue(values[i]);
		else if (keys[i].compare(info_.rowDirCosZ.key()) == 0)
			info_.rowDirCosZ.setValue(values[i]);
		else if (keys[i].compare(info_.colDirCosX.key()) == 0)
			info_.colDirCosX.setValue(values[i]);
		else if (keys[i].compare(info_.colDirCosY.key()) == 0)
			info_.colDirCosY.setValue(values[i]);
		else if (keys[i].compare(info_.colDirCosZ.key()) == 0)
			info_.colDirCosZ.setValue(values[i]);
    else if (keys[i].compare(info_.noiseSigma.key()) == 0)
      info_.noiseSigma.setValue(values[i]);
		else
		{
			//Throw exception that key not recognised
			std::cerr << "Error: Key " << keys[i] << " not recognised." << std::endl;
			std::abort();
		}

  }
}

//
//---------------------------------------------------------------------------------
MDM_API void mdm_Image3D::getSetKeyValuePairs(std::vector<std::string> &keys,
	std::vector<double> &values) const
{
	//Make sure containers are empty to start with
	keys.clear();
	values.clear();

	//Check if each info field is set, if so, append key and value to their
	//respective containers
	if (info_.TimeStamp.isSet()){
		keys.push_back(info_.TimeStamp.key()); 
		values.push_back(info_.TimeStamp.value());
	}
	if (info_.flipAngle.isSet()){
		keys.push_back(info_.flipAngle.key()); 
		values.push_back(info_.flipAngle.value());
	}
	if (info_.TR.isSet()){
		keys.push_back(info_.TR.key()); 
		values.push_back(info_.TR.value());
	}
	if (info_.TE.isSet()){
		keys.push_back(info_.TE.key()); 
		values.push_back(info_.TE.value());
	}
	if (info_.B.isSet()){
		keys.push_back(info_.B.key()); 
		values.push_back(info_.B.value());
	}
	if (info_.TI.isSet()){
		keys.push_back(info_.TI.key()); 
		values.push_back(info_.TI.value());
	}
	if (info_.TA.isSet()){
		keys.push_back(info_.TA.key()); 
		values.push_back(info_.TA.value());
	}
	if (info_.ETL.isSet()){
		keys.push_back(info_.ETL.key()); 
		values.push_back(info_.ETL.value());
	}
	if (info_.X0.isSet()){
		keys.push_back(info_.X0.key()); 
		values.push_back(info_.X0.value());
	}
	if (info_.Y0.isSet()){
		keys.push_back(info_.Y0.key()); 
		values.push_back(info_.Y0.value());
	}
	if (info_.Z0.isSet()){
		keys.push_back(info_.Z0.key()); 
		values.push_back(info_.Z0.value());
	}
	if (info_.rowDirCosX.isSet()){
		keys.push_back(info_.rowDirCosX.key()); 
		values.push_back(info_.rowDirCosX.value());
	}
	if (info_.rowDirCosY.isSet()){
		keys.push_back(info_.rowDirCosY.key()); 
		values.push_back(info_.rowDirCosY.value());
	}
	if (info_.rowDirCosZ.isSet()){
		keys.push_back(info_.rowDirCosZ.key()); 
		values.push_back(info_.rowDirCosZ.value());
	}
	if (info_.colDirCosX.isSet()){
		keys.push_back(info_.colDirCosX.key()); 
		values.push_back(info_.colDirCosX.value());
	}
	if (info_.colDirCosY.isSet()){
		keys.push_back(info_.colDirCosY.key()); 
		values.push_back(info_.colDirCosY.value());
	}
	if (info_.colDirCosZ.isSet()){
		keys.push_back(info_.colDirCosZ.key()); 
		values.push_back(info_.colDirCosZ.value());
	}
	if (info_.noiseSigma.isSet()){
		keys.push_back(info_.noiseSigma.key()); 
		values.push_back(info_.noiseSigma.value());
	}
}

//
MDM_API bool mdm_Image3D::initDataArray()
{
	//Don't bother error checking this anymore, just assume memory will be managed correctly
	//that's what the vector container class is for
  int nVoxels = getNvoxels();
  assert(nVoxels != 0);

	data_.resize(nVoxels);

  return true;
}

//
MDM_API void mdm_Image3D::flipSlice(const int sliceNo)
{
  double  *slice_tmp, *tmp, *tmp_start, *slicePtr;

  assert(getNvoxels() > 0);
  assert(sliceNo > 0 && sliceNo <= nZ_);

  int npixels   = nX_ * nY_;
  tmp_start = (double *) malloc(sizeof(double) * npixels);
  slicePtr  = &data_[((sliceNo - 1) * npixels)];

  /* first copy slice into temp buffer */
  tmp       = tmp_start;
  slice_tmp = slicePtr;
  for(int i = 0; i < npixels; i++)
    *tmp++ = *slice_tmp++;

  /* Now flip the slice back into the original buffer */
  tmp = tmp_start;

  /* *** Corrected offset bug 23/5/91 AMKS *** */
  /* was slice_tmp = slice + (npixels-1-xdim); */
  slice_tmp = slicePtr + (npixels - nX_);
  for(int j = 0; j < nY_; j++)
  {
    for(int i = 0; i < nX_; i++)
    {
      *slice_tmp++ = *tmp++;
    }
    slice_tmp = slice_tmp - 2 * nX_;
  }
  free(tmp_start);
}

//
MDM_API bool mdm_Image3D::voxelMatsMatch(const mdm_Image3D &img2)
{
  bool incompatible = false;
  bool compatible   = true;

  /* Following is crude test that fields have been set */
  assert(getNvoxels()>0);
  assert(img2.getNvoxels()>0);

  /* Test that voxel matrix dimensions match */
	int nx, ny, nz;
	img2.getMatrixDims(nx, ny, nz);
  if ((nX_ != nx)
       || (nY_ != ny)
       || (nZ_ != nz))
    return incompatible;

  /* Test that individual voxel mm dimensions match */
	double xmm, ymm, zmm;
	img2.getVoxelDims(xmm, ymm, zmm);
  if ((fabs(xmm_ - xmm) > 0.01)
       || (fabs(ymm_ - ymm) > 0.01)
       || (fabs(zmm_ - zmm) > 0.01))
    return incompatible;

  /* If we got here we're OK */
  return compatible;
}

//
MDM_API void mdm_Image3D::copyFields(const mdm_Image3D &imgToCopy)
{
	copyMatrix(imgToCopy);
	info_ = imgToCopy.info_;

  /*for (int iKey = 0; iKey < QBIINFO_NKEYS; iKey++)
    setInfo(iKey, imgToCopy.getInfo(iKey));*/

}

//
MDM_API void mdm_Image3D::copyMatrix(const mdm_Image3D &imgToCopy)
{

  /* Following is crude test that fields have been set */
  assert(imgToCopy.getNvoxels() > 0);

  setMatrixDims(imgToCopy.nX_, imgToCopy.nY_, imgToCopy.nZ_);
  setVoxelDims(imgToCopy.xmm_, imgToCopy.ymm_, imgToCopy.zmm_);

  //Allocate but don't copy the data array
  initDataArray();
}

/*
 * ToDo:
 * -   Print the image type string, rather than just the integer
 */
MDM_API void mdm_Image3D::toString(std::string &imgString) const
{

  /* First clear existing contents of imgString */
	std::stringstream ss;

	ss << 
		"mdm_Image3D:   type " << imgType_ << " image struct at location " << this << "\n" <<
		"voxel matrix is " << nX_ << " x " << nY_ << " x " << nZ_ << 
		", with dimensions " << xmm_ << " mm x " << ymm_ << " mm x " << zmm_ << " mm\n" <<
		"time stamp is " << info_.TimeStamp.value() << "\n" <<
		"info fields: flip angle is " << info_.flipAngle.value() << ", TR is " << info_.TR.value() << ",\n" <<
    "TE is " << info_.TE.value() << " and B is " << info_.B.value() << " (value < 0.0 => not set)\n" <<
    "and the image data is held at " << &data_ << "\n",
  
  imgString = ss.str();
}

//
MDM_API void mdm_Image3D::nonZero(std::vector<int> &idx, std::vector<double> &vals) const
{
	idx.clear();
	vals.clear();
	for (int i = 0, n = getNvoxels(); i++; i < n)
	{
		if (data_[i])
		{
			idx.push_back(i);
			vals.push_back(data_[i]);
		}
	}
}

//
MDM_API template <class T> bool mdm_Image3D::toBinaryStream(std::ostream &ofs, bool nonZero)  const
{
	size_t elSize = sizeof(T);
	if (nonZero)
	{
		int nVoxels = getNvoxels();
		std::vector<int> idx;
		
		for (int i = 0; i < nVoxels; i++)
		{
			//Check if element non-zeor (TODO: behaviour of NAN?)
			if (data_[i])
			{
				//If non-zero, cast data to output type and write to buffer
				T t = (T)data_[i];
				ofs.write(reinterpret_cast<const char*>(&t), elSize);

				//Append index
				idx.push_back(i);
			}
		}
		//For the indices we can now write in one chunk as only casting once
		//from int to char*
		ofs.write(reinterpret_cast<const char*>(&idx[0]), sizeof(int)*idx.size());
	}
	else
	{
		//Simple in this case: loop through data vector, cast each element
		//to output type and then write to buffer
		for (double d : data_)
		{
			T t = (T)d;
			ofs.write(reinterpret_cast<const char*>(&t), elSize);
		}
			
	}
	return true;
}

//Now force instantiation of the templated functions for the datatype we
//need or we'll get linker errors
MDM_API	template bool mdm_Image3D::toBinaryStream<char>(
	std::ostream &ofs, bool nonZero)  const;
MDM_API	template bool mdm_Image3D::toBinaryStream<short>(
	std::ostream &ofs, bool nonZero)  const;
MDM_API	template bool mdm_Image3D::toBinaryStream<int>(
	std::ostream &ofs, bool nonZero)  const;
MDM_API	template bool mdm_Image3D::toBinaryStream<float>(
	std::ostream &ofs, bool nonZero)  const;
MDM_API	template bool mdm_Image3D::toBinaryStream<double>(
	std::ostream &ofs, bool nonZero)  const;

MDM_API template <class T> bool mdm_Image3D::fromBinaryStream(
	std::istream &ifs, bool nonZero, bool swapBytes)
{
	//TODO endian check if swap is true - not currently implemented
	//so throw error for now
	if (swapBytes)
		return false;

	//Get size input type
	size_t elSize = sizeof(T);

	//Get size of stream by going to end, calling tellg, then returning to beginning
	ifs.seekg(0, std::ios_base::end);
	size_t bufferSize = ifs.tellg();
	ifs.seekg(0, std::ios_base::beg);
	if (nonZero)
	{
		//Get number of elements, by dividing buffer size by the
		//combined size of input type + integer (because each element
		//is stored by input type value + integer index
		size_t intSize = sizeof(int);
		size_t nNonZero = bufferSize / (intSize + elSize); 
		
		//Check here if not divisible
		if ((nNonZero*(intSize + elSize)) != bufferSize)
			return false;

		//Set up vectors to store data and indices
		std::vector<T> data(nNonZero);
		std::vector<int> idx(nNonZero);
		
		//TODO: We really should do some error checking of EOF etc here
		//although we have checked buffer size?

		//First read data values, casting from char* to input type
		int t;
		for (T &t : data)
			ifs.read(reinterpret_cast<char*>(&t), elSize);

		//Read indices - directly casting from char* to int
		for (int &i : idx)
			ifs.read(reinterpret_cast<char*>(&i), sizeof(int));

		//Finally loop through data and indices saving into main data array
		for (size_t i = 0; i < nNonZero; i++)
		{
			data_[idx[i]] = (double)data[i];
		}
	}
	else
	{
		//Check here buffer size matche nVoxels * size of input type
		size_t expectedSize = (size_t)getNvoxels()*elSize;
		if (expectedSize != bufferSize)
			return false;

		//Now we know buffer is correct size, loop through, casting each element
		//from char* to input type, and then to double for storing in main data array
		T t;
		for (double &d : data_)
		{
			ifs.read(reinterpret_cast<char*>(&t), elSize);
			d = (double)t;
		}
	}
	//If here, everything should have worked
	return true;
}

//Now force instantiation of the templated functions for the datatype we
//need or we'll get linker errors
MDM_API	template bool mdm_Image3D::fromBinaryStream<char>(
	std::istream &ifs, bool nonZero, bool swapBytes);
MDM_API	template bool mdm_Image3D::fromBinaryStream<short>(
	std::istream &ifs, bool nonZero, bool swapBytes);
MDM_API	template bool mdm_Image3D::fromBinaryStream<int>(
	std::istream &ifs, bool nonZero, bool swapBytes);
MDM_API	template bool mdm_Image3D::fromBinaryStream<float>(
	std::istream &ifs, bool nonZero, bool swapBytes);
MDM_API	template bool mdm_Image3D::fromBinaryStream<double>(
	std::istream &ifs, bool nonZero, bool swapBytes);

/*
 *  Modifications:
 *  9-24 February 2006 (GAB)
 *  - Created & ran basic tests
 *  23 March 2006 (GAB)
 *  - Moved documentation comments here
 *  27-29 March 2006 (GAB)
 *  - Removed offsets & default values & added some more DbC functionality
 *  - Added the copy function
 *  ============================    Version 0.2    ============================
 *  4 April 2006 (GAB)
 *  - Changed mdm_Image3DCopyFields() so it no longer copies type data
 *  ============================    Version 0.3    ============================
 *  28 Nov 2007 (GAB)
 *  - Added mdm_Image3DgetNvoxels() for convenience
 *  ============================    Version 0.4    ============================
 *  24 March 2009 (GAB)
 *  - Added new xtr file capability
 *  ============================    Version 0.5    ============================
 *  18 Nov 2009 (GAB)
 *  - Added deleteQbiImage3DdataArray() for when we declare struct on stack rather than heap
 *  - Added mdm_Image3DCopyMatrix() for when we don't want the info fields
 *  ============================    Version 0.6    ============================
 *  10 May 2017 (GAB)
 *  - Updated xtr info indices and string keys to match dicom-to-analyze-converter versions 0.7.x
 */
