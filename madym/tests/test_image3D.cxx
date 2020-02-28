#include <testlib/testlib_test.h>
#include <vcl_compiler.h>
#include <iostream>
#include <vector>
#include <madym/mdm_Image3D.h>

void run_test_image3D()
{

	std::cout << "======= Testing class mdm_Image3D =======" << std::endl;

	mdm_Image3D img;
	int nx = 2, ny = 2, nz = 2;
	img.setMatrixDims(nx, ny, nz);

	int nVoxels = img.getNvoxels();
	TEST("Image initialised", nVoxels, nx*ny*nz);

	std::vector<double> data_in = { 1, 2, 3, 4, 1.1, 2.2, 3.3, 4.4 };
	std::vector<double> data_out(nVoxels);

	//Run a simple test that the value we set to each voxel is
	//obtained if we then call get voxel
	for (int i = 0; i < nVoxels; i++)
		img.setVoxel(i, data_in[i]);

	for (int i = 0; i < nVoxels; i++)
		data_out[i] = img.getVoxel(i);
	
	TEST("Data set/get per voxel", data_in, data_out);

	//Also test against the const ref to the data returned
	TEST("Data get ref to data", img.getData(), data_in);

	//TODO: We probably we want to add more tests here for setting of voxel
	//dimensions, setting meta-info etc.
}

void test_image3D()
{
	run_test_image3D();
}

TESTMAIN(test_image3D);
