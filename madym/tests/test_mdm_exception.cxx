#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <vector>
#include <madym/mdm_Image3D.h>
#include <madym/tests/mdm_test_utils.h>
#include <madym/mdm_VolumeAnalysis.h>
#include <madym/dce_models/mdm_DCEModelGenerator.h>

BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_mdm_exception) {
	BOOST_TEST_MESSAGE("======= Testing mdm_exception(s) =======");

  //Test dimension mismatch
  {
    mdm_Image3D ROI;
    ROI.setDimensions(1, 1, 1);
    ROI.setVoxelDims(1, 1, 1);

    //Reference image has size 1, 1, 1
    mdm_VolumeAnalysis v;
    v.setROI(ROI);

    //Trying to set an image of any other dimension should throw a mismatch exception
    mdm_Image3D img;
    img.setDimensions(1, 1, 2);
    img.setVoxelDims(1, 1, 1);

    BOOST_CHECK_THROW(v.setAIFmap(img), mdm_dimension_mismatch);
    BOOST_CHECK_THROW(v.addStDataMap(img), mdm_dimension_mismatch);
    BOOST_CHECK_THROW(v.addCtDataMap(img), mdm_dimension_mismatch);
    BOOST_CHECK_THROW(v.T1Mapper().addInputImage(img), mdm_dimension_mismatch);
    BOOST_CHECK_THROW(v.T1Mapper().setM0(img), mdm_dimension_mismatch);
    BOOST_CHECK_THROW(v.T1Mapper().setT1(img), mdm_dimension_mismatch);

    //Now mismatch voxel sizes
    img.setDimensions(1, 1, 1);
    img.setVoxelDims(1, 1, 2);
    BOOST_CHECK_THROW(v.setAIFmap(img), mdm_voxelsize_mismatch);
    BOOST_CHECK_THROW(v.addStDataMap(img), mdm_voxelsize_mismatch);
    BOOST_CHECK_THROW(v.addCtDataMap(img), mdm_voxelsize_mismatch);
    BOOST_CHECK_THROW(v.T1Mapper().addInputImage(img), mdm_voxelsize_mismatch);
    BOOST_CHECK_THROW(v.T1Mapper().setM0(img), mdm_voxelsize_mismatch);
    BOOST_CHECK_THROW(v.T1Mapper().setT1(img), mdm_voxelsize_mismatch);
  }

  //Test some things we're not allowed to do in volume analysis
  {
    mdm_VolumeAnalysis v;
    mdm_Image3D img;
    img.setDimensions(1, 1, 1);

    //Stuff we can't do before we've set any dynamics
    {
      //Get a map/time we havent set
      BOOST_CHECK_THROW(v.CtDataMap(0), mdm_exception);
      BOOST_CHECK_THROW(v.StDataMap(0), mdm_exception);
      BOOST_CHECK_THROW(v.dynamicTime(0), mdm_exception);

      //Compute an AIF
      BOOST_CHECK_THROW(v.AIFfromMap(), mdm_exception);

      //Fit a model
      BOOST_CHECK_THROW(v.fitDCEModel(), mdm_exception);
    }
    //Now add a map
    BOOST_CHECK_NO_THROW(v.addStDataMap(img));

    //Stuff we can't do before we've set a model
    {
      //Get/set a DCE map
      BOOST_CHECK_THROW(v.DCEMap("abc"), mdm_exception);
      BOOST_CHECK_THROW(v.setDCEMap("abc", img), mdm_exception);

      //Fit a model
      BOOST_CHECK_THROW(v.fitDCEModel(), mdm_exception);
    }
    //Now set a model
    auto modelType = mdm_DCEModelGenerator::ModelTypes::ETM;
    mdm_AIF AIF;
    AIF.setAIFType(mdm_AIF::AIF_TYPE::AIF_POP);
    AIF.setPIFType(mdm_AIF::PIF_TYPE::PIF_POP);
    auto model = mdm_DCEModelGenerator::createModel(AIF,
      modelType, {},
      {}, {}, {}, {}, {}, {}, {});

    //Stuff we still can't do...

    //Fit model to signal with no T1 set
    v.setComputeCt(true);
    BOOST_CHECK_THROW(v.fitDCEModel(), mdm_exception);

    //Get/set an unknown map
    BOOST_CHECK_THROW(v.DCEMap("abc"), mdm_exception);
    BOOST_CHECK_THROW(v.setDCEMap("abc", img), mdm_exception);

    //Compute an AIF before we've set an AIF map
    BOOST_CHECK_THROW(v.AIFfromMap(), mdm_exception);
  }

  //Test things we can't do with images
  {
    mdm_Image3D img;
    
    //Attempt to access empty image
    BOOST_CHECK_THROW(img.voxel(0), mdm_exception);
    BOOST_CHECK_THROW(img.setVoxel(0, 1), mdm_exception);
    
    //Attempt to access out of range image
    img.setDimensions(1, 1, 1);
    BOOST_CHECK_THROW(img.voxel(1), mdm_exception);
    BOOST_CHECK_THROW(img.setVoxel(1, 1), mdm_exception);
  }

  //Test things we cant do with AIFs
  {
    mdm_AIF AIF;
  }
	
}

BOOST_AUTO_TEST_SUITE_END() //
