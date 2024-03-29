/**
*  @file    mdm_Image3D.cxx
*  @brief   implementation of mdm_Image3D class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif
#include <madym/utils/mdm_Image3D.h>

#include <string>
#include <cassert>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <boost/date_time.hpp>
#include <boost/format.hpp>

const std::string mdm_Image3D::MetaData::ImageTypeKey = "ImageType";
const std::string mdm_Image3D::MetaData::TimeStampKey = "TimeStamp";
const double mdm_Image3D::voxelSizeTolerance_ = 0.01;

//
mdm_Image3D::MetaData::MetaData()
	: /* Defaulting to NaN llows test isnan() for unset */
	flipAngle("FlipAngle"),
	TR("RepetitionTime"),
	TE("EchoTime"),
	B("B"),
	TI("InversionTime"),
	TA("TA"),
	ETL("ETL"),
	gradOriX("gradOriX"),
	gradOriY("gradOriY"),
	gradOriZ("gradOriZ"),
	Xmm("Xmm"),
	Ymm("Ymm"),
	Zmm("Zmm"),
	originX("OriginX", 0.0),
	originY("OriginY", 0.0),
	originZ("OriginZ", 0.0),
	rowDirCosX("RowDirCosX", 1.0),
	rowDirCosY("RowDirCosY", 0.0),
	rowDirCosZ("RowDirCosZ", 0.0),
	colDirCosX("ColDirCosX", 0.0),
	colDirCosY("ColDirCosY", 1.0),
	colDirCosZ("ColDirCosZ", 0.0),
	flipX("FlipX", 0),
	flipY("FlipY", 0),
	flipZ("FlipZ", 0),
	zDirection("ZDirection", 1.0),
	temporalResolution("TemporalResolution"),
	sclSlope("SclSlope"),
	sclInter("SclInter"),
	noiseSigma("NoiseSigma"),
	xtrSource("from API")
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
	//Set time stamp this can be overridden later using setTimeStamp or read during
	//setMetaDataFromString
	setTimeStampFromNow();
}

//Destructor
MDM_API mdm_Image3D::~mdm_Image3D()
{

}

//
MDM_API void mdm_Image3D::reset()
{
  imgType_ = TYPE_UNDEFINED;
  MetaData empty;
  info_ = empty;
  setDimensions(0, 0, 0);
}

//Return const ref to data
MDM_API const std::vector<double>& mdm_Image3D::data() const
{
	return data_;
}

//
MDM_API double mdm_Image3D::voxel(size_t i) const
{
  if (i >= data_.size())
    throw mdm_exception(__func__, boost::format(
      "Attempting to access voxel %1% when there are only %2% voxels")
      % i % data_.size());

  return data_[i];
}

//
MDM_API void mdm_Image3D::setVoxel(size_t i, double value)
{
  if (i >= data_.size())
    throw mdm_exception(__func__, boost::format(
      "Attempting to access voxel %1% when there are only %2% voxels")
      % i % data_.size());

  data_[i] = value;
}

//
MDM_API double mdm_Image3D::voxel(size_t x, size_t y, size_t z) const
{
  return voxel(sub2ind(x, y, z));
}

//
MDM_API void mdm_Image3D::setVoxel(size_t x, size_t y, size_t z, double value)
{
  setVoxel(sub2ind(x, y, z), value);
}

//
MDM_API void mdm_Image3D::setSlice(size_t z, std::vector<double> values)
{
  if (z >= nZ_)
    throw mdm_exception(__func__, boost::format(
      "Attempting to access slice index %1% when there are only %2% slices")
      % z % nZ_);

  if (values.size() != nX_ * nY_)
    throw mdm_exception(__func__,
      boost::format(
        "Invalid insert size: number of insert values = %1%, does not match slice size %2%x%3% = %4%")
      % values.size() % nX_ % nY_ % (nX_*nY_));

  size_t offset = nX_ * nY_*z;

  std::copy(values.begin(), values.end(), data_.begin() + offset);
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
MDM_API void mdm_Image3D::setDimensions(const size_t nX, const size_t nY, const size_t nZ)
{

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
MDM_API void mdm_Image3D::getDimensions(size_t &nX, size_t &nY, size_t &nZ) const
{
  nX = nX_;
  nY = nY_;
  nZ = nZ_;
}

//
MDM_API size_t mdm_Image3D::numVoxels() const
{
  return data_.size();
}

//
MDM_API void mdm_Image3D::setVoxelDims(const double xmm, const double ymm, const double zmm)
{
  if (xmm <= 0)
    throw mdm_exception(__func__, boost::format(
      "Invalid voxel dimension: trying to set xmm = %1%, should be strictly positive")
      % xmm);

  if (ymm <= 0)
    throw mdm_exception(__func__, boost::format(
      "Invalid voxel dimension: trying to set ymm = %1%, should be strictly positive")
      % ymm);

  if (zmm <= 0)
    throw mdm_exception(__func__, boost::format(
      "Invalid voxel dimension: trying to set zmm = %1%, should be strictly positive")
      % zmm);

  info_.Xmm.setValue(xmm);
	info_.Ymm.setValue(ymm);
	info_.Zmm.setValue(zmm);
}

//
MDM_API void mdm_Image3D::setTimeStampFromDoubleStr(const double timeStamp)
{
  timeStamp_ = timeStamp;
}

//
MDM_API void mdm_Image3D::setTimeStampFromNow()
{
	boost::posix_time::ptime timeLocal =
		boost::posix_time::second_clock::local_time();

	double hh = timeLocal.time_of_day().hours();
	double mm = timeLocal.time_of_day().minutes();
	double ss = timeLocal.time_of_day().seconds();
	timeStamp_ = 10000 * hh + 100 * mm + ss;
}

//
MDM_API void mdm_Image3D::setTimeStampFromMins(const double timeInMins)
{
	setTimeStampFromSecs(60 * timeInMins);
}

//
MDM_API void mdm_Image3D::setTimeStampFromSecs(const double timeInSecs)
{
	// Convert time in seconds into the xtr timestamp format
	//hhmmss.msecs represented as a single decimal number
	timeStamp_ = secsToTimestamp(timeInSecs);
}

//
MDM_API double mdm_Image3D::timeStamp() const
{
  return timeStamp_;
}

//
MDM_API double mdm_Image3D::minutesFromTimeStamp() const
{
	return secondsFromTimeStamp() / 60.0; //time in minutes as used as standard throughout DCE analysis
}

//
MDM_API double mdm_Image3D::secondsFromTimeStamp() const
{
	return timestampToSecs(timeStamp_); //time in minutes as used as standard throughout DCE analysis
}


//
MDM_API double mdm_Image3D::secsToTimestamp(const double secs)
{
  double hh = std::floor(secs / (3600));
  double mm = std::floor((secs - 3600 * hh) / 60);
  double ss = secs - 3600 * hh - 60 * mm;
  return 10000 * hh + 100 * mm + ss;
}

MDM_API double mdm_Image3D::timestampToSecs(const double timestamp)
{
  int hours = (int)(timestamp / 10000);
  int minutes = (int)(timestamp - 10000 * hours) / 100;
  double seconds = (timestamp
    - 10000 * hours
    - 100 * minutes);
  double timeInSecs = double(hours) * 60 * 60
    + double(minutes) * 60
    + seconds;
  return timeInSecs;
}

//
MDM_API mdm_Image3D::MetaData& mdm_Image3D::info()
{
	return info_;
}

//
MDM_API const mdm_Image3D::MetaData& mdm_Image3D::info() const
{
	return info_;
}

//
MDM_API void mdm_Image3D::setMetaData(const std::string &key, const double &value)
{

	if (boost::iequals(key, info_.TimeStampKey))
		setTimeStampFromDoubleStr(value);
	else if (boost::iequals(key, info_.ImageTypeKey))
		setType(static_cast<ImageType>(int(value)));		
	else if (boost::iequals(key, info_.flipAngle.key()))
		info_.flipAngle.setValue(value);
	else if (boost::iequals(key, info_.TR.key()) || boost::iequals(key, "TR")) //TR for back compatibility
		info_.TR.setValue(value);
	else if (boost::iequals(key, info_.TE.key()) || boost::iequals(key, "TE")) //TI for back compatibility
		info_.TE.setValue(value);
	else if (boost::iequals(key, info_.B.key()))
		info_.B.setValue(value);
	else if (boost::iequals(key, info_.TI.key()) || boost::iequals(key, "TI")) //TI for back compatibility
		info_.TI.setValue(value);
	else if (boost::iequals(key, info_.TA.key()))
		info_.TA.setValue(value);
	else if (boost::iequals(key, info_.ETL.key()))
		info_.ETL.setValue(value);
	else if (boost::iequals(key, info_.gradOriX.key()))
		info_.gradOriX.setValue(value);
	else if (boost::iequals(key, info_.gradOriY.key()))
		info_.gradOriY.setValue(value);
	else if (boost::iequals(key, info_.gradOriZ.key()))
		info_.gradOriZ.setValue(value);
	else if (boost::iequals(key, info_.Xmm.key()))
		info_.Xmm.setValue(value);
	else if (boost::iequals(key, info_.Ymm.key()))
		info_.Ymm.setValue(value);
	else if (boost::iequals(key, info_.Zmm.key()))
		info_.Zmm.setValue(value);
	else if (boost::iequals(key, info_.originX.key()))
		info_.originX.setValue(value);
	else if (boost::iequals(key, info_.originY.key()))
		info_.originY.setValue(value);
	else if (boost::iequals(key, info_.originZ.key()))
		info_.originZ.setValue(value);
	else if (boost::iequals(key, info_.rowDirCosX.key()))
		info_.rowDirCosX.setValue(value);
	else if (boost::iequals(key, info_.rowDirCosY.key()))
		info_.rowDirCosY.setValue(value);
	else if (boost::iequals(key, info_.rowDirCosZ.key()))
		info_.rowDirCosZ.setValue(value);
	else if (boost::iequals(key, info_.colDirCosX.key()))
		info_.colDirCosX.setValue(value);
	else if (boost::iequals(key, info_.colDirCosY.key()))
		info_.colDirCosY.setValue(value);
	else if (boost::iequals(key, info_.colDirCosZ.key()))
		info_.colDirCosZ.setValue(value);
	else if (boost::iequals(key, info_.flipX.key()))
		info_.flipX.setValue(value);
	else if (boost::iequals(key, info_.flipY.key()))
		info_.flipY.setValue(value);
	else if (boost::iequals(key, info_.flipZ.key()))
		info_.flipZ.setValue(value);
	else if (boost::iequals(key, info_.zDirection.key()))
		info_.zDirection.setValue(value);
	else if (boost::iequals(key, info_.temporalResolution.key()))
		info_.temporalResolution.setValue(value);
	else if (boost::iequals(key, info_.sclSlope.key()))
		info_.sclSlope.setValue(value);
	else if (boost::iequals(key, info_.sclInter.key()))
		info_.sclInter.setValue(value);
  else if (boost::iequals(key, info_.noiseSigma.key()))
    info_.noiseSigma.setValue(value);
	else
	{
		//Throw exception that key not recognised
    throw mdm_exception(__func__, boost::format("Key %1% not recognised") % key);
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
	if (info_.gradOriX.isSet()) {
		keys.push_back(info_.gradOriX.key());
		values.push_back(info_.gradOriX.value());
	}
	if (info_.gradOriY.isSet()) {
		keys.push_back(info_.gradOriY.key());
		values.push_back(info_.gradOriY.value());
	}
	if (info_.gradOriZ.isSet()) {
		keys.push_back(info_.gradOriZ.key());
		values.push_back(info_.gradOriZ.value());
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
	if (info_.originX.isSet()) {
		keys.push_back(info_.originX.key());
		values.push_back(info_.originX.value());
	}
	if (info_.originY.isSet()) {
		keys.push_back(info_.originY.key());
		values.push_back(info_.originY.value());
	}
	if (info_.originZ.isSet()) {
		keys.push_back(info_.originZ.key());
		values.push_back(info_.originZ.value());
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
	if (info_.flipX.isSet()) {
		keys.push_back(info_.flipX.key());
		values.push_back(info_.flipX.value());
	}
	if (info_.flipY.isSet()) {
		keys.push_back(info_.flipY.key());
		values.push_back(info_.flipY.value());
	}
	if (info_.flipZ.isSet()) {
		keys.push_back(info_.flipZ.key());
		values.push_back(info_.flipZ.value());
	}
	if (info_.zDirection.isSet()) {
		keys.push_back(info_.zDirection.key());
		values.push_back(info_.zDirection.value());
	}
	if (info_.temporalResolution.isSet()) {
		keys.push_back(info_.temporalResolution.key());
		values.push_back(info_.temporalResolution.value());
	}
	if (info_.sclSlope.isSet()) {
		keys.push_back(info_.sclSlope.key());
		values.push_back(info_.sclSlope.value());
	}
	if (info_.sclInter.isSet()) {
		keys.push_back(info_.sclInter.key());
		values.push_back(info_.sclInter.value());
	}
	if (info_.noiseSigma.isSet()){
		keys.push_back(info_.noiseSigma.key()); 
		values.push_back(info_.noiseSigma.value());
	}
}

//
MDM_API bool mdm_Image3D::dimensionsMatch(const mdm_Image3D &img) const
{
  // Test that voxel matrix dimensions match
  size_t nx, ny, nz;
  img.getDimensions(nx, ny, nz);
  return ((nX_ == nx)
    && (nY_ == ny)
    && (nZ_ == nz));
}

//
MDM_API bool mdm_Image3D::voxelSizesMatch(const mdm_Image3D &img) const
{

  // Test that individual voxel mm dimensions match
  return ((fabs(info_.Xmm.value() - img.info_.Xmm.value()) <= voxelSizeTolerance_)
    && (fabs(info_.Ymm.value() - img.info_.Ymm.value()) <= voxelSizeTolerance_)
    && (fabs(info_.Zmm.value() - img.info_.Zmm.value()) <= voxelSizeTolerance_));
}



//
MDM_API void mdm_Image3D::copy(const mdm_Image3D &imgToCopy)
{
	//This does NOT copy:
	//1) Data values
	//2) Timestamp
	//3) Image type

	//Copy image data
	info_ = imgToCopy.info();

	//Set dimension from the copy, this will reset and resize the data array
	setDimensions(imgToCopy);

	//Reset sclSlope and Inter as we do NOT want these copied
	info_.sclSlope.setValue(1.0);
	info_.sclInter.setValue(0.0);

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
		"time stamp is " << timeStamp_ << "\n" <<
		"info fields: flip angle is " << info_.flipAngle.value() << ", TR is " << info_.TR.value() << ",\n" <<
    "TE is " << info_.TE.value() << " and B is " << info_.B.value() << " (value < 0.0 => not set)\n" <<
    "and the image data is held at " << &data_ << "\n",
  
  imgString = ss.str();
}

//
MDM_API void mdm_Image3D::metaDataToStream(std::ostream &ofs) const
{
	//Write the time stamp
	ofs << info_.TimeStampKey << "\t"
		<< std::fixed << std::setw(13) << std::setfill('0') << std::setprecision(6) << timeStamp() << std::endl;

	//Write the image type
	ofs << info_.ImageTypeKey << "\t" << type() << std::endl;

	std::vector<std::string> keys;
	std::vector<double> values;
	getSetKeyValuePairs(keys, values);

	for (int i = 0; i < keys.size(); i++)
		ofs << keys[i] << "\t" << values[i] << std::endl;
}

//
MDM_API void mdm_Image3D::setMetaDataFromStream(std::istream &ifs)
{
	std::string key;
	double value;
	while (!ifs.eof())
	{
		ifs >> key >> value;
		setMetaData(key, value);
	}
}

//
MDM_API void mdm_Image3D::setMetaDataFromStreamOld(std::istream &ifs)
{
	std::string str;
	double f;
	ifs >> str >> str >> f >> f >> f;
	ifs >> str >> str >> f;
	info_.flipAngle.setValue(f);
	ifs >> str >> f;
	info_.TR.setValue(f);
	ifs >> str >> f >> f >> f >> f;
	setTimeStampFromDoubleStr(f);
}

//
MDM_API void mdm_Image3D::setMetaDataSource(const std::string& xtrFile)
{
	info_.xtrSource = xtrFile;
}

//
MDM_API void mdm_Image3D::nonZeroVoxels(std::vector<size_t> &idx, std::vector<double> &vals) const
{
	idx.clear();
	vals.clear();
	for (size_t i = 0, n = numVoxels(); i < n; i++)
	{
		if (data_[i])
		{
			idx.push_back(i);
			vals.push_back(data_[i]);
		}
	}
}

//
template <class T> MDM_API void mdm_Image3D::toBinaryStream(std::ostream &ofs, bool nonZero)  const
{
	size_t elSize = sizeof(T);
	if (nonZero)
	{
		auto nVoxels = numVoxels();
		std::vector<unsigned int> idx;
		
		for (size_t i = 0; i < nVoxels; i++)
		{
			//Check if element non-zero (TODO: behaviour of NAN?)
			if (data_[i])
			{
				//If non-zero, cast data to output type and write to buffer
				T t = (T)data_[i];
				ofs.write(reinterpret_cast<const char*>(&t), elSize);

				//Append index
				idx.push_back((unsigned int)i);
			}
		}
		//For the indices we can now write in one chunk as only casting once
		//from int to char*
		if (!idx.empty())
			ofs.write(reinterpret_cast<const char*>(&idx[0]), sizeof(unsigned int)*idx.size());
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
}

//Now force instantiation of the templated functions for the datatype we
//need or we'll get linker errors

//! Template specialization declaration of toBinaryStream for char datatype
template MDM_API	void mdm_Image3D::toBinaryStream<char>(
	std::ostream &ofs, bool nonZero)  const;
//! Template specialization declaration of toBinaryStream for short datatype
template MDM_API	void mdm_Image3D::toBinaryStream<short>(
	std::ostream &ofs, bool nonZero)  const;
//! Template specialization declaration of toBinaryStream for int datatype
template MDM_API	void mdm_Image3D::toBinaryStream<int>(
	std::ostream &ofs, bool nonZero)  const;
//! Template specialization declaration of toBinaryStream for float datatype
template MDM_API	void mdm_Image3D::toBinaryStream<float>(
	std::ostream &ofs, bool nonZero)  const;
//! Template specialization declaration of toBinaryStream for double datatype
template MDM_API	void mdm_Image3D::toBinaryStream<double>(
	std::ostream &ofs, bool nonZero)  const;

template <class T> MDM_API void mdm_Image3D::fromBinaryStream(
	std::istream &ifs, bool nonZero, bool swap)
{

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
			throw mdm_exception(__func__, boost::format(
        "Failed to load sparse format data. "
        "Buffer size (%1%) is not divisble by combined index and value size (%2%)")
        % bufferSize % (intSize + elSize));

		//Set up vectors to store data and indices
		std::vector<T> data(nNonZero);
		std::vector<size_t> idx(nNonZero);
		
		//TODO: We really should do some error checking of EOF etc here
		//although we have checked buffer size?

		//First read data values, casting from char* to input type
		for (T &t : data)
		{
			ifs.read(reinterpret_cast<char*>(&t), elSize);
			if (swap)
				swapBytes(t);
		}
			

		//Read indices - directly casting from char* to int
		for (size_t &i : idx)
		{
			ifs.read(reinterpret_cast<char*>(&i), sizeof(unsigned int));
			if (swap)
				swapBytes(i);
		}
			

		//Finally loop through data and indices saving into main data array
		for (size_t i = 0; i < nNonZero; i++)
			data_[idx[i]] = (double)data[i];
		
	}
	else
	{
		//Check here buffer size matche nVoxels * size of input type
		size_t expectedSize = (size_t)numVoxels()*elSize;
		if (expectedSize != bufferSize)
      throw mdm_exception(__func__, boost::format(
        "Failed to image data. "
        "Buffer size (%1%) does not match expected size (%2%)")
        % bufferSize % expectedSize);

		//Now we know buffer is correct size, loop through, casting each element
		//from char* to input type, and then to double for storing in main data array
		T t;
		for (double &d : data_)
		{
			ifs.read(reinterpret_cast<char*>(&t), elSize);
			if (swap)
				swapBytes(t);

			d = (double)t;
		}
	}
}

//Now force instantiation of the templated functions for the datatype we
//need or we'll get linker errors
//! Template specialization declaration of fromBinaryStream for char datatype
template MDM_API void mdm_Image3D::fromBinaryStream<char>(
	std::istream &ifs, bool nonZero, bool swap);
//! Template specialization declaration of fromBinaryStream for short datatype
template MDM_API	void mdm_Image3D::fromBinaryStream<short>(
	std::istream &ifs, bool nonZero, bool swap);
//! Template specialization declaration of fromBinaryStream for int datatype
template MDM_API	void mdm_Image3D::fromBinaryStream<int>(
	std::istream &ifs, bool nonZero, bool swap);
//! Template specialization declaration of fromBinaryStream for float datatype
template MDM_API	void mdm_Image3D::fromBinaryStream<float>(
	std::istream &ifs, bool nonZero, bool swap);
//! Template specialization declaration of fromBinaryStream for double datatype
template MDM_API	void mdm_Image3D::fromBinaryStream<double>(
	std::istream &ifs, bool nonZero, bool swap);

//
//MDM_API void mdm_Image3D::swapBytes(char *data, int nBytes)
template <class T> MDM_API void mdm_Image3D::swapBytes(T& data)
{
	const int MAX_BYTES = 32;
	int nBytes = sizeof(data);

	if(!nBytes)
    throw mdm_exception(__func__, "Attempting to swap empty bytes buffer");
  
  if (nBytes > MAX_BYTES)
    throw mdm_exception(__func__, boost::format(
      "Cannot swap bytes in buffer size %1%, must be <= 32 bytes") % nBytes);

	char  temp[MAX_BYTES];
	char *data_p = (char *)&data;

	for (int i = 0; i < nBytes; i++)
		temp[i] = data_p[i];

	for (int i = 0; i < nBytes; i++)
		data_p[i] = temp[nBytes - i - 1];
}

//Now force instantiation of the templated functions for the datatype we
//need or we'll get linker errors
//! Template specialization declaration of swapBytes for char datatype
template  MDM_API void mdm_Image3D::swapBytes<char>(char& data);
//! Template specialization declaration of swapBytes for short datatype
template  MDM_API void mdm_Image3D::swapBytes<short>(short& data);
//! Template specialization declaration of swapBytes for int datatype
template  MDM_API void mdm_Image3D::swapBytes<int>(int& data);
//! Template specialization declaration of swapBytes for float datatype
template  MDM_API void mdm_Image3D::swapBytes<float>(float& data);
//! Template specialization declaration of swapBytes for double datatype
template  MDM_API void mdm_Image3D::swapBytes<double>(double& data);

//
MDM_API size_t mdm_Image3D::sub2ind(size_t x, size_t y, size_t z) const
{
  return x + (y * nX_) + (z * nX_ * nY_);
}

//
MDM_API mdm_Image3D& mdm_Image3D::operator+=(const mdm_Image3D& rhs)
{
  if (!dimensionsMatch(rhs))
    throw mdm_dimension_mismatch(__func__, *this, rhs);

  size_t v2 = 0;
  for (auto & v : this->data_)
    v += rhs.voxel(v2++);
  return *this;
}

//
MDM_API mdm_Image3D& mdm_Image3D::operator+=(const double d)
{
  for (auto & v : this->data_)
    v += d;
  return *this;
}

//
MDM_API mdm_Image3D& mdm_Image3D::operator*=(const mdm_Image3D& rhs)
{
  if (!dimensionsMatch(rhs))
    throw mdm_dimension_mismatch(__func__, *this, rhs);

  size_t v2 = 0;
  for (auto & v : this->data_)
    v *= rhs.voxel(v2++);
  return *this;
}

//
MDM_API mdm_Image3D& mdm_Image3D::operator*=(const double d)
{
  for (auto & v : this->data_)
    v *= d;
  return *this;
}

//
MDM_API mdm_Image3D& mdm_Image3D::operator-=(const mdm_Image3D& rhs)
{
  if (!dimensionsMatch(rhs))
    throw mdm_dimension_mismatch(__func__, *this, rhs);

  size_t v2 = 0;
  for (auto & v : this->data_)
    v -= rhs.voxel(v2++);
  return *this;
}

//
MDM_API mdm_Image3D& mdm_Image3D::operator-=(const double d)
{
  for (auto & v : this->data_)
    v -= d;
  return *this;
}

//
MDM_API mdm_Image3D& mdm_Image3D::operator/=(const mdm_Image3D& rhs)
{
  if (!dimensionsMatch(rhs))
    throw mdm_dimension_mismatch(__func__, *this, rhs);

  size_t v2 = 0;
  for (auto & v : this->data_)
    v /= rhs.voxel(v2++);
  return *this;
}

//
MDM_API mdm_Image3D& mdm_Image3D::operator/=(const double d)
{
  for (auto & v : this->data_)
    v /= d;
  return *this;
}

//**************************************************************************
// Private functions
//**************************************************************************
//
void mdm_Image3D::initDataArray()
{
	//Don't bother error checking this anymore, just assume memory will be managed correctly
	//that's what the vector container class is for
	data_.resize(nX_ * nY_ * nZ_);
}
