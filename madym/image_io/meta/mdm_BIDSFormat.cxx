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

void setValue(json::value v, std::string key, mdm_Image3D& img)
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
      mdm_ProgramLogger::logProgramWarning(__func__,
        "Key in image JSON file, with value type ignored by Madym: " + key);
    }
  }
  catch (mdm_exception& e)
  {
    mdm_ProgramLogger::logProgramWarning(__func__,
      "Extra key in image JSON file, ignored by Madym: " + key);
  }
}

 //
MDM_API void mdm_BIDSFormat::readImageJSON(const std::string &jsonFileName,
  mdm_Image3D &img)
{
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
MDM_API void mdm_BIDSFormat::readImageJSON(const std::string& jsonFileName,
  std::vector<mdm_Image3D>& imgs)
{
  std::ifstream t(jsonFileName);
  std::stringstream buffer;
  buffer << t.rdbuf();
  std::string jsonText;
  auto jv = json::parse(buffer.str());

  auto arrayKeys = { "FlipAngles", "RepetitionTimes", "EchoTimes", "InversionTimes", "DynamicTimes"};
  auto nImages = imgs.size();

  auto const& obj = jv.get_object();
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
            "Error reading %1%, size of DynamicTimes array (%2%) does not match expected number of images (%3%)")
            % jsonFileName % keyArray.size() % nImages);

        if (key == "DynamicTimes")
        {
          //Dynamic times need setting with respect to the timestamp of the first image
          //but what if this is not set?
          auto secs0 = imgs[0].secondsFromTimeStamp();
          for (size_t i = 1; i < nImages; i++)
            imgs[i].setTimeStampFromSecs(secs0 + keyArray[i].get_double());
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

  std::ofstream jsonStreamOut(baseName.c_str(), std::ios::out);
  JSONtoFile(jsonStreamOut, imgMeta);
  jsonStreamOut.close();
}

//
void mdm_BIDSFormat::writeImageJSON(const std::string& baseName,
  const std::vector<mdm_Image3D>& imgs)
{
  json::object imgMeta;

  /*;

  if (img.info().flipAngle.isSet())
    imgMeta["FlipAngle"] = img.info().flipAngle.value();

  if (img.info().TR.isSet())
    imgMeta["RepetitionTime"] = img.info().TR.value();

  if (img.info().TE.isSet())
    imgMeta["EchoTime"] = img.info().TE.value();

  if (img.info().TI.isSet())
    imgMeta["InversionTime"] = img.info().TI.value();

  if (img.info().Xmm.isSet())
    imgMeta["VoxelSize"] = {
        img.info().Xmm.value(),
        img.info().Ymm.value(),
        img.info().Zmm.value()};
    imgMeta["VoxelUnits"] = "mm";

  if (img.info().rowDirCosX.isSet())
    imgMeta["RowDirection"] = {
        img.info().rowDirCosX.value(),
        img.info().rowDirCosY.value(),
        img.info().rowDirCosZ.value()};

  if (img.info().colDirCosX.isSet())
    imgMeta["ColDirection"] = {
        img.info().colDirCosX.value(),
        img.info().colDirCosY.value(),
        img.info().colDirCosZ.value()};

  if (img.info().flipX.isSet())
    imgMeta["Flip"] = {
        img.info().flipX.value(),
        img.info().flipY.value(),
        img.info().flipZ.value()};

  if (img.info().zDirection.isSet())
    imgMeta["zDirection"] = img.info().zDirection.value();

  if (img.info().sclSlope.isSet())
    imgMeta["sclSlope"] = img.info().sclSlope.value();

  if (img.info().sclInter.isSet())
    imgMeta["sclInter"] = img.info().sclInter.value();*/

    //Set the time and type
  const auto& img = imgs[0];
  imgMeta["TimeStamp"] = img.timeStamp();
  imgMeta["ImageType"] = img.type();

  //Get all other set values
  std::vector<std::string> keys;
  std::vector<double> values;
  imgs[0].getSetKeyValuePairs(keys, values);

  for (int i = 0; i < keys.size(); i++)
    imgMeta[keys[i]] = values[i];

  //Set dynamic times in seconds
  json::array dynTimes(imgs.size(), 0.0);
  auto secs0 = img.secondsFromTimeStamp();
  for (size_t i = 1; i < imgs.size(); i++)
    dynTimes[i] = imgs[i].secondsFromTimeStamp() - secs0;
    
  imgMeta["DynamicTimes"] = dynTimes;

  std::ofstream jsonStreamOut(baseName.c_str(), std::ios::out);
  JSONtoFile(jsonStreamOut, imgMeta);
  jsonStreamOut.close();
}

//**********************************************************************
//Private 
//**********************************************************************
//
