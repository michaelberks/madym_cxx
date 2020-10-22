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

#include <mdm_RunTools.h>
#include <mdm_OptionsParser.h>

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
  
	int parse_error = options_parser_.calculate_T1_lite_inputs(argc, (const char **)argv, options_);

	if (parse_error)
		return parse_error;

	//Instantiate new madym_exe object with these options and run
	mdm_RunTools madym_exe(options_, options_parser_);
	return madym_exe.run_CalculateT1_lite();
}
