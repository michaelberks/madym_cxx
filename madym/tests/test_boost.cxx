/**
 *
 */
#include <testlib/testlib_test.h>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include "mdm_test_utils.h"

using namespace boost::filesystem;

/**
 */
void run_test_boost()
{
  std::string dir_name = "test_boost";

	std::cout << "======= Testing boost library =======" << std::endl;
	
	//Using boost filesyetm, can call one line to make absolute path from input
	//regardless of whether relative or absolute path has been given
	path dir_path = mdm_test_utils::temp_dir() + "/test_boost";

	//We probably don't need to check if directory exists, just call create... regardless
	//but we do so anyway
	bool created = create_directories(dir_path);
	bool exists = is_directory(dir_path);
	bool deleted = boost::filesystem::remove(dir_path);
	bool still_exists = is_directory(dir_path);

	TEST("Test directory created", created, true);
	TEST("Test directory exists", exists, true);
	TEST("Test directory deleted", deleted, true);
	TEST("Test directory no longer exists", still_exists, false);

}

void test_boost()
{
	run_test_boost();
}

TESTMAIN(test_boost);