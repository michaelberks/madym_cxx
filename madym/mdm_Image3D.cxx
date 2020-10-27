/**
 *  @file    mdm_Image3D.cxx
 *  @brief   implementation of mdm_Image3D class
 *
 * @author   Mike Berks
 */

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif
#include "mdm_Image3D.h"

#include <string>
#include <cassert>
#include <sstream>
#include <iostream>

//
mdm_Image3D::MetaData::MetaData()
	: /* Defaulting to NaN llows test isnan() for unset */
	TimeStamp("TimeStamp"),
	flipAngle("FlipAngle"),
	TR("TR"),
	TE("TE"),
	B("B"),
	TI("TI"),
	TA("TA"),
	ETL("ETL"),
	Xmm("Xmm"),
	Ymm("Ymm"),
	Zmm("Zmm"),
	rowDirCosX("RowDirCosX"),
	rowDirCosY("RowDirCosY"),
	rowDirCosZ("RowDirCosZ"),
	colDirCosX("ColDirCosX"),
	colDirCosY("ColDirCosY"),
	colDirCosZ("ColDirCosZ"),
	noiseSigma("NoiseSigma")
{}

//
MDM_API mdm_Image3D::mdm_Image3D()
	:
	imgType_(TYPE_UNDEFINED),
	nX_(0),
	nY_(0),
	nZ_(0),
	info_(),
	data_(0)
{   
}

//Destructor
MDM_API mdm_Image3D::~mdm_Image3D()
{

}

//Return const ref to data
MDM_API const std::vector<double>& mdm_Image3D::data() const
{
	return data_;
}

//
MDM_API double mdm_Image3D::voxel(int i) const
{
	//Should we check the input is in range - possibly, although adds overhead?
	//Maybe have a separate safe method?
	assert(i >= 0 && i < numVoxels());
	return data_[i];
}

//
MDM_API void mdm_Image3D::setVoxel(int i, double value)
{
	//Should we check the input is in range - possibly, although adds overhead?
	//Maybe have a separate safe method?
	assert(i >= 0 && i < numVoxels());
	data_[i] = value;
}

//
MDM_API void mdm_Image3D::setType(ImageType newType)
{
  imgType_ = newType;
}

//
MDM_API mdm_Image3D::ImageType mdm_Image3D::type() const
{
  return imgType_;
}

//
MDM_API void mdm_Image3D::setDimensions(const int nX, const int nY, const int nZ)
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
MDM_API void mdm_Image3D::setDimensions(const mdm_Image3D &img)
{
	setDimensions(img.nX_, img.nY_, img.nZ_);
	setVoxelDims(img.info_.Xmm.value(), img.info_.Ymm.value(), img.info_.Zmm.value());
}

//
MDM_API void mdm_Image3D::getDimensions(int &nX, int &nY, int &nZ) const
{
  nX = nX_;
  nY = nY_;
  nZ = nZ_;
}

//
MDM_API int mdm_Image3D::numVoxels() const
{
  return (int) (nX_ * nY_ * nZ_);
}

//
MDM_API void mdm_Image3D::setVoxelDims(const double xmm, const double ymm, const double zmm)
{
  assert((xmm >= 0.0) && (ymm >= 0.0) && (zmm >= 0.0));

  info_.Xmm.setValue(xmm);
	info_.Ymm.setValue(ymm);
	info_.Zmm.setValue(zmm);
}

//
MDM_API void mdm_Image3D::setTimeStamp(const double timeStamp)
{
  info_.TimeStamp.setValue(timeStamp);
}

//
MDM_API double mdm_Image3D::timeStamp() const
{
  return info_.TimeStamp.value();
}

//
MDM_API void mdm_Image3D::setMetaData(const std::vector<std::string> &keys,
	const std::vector<double> &values)
{
	auto nKeys = keys.size();
  assert(nKeys == values.size());

	for (auto i = 0; i < nKeys; i++)
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
		else if (keys[i].compare(info_.Xmm.key()) == 0)
			info_.Xmm.setValue(values[i]);
		else if (keys[i].compare(info_.Ymm.key()) == 0)
			info_.Ymm.setValue(values[i]);
		else if (keys[i].compare(info_.Zmm.key()) == 0)
			info_.Zmm.setValue(values[i]);
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
	if (info_.Xmm.isSet()){
		keys.push_back(info_.Xmm.key()); 
		values.push_back(info_.Xmm.value());
	}
	if (info_.Ymm.isSet()){
		keys.push_back(info_.Ymm.key()); 
		values.push_back(info_.Ymm.value());
	}
	if (info_.Zmm.isSet()){
		keys.push_back(info_.Zmm.key()); 
		values.push_back(info_.Zmm.value());
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
MDM_API bool mdm_Image3D::dimensionsMatch(const mdm_Image3D &img)
{
  bool incompatible = false;
  bool compatible   = true;

  /* Following is crude test that fields have been set */
  assert(numVoxels()>0);
  assert(img.numVoxels()>0);

  /* Test that voxel matrix dimensions match */
	int nx, ny, nz;
	img.getDimensions(nx, ny, nz);
  if ((nX_ != nx)
       || (nY_ != ny)
       || (nZ_ != nz))
    return incompatible;

  /* Test that individual voxel mm dimensions match */
	if ((fabs(info_.Xmm.value() - img.info_.Xmm.value()) > 0.01)
       || (fabs(info_.Ymm.value() - img.info_.Ymm.value()) > 0.01)
       || (fabs(info_.Zmm.value() - img.info_.Zmm.value()) > 0.01))
    return incompatible;

  /* If we got here we're OK */
  return compatible;
}

//
MDM_API void mdm_Image3D::copy(const mdm_Image3D &imgToCopy)
{
	//Copy image data, but do not copy timestamp
	auto t = timeStamp();
	info_ = imgToCopy.info_;
	setTimeStamp(t);

	//Set dimension from the copy, this will reset and resize the data array
	setDimensions(imgToCopy);

}

//
MDM_API void mdm_Image3D::toString(std::string &imgString) const
{

  std::stringstream ss;

	ss << 
		"mdm_Image3D:   type " << imgType_ << " image struct at location " << this << "\n" <<
		"voxel matrix is " << nX_ << " x " << nY_ << " x " << nZ_ << 
		", with dimensions " << 
		info_.Xmm.value() << " mm x " << 
		info_.Ymm.value() << " mm x " << 
		info_.Zmm.value() << " mm\n" <<
		"time stamp is " << info_.TimeStamp.value() << "\n" <<
		"info fields: flip angle is " << info_.flipAngle.value() << ", TR is " << info_.TR.value() << ",\n" <<
    "TE is " << info_.TE.value() << " and B is " << info_.B.value() << " (value < 0.0 => not set)\n" <<
    "and the image data is held at " << &data_ << "\n",
  
  imgString = ss.str();
}

//
MDM_API void mdm_Image3D::nonZeroVoxels(std::vector<int> &idx, std::vector<double> &vals) const
{
	idx.clear();
	vals.clear();
	for (int i = 0, n = numVoxels(); i < n; i++)
	{
		if (data_[i])
		{
			idx.push_back(i);
			vals.push_back(data_[i]);
		}
	}
}

//
template <class T> MDM_API bool mdm_Image3D::toBinaryStream(std::ostream &ofs, bool nonZero)  const
{
	size_t elSize = sizeof(T);
	if (nonZero)
	{
		int nVoxels = numVoxels();
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
		if (!idx.empty())
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

//! Template specialization declaration of toBinaryStream for char datatype
template MDM_API	bool mdm_Image3D::toBinaryStream<char>(
	std::ostream &ofs, bool nonZero)  const;
//! Template specialization declaration of toBinaryStream for short datatype
template MDM_API	bool mdm_Image3D::toBinaryStream<short>(
	std::ostream &ofs, bool nonZero)  const;
//! Template specialization declaration of toBinaryStream for int datatype
template MDM_API	bool mdm_Image3D::toBinaryStream<int>(
	std::ostream &ofs, bool nonZero)  const;
//! Template specialization declaration of toBinaryStream for float datatype
template MDM_API	bool mdm_Image3D::toBinaryStream<float>(
	std::ostream &ofs, bool nonZero)  const;
//! Template specialization declaration of toBinaryStream for double datatype
template MDM_API	bool mdm_Image3D::toBinaryStream<double>(
	std::ostream &ofs, bool nonZero)  const;

template <class T> MDM_API bool mdm_Image3D::fromBinaryStream(
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
		size_t expectedSize = (size_t)numVoxels()*elSize;
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
//! Template specialization declaration of fromBinaryStream for char datatype
template MDM_API bool mdm_Image3D::fromBinaryStream<char>(
	std::istream &ifs, bool nonZero, bool swapBytes);
//! Template specialization declaration of fromBinaryStream for short datatype
template MDM_API	bool mdm_Image3D::fromBinaryStream<short>(
	std::istream &ifs, bool nonZero, bool swapBytes);
//! Template specialization declaration of fromBinaryStream for int datatype
template MDM_API	bool mdm_Image3D::fromBinaryStream<int>(
	std::istream &ifs, bool nonZero, bool swapBytes);
//! Template specialization declaration of fromBinaryStream for float datatype
template MDM_API	bool mdm_Image3D::fromBinaryStream<float>(
	std::istream &ifs, bool nonZero, bool swapBytes);
//! Template specialization declaration of fromBinaryStream for double datatype
template MDM_API	bool mdm_Image3D::fromBinaryStream<double>(
	std::istream &ifs, bool nonZero, bool swapBytes);

//**************************************************************************
// Private functions
//**************************************************************************
//
bool mdm_Image3D::initDataArray()
{
	//Don't bother error checking this anymore, just assume memory will be managed correctly
	//that's what the vector container class is for
	int nVoxels = numVoxels();
	assert(nVoxels != 0);

	data_.resize(nVoxels);

	return true;
}

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
