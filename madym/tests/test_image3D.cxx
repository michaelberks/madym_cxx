#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <vector>
#include <madym/mdm_Image3D.h>
#include <madym/tests/mdm_test_utils.h>

BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_image3D) {
	BOOST_TEST_MESSAGE("======= Testing class mdm_Image3D =======");

	mdm_Image3D img;
	int nx = 2, ny = 2, nz = 2;
	img.setDimensions(nx, ny, nz);

	auto nVoxels = img.numVoxels();
	BOOST_TEST_MESSAGE("Image initialised");
	BOOST_CHECK_EQUAL(nVoxels, nx*ny*nz);

	std::vector<double> data_in = { 1, 2, 3, 4, 1.1, 2.2, 3.3, 4.4 };
	std::vector<double> data_out(nVoxels);

	//Run a simple test that the value we set to each voxel is
	//obtained if we then call get voxel
	for (int i = 0; i < nVoxels; i++)
		img.setVoxel(i, data_in[i]);

	for (int i = 0; i < nVoxels; i++)
		data_out[i] = img.voxel(i);
	
	BOOST_TEST_MESSAGE("Data set / get per voxel");
	BOOST_CHECK_VECTORS(data_in, data_out);

	//Also test against the const ref to the data returned
	BOOST_TEST_MESSAGE("Data get ref to data");
	BOOST_CHECK_VECTORS(img.data(), data_in);

	//TODO: We probably we want to add more tests here for setting of voxel
	//dimensions, setting meta-info etc.
}

BOOST_AUTO_TEST_SUITE_END() //
