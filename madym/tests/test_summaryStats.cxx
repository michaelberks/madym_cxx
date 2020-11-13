#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <vector>
#include <madym/mdm_Image3D.h>
#include <madym/mdm_ParamSummaryStats.h>
#include <madym/tests/mdm_test_utils.h>

BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_summaryStats) {
	BOOST_TEST_MESSAGE("======= Testing class mdm_ParamSummaryStats =======");

	mdm_Image3D img;
	int nx = 5, ny = 1, nz = 1;
	img.setDimensions(nx, ny, nz);

	for (int i = 0; i < nx; i++)
		img.setVoxel(i, i + 1);

	//Create stats object
	mdm_ParamSummaryStats stats; 

	//Don't set ROI, it should use all voxels
	stats.makeStats(img, "dummy");

	//Check the stats values - data array is {1, 2, 3, 4, 5}
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().mean_, 3.0, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: stddev");
	BOOST_CHECK_CLOSE(stats.stats().stddev_, 1.5811, 0.1);
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().median_, 3.0, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: lowerQ");
	BOOST_CHECK_CLOSE(stats.stats().lowerQ_, 1.5, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().upperQ_, 4.5, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().iqr_, 3.0, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: validVoxels");
	BOOST_CHECK_EQUAL(stats.stats().validVoxels_, 5);
	BOOST_TEST_MESSAGE("Test whole ROI: invalidVoxels");
	BOOST_CHECK_EQUAL(stats.stats().invalidVoxels_, 0);

	//Repeat the test with scale set
	double scale = 2.0;
	stats.makeStats(img, "dummy", scale);
	
	//data array is{ 2, 4, 6, 8, 10 }
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().mean_, scale*3.0, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: stddev");
	BOOST_CHECK_CLOSE(stats.stats().stddev_, scale*1.5811, 0.1);
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().median_, scale*3.0, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: lowerQ");
	BOOST_CHECK_CLOSE(stats.stats().lowerQ_, scale*1.5, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().upperQ_, scale*4.5, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().iqr_, scale*3.0, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: validVoxels");
	BOOST_CHECK_EQUAL(stats.stats().validVoxels_, 5);
	BOOST_TEST_MESSAGE("Test whole ROI: invalidVoxels");
	BOOST_CHECK_EQUAL(stats.stats().invalidVoxels_, 0);

	//Now set an ROI and repeat the test
	mdm_Image3D roi;
	roi.setDimensions(nx, ny, nz);

	for (int i = 0; i < 4; i++)
		roi.setVoxel(i, 1);

	stats.setROI(roi);
	stats.makeStats(img, "dummy");
	
	//Should now only use {1, 2, 3, 4}
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().mean_, 2.5, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: stddev");
	BOOST_CHECK_CLOSE(stats.stats().stddev_, 1.2910, 0.1);
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().median_, 2.5, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: lowerQ");
	BOOST_CHECK_CLOSE(stats.stats().lowerQ_, 1.25, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().upperQ_, 3.75, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().iqr_, 2.5, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: validVoxels");
	BOOST_CHECK_EQUAL(stats.stats().validVoxels_, 4);
	BOOST_TEST_MESSAGE("Test whole ROI: invalidVoxels");
	BOOST_CHECK_EQUAL(stats.stats().invalidVoxels_, 0);

	//Set some negative values
	for (int i = 0; i < 3; i++)
		img.setVoxel(i, 1.0 / double(i + 1));

	img.setVoxel(3, -1);
	stats.makeStats(img, "dummy", 1.0, true);

	//Should now only use {1, 2, 3}
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().mean_, 2.0, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: stddev");
	BOOST_CHECK_CLOSE(stats.stats().stddev_, 1, 0.1);
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().median_, 2.0, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: lowerQ");
	BOOST_CHECK_CLOSE(stats.stats().lowerQ_, 1.0, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().upperQ_, 3.0, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: median");
	BOOST_CHECK_CLOSE(stats.stats().iqr_, 2.0, 0.00001);
	BOOST_TEST_MESSAGE("Test whole ROI: validVoxels");
	BOOST_CHECK_EQUAL(stats.stats().validVoxels_, 3);
	BOOST_TEST_MESSAGE("Test whole ROI: invalidVoxels");
	BOOST_CHECK_EQUAL(stats.stats().invalidVoxels_, 1);
}

BOOST_AUTO_TEST_SUITE_END() //
