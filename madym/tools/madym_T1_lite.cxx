/**
*
* Standalone T1 calculator, takes simple text files of input data.
* Suitable for easy use with python/Matlab wrappers. Aim is to 
* support all commonly used methods for calculating T1, however
* currently only the variable flip-angle method is implemented.
*
* @author   Mike Berks modifying GA Buonaccorsi, based on GJM Parker
* @brief    Lite version of standalone T1 calculator, takes simple text files of input data.
* @param
*/

#include <madym/run/mdm_RunTools_madym_T1_lite.h>
#include <madym/mdm_OptionsParser.h>

mdm_OptionsParser options_parser_;
mdm_InputOptions options_;

/******************************************************************************************
*       Model fitting                                                                    *
******************************************************************************************/

/**
* @author   
* @brief    
* @version  
* @param    
*/
int main(int argc, char *argv[])
{
  
	//Instantiate new madym_exe object
	mdm_RunTools_madym_T1_lite madym_exe(options_, options_parser_);

	//Parse inputs
	int parse_error = madym_exe.parse_inputs(argc, (const char **)argv);
	if (parse_error)
		return parse_error;

	//If inputs ok, then run
	return madym_exe.run();
}
