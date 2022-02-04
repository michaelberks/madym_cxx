#include <boost/test/unit_test.hpp>

#include <iostream>
#include <madym/dce/mdm_AIF.h>
#include <madym/tests/mdm_test_utils.h>

#include <madym/image_io/mdm_ImageDatatypes.h>
#include <madym/image_io/nifti/mdm_NiftiFormat.h>
#include <madym/utils/mdm_Image3D.h>
#include <madym/utils/mdm_exception.h>


void test_nifti_io(const mdm_Image3D &img, const mdm_ImageDatatypes::DataType format, const bool compress)
{
	std::string format_str;
	switch (format)
	{
	case mdm_ImageDatatypes::DT_UNSIGNED_CHAR:
		format_str = "DT_UNSIGNED_CHAR"; break;
	case mdm_ImageDatatypes::DT_SIGNED_SHORT:
		format_str = "DT_SIGNED_SHORT"; break;
	case mdm_ImageDatatypes::DT_SIGNED_INT:
		format_str = "DT_SIGNED_INT"; break;
	case mdm_ImageDatatypes::DT_FLOAT:
		format_str = "DT_FLOAT"; break;
	case mdm_ImageDatatypes::DT_DOUBLE:
		format_str = "DT_DOUBLE"; break;
	}
	std::string compress_str = "";
  std::string extgz = "";
  if (compress)
  {
    compress_str = " - compress";
    extgz = ".gz";
  }
    
	std::string test_str;

	//Double format
	std::string img_name = mdm_test_utils::temp_dir() + "/" + format_str;

  BOOST_TEST_MESSAGE("Test write: format " + format_str + compress_str);
	BOOST_CHECK_NO_THROW(mdm_NiftiFormat::writeImage3D(
		img_name, img, format, mdm_XtrFormat::NO_XTR, compress));


	mdm_Image3D img_r = mdm_NiftiFormat::readImage3D(img_name, false);

	BOOST_TEST_MESSAGE("Test read, correct size: format " + format_str + compress_str);
	BOOST_CHECK_EQUAL(img.numVoxels(), img_r.numVoxels());

	BOOST_TEST_MESSAGE("Test read, correct data: format " + format_str + compress_str);
	BOOST_CHECK_VECTORS(img.data(), img_r.data());

  boost::filesystem::remove(img_name + ".nii" + extgz);
}


BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_nifti) {
	BOOST_TEST_MESSAGE("======= Testing NIFTI format image reading/writing =======");

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
	bool compress = false;

	//unsigned char (uint8) format
	test_nifti_io(img_integer, mdm_ImageDatatypes::DT_UNSIGNED_CHAR, compress);

	//Short format (int 16)
	test_nifti_io(img_integer, mdm_ImageDatatypes::DT_SIGNED_SHORT, compress);

	//Integer format
	test_nifti_io(img_integer, mdm_ImageDatatypes::DT_SIGNED_INT, compress);

	//Float format
	test_nifti_io(img_integer, mdm_ImageDatatypes::DT_FLOAT, compress);

	//Double format
	test_nifti_io(img_integer, mdm_ImageDatatypes::DT_DOUBLE, compress);

	//-----------------------------------------------------------------
	// Check it works for real valued data
	//-----------------------------------------------------------------
	//Int format - real data, for a sanity check, uncomment this should cause a test fail
	//test_nifti_io(img_real, mdm_ImageDatatypes::DT_SIGNED_INT, compress);
	
	//Float format - real data
	test_nifti_io(img_real, mdm_ImageDatatypes::DT_FLOAT, compress);

	//Double format - real data
	test_nifti_io(img_real, mdm_ImageDatatypes::DT_DOUBLE, compress);

  //-----------------------------------------------------------------
  // Now repeat the tests for sparse writing/reading
  //-----------------------------------------------------------------
#ifdef HAVE_ZLIB
  compress = true;

  //unsigned char (uint8) format
  test_nifti_io(img_integer, mdm_ImageDatatypes::DT_UNSIGNED_CHAR, compress);

  //Short format (int 16)
  test_nifti_io(img_integer, mdm_ImageDatatypes::DT_SIGNED_SHORT, compress);

  //Integer format
  test_nifti_io(img_integer, mdm_ImageDatatypes::DT_SIGNED_INT, compress);

  //Float format
  test_nifti_io(img_integer, mdm_ImageDatatypes::DT_FLOAT, compress);

  //Double format
  test_nifti_io(img_integer, mdm_ImageDatatypes::DT_DOUBLE, compress);

  //Float format - real data
  test_nifti_io(img_real, mdm_ImageDatatypes::DT_FLOAT, compress);

  //Double format - real data
  test_nifti_io(img_real, mdm_ImageDatatypes::DT_DOUBLE, compress);
#endif
}

BOOST_AUTO_TEST_SUITE_END() //
