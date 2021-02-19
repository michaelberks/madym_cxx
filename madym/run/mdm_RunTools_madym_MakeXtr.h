/*!
*  @file    mdm_RunTools_madym_MakeXtr.h
*  @brief   Defines class mdm_RunTools_madym_MakeXtr to run the Xtr file maker tool
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_MADYM_MAKEXTR_HDR
#define MDM_RUNTOOLS_MADYM_MAKEXTR_HDR
#include "mdm_api.h"
#include <madym/mdm_RunTools.h>

//! Class to run the lite version of the DCE analysis tool
/*!
*/
class mdm_RunTools_madym_MakeXtr : public mdm_RunTools {

public:

		
	//! Constructor
	MDM_API mdm_RunTools_madym_MakeXtr();
		
	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_RunTools_madym_MakeXtr();

	//! parse user inputs specific to DCE analysis
	/*!
	\param argc count of command line arguments from main exe
	\param argv list of arguments from main exe
	\return 0 on success, non-zero if error or help/version options specified
	\see mdm_OptionsParser#parseInputs
	*/
	using mdm_RunTools::parseInputs;
	MDM_API int parseInputs(int argc, const char *argv[]);

	//! Return name of the tool
	/*!
  Must be implemented by the derived classes that will be instantiated into run tools objects.
	\return name of the tool 
  */
  MDM_API std::string who() const;
	
protected:
  //! Runs the lite version of DCE analysis
  /*!
  1. Parses and validates input options
  2. Sets specified tracer-kinetic model
  3. Opens input data file
  4. Processes each line in input data file, fitting tracer-kineti model to input signals/concentrations,
  writing fited parameters and IAUC measurements to output file
  5. Closes input/output file and reports the number of samples processed.
  Throws mdm_exception if errors encountered
  */
  MDM_API void run();

private:
  //Methods:
  
  //
  void makeT1InputXtr();

  //
  void makeVFAXtr();

  //
  void makeDynamicXtr();

  //
  void readDynamicTimes();

  //
  double getDynamicTime(int dynNum);

  std::string makeSequenceFilename(
    const fs::path &filepath, const std::string &prefix,
    const int fileNumber, const std::string &fileNumberFormat);

	//Variables:
  std::vector<double> dynamicTimes_;
};

#endif
