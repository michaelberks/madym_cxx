/**
 *  @file    mdm_XtrFormat.cxx
 *  @brief   Implementation of class for Analyze image format reading and writing
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif

#include "mdm_BIDSFormat.h"

#include <string>
#include <sstream>
#include <istream>
#include <ostream>
#include <iostream>
#include <algorithm>

#include <boost/filesystem.hpp>
#include <boost/json/src.hpp>

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>

namespace json = boost::json;
namespace fs = boost::filesystem;

std::string parseJSONExt(const std::string& fileName, const std::string &ext)
{
  fs::path p(fileName);
  auto ext_p = p.extension();
  return (ext_p == ext) ? fileName : fileName + ext;
}

void JSONtoFile(std::ostream& os, json::value const& jv, std::string* indent = nullptr)
{
  std::string indent_;
  if (!indent)
    indent = &indent_;
  switch (jv.kind())
  {
  case json::kind::object:
  {
    os << "{\n";
    indent->append(4, ' ');
    auto const& obj = jv.get_object();
    if (!obj.empty())
    {
      auto it = obj.begin();
      for (;;)
      {
        os << *indent << json::serialize(it->key()) << " : ";
        JSONtoFile(os, it->value(), indent);
        if (++it == obj.end())
          break;
        os << ",\n";
      }
    }
    os << "\n";
    indent->resize(indent->size() - 4);
    os << *indent << "}";
    break;
  }

  case json::kind::array:
  {
    os << "[\n";
    indent->append(4, ' ');
    auto const& arr = jv.get_array();
    if (!arr.empty())
    {
      auto it = arr.begin();
      for (;;)
      {
        os << *indent;
        JSONtoFile(os, *it, indent);
        if (++it == arr.end())
          break;
        os << ",\n";
      }
    }
    os << "\n";
    indent->resize(indent->size() - 4);
    os << *indent << "]";
    break;
  }

  case json::kind::string:
  {
    os << json::serialize(jv.get_string());
    break;
  }

  case json::kind::uint64:
    os << jv.get_uint64();
    break;

  case json::kind::int64:
    os << jv.get_int64();
    break;

  case json::kind::double_:
    os << boost::format("%7.6f") % jv.get_double();
    break;

  case json::kind::bool_:
    if (jv.get_bool())
      os << "true";
    else
      os << "false";
    break;

  case json::kind::null:
    os << "null";
    break;
  }

  if (indent->empty())
    os << "\n";
}

bool isMember(const std::vector<const char*> vec, std::string str)
{
  return std::find(vec.begin(), vec.end(), str) != vec.end();
}

void setValue(json::value v, std::string key, mdm_Image3D& img, bool extra_keys_warning = false)
{
  try
  {
    switch (v.kind())
    {
    case json::kind::double_:
      img.setMetaData(key, v.get_double());
      break;
    case json::kind::uint64:
      img.setMetaData(key, v.get_uint64());
      break;
    case json::kind::int64:
      img.setMetaData(key, v.get_int64());
      break;
    case json::kind::bool_:
      img.setMetaData(key, v.get_bool());
      break;
    default:
      if (extra_keys_warning)
        mdm_ProgramLogger::logProgramWarning(__func__,
          "Key in image JSON file, with value type ignored by Madym: " + key);
    }
  }
  catch (mdm_exception& e)
  {
    if (extra_keys_warning)
      mdm_ProgramLogger::logProgramWarning(__func__,
        "Extra key in image JSON file, ignored by Madym: " + key);
  }
}

 //
MDM_API void mdm_BIDSFormat::readImageJSON(const std::string &fileName,
  mdm_Image3D &img)
{
  auto jsonFileName = parseJSONExt(fileName, ".json");

  std::ifstream t(jsonFileName);
  std::stringstream buffer;
  buffer << t.rdbuf();
  std::string jsonText;
  auto jv = json::parse(buffer.str());

  auto const& obj = jv.get_object();
  if (!obj.empty())
  {
    for (auto it = obj.begin(); it < obj.end(); ++it)
      setValue(it->value(), it->key().to_string(), img);
  }

  img.setMetaDataSource(jsonFileName);
}

//
MDM_API void mdm_BIDSFormat::readImageJSON(const std::string& baseName,
  std::vector<mdm_Image3D>& imgs)
{
  auto jsonFileName = parseJSONExt(baseName, ".json");
  std::ifstream t(jsonFileName);
  std::stringstream buffer;
  buffer << t.rdbuf();
  std::string jsonText;
  auto jv = json::parse(buffer.str());

  auto arrayKeys = { "FlipAngles", "RepetitionTimes", "EchoTimes", "InversionTimes", "DynamicTimes"};
  auto nImages = imgs.size();

  auto const& obj = jv.get_object();
  json::array dynTimes;
  if (!obj.empty())
  {
    for (auto it = obj.begin(); it < obj.end(); ++it)
    {
      const auto key = it->key().to_string();
      const auto value = it->value();

      //Try to set array values that distribute over images
      if (isMember(arrayKeys, it->key().to_string()))
      {
        auto keyArray = value.as_array();
        //Array size must match number of images
        if (keyArray.size() != nImages)
          throw mdm_exception(__func__, boost::format(
            "Error reading %1% in %2%, size of array (%3%) does not match expected number of images (%4%)")
            % it->key().to_string() % jsonFileName % keyArray.size() % nImages);

        if (key == "DynamicTimes")
        {
          //Defer saving dynamic times to image header until all fields have been read. This ensures
          //the first image timestamp won't override the set times
          dynTimes = keyArray;
          
        }
        else
        {
          const auto key_single = key.substr(0, key.size() - 1);
          //Set each image with matching value from array
          for (size_t i = 0; i < nImages; i++)
            setValue(keyArray[i], key_single, imgs[i]);
        }
        
      }
      else
        //Set single value key to all images - what about timestamp?
        for (auto& img : imgs)
          setValue(value, key, img);
    }
      
  }
  for (auto& img : imgs)
    img.setMetaDataSource(jsonFileName);

  //Set dyn times if necessary
  if (!dynTimes.empty())
  {
    //Dynamic times need setting with respect to the timestamp of the first image
    auto secs0 = imgs[0].secondsFromTimeStamp();
    for (size_t i = 1; i < nImages; i++)
      imgs[i].setTimeStampFromSecs(secs0 + dynTimes[i].get_double());
  }

  //Set B-values and gradients if necessary
  auto bValFileName = parseJSONExt(baseName, ".bval");
  auto bVecFileName = parseJSONExt(baseName, ".bvec");
  auto bExists = fs::exists(bValFileName) && fs::exists(bVecFileName);

  if (bExists)
  {
    std::vector< double> bVals;
    std::vector< std::vector< double> > bVecXYZ(3);

    //Open the b-val file and read the contents
    std::ifstream bValFileStream(bValFileName.c_str(), std::ios::in);
    if (!bValFileStream)
      throw mdm_exception(__func__, "Can't open BIDS bval file" + bValFileName);
    
    double bVal;
    for (size_t b = 0; b < nImages; b++)
    {
      bValFileStream >> bVal;
      bVals.push_back(bVal);
    }
    bValFileStream.close();

    //Open the b-Vec file and read the contents
    std::ifstream bVecFileStream(bVecFileName.c_str(), std::ios::in);
    if (!bVecFileStream)
      throw mdm_exception(__func__, "Can't open BIDS bvec file" + bVecFileName);
    double bVec;
    for (size_t ax = 0; ax < 3; ax++)
    {
      for (size_t b = 0; b < nImages; b++)
      {
        bVecFileStream >> bVec;
        bVecXYZ[ax].push_back(bVec);
      }
    }
    bVecFileStream.close();

    //Now loop through images adding the b-Values and gradients to the image meta info
    for (size_t b = 0; b < nImages; b++)
    {
      auto& img_info = imgs[b].info();
      img_info.B.setValue(bVals[b]);
      img_info.gradOriX.setValue(bVecXYZ[0][b]);
      img_info.gradOriY.setValue(bVecXYZ[1][b]);
      img_info.gradOriZ.setValue(bVecXYZ[2][b]);

      //Make sure the image type is set to DWI
      imgs[b].setType(mdm_Image3D::ImageType::TYPE_DWI);
    }

    
  }
  else if (imgs[0].type() == mdm_Image3D::ImageType::TYPE_DWI)
    throw mdm_exception(__func__, "Error reading " + jsonFileName + 
      ": 4D images of type DWI must have a matching .bval and .bvec files");
}

//
void mdm_BIDSFormat::writeImageJSON(const std::string &baseName,
  const mdm_Image3D &img)
{
  json::object imgMeta;

  //Set the time and type
  imgMeta["TimeStamp"] = img.timeStamp();
  imgMeta["ImageType"] = img.type();

  //Get all other set values
  std::vector<std::string> keys;
  std::vector<double> values;
  img.getSetKeyValuePairs(keys, values);

  for (int i = 0; i < keys.size(); i++)
    imgMeta[keys[i]] = values[i];

  auto jsonFileName = parseJSONExt(baseName, ".json");
  std::ofstream jsonStreamOut(jsonFileName.c_str(), std::ios::out);
  JSONtoFile(jsonStreamOut, imgMeta);
  jsonStreamOut.close();
}

//
void mdm_BIDSFormat::writeImageJSON(const std::string& baseName,
  const std::vector<mdm_Image3D>& imgs)
{
  json::object imgMeta;

  //Set the time and type
  const auto& img0 = imgs[0];
  auto img_type = img0.type();
  imgMeta["TimeStamp"] = img0.timeStamp();
  imgMeta["ImageType"] = img_type;

  //Get all other set values
  std::vector<std::string> keys;
  std::vector<double> values;
  imgs[0].getSetKeyValuePairs(keys, values);

  for (int i = 0; i < keys.size(); i++)
    imgMeta[keys[i]] = values[i];

  //Set dynamic times in seconds
  if (img_type == mdm_Image3D::ImageType::TYPE_CAMAP || img_type == mdm_Image3D::ImageType::TYPE_T1DYNAMIC)
  {
    json::array dynTimes(imgs.size(), 0.0);
    auto secs0 = img0.secondsFromTimeStamp();
    for (size_t i = 1; i < imgs.size(); i++)
      dynTimes[i] = imgs[i].secondsFromTimeStamp() - secs0;

    imgMeta["DynamicTimes"] = dynTimes;
  }
  else if (img_type == mdm_Image3D::ImageType::TYPE_DWI)
  {
    //Create B-value and B-vec files - because B-vecs files are written 3 x n (with each
    // axis on 1 line), it is easier to collect them in the first pass loop before writing
     
    // //Open file for B-values
    auto bValFileName = parseJSONExt(baseName, ".bval");
    std::ofstream bValFileStream(bValFileName.c_str(), std::ios::out);
    if (!bValFileStream)
      throw mdm_exception(__func__, "Can't open BIDS bval file" + bValFileName);
    
    std::vector< std::vector<double> > bVecXYZ(3);
    for (const auto& img : imgs)
    {
      //Write b-Val
      bValFileStream << img.info().B.value() << " ";

      //Get B-vecs
      bVecXYZ[0].push_back(img.info().gradOriX.value());
      bVecXYZ[1].push_back(img.info().gradOriY.value());
      bVecXYZ[2].push_back(img.info().gradOriZ.value());
    }
    //Close the b-Val file
    bValFileStream.close();

    //Now on second loop write the b-Vec file
    auto bVecFileName = parseJSONExt(baseName, ".bvec");
    std::ofstream bVecFileStream(bVecFileName.c_str(), std::ios::out);
    if (!bVecFileStream)
      throw mdm_exception(__func__, "Can't open BIDS bvec file" + bVecFileName);
    for (size_t ax = 0; ax < 3; ax++)
    {
      for (size_t b = 0; b < imgs.size(); b++)
        bVecFileStream << bVecXYZ[ax][b] << " ";
      bVecFileStream << '\n';
    }
    bVecFileStream.close();
  }

  auto jsonFileName = parseJSONExt(baseName, ".json");
  std::ofstream jsonStreamOut(jsonFileName.c_str(), std::ios::out);
  JSONtoFile(jsonStreamOut, imgMeta);
  jsonStreamOut.close();
}

//**********************************************************************
//Private 
//**********************************************************************
//
