/*!
 *  @file    mdm_AIF.h
 *  @brief Class for reading, writing, generating and resampling vascular input functions
 *  @details More details here...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_AIF_HDR
#define MDM_AIF_HDR
#include "mdm_api.h"

#include "mdm_Image3D.h"

 //! Reading, writing, generating and resampling vascular input functions for DCE TK models
	/*!
	*/
class mdm_AIF {

public:
	//! Values for AIF_type
	/*!
	 Specifies the type of AIF that will be stored and returned
	*/
	enum AIF_TYPE {
		AIF_UNDEFINED = -1, ///< AIF not recognised or not yet set
    AIF_POP = 0, ///< Population AIF generated from fucntional form developed by Parker et al.
    AIF_MAP = 1, ///< AIF computed voxels specified in map (requires dynamic volumes to be loaded)
		AIF_FILE = 2, ///< AIF loaded from file
    AIF_STD = 3 ///< Legacy STD format AIF, not expected to be used	
	};

  //! Return string name of AIF type
  MDM_API static std::string typeToString(AIF_TYPE type)
  {
    switch (type)
    {
    case AIF_UNDEFINED:
      return "UNDEFINED";
    case AIF_POP:
      return "PARKER";
    case AIF_MAP:
      return "MAP";
    case AIF_FILE:
      return "FILE";
    case AIF_STD:
      return "STD";
    default:
      throw mdm_exception(__func__, "AIF type " + std::to_string(type) + " not recognised");
    }
  }

  //! Return AIF type from string
  MDM_API static AIF_TYPE typeFromString(const std::string &type)
  {
    if (type == "UNDEFINED")
      return AIF_UNDEFINED;
    else if (type == "PARKER")
      return AIF_POP;
    else if (type == "MAP")
      return AIF_MAP;
    else if (type == "FILE")
      return AIF_FILE;
    else if (type == "STD")
      return AIF_STD;
    else
      throw mdm_exception(__func__, "AIF type " + type + " not recognised");
  }

  //! Values for AIF_map
  /*!
   Specifies the type of AIF that will be stored and returned
  */
  enum AIFmapVoxel {
    BELOW_T1_THRESH = 0, ///< T1 below threshold to be considered
    PEAK_TOO_EARLY = -6, ///< Peak arrives before bolus injection
    PEAK_TOO_LATE = -5, ///< Peak arrives too late after bolus injection
    DOUBLE_DIP = -4, ///< Not monotonic increase from arrival to peak
    BELOW_NOISE_THRESH = -3, ///< Peak not significantly different from pre-bolus signal
    CANDIDATE = -2, ///< Considered as candidate but not included in final selection
    INVALID_CT = -1, ///< Rejected because of invalid conversion to Ct
    SELECTED = 1 ///< Selected voxel for computing AIF   
  };

	//! Values for PIFType
	/*!
	 Specifies the type of AIF that will be stored and returned
	*/
	enum PIF_TYPE {
    PIF_UNDEFINED = -1, ///< Invalid PIF supplied, not expected to be used 
		PIF_FILE = 1, ///< PIF loaded from file
		PIF_POP = 2 ///< PIF auto-generated from AIF using an empirically derived delay and dispersion IRF
	};

	//! Default constructor
	/*! 
	*/
	MDM_API mdm_AIF();

	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_AIF();

	//! Read AIF from given filename
	/*!
	* The AIF file should be in a simple text format with one time-point per row
	* each row should contain exactly 2 values: 1) the time in minutes 2) the CA
	* concentration at that time.
	\param filename (can absolute or relative to current working directory)
	\param nDynamics the number of time-points to read.
	*/
	MDM_API void readAIF(const std::string &filename, const size_t nDynamics);

	//! Read PIF from given filename
	/*!
	 The PIF file should be in a simple text format with one time-point per row
	 each row should contain exactly 2 values: 1) the time in minutes 2) the CA
	 concentration at that time.
	\param filename (can absolute or relative to current working directory)
	\param nDynamics the number of time-points to read.
	*/
	MDM_API void readPIF(const std::string &filename, const size_t nDynamics);

	//! Write AIF to given filename
	/*!
	\param filename (can absolute or relative to current working directory)
	*/
  MDM_API void writeAIF(const std::string &filename);

	//! Write PIF to given fileame
	/*!
	\param filename (can absolute or relative to current working directory)
	*/
  MDM_API void writePIF(const std::string &filename);

  //! Set AIF from vector of C(t) values
  /*!
  \param aifVals contrast-agent concentration associated with each time point
  */
  MDM_API void setBaseAIF(const std::vector<double> &aifVals);

	//! Return the current AIF
	/*!
	 This returns the values of the AIF from whenever it was last resampled
	*/
	MDM_API const std::vector<double>& AIF() const;

	//! Return the current PIF
	/*!
	 This returns the values of the PIF from whenever it was last resampled
	*/
	MDM_API const std::vector<double>& PIF() const;


	/*!
	//! Resample the AIF at given time offset
	 For AIFs loaded from file, this returns a bilinear interpolation of the AIF values
	 at times + tOffset. For population forms, the AIF function is recomputed at the offset times.
	*/
	MDM_API void resample_AIF(double tOffset);

	//! Resample the PIF at given time offset
	/*!
	*/
	MDM_API void resample_PIF(double tOffset, bool offsetAIF = true, bool resampleIRF = true);

	//! Set the AIF type
	/*!
	\see AIF_TYPE
	*/
	MDM_API bool  setAIFType(AIF_TYPE value);

	//! Get the current AIF type
	/*!
	\see AIF_TYPE
	*/
	MDM_API AIF_TYPE  AIFType() const;

	//! Set the PIF type
	/*!
	\see PIF_TYPE
	*/
	MDM_API bool  setPIFType(PIF_TYPE value);

	//! Get the PIF type
	/*!
	\see PIF_TYPE
	*/
	MDM_API PIF_TYPE  PIFType() const;

	//! Get time (in minutes) of each AIF time-point
	/*!
	*/
	MDM_API const std::vector<double>& AIFTimes() const;

	//! Get time (in minutes) at specific AIF timepoint
	/*!
	\param timepoint must be >=0 and < nTimes
	*/
	MDM_API double AIFTime(size_t timepoint) const;

	//! Set the time (in mintues) of each AIF time-point
	/*!
	*/
	MDM_API void setAIFTimes(const std::vector<double> times);

	//! Set the time point at which the contrast bolus was injected
	/*!
	\param timepoint must be >=0 and < nTimes
	*/
	MDM_API void  setPrebolus(size_t timepoint);

	//! Set haematocrit correction
	/*!
	\param hct AIF values loaded from file will be divided by (1 - hct). If they have already
	 been corrected, set hct = 0. In population AIFs the hct is required to compute appropriate values.
	 If not set, the default is hct = 0.42
	*/
	MDM_API void  setHct(double hct);

	//! Set the dose of contrast bolus for generating population AIFs
	/*!
	\param dose specified in mMol per Kg. Required if computing population AIF. Ignored for AIFs loaded from file.
	*/
	MDM_API void  setDose(double dose);


	//! Get the time point contrast bolus was injected
	/*!
	*/
	MDM_API size_t prebolus() const;

	//! Get haematocrit correction
	/*!
	*/
	MDM_API double Hct() const;

	//! Get the dose of contrast bolus for generating population AIFs
	/*!
	*/
	MDM_API double dose() const;



protected:

private:

	/*!
	*/
	void aifPopGJMP(size_t nData, double tOffset);

	void aifPopHepaticAB(size_t nData, double tOffset, bool resample_AIF, bool resampleIRF);

	/*!
	*/
	void aifWeinman(size_t nData, double tOffset);

	/*!
	*/
	void aifFromBase(size_t nData, double tOffset);

	//Resample hepatic portal vein input funtion previously loaded from file
	void pifFromBase(size_t nData, double tOffset);

	//Load input funtion previously loaded from file
	void resampleBase(std::vector<double> &resampled_if, const std::vector<double> &loaded_if,
    size_t nData, double tOffset);

	//Load/save an AIF from/to file
	void readIFFromFile(std::vector<double> &loaded_if, const std::string &filename, const size_t nDynamics);
  void writeIFToFile(const std::vector<double> &if_to_save, const std::string &filename);

  //----------------------------------------------------------
  //Variables

	AIF_TYPE    AIFtype_;
	PIF_TYPE    PIFtype_;
	std::vector<double> resampled_AIF_;
	std::vector<double> base_AIF_;
	std::vector<double> resampled_PIF_;
	std::vector<double> base_PIF_;
	std::vector<double> PIF_IRF_;
	std::vector<double> AIFTimes_;
	size_t prebolus_;
	double  Hct_;
	double  dose_;

};

#endif /* MDM_AIF_HDR */
