
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <madym/tests/mdm_test_utils.h>
#include <madym/utils/mdm_Image3D.h>
#include <madym/image_io/meta/mdm_BIDSFormat.h>
#include <madym/image_io/nifti/mdm_NiftiFormat.h>
#include <madym/utils/mdm_exception.h>

void test_json(mdm_Image3D& img)
{
	auto img_oe = mdm_NiftiFormat::readImage3D("Q:/data/MB/BIDS_bruker_test/20210705_123459_210363_1_1/IR/OE_400.nii.gz", true, false);

	double FA = 20;
	double TR = 3;
	double TE = 1;
	double time = 123456.789;
	img.info().flipAngle.setValue(FA);
	img.info().TR.setValue(TR);
	img.info().TE.setValue(TE);
	img.setTimeStampFromDoubleStr(time);
	img.setType(mdm_Image3D::ImageType::TYPE_T1WTSPGR);

	std::string jsonFileName = mdm_test_utils::temp_dir() + "/test";

	BOOST_TEST_MESSAGE("Testing BIDS JSON write:");
	BOOST_CHECK_NO_THROW(mdm_BIDSFormat::writeImageJSON(jsonFileName, img));

	mdm_Image3D img_r;
  mdm_BIDSFormat::readImageJSON(jsonFileName, img_r);

	BOOST_TEST_MESSAGE("Testing JSON read: FA");
	BOOST_CHECK_CLOSE(FA, img_r.info().flipAngle.value(), 1e-3);//
	BOOST_TEST_MESSAGE("Testing JSON read: TR");
	BOOST_CHECK_CLOSE(TR, img_r.info().TR.value(), 1e-3);
	BOOST_TEST_MESSAGE("Testing JSON read: TE");
	BOOST_CHECK_CLOSE(TE, img_r.info().TE.value(), 1e-3);
	BOOST_TEST_MESSAGE("Testing JSON read: timestamp");
	BOOST_CHECK_CLOSE(time, img_r.timeStamp(), 1e-3);
	BOOST_TEST_MESSAGE("Testing JSON read: image type");
	BOOST_CHECK_EQUAL(mdm_Image3D::ImageType::TYPE_T1WTSPGR, img_r.type());

	//Now try the same writing the whole image
	std::string img_name = mdm_test_utils::temp_dir() + "/BIDS_img";
	BOOST_CHECK_NO_THROW(mdm_NiftiFormat::writeImage3D(
		img_name, img, mdm_ImageDatatypes::DT_FLOAT, mdm_XtrFormat::BIDS, false, false));

	//Read in the image and check the voxel values are scaled correctly

	//With scaling on read, the voxel values should match the written image 
	auto img_r2 = mdm_NiftiFormat::readImage3D(img_name, true, false);
	BOOST_CHECK(mdm_test_utils::vectors_near_equal(img.data(), img_r2.data(), 1e-3));
	BOOST_TEST_MESSAGE("Testing BIDS read: FA");
	BOOST_CHECK_CLOSE(FA, img_r2.info().flipAngle.value(), 1e-3);//
	BOOST_TEST_MESSAGE("Testing BIDS read: TR");
	BOOST_CHECK_CLOSE(TR, img_r2.info().TR.value(), 1e-3);
	BOOST_TEST_MESSAGE("Testing BIDS read: TE");
	BOOST_CHECK_CLOSE(TE, img_r2.info().TE.value(), 1e-3);
	BOOST_TEST_MESSAGE("Testing BIDS read: timestamp");
	BOOST_CHECK_CLOSE(time, img_r2.timeStamp(), 1e-3);
	BOOST_TEST_MESSAGE("Testing BIDS read: image type");
	BOOST_CHECK_EQUAL(mdm_Image3D::ImageType::TYPE_T1WTSPGR, img_r2.type());
}

void test_json_dynTimes(mdm_Image3D& img)
{
	int nTimes = 10;
	std::vector<double> dynTimes(nTimes);
	std::vector< mdm_Image3D > imgs(nTimes);

	for (int t = 0; t < nTimes; t++)
	{
		imgs[t] = mdm_Image3D(img);
		imgs[t].setType(mdm_Image3D::ImageType::TYPE_T1DYNAMIC);

		if (t)
		{
			dynTimes[t] = t * 6;
			auto timestamp_secs = imgs[0].secondsFromTimeStamp() + dynTimes[t];
			imgs[t].setTimeStampFromSecs(timestamp_secs);
			imgs[t].setVoxel(0, dynTimes[t]);
		}
		else
		{
			imgs[t].setTimeStampFromNow();
		}
		
	}

	std::string img_name = mdm_test_utils::temp_dir() + "/BIDS_img_4D";
	BOOST_CHECK_NO_THROW(mdm_NiftiFormat::writeImage4D(
		img_name, imgs, mdm_ImageDatatypes::DT_FLOAT, mdm_XtrFormat::BIDS, false, false));

	auto imgs2 = mdm_NiftiFormat::readImage4D(img_name, true, false);
	std::vector<double> dynTimes2(nTimes);
	auto t0 = imgs2[0].secondsFromTimeStamp();

	BOOST_TEST_MESSAGE("Testing BIDS read 4D: voxel data");
	for (int t = 0; t < nTimes; t++)
	{
		dynTimes2[t] = imgs2[t].secondsFromTimeStamp() - t0;
		BOOST_CHECK(mdm_test_utils::vectors_near_equal(imgs[t].data(), imgs2[t].data(), 1e-3));
	}
	BOOST_TEST_MESSAGE("Testing BIDS read 4D: dynamic times");
	BOOST_CHECK(mdm_test_utils::vectors_near_equal(dynTimes, dynTimes2, 1e-3));
}

void test_json_DWI(mdm_Image3D& img)
{
	std::vector< double > bVals = { 0, 100, 300 };
	std::vector< double > bVecX = { 0, 0.3303, -0.5380 };
	std::vector< double > bVecY = { 0, 0.6579, -0.6764 };
	std::vector< double > bVecZ = { 0, 0.6768, -0.5030 };
	auto nBs = bVals.size();
	std::vector< mdm_Image3D > imgs(nBs);
	for (size_t b = 0; b < nBs; b++)
	{
		imgs[b] = mdm_Image3D(img);
		auto& img_info = imgs[b].info();
		img_info.B.setValue(bVals[b]);
		img_info.gradOriX.setValue(bVecX[b]);
		img_info.gradOriY.setValue(bVecY[b]);
		img_info.gradOriZ.setValue(bVecZ[b]);
		imgs[b].setType(mdm_Image3D::ImageType::TYPE_DWI);
	}

	std::string img_name = mdm_test_utils::temp_dir() + "/BIDS_img_DWI";
	BOOST_CHECK_NO_THROW(mdm_NiftiFormat::writeImage4D(
		img_name, imgs, mdm_ImageDatatypes::DT_FLOAT, mdm_XtrFormat::BIDS, false, false));

	auto imgs2 = mdm_NiftiFormat::readImage4D(img_name, true, false);
	BOOST_TEST_MESSAGE("Checking B-values and gradients");
	for (size_t b = 0; b < nBs; b++)
	{
		const auto& img_info1 = imgs[b].info();
		const auto& img_info2 = imgs2[b].info();
		BOOST_CHECK_CLOSE(img_info1.B.value(), img_info2.B.value(), 1e-3);
		BOOST_CHECK_CLOSE(img_info1.gradOriX.value(), img_info2.gradOriX.value(), 1e-3);
		BOOST_CHECK_CLOSE(img_info1.gradOriY.value(), img_info2.gradOriY.value(), 1e-3);
		BOOST_CHECK_CLOSE(img_info1.gradOriZ.value(), img_info2.gradOriZ.value(), 1e-3);
	}
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
	test_json_dynTimes(img_real);
	test_json_DWI(img_real);
}

BOOST_AUTO_TEST_SUITE_END() //
