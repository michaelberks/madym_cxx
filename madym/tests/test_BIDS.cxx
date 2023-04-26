#include <boost/test/unit_test.hpp>

#include <iostream>
#include <madym/tests/mdm_test_utils.h>
#include <madym/utils/mdm_Image3D.h>
#include <madym/image_io/meta/mdm_BIDSFormat.h>
#include <madym/utils/mdm_exception.h>

void test_json(mdm_Image3D& img)
{
	double FA = 20;
	double TR = 3;
	double TE = 1;
	double time = 123456.789;
	img.info().flipAngle.setValue(FA);
	img.info().TR.setValue(TR);
	img.info().TE.setValue(TE);
	img.setTimeStampFromDoubleStr(time);
	img.setType(mdm_Image3D::ImageType::TYPE_DEGR);

	std::string jsonFileName = mdm_test_utils::temp_dir() + "/test.json";

	BOOST_TEST_MESSAGE("Tesing BIDS JSON write:");
	BOOST_CHECK_NO_THROW(mdm_BIDSFormat::writeImageJSON(jsonFileName, img));

	mdm_Image3D img_r;
  mdm_BIDSFormat::readImageJSON(jsonFileName, img_r);

	BOOST_TEST_MESSAGE("Tesing JSON read: FA");
	BOOST_CHECK_CLOSE(FA, img_r.info().flipAngle.value(), 1e-3);//
	BOOST_TEST_MESSAGE("Tesing JSON read: TR");
	BOOST_CHECK_CLOSE(TR, img_r.info().TR.value(), 1e-3);
	BOOST_TEST_MESSAGE("Tesing JSON read: TE");
	BOOST_CHECK_CLOSE(TE, img_r.info().TE.value(), 1e-3);
	BOOST_TEST_MESSAGE("Tesing JSON read: timestamp");
	BOOST_CHECK_CLOSE(time, img_r.timeStamp(), 1e-3);
	BOOST_TEST_MESSAGE("Tesing JSON read: image type");
	BOOST_CHECK_EQUAL(mdm_Image3D::ImageType::TYPE_DEGR, img_r.type());
	
}

BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_BIDS) {
	BOOST_TEST_MESSAGE("======= Testing BIDS format image metadata reading/writing =======");

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

	//Test for writing/reading xtr file
	test_json(img_real);
}

BOOST_AUTO_TEST_SUITE_END() //
