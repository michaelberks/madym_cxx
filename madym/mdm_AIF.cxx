/**
 *  @file    mdm_AIF.cxx
 *  @brief   Implementation of mdm_AIF class
 */

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_AIF.h"

#include <cstdio>
#include <cassert>
#include <iostream>
#include <cmath>
#include <istream>

#include "mdm_version.h"


#include "mdm_DCEVoxel.h"
#include "mdm_ProgramLogger.h"

//
MDM_API mdm_AIF::mdm_AIF()
	:
  AIFtype_(mdm_AIF::AIF_TYPE::AIF_UNDEFINED),
  PIFtype_(mdm_AIF::PIF_TYPE::PIF_UNDEFINED),
  AIFTimes_(0),
	base_AIF_(0),
	resampled_AIF_(0),
  base_PIF_(0),
  resampled_PIF_(0),
	Hct_(0.42),
	prebolus_(8),
	dose_(0.1)
{

}

//
MDM_API mdm_AIF::~mdm_AIF()
{

}

//
MDM_API bool mdm_AIF::readAIF(const std::string &full_AIF_filename, const int nDynamics)
{
	if (readIFFromFile(base_AIF_, full_AIF_filename, nDynamics))
	{
		setAIFType(AIF_FILE);
		return true;
	}
	else
	{
		setAIFType(AIF_UNDEFINED);
    throw "Unable to read AIF from...";
		return false;
	}
}

//
MDM_API bool mdm_AIF::readPIF(const std::string &full_PIF_filename, const int nDynamics)
{
	if (readIFFromFile(base_PIF_, full_PIF_filename, nDynamics))
	{
		setPIFType(PIF_FILE);
		return true;
	}
	else
	{
		setPIFType(PIF_UNDEFINED);
    throw "Unable to read PIF from...";
		return false;
	}
}

//
MDM_API bool mdm_AIF::writeAIF(const std::string &filename)
{
	if (base_AIF_.empty())
		base_AIF_ = resampled_AIF_;
	return writeIFToFile(base_AIF_, filename);
}

//
MDM_API bool mdm_AIF::writePIF(const std::string &filename)
{
	if (base_PIF_.empty())
		base_PIF_ = resampled_PIF_;
	return writeIFToFile(base_PIF_, filename);
}

//
MDM_API bool mdm_AIF::setBaseAIF(const std::vector<double> &aifVals)
{
  if (aifVals.size() == AIFTimes_.size())
  {
    base_AIF_ = aifVals;
    return true;
  }
  else
    return false;
}

//
MDM_API const std::vector<double>& mdm_AIF::AIF() const
{
	return resampled_AIF_;
}

//
MDM_API const std::vector<double>& mdm_AIF::PIF() const
{
	return resampled_PIF_;
}

//
MDM_API void mdm_AIF::resample_AIF(double tOffset)
{
	//Important this is only called after AIFTimes has been set
	const int nTimes = AIFTimes_.size();

	switch (AIFtype_)
	{
	case AIF_STD:
		aifWeinman(nTimes, tOffset);
		break;
	case AIF_FILE:
  case AIF_MAP:
		aifFromBase(nTimes, tOffset);
		break;
	case AIF_POP:
		aifPopGJMP(nTimes, tOffset);
		break;

	case AIF_UNDEFINED:
	default:
		// Add warning, quit here
    throw "Tried to resample undefined AIF";
		break;
	}
}

//
MDM_API void mdm_AIF::resample_PIF(double tOffset, bool offsetAIF/* = true*/, bool resampleIRF/* = true*/)
{
	//Important this is only called after AIFTimes has been set
	const int nTimes = AIFTimes_.size();

	switch (PIFtype_)
	{

	case PIF_FILE:
		pifFromBase(nTimes, tOffset);
		break;

	case PIF_POP:
		aifPopHepaticAB(nTimes, tOffset, offsetAIF, resampleIRF);
		break;
	case PIF_UNDEFINED:
	default:
		// Add warning, quit here
    throw "Tried to resample undefined AIF";
		break;
	}
}

//
MDM_API bool mdm_AIF::setAIFType(AIF_TYPE value)
{
  bool setOK;

  switch (value)
  {
    case AIF_STD:
    case AIF_FILE:
    case AIF_POP:
    case AIF_MAP:
      AIFtype_ = value;
      setOK = true;
      break;
    default:
      AIFtype_ = AIF_UNDEFINED;
      setOK = false;
      throw "AIF type not recognised";
      break;
  }

  return setOK;
}

//
MDM_API bool mdm_AIF::setPIFType(PIF_TYPE value)
{
	bool setOK;

	switch (value)
	{
	case PIF_FILE:
	case PIF_POP:
		PIFtype_ = value;
		setOK = true;
		break;
	default:
		PIFtype_ = PIF_UNDEFINED;
		setOK = false;
    throw "AIF type not recognised";
		break;
	}

	return setOK;
}

//
MDM_API mdm_AIF::AIF_TYPE mdm_AIF::AIFType() const
{
  return AIFtype_;
}

//
MDM_API mdm_AIF::PIF_TYPE mdm_AIF::PIFType() const
{
	return PIFtype_;
}

//
MDM_API const std::vector<double>& mdm_AIF::AIFTimes() const
{
	return AIFTimes_;
}

//
MDM_API double mdm_AIF::AIFTime(int i) const
{
	return AIFTimes_[i];
}

//
MDM_API void mdm_AIF::setAIFTimes(const std::vector<double> times)
{
	int nTimes = times.size();
	AIFTimes_.resize(nTimes);
	for (int i = 0; i < nTimes; i++)
		AIFTimes_[i] = times[i] - times[0];
}

//
MDM_API void  mdm_AIF::setPrebolus(int p)
{
	prebolus_ = p;
}

//
MDM_API void  mdm_AIF::setHct(double h)
{
	Hct_ = h;
}

//
MDM_API void  mdm_AIF::setDose(double d)
{
	dose_ = d;
}

//
MDM_API int mdm_AIF::prebolus() const
{
	return prebolus_;
}

//
MDM_API double mdm_AIF::Hct() const
{
	return Hct_;
}

//
MDM_API double mdm_AIF::dose() const
{
	return dose_;
}

//**************************************************************************
// Private functions
//**************************************************************************

//
void mdm_AIF::aifPopGJMP(int nTimes, double tOffset)
{
  double gaussian1, gaussian2, sigmoid;
  std::vector<double> offsetTimes(nTimes);

  // These parameters are from Parker et al MRM 56:993(2006)
  // For gaussian1
  const double kA1     = 5.7326;
  const double kMu1    = 0.17046;
  const double kSigma1 = 0.0563;
  // For gaussian2
  const double kA2     = 0.9974;
  const double kMu2    = 0.365;
  const double kSigma2 = 0.132;
  // For sigmoid
  const double kAlpha  = 1.050;
  const double kBeta   = 0.1685;
  const double kS      = 38.078;
  const double kTau    = 0.483;

  // Get AIF timing data
  for (int i = 0; i < nTimes; i++)
    offsetTimes[i] = (double) (AIFTimes_[i] - AIFTimes_[0] + tOffset);

  // TODO Trying out Anita's pb - 1 instead of 2 in GJMP AIF
	resampled_AIF_.resize(nTimes);
  for (int i = 0; i < nTimes; i++)
  {
    gaussian1 = kA1 * exp(-1.0 * ((double) AIFTimes_[i] - kMu1 - offsetTimes[(int) prebolus_ - 1])
                               * ((double) AIFTimes_[i] - kMu1 - offsetTimes[(int) prebolus_ - 1])
                               / (2.0 * kSigma1 * kSigma1));
    gaussian2 = kA2 * exp(-1.0 * ((double) AIFTimes_[i] - kMu2 - offsetTimes[(int) prebolus_ - 1])
                               * ((double) AIFTimes_[i] - kMu2 - offsetTimes[(int) prebolus_ - 1])
                               / (2.0 * kSigma2 * kSigma2));
    sigmoid   = kAlpha * exp(-kBeta * ((double) AIFTimes_[i] - offsetTimes[(int) prebolus_ - 1]))
                       / (1 + exp(-kS * ((double) AIFTimes_[i] - kTau - offsetTimes[(int) prebolus_ - 1])));
    resampled_AIF_[i] = (double) (((dose_ / 0.1) * (gaussian1 + gaussian2 + sigmoid))
    		                    / (1.0 - Hct_));
  }
}

void mdm_AIF::aifPopHepaticAB(int nTimes, double tOffset, bool offsetAIF, bool resampleIRF)
{
	//If we've got an offset, make sure AIF has been resampled
	if (offsetAIF || resampled_AIF_.size() != nTimes)
		resample_AIF( tOffset);

	//generate a population IRF according to Anita's model
	if (resampleIRF || PIF_IRF_.size() != nTimes || std::isnan(PIF_IRF_[0]))
	{
		PIF_IRF_.resize(nTimes);
		double irf_sum = 0.0;
		for (int i_t = 0; i_t < nTimes; i_t++)
		{
			const double &t = AIFTimes_[i_t] - tOffset;
			if (t < 0.08)
        PIF_IRF_[i_t] = 0;//This might have been set to NaN, so make sure we set back to zero
			else if (t < 0.17)
				PIF_IRF_[i_t] = 24.16*t - 2.01;
			else
				PIF_IRF_[i_t] = 2.83*exp(-10.80*t) + 2.12*exp(-1.82*t);

			irf_sum += PIF_IRF_[i_t];
		}
		for (int i_t = 0; i_t < nTimes; i_t++)
			PIF_IRF_[i_t] /= irf_sum;
	}
	
	//Convolve the AIF with the IRF to generate the PIF
	resampled_PIF_.resize(nTimes);

	//literal convolution operation
	for (int i_t = 0; i_t < nTimes; i_t++)
	{
		double pif_sum = 0.0;
		for (int j_t = 0, k_t = i_t; j_t <= i_t; j_t++, k_t--)
		{
			pif_sum += resampled_AIF_[j_t] * PIF_IRF_[k_t];
		}
		resampled_PIF_[i_t] = pif_sum;
	}
}

//
void mdm_AIF::aifWeinman(int nTimes, double tOffset)
{
  std::vector<double> AIF(nTimes), offsetTimes(nTimes);
  double delta_t, remainder;

  /* From original paper TODO get Weinman ref */
  const double kAlpha1 = 3.99;
  const double kBeta1  = 0.144;
  const double kAlpha2 = 4.78;
  const double kBeta2  = 0.0111;

  /* Get AIF timing data */
  for (int i = 0; i < nTimes; i++)
    offsetTimes[i] = (double) (AIFTimes_[i] - AIFTimes_[0] + tOffset);

  AIF[0] = 0.0;
  for (int i = 1; i < nTimes; i++)
  {
    if (i < prebolus_)
      AIF[i] = 0.0;
    else
      AIF[i] = dose_ * (kAlpha1 * exp(-kBeta1 * (AIFTimes_[i - 1]))
                             + kAlpha2 * exp(-kBeta2 * (AIFTimes_[i - 1])));
  }
  
	//Linear resample AIF to shifted time points
  resampled_AIF_[0] = 0.0;
  for (int i = 1; i < nTimes; i++)
  {
    if (AIFTimes_[i] <= offsetTimes[0])
      resampled_AIF_[i] = 0.0;
    for (int j = 1; j < nTimes; j++)
      // find where in the AIF time series the current tissue time point falls
      if (AIFTimes_[i] > offsetTimes[j - 1] && AIFTimes_[i] <= offsetTimes[j])
      {
        delta_t = offsetTimes[j] - offsetTimes[j - 1];
        remainder = AIFTimes_[i] - offsetTimes[j - 1];
        resampled_AIF_[i] = remainder / delta_t * AIF[j]
                           + (1.0 - remainder / delta_t) * AIF[j - 1];
      }
  }
}

//
void mdm_AIF::aifFromBase(int nTimes, double tOffset)
{
	resampleBase(resampled_AIF_, base_AIF_,
		nTimes, tOffset);
}

//
//Load an hepatic portal vein from file
void mdm_AIF::pifFromBase(int nTimes, double tOffset)
{
	resampleBase(resampled_PIF_, base_PIF_,
		nTimes, tOffset);
}

//Load input funtion previously loaded from file
void mdm_AIF::resampleBase(std::vector<double> &resampled_if, const std::vector<double> &loaded_if,
	int nTimes, double tOffset)
{
	std::vector<double> offsetTimes(nTimes);
	double delta_t, remainder;

	// Get AIF timing data
	for (int i = 0; i < nTimes; i++)
		offsetTimes[i] = AIFTimes_[i] + tOffset;


	// Linear resample AIF to shifted time points
	resampled_if.resize(nTimes);
	for (int i = 1; i < nTimes; i++)
	{
		for (int j = 1; j < nTimes; j++)
		{
			// find where in the AIF time series the current tissue time point falls
			if (AIFTimes_[i] > offsetTimes[j - 1] && AIFTimes_[i] <= offsetTimes[j])
			{
				delta_t = offsetTimes[j] - offsetTimes[j - 1];
				remainder = AIFTimes_[i] - offsetTimes[j - 1];
				resampled_if[i] = ((remainder / delta_t) * loaded_if[j]
					+ (1.0 - remainder / delta_t) * loaded_if[j - 1])
					/ (1.0 - Hct_);

				break;
			}
		}
	}
}

//Load an AIF from file
bool mdm_AIF::readIFFromFile(std::vector<double> &loaded_if, const std::string &filename, const int nDynamics)
{
	std::vector<double>  timesFromFile(nDynamics, 0);
	loaded_if.resize(nDynamics);

	std::ifstream aifStream(filename, std::ios::in);

	if (!aifStream.is_open())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AIF::readIFFromFile: "
			"IF file " + filename + " not found\n");
		return false;
	}

	//Load times and values from file - MB added check we don't reach EOF
	//If we do this suggests the AIF file does not have sufficient time points -
	//most likely we've loaded a file with data organised in the wrong format
	for (int i = 0; i < nDynamics; i++)
	{
		if (aifStream.eof())
		{
			mdm_ProgramLogger::logProgramMessage(
				"ERROR: mdm_AIF::readIFFromFile: "
				"IF does not contain sufficient time points. EOF reached after " +
				std::to_string(i) + "points. Expected " + std::to_string(nDynamics) + "\n");
			aifStream.close();
			return false;
		};
		aifStream >> timesFromFile[i] >> loaded_if[i];
	}
	aifStream.close();

	//Check if we have existing times, if not, use the times we've just read from the file
	if (AIFTimes_.size() != nDynamics)
		AIFTimes_ = timesFromFile;

	mdm_ProgramLogger::logProgramMessage(
		"IF successfully read from " + filename + "\n");

	return true;
}

//
bool mdm_AIF::writeIFToFile(const std::vector<double> &if_to_save, const std::string &filename)
{
  std::ofstream aifStream(filename, std::ios::out);

  if (!aifStream.is_open())
  {
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AIF::writeIFToFile: "
			"Unable to open IF file " + filename + " for writing.\n");
    return false;
  }

  assert(if_to_save.size() == AIFTimes_.size());

  //
  for (int i = 0; i < if_to_save.size(); i++)
    aifStream << AIFTimes_[i] << " " << if_to_save[i] << std::endl;

	mdm_ProgramLogger::logProgramMessage(
		"IF successfully written to " + filename + "\n");

  return true;
}

//

