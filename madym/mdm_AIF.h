/*!
 *  @file    mdm_AIF.h
 *  @brief Class for reading, writing, generating and resampling vascular input functions
 *  @details More details here...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_AIF_HDR
#define MDM_AIF_HDR
#include "mdm_api.h"
#include "mdm_version.h"

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
	enum AIFtype {
		AIF_INVALID = -1, ///< Invalid AIF supplied, not expected to be used
		AIF_STD = 0, ///< Legacy STD format AIF, not expected to be used
		AIF_FILE = 1, ///< AIF loaded from file
		AIF_POP = 2 ///< Population AIF generated from fucntional form developed by Parker et al.
	};

	//! Values for PIFType
	/*!
	 Specifies the type of AIF that will be stored and returned
	*/
	enum PIFtype {
		PIF_INVALID = -1, ///< Invalid PIF supplied, not expected to be used 
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
	MDM_API bool readAIF(const std::string &filename, const int nDynamics);

	//! Read PIF from given filename
	/*!
	 The PIF file should be in a simple text format with one time-point per row
	 each row should contain exactly 2 values: 1) the time in minutes 2) the CA
	 concentration at that time.
	\param filename (can absolute or relative to current working directory)
	\param nDynamics the number of time-points to read.
	*/
	MDM_API bool readPIF(const std::string &filename, const int nDynamics);

	//! Write AIF to given filename
	/*!
	\param filename (can absolute or relative to current working directory)
	*/
  MDM_API bool writeAIF(const std::string &filename);

	//! Write PIF to given fileame
	/*!
	\param filename (can absolute or relative to current working directory)
	*/
  MDM_API bool writePIF(const std::string &filename);

	//! Compute AIF automatically from sequence of dynamic images (NOT YET IMPLEMETED)
	/*!
	 To be tested fully before release...
	\param dynamicImages time-series of image volumes in which to detect and measure an AIF
	\param T1 image volume of baseline T1 (required if inputCt is False)
	\param slice  (ie index into 3rd axis) in which to detect AIF
	\param r1 relaxivity constant of CA is blood (required if inputCt is False)
	\param inputCt flag if dynamic series is CA concentraction (inputCt is True) or signal 
	that needs converting to CA (inputCt is False) 
	*/
  MDM_API bool computeAutoAIF(const std::vector<mdm_Image3D> &dynamicImages, 
    const mdm_Image3D &T1, const int slice, const double &r1, bool inputCt);

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
	\see AIFtype
	*/
	MDM_API bool  setAIFType(AIFtype value);

	//! Get the current AIF type
	/*!
	\see AIFtype
	*/
	MDM_API AIFtype  AIFType() const;

	//! Set the PIF type
	/*!
	\see PIFtype
	*/
	MDM_API bool  setPIFType(PIFtype value);

	//! Get the PIF type
	/*!
	\see PIFtype
	*/
	MDM_API PIFtype  PIFType() const;

	//! Get time (in minutes) of each AIF time-point
	/*!
	*/
	MDM_API const std::vector<double>& AIFTimes() const;

	//! Get time (in minutes) at specific AIF timepoint
	/*!
	\param timepoint must be >=0 and < nTimes
	*/
	MDM_API double AIFTime(int timepoint) const;

	//! Set the time (in mintues) of each AIF time-point
	/*!
	*/
	MDM_API void setAIFTimes(const std::vector<double> times);

	//! Set the time point at which the contrast bolus was injected
	/*!
	\param timepoint must be >=0 and < nTimes
	*/
	MDM_API void  setPrebolus(int timepoint);

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
	MDM_API int prebolus() const;

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
	void aifPopGJMP(int nData, double tOffset);

	void aifPopHepaticAB(int nData, double tOffset, bool resample_AIF, bool resampleIRF);

	/*!
	*/
	void aifWeinman(int nData, double tOffset);

		/*!
		*/
		void aifFromFile(int nData, double tOffset);

		//Resample hepatic portal vein input funtion previously loaded from file
		void pifFromFile(int nData, double tOffset);

		//Load input funtion previously loaded from file
		void resampleLoaded(std::vector<double> &resampled_if, const std::vector<double> &loaded_if,
			int nData, double tOffset);

		//Load/save an AIF from/to file
		bool readIFFromFile(std::vector<double> &loaded_if, const std::string &filename, const int nDynamics);
    bool writeIFToFile(const std::vector<double> &if_to_save, const std::string &filename);

    //Helper functions for computing auto AIF
    void getMaxSignal(const std::vector<mdm_Image3D> &dynImages, const mdm_Image3D &T1, int voxelIndex, double noiseEstimate,
      bool &valid, double &maxSignal, int &maxTime, int &time10);

    void getConcTimeCourse(const std::vector<mdm_Image3D> &dynImages, const mdm_Image3D &T1, 
      int voxelIndex, const double &r1, const int &time10, std::vector<double> &conc, bool inputCt);

    //----------------------------------------------------------
    //Variables

	  AIFtype    AIFtype_;
	  PIFtype    PIFtype_;
	  std::vector<double> resampled_AIF_;
	  std::vector<double> base_AIF_;
	  std::vector<double> resampled_PIF_;
	  std::vector<double> base_PIF_;
	  std::vector<double> PIF_IRF_;
	  std::vector<double> AIFTimes_;
	  int prebolus_;
	  double  Hct_;
	  double  dose_;

};

#endif /* MDM_AIF_HDR */
