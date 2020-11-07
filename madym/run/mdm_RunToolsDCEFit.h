/*!
*  @file    mdm_RunToolsDCEFit.h
*  @brief    Defines abstract class mdm_RunToolsDCEFit providing methods for analysis pipelines common to different DCE analysis tools

*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_DCE_FIT_HDR
#define MDM_RUNTOOLS_DCE_FIT_HDR
#include "mdm_api.h"
#include <madym/mdm_RunTools.h>
#include <memory>

//! Abstract base class providing methods common to DCE analysis tools
/*!
 \see mdm_RunTools
*/
class mdm_RunToolsDCEFit : public virtual mdm_RunTools {

public:

		
	//! Constructor
	/*!	
	*/
	MDM_API mdm_RunToolsDCEFit(mdm_InputOptions &options, mdm_OptionsParser &options_parser);
		
	//! Virtual destructor
	/*!
	*/
	MDM_API virtual ~mdm_RunToolsDCEFit();

protected:
	//Methods:

	//! Set-up tracer-kinetic model
	/*!
	Calls factory method in mdm_DCEModelGenerator to instantiate tracer-kinetic model
	from user specified model name, configuring it with user specified options for
	parameter initialization, optimisation limits and AIF/PIF form.

	\param model_name user-specified tracer-kinetic model
	\param auto_aif flag if the AIF associated with model is loaded from file. False if population used.
	\param auto_pif flag if the PIF associated with model is loaded from file. False if derived from AIF.
	\param paramNames override default names for model parameters. If empty defaults used.
	\param initialParams override default initial values for model parameters. If empty defaults used.
	\param fixedParams specify parameters that are fixed to their initial parameters. If empty, all parameters optimised.
	\param fixedValues specify initial values for fixed parameters.
	\param relativeLimitParams specify parameters to which relative optimisation limits applied.
	\param relativeLimitValues specify limit values for parameters with relative limits applied.
	\see mdm_DCEModelGenerator
	\see mdm_DCEModelBase
	*/
	void setModel(const std::string &model_name, bool auto_aif, bool auto_pif,
		const std::vector<std::string> &paramNames,
		const std::vector<double> &initialParams,
		const std::vector<int> fixedParams,
		const std::vector<double> fixedValues,
		const std::vector<int> relativeLimitParams,
		const std::vector<double> relativeLimitValues);

	//Variables:
	//! Shared pointer to tracer-kinetic model, which will be instantiated to specified model type
	std::shared_ptr<mdm_DCEModelBase> model_;

	//! AIF used by tracer-kinetic model
	mdm_AIF AIF_;

private:

};

#endif
