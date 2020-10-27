#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunToolsT1Fit.h"

#include <madym/mdm_ProgramLogger.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunToolsT1Fit::mdm_RunToolsT1Fit()
{
}


MDM_API mdm_RunToolsT1Fit::~mdm_RunToolsT1Fit()
{

}

/*
*/
bool mdm_RunToolsT1Fit::setT1Method(const std::string &method)
{
	//TODO currently only variable flip-angle method implemented
	//when other options implemented, set method for choosing between them in T1 map
	if (method == "VFA")
	{
		std::cout << "Using variable flip angle method" << std::endl;
	}
	else if (method == "IR")
	{
		std::cout << "Using inversion recovery method" << std::endl;
	}
	else
	{
		mdm_progAbort("method " + method + " not recognised");
	}
	return true;
}