/**
 *
 */
#include <boost/test/unit_test.hpp>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <madym/tests/mdm_test_utils.h>

using namespace boost::filesystem;

BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_boost) {
	std::string dir_name = "test_boost";

	BOOST_TEST_MESSAGE("======= Testing boost library =======");

	//Using boost filesyetm, can call one line to make absolute path from input
	//regardless of whether relative or absolute path has been given
	path dir_path = mdm_test_utils::temp_dir() + "/test_boost";

	//We probably don't need to check if directory exists, just call create... regardless
	//but we do so anyway
	bool created = create_directories(dir_path);
	bool exists = is_directory(dir_path);
	bool deleted = boost::filesystem::remove(dir_path);
	bool still_exists = is_directory(dir_path);

	BOOST_TEST_MESSAGE("Test directory created");
	BOOST_CHECK(created);//
	BOOST_CHECK(exists);//"Test directory exists"
	BOOST_CHECK(deleted);//"Test directory deleted"
	BOOST_CHECK(!still_exists);//"Test directory no longer exists"
}

BOOST_AUTO_TEST_SUITE_END() //