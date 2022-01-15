#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <vector>
#include <madym/mdm_Image3D.h>
#include <madym/tests/mdm_test_utils.h>
#include <madym/mdm_VolumeAnalysis.h>
#include <madym/dce/mdm_DCEModelGenerator.h>

BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_volumeAnalysis) {
	BOOST_TEST_MESSAGE("======= Testing mdm_exception(s) =======");

  //Test setting volumes
  mdm_Image3D ROI;
  ROI.setDimensions(1, 1, 1);
  ROI.setVoxelDims(1, 1, 1);

  //Reference image has size 1, 1, 1
  mdm_VolumeAnalysis v;
  v.setROI(ROI);

  //Trying to set an image of any other dimension should throw a mismatch exception
  mdm_Image3D img;
  img.setDimensions(1, 1, 1);
  img.setVoxelDims(1, 1, 1);

  //Check setting of volumes
  v.setComputeCt(true);
  v.setOutputCtSig(true);
  v.setOutputCtMod(true);
  BOOST_CHECK_NO_THROW(v.setAIFmap(img));
  BOOST_CHECK_NO_THROW(v.addStDataMap(img));
  BOOST_CHECK_NO_THROW(v.T1Mapper().addInputImage(img));
  BOOST_CHECK_NO_THROW(v.T1Mapper().setM0(img));
  BOOST_CHECK_NO_THROW(v.T1Mapper().setT1(img));
  BOOST_CHECK(v.numDynamics() == 1);

  auto modelType = mdm_DCEModelGenerator::ModelTypes::ETM;
  mdm_AIF AIF;
  AIF.setAIFType(mdm_AIF::AIF_TYPE::AIF_POP);
  AIF.setPIFType(mdm_AIF::PIF_TYPE::PIF_POP);
  auto model = mdm_DCEModelGenerator::createModel(AIF,
    modelType, {},
    {}, {}, {}, {}, {}, {}, {});

  BOOST_CHECK_NO_THROW(v.setModel(model));
  BOOST_CHECK_NO_THROW(v.setDCEMap(model->paramName(0), img));

  //Check getting of volumes - dimensions match
  BOOST_CHECK(img.dimensionsMatch(v.ROI()));
  BOOST_CHECK(img.dimensionsMatch(v.AIFmap()));
  BOOST_CHECK(img.dimensionsMatch(v.StDataMap(0)));
  BOOST_CHECK(img.dimensionsMatch(v.CtDataMap(0)));
  BOOST_CHECK(img.dimensionsMatch(v.CtModelMap(0)));
  BOOST_CHECK(img.dimensionsMatch(v.T1Mapper().inputImage(0)));
  BOOST_CHECK(img.dimensionsMatch(v.T1Mapper().M0()));
  BOOST_CHECK(img.dimensionsMatch(v.T1Mapper().T1()));

  //Check getting of volumes - voxel sizes match
  BOOST_CHECK(img.voxelSizesMatch(v.ROI()));
  BOOST_CHECK(img.voxelSizesMatch(v.AIFmap()));
  BOOST_CHECK(img.voxelSizesMatch(v.StDataMap(0)));
  BOOST_CHECK(img.voxelSizesMatch(v.CtDataMap(0)));
  BOOST_CHECK(img.voxelSizesMatch(v.CtModelMap(0)));
  BOOST_CHECK(img.voxelSizesMatch(v.T1Mapper().inputImage(0)));
  BOOST_CHECK(img.voxelSizesMatch(v.T1Mapper().M0()));
  BOOST_CHECK(img.voxelSizesMatch(v.T1Mapper().T1()));

  //Check setting of values - these all set with no get, so just check no throw
  BOOST_CHECK_NO_THROW(v.setR1Const(5.0));
  BOOST_CHECK_NO_THROW(v.setPrebolusImage(10));
  BOOST_CHECK_NO_THROW(v.setTestEnhancement(false));
  BOOST_CHECK_NO_THROW(v.setM0Ratio(false));
  BOOST_CHECK_NO_THROW(v.setComputeCt(false));
  BOOST_CHECK_NO_THROW(v.setOutputCtSig(true));
  BOOST_CHECK_NO_THROW(v.setOutputCtMod(true));
  
  BOOST_CHECK_NO_THROW(v.setUseNoise(true));
  BOOST_CHECK_NO_THROW(v.setFirstImage(10));
  BOOST_CHECK_NO_THROW(v.setLastImage(10));
  BOOST_CHECK_NO_THROW(v.setMaxIterations(50));
  
  //Tests of actually using volumeAnalysis are performed as part
  //of test_mdm_tools
	
}

BOOST_AUTO_TEST_SUITE_END() //
