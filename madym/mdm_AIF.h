/**
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

 /**
	* @brief Reading, writing, generating and resampling vascular input functions for DCE models
	*/
class mdm_AIF {

public:
	/*
	* Values for AIF_flag
	*/
	enum AIFtype {
		AIF_INVALID = -1, /*Invalid AIF supplied, not expected to be used*/
		AIF_STD = 0, /*Legacy STD format AIF, not expected to be used*/
		AIF_FILE = 1, /*AIF loaded from file*/
		AIF_POP = 2 /*Population AIF generated*/
	};

	/*
	* Values for AIF_flag
	*/
	enum PIFtype {
		PIF_INVALID = -1,/*Invalid PIF supplied, not expected to be used*/
		PIF_FILE = 1, /*PIF loaded from file*/
		PIF_POP = 2 /*PIF auto-generated from AIF using population values*/
	};

	/**
	* @brief Default constructor
	*/
	MDM_API mdm_AIF();

	/**
	* @brief Default destructor
	*/
	MDM_API ~mdm_AIF();

	/**
	* @brief Read AIF from given filename
	*/
	MDM_API bool readAIF(const std::string &filename, const int nDynamics);

	/**
	* @brief Read PIF from given filename
	*/
	MDM_API bool readPIF(const std::string &filename, const int nDynamics);

	/**
	* @brief Write AIF to given file
	*/
  MDM_API bool writeAIF(const std::string &filename);

	/**
	* @brief Write PIF to given file
	*/
  MDM_API bool writePIF(const std::string &filename);

	/**
	* @brief Compute AIF automatically from sequence of dynamic images
	*/
  MDM_API bool computeAutoAIF(const std::vector<mdm_Image3D> &dynamicImages, 
    const mdm_Image3D &T1, const int slice, const double &r1, bool inputCt);

	/**
	* @brief Return the current AIF
	*/
	MDM_API std::vector<double> AIF() const;

	/**
	* @brief Return the current PIF
	*/
	MDM_API std::vector<double> PIF() const;


	/**
	* @brief Resample the AIF at given time offset
	*/
	MDM_API void resample_AIF(int nTimes, double tOffset);

	/**
	* @brief Resample the PIF at given time offset
	*/
	MDM_API void resample_PIF(int nTimes, double tOffset, bool offsetAIF = true, bool resampleIRF = true);

	/**
	* @brief Set the AIF flag
	*/
	MDM_API bool  setAIFflag(AIFtype value);

	/**
	* @brief Get the current AIF flag
	*/
	MDM_API AIFtype  AIFflag() const;

	/**
	* @brief Set the PIF flag
	*/
	MDM_API bool  setPIFflag(PIFtype value);

	/**
	* @brief Get the PIF flag
	*/
	MDM_API PIFtype  PIFflag() const;

	/**
	* @brief Get time (in minutes) of each AIF sample
	*/
	MDM_API std::vector<double> AIFTimes() const;

	/**
	* @brief Get time (in minutes) of selected AIF sample
	*/
	MDM_API double AIFTime(int i) const;

	/**
	* @brief Set the time (in mintues) of each AIF sample
	*/
	MDM_API void setAIFTimes(const std::vector<double> times);

	/**
	* @brief Set the time point at which the contrast bolus was injected
	*/
	MDM_API void  setPrebolus(int p);

	/**
	* @brief Set haematocrit correction
	*/
	MDM_API void  setHct(double h);

	/**
	* @brief Set the dose of contrast bolus for generating population AIFs
	*/
	MDM_API void  setDose(double d);


	/**
	* @brief Get the time point contrast bolus was injected
	*/
	MDM_API int prebolus() const;

	/**
	* @brief Get haematocrit correction
	*/
	MDM_API double Hct() const;

	/**
	* @brief Get the dose of contrast bolus for generating population AIFs
	*/
	MDM_API double dose() const;



protected:

private:

	/**
	*/
	void aifPopGJMP(int nData, double tOffset);

	void aifPopHepaticAB(int nData, double tOffset, bool resample_AIF, bool resampleIRF);

	/**
	*/
	void aifWeinman(int nData, double tOffset);

		/**
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
