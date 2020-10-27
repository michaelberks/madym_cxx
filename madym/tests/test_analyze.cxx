#include <boost/test/unit_test.hpp>

#include <iostream>
#include <madym/mdm_AIF.h>
#include <madym/tests/mdm_test_utils.h>

#include <madym/mdm_AnalyzeFormat.h>
#include <madym/mdm_Image3D.h>


void test_write_read(const mdm_Image3D &img, const mdm_AnalyzeFormat::Data_type format, const bool sparse)
{
	std::string format_str;
	switch (format)
	{
	case mdm_AnalyzeFormat::DT_UNSIGNED_CHAR:
		format_str = "DT_UNSIGNED_CHAR"; break;
	case mdm_AnalyzeFormat::DT_SIGNED_SHORT:
		format_str = "DT_SIGNED_SHORT"; break;
	case mdm_AnalyzeFormat::DT_SIGNED_INT:
		format_str = "DT_SIGNED_INT"; break;
	case mdm_AnalyzeFormat::DT_FLOAT:
		format_str = "DT_FLOAT"; break;
	case mdm_AnalyzeFormat::DT_DOUBLE:
		format_str = "DT_DOUBLE"; break;
	}
	std::string sparse_str = "";
	if (sparse)
		sparse_str = " - sparse";
	std::string test_str;

	//Double format
	std::string img_name = mdm_test_utils::temp_dir() + "/" + format_str;
	bool success = mdm_AnalyzeFormat::writeImage3D(
		img_name, img, format, mdm_AnalyzeFormat::NO_XTR, sparse);

	mdm_Image3D img_r = mdm_AnalyzeFormat::readImage3D(img_name, false);

	BOOST_TEST_MESSAGE( "Test write: format " + format_str + sparse_str);
	BOOST_CHECK(success);

	BOOST_TEST_MESSAGE("Test read, correct size: format " + format_str + sparse_str);
	BOOST_CHECK_EQUAL(img.numVoxels(), img_r.numVoxels());

	BOOST_TEST_MESSAGE("Test read, correct data: format " + format_str + sparse_str);
	BOOST_CHECK_VECTORS(img.data(), img_r.data());
}

void test_xtr(mdm_Image3D &img)
{
	double FA = 20;
	double TR = 3;
	double TE = 1;
	double time = 123456.789;
	img.info_.flipAngle.setValue(FA);
	img.info_.TR.setValue(TR);
	img.info_.TE.setValue(TE);
	img.setTimeStamp(time);

	std::string img_name = mdm_test_utils::temp_dir() + "/xtr_test";
	bool success = mdm_AnalyzeFormat::writeImage3D(
		img_name, img, mdm_AnalyzeFormat::DT_FLOAT, mdm_AnalyzeFormat::NEW_XTR, false);
	BOOST_TEST_MESSAGE("Testing: xtr write");
	BOOST_CHECK(success);

	mdm_Image3D img_r = mdm_AnalyzeFormat::readImage3D(img_name, true);
	BOOST_TEST_MESSAGE("Tesing xtr read: FA");
	BOOST_CHECK_CLOSE(FA, img_r.info_.flipAngle.value(), 1e-3);//
	BOOST_TEST_MESSAGE("Tesing xtr read: TR");
	BOOST_CHECK_CLOSE(TR, img_r.info_.TR.value(), 1e-3);
	BOOST_TEST_MESSAGE("Tesing xtr read: TE");
	BOOST_CHECK_CLOSE(TE, img_r.info_.TE.value(), 1e-3);
	BOOST_TEST_MESSAGE("Tesing xtr read: timestamp");
	BOOST_CHECK_CLOSE(time, img_r.timeStamp(), 1e-3);
}


BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_analyze) {
	BOOST_TEST_MESSAGE("======= Testing analyze format image reading/writing =======");

	mdm_Image3D img_integer, img_real;
	int nx = 2, ny = 2, nz = 2;
	int n_voxels = nx * ny * nz;
	img_integer.setDimensions(nx, ny, nz);
	img_real.setDimensions(nx, ny, nz);
	img_integer.setVoxelDims(1, 1, 1);
	img_real.setVoxelDims(1, 1, 1);

	std::vector<double> integer_data = { 1, 2, 3, 4, 0, 0, 0, 0 };
	std::vector<double> real_data = { 1.1, 2.2, 3.3, 4.4, 0, 0, 0, 0 };
	
	//Set these data into the images - note we don't need to test
	//the get/set function because these are tested in test_image3D which precedes
	//this
	for (int i = 0; i < n_voxels; i++)
	{
		img_integer.setVoxel(i, integer_data[i]);
		img_real.setVoxel(i, (float)real_data[i]); //Cast to float so we can test against float read
	}
	
	//-----------------------------------------------------------------
	// Test writing out analyze image in various formats
	//-----------------------------------------------------------------
	bool sparse = false;

	//unsigned char (uint8) format
	test_write_read(img_integer, mdm_AnalyzeFormat::DT_UNSIGNED_CHAR, sparse);

	//Short format (int 16)
	test_write_read(img_integer, mdm_AnalyzeFormat::DT_SIGNED_SHORT, sparse);

	//Integer format
	test_write_read(img_integer, mdm_AnalyzeFormat::DT_SIGNED_INT, sparse);

	//Float format
	test_write_read(img_integer, mdm_AnalyzeFormat::DT_FLOAT, sparse);

	//Double format
	test_write_read(img_integer, mdm_AnalyzeFormat::DT_DOUBLE, sparse);

	//-----------------------------------------------------------------
	// Check it works for real valued data
	//-----------------------------------------------------------------
	//Int format - real data, for a sanity check, uncomment this should cause a test fail
	//test_write_read(img_real, mdm_AnalyzeFormat::DT_SIGNED_INT, sparse);
	
	//Float format - real data
	test_write_read(img_real, mdm_AnalyzeFormat::DT_FLOAT, sparse);

	//Double format - real data
	test_write_read(img_real, mdm_AnalyzeFormat::DT_DOUBLE, sparse);

	//-----------------------------------------------------------------
	// Now repeat the tests for sparse writing/reading
	//-----------------------------------------------------------------
	sparse = true;

	//unsigned char (uint8) format
	test_write_read(img_integer, mdm_AnalyzeFormat::DT_UNSIGNED_CHAR, sparse);

	//Short format (int 16)
	test_write_read(img_integer, mdm_AnalyzeFormat::DT_SIGNED_SHORT, sparse);

	//Integer format
	test_write_read(img_integer, mdm_AnalyzeFormat::DT_SIGNED_INT, sparse);

	//Float format
	test_write_read(img_integer, mdm_AnalyzeFormat::DT_FLOAT, sparse);

	//Double format
	test_write_read(img_integer, mdm_AnalyzeFormat::DT_DOUBLE, sparse);

	//Float format - real data
	test_write_read(img_real, mdm_AnalyzeFormat::DT_FLOAT, sparse);

	//Double format - real data
	test_write_read(img_real, mdm_AnalyzeFormat::DT_DOUBLE, sparse);

	//Test for writing/reading xtr file
	test_xtr(img_real);
}

BOOST_AUTO_TEST_SUITE_END() //
