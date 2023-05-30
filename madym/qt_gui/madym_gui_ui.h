/*!
*  @file    madym_gui_ui.h
*  @brief   QT UI class for the main madym GUI
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MADYM_GUI_UI_H
#define MADYM_GUI_UI_H

#include <QMainWindow>
#include <QWheelEvent>
#include <QTime>
#include <QFileDialog>
#include <QBitmap>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QDebug>
#include <QLabel>
#include <QProgressBar>
#include <QThread>
#include <QButtonGroup>

#include <iostream>
#include <cmath>
#include <iomanip>
#include <string>
#include <sstream>
#include <iosfwd>
#include <vector>
#include <cstddef>
#include <memory>

#include "madym_gui_processor.h"

#include <madym/run/mdm_OptionsParser.h>
#include <madym/dce/mdm_DCEModelBase.h>
#include <madym/run/mdm_RunTools_madym_T1.h>

#include "ui_madym_gui.h"

//! QT UI class for the main madym GUI
class madym_gui_ui : public QMainWindow
{
  Q_OBJECT

public: // Methods
  
	//! Constructor
	/*!
	\param parent default QT input
	*/
  madym_gui_ui(QWidget *parent = 0);

	//! Destructor
  ~madym_gui_ui();

protected:
  
  //void timerEvent( QTimerEvent * );

	//! Event callback method when GUI closed
  void closeEvent(QCloseEvent* ev);

signals:

	//! Signal sent to processor to do some processing. Not currently used.
  void start_processing();
  
private slots:
	//Private QT callbacks for all the widgets in the GUI

	// Menu file
	void on_actionExit_triggered();
  void on_actionAbout_triggered();
  void on_actionUser_wiki_triggered();

	//Main functions
	void on_computeT1Button_clicked();
	void on_computeIFButton_clicked();
  void on_fitModelButton_clicked();
  void on_dwiModelButton_clicked();
  void on_dicomButton_clicked();
  void on_xtrButton_clicked();

  //Buttons in run window
  void on_loadConfigButton_clicked();
  void on_saveConfigButton_clicked();
  void on_homeButton_clicked();
  void on_helpButton_clicked();
  void on_runButton_clicked();

  //General input options
  void on_dataDirLineEdit_textChanged(const QString &text);
  void on_dataDirSelect_clicked();
  void on_roiPathLineEdit_textChanged(const QString &text);
  void on_roiPathSelect_clicked();
  void on_errorTrackerLineEdit_textChanged(const QString &text);
  void on_errorTrackerSelect_clicked();

	//DCE data options
  void on_dceInputLineEdit_textChanged(const QString &text);
  void on_dceInputSelect_clicked();
  void on_dceNameLineEdit_textChanged(const QString &text);
  void on_dceFormatLineEdit_textChanged(const QString &text);  
  void on_dceStartSpinBox_valueChanged(int value);
  void on_dceStepSpinBox_valueChanged(int value);
  void on_nDynSpinBox_valueChanged(int value);
  void on_injectionImageSpinBox_valueChanged(int value);

  //T1 calculation options
  void on_t1MethodComboBox_currentIndexChanged(const QString &text);
  void on_t1InputTextEdit_textChanged();
  void on_t1InputSelect_clicked();
  void on_t1ThresholdLineEdit_textChanged(const QString &text);
  void on_b1MapLineEdit2_textChanged(const QString &text);
  void on_b1MapPathSelect2_clicked();
  void on_b1ScalingSpinBox2_valueChanged(double value);

  //Signal to concentration_options
  void on_m0RatioCheckBox_stateChanged(int state);
  void on_t1UsePrecomputedCheckBox_stateChanged(int state);
  void on_t1MapLineEdit_textChanged(const QString &text);
  void on_t1MapPathSelect_clicked();
  void on_m0MapLineEdit_textChanged(const QString &text);
  void on_m0MapPathSelect_clicked();
	void on_r1LineEdit_textChanged(const QString &text);
  void on_b1CorrectionCheckBox_stateChanged(int state);
  void on_b1MapLineEdit_textChanged(const QString &text);
  void on_b1MapPathSelect_clicked();
  void on_b1ScalingSpinBox_valueChanged(double value);

  //Image format options
  void on_imageReadComboBox_currentIndexChanged(const QString &text);
  void on_imageWriteComboBox_currentIndexChanged(const QString &text);
  void on_niftiScalingCheckBox_stateChanged(int state);
  void on_nifti4DCheckBox_stateChanged(int state);
  void on_bidsCheckBox_stateChanged(int state);

  //Logging options
  void on_logNameLineEdit_textChanged(const QString &text);
  void on_auditNameLineEdit_textChanged(const QString &text);
  void on_auditDirLineEdit_textChanged(const QString &text);
  void on_auditDirSelect_clicked();
  void on_noAuditCheckBox_stateChanged(int state);
  void on_noLogCheckBox_stateChanged(int state);
  void on_quietCheckBox_stateChanged(int state);


  //AIF options
  void on_AIFtypeComboBox_currentIndexChanged(const QString &text);
  void on_AIFfileLineEdit_textChanged(const QString &text);
  void on_AIFfileSelect_clicked();
  void on_AIFmapLineEdit_textChanged(const QString &text);
  void on_AIFmapSelect_clicked();
  void on_populationPIFCheckbox_stateChanged(int state);
  void on_autoPIFPathLineEdit_textChanged(const QString &text);
  void on_autoPIFPathSelect_clicked();
	void on_doseLineEdit_textChanged(const QString &text);
	void on_hctLineEdit_textChanged(const QString &text);

  //AIF detection options
  void on_xRangeLineEdit_textChanged(const QString &text);
  void on_yRangeLineEdit_textChanged(const QString &text);
  void on_slicesLineEdit_textChanged(const QString &text);
  void on_minT1lineEdit_textChanged(const QString &text);
  void on_peakTimeLineEdit_textChanged(const QString &text);
  void on_prebolusMinSpinBox_valueChanged(int value);
  void on_prebolusNoiseLineEdit_textChanged(const QString &text);
  void on_selectPctSpinBox_valueChanged(double value);

  //Output options
  void on_outputDirLineEdit_textChanged(const QString &text);
  void on_outputDirSelect_clicked();
  void on_iaucTimesLineEdit_textChanged(const QString &text);
	void on_overwriteCheckBox_stateChanged(int state);
	void on_outputCsCheckBox_stateChanged(int state);
	void on_outputCmCheckBox_stateChanged(int state);

  //Model fitting
  void on_modelSelectComboBox_currentIndexChanged(const QString &text);
  void on_configureModelButton_clicked();
  void on_firstImageSpinBox_valueChanged(int value);
  void on_lastImageSpinBox_valueChanged(int value);
  void on_temporalNoiseCheckBox_stateChanged(int state);
	void on_optimiseFitCheckBox_stateChanged(int state);
	void on_testEnhancementCheckBox_stateChanged(int state);
  void on_optTypeComboBox_currentIndexChanged(const QString& text);
  void on_maxIterationsLineEdit_textChanged(const QString &text);
	void on_initMapsLineEdit_textChanged(const QString &text);
	void on_initMapsDirSelect_clicked();
  void on_residualsLineEdit_textChanged(const QString &text);
  void on_residualsSelect_clicked();

  //DWI model options
  void on_dwiModelComboBox_currentIndexChanged(const QString& text);
  void on_dwiInputTextEdit_textChanged();
  void on_bThresholdsLineEdit_textChanged(const QString& text);
  void on_dwiInputSelect_clicked();

  //Dicom tabs...
  //Dicom image format
  void on_dicomImageWriteComboBox_currentIndexChanged(const QString& text);
  void on_dicomDataTypeComboBox_currentIndexChanged(const QString& text);
  void on_flipXCheckBox_stateChanged(int state);
  void on_flipYCheckBox_stateChanged(int state);
  void on_flipZCheckBox_stateChanged(int state);
  void on_dicomScaleSpinBox_valueChanged(double value);
  void on_dicomOffsetSpinBox_valueChanged(double value);
  void on_autoScaleTagLineEdit_textChanged(const QString& text);
  void on_autoOffsetTagLineEdit_textChanged(const QString& text);

  //Dicom sequence naming
  void on_sequenceFormatLineEdit_textChanged(const QString& text);
  void on_sequenceStartSpinBox_valueChanged(int value);
  void on_sequenceStepSpinBox_valueChanged(int value);
  void on_meanSuffixLineEdit_textChanged(const QString& text);
  void on_repeatPrefixLineEdit_textChanged(const QString& text);

  //Dicom main options
  void on_dicomDirLineEdit_textChanged(const QString& text);
  void on_dicomDirSelect_clicked();
  void on_seriesNameLineEdit_textChanged(const QString& text);
  void on_sortCheckBox_stateChanged(int state);
  void on_makeDynCheckBox_stateChanged(int state);
  void on_makeT1InputsCheckBox_stateChanged(int state);
  void on_makeDWIInputsCheckBox_stateChanged(int state);
  void on_makeSingleCheckBox_stateChanged(int state);

  //Dicom sort options
  void on_dicomFileFilterLineEdit_textChanged(const QString& text);
  void on_sliceFilterTagLineEdit_textChanged(const QString& text);
  void on_sliceFilterMatchValueLineEdit_textChanged(const QString& text);

  //Dicom make dynamic options
  void on_dicomDynSeriesLineEdit_textChanged(const QString& text);
  void on_makeDynMeanCheckBox_stateChanged(int state);
  void on_dynamicDirLineEdit_textChanged(const QString& text);
  void on_dynamicDirSelect_clicked();
  void on_dynamicNameLineEdit_textChanged(const QString& text);
  void on_dicomNDynSpinBox_valueChanged(int value);
  void on_temporalResolutionSpinBox_valueChanged(double value);

  //Dicom T1 inputs
  void on_dicomT1InputSeriesLineEdit_textChanged(const QString& text);
  void on_makeT1MeansCheckBox_stateChanged(int state);
  void on_T1DirLineEdit_textChanged(const QString& text);
  void on_T1DirSelect_clicked();
  void on_dicomT1InputTextEdit_textChanged();
  void on_xtrT1MethodComboBox_currentIndexChanged(const QString& text);

  //Dicom DWI inputs
  void on_dicomDWIInputSeriesLineEdit_textChanged(const QString& text);
  void on_makeBvalueMeansCheckBox_stateChanged(int state);
  void on_DWIDirLineEdit_textChanged(const QString& text);
  void on_DWIDirSelect_clicked();
  void on_dicomDWIInputTextEdit_textChanged();

  //Dicom single volume inputs
  void on_dicomSingleSeriesLineEdit_textChanged(const QString& text);
  void on_dicomSingleVolNamesTextEdit_textChanged();

  //Dicom scanner attributes
  void on_dynTimeTagLineEdit_textChanged(const QString& text);
  void on_dynTimeRequiredCheckBox_stateChanged(int state);
  void on_FATagLineEdit_textChanged(const QString& text);
  void on_FARequiredCheckBox_stateChanged(int state);
  void on_TRTagLineEdit_textChanged(const QString& text);
  void on_TRRequiredCheckBox_stateChanged(int state);
  void on_TITagLineEdit_textChanged(const QString& text);
  void on_TIRequiredCheckBox_stateChanged(int state);
  void on_TETagLineEdit_textChanged(const QString& text);
  void on_TERequiredCheckBox_stateChanged(int state);
  void on_BTagLineEdit_textChanged(const QString& text);
  void on_BRequiredCheckBox_stateChanged(int state);
  void on_gradOriTagLineEdit_textChanged(const QString& text);
  void on_gradOriRequiredCheckBox_stateChanged(int state);

  //XTR tabs
  void on_xtrSequenceFormatLineEdit_textChanged(const QString& text);
  void on_xtrSequenceStartSpinBox_valueChanged(int value);
  void on_xtrSequenceStepSpinBox_valueChanged(int value);
  void on_xtrFALineEdit_textChanged(const QString& text);
  void on_xtrVFAsLineEdit_textChanged(const QString& text);
  void on_xtrTIsLineEdit_textChanged(const QString& text);
  void on_xtrBvaluesLineEdit_textChanged(const QString& text);
  void on_xtrTRLineEdit_textChanged(const QString& text);
  void on_dynamicTimesFileLineEdit_textChanged(const QString& text);
  void on_dynamicTimesFileSelect_clicked();

  //Log window
  void on_clearLogButton_clicked();

  //: Other slots
  void on_logMessageReceived(QString msg);
  void change_input_type(int type);

  //:
  void on_processingFinished(int result);

private: // Methods

  void initialize_processor_thread();
  void connect_signals_to_slots();

  void setup_general_tab(bool show);
  void setup_image_format_tab(bool show);
  void setup_logging_tab(bool show);
  void setup_DCE_data_tab(bool show);
  void setup_conc_tab(bool show);
  void setup_t1_mapping_tab(bool show);
  void setup_DWI_model_tab(bool show);
  void setup_DCE_model_tab(bool show);
  void setup_IF_tab(bool show);
  void setup_AIF_detection_tab(bool show);
  void setup_fitting_tab(bool show);

  void setup_dicom_tabs(bool show);
  void setup_dicom_format_tab(bool show);
  void setup_dicom_sequence_tab(bool show);
  void setup_dicom_options_tab(bool show);
  void setup_dicom_sort_tab(bool show);
  void setup_dicom_dynamic_tab(bool show);
  void setup_dicom_t1_tab(bool show);
  void setup_dicom_DWI_tab(bool show);
  void setup_dicom_single_tab(bool show);
  void setup_dicom_scanner_tab(bool show);
  void setup_xtr_scanner_tab(bool show);

  void initialize_widget_values();

  void initialize_model_options();
  void initialize_T1_options(QComboBox& b);
  void initialize_AIF_options();
  void initialize_DWI_options();
  void initialize_image_format_options(QComboBox& b, const mdm_input_string& option);
  void initialize_image_datatype_options(QComboBox& b);
  void initialize_optimisation_options();
  void set_AIF_enabled();
  void set_BValsThresholds_enabled();
  bool check_required_options();
  void reset_user_set_options();
  void setB1Name(const QString &text);
  void makeB1Consistent(bool useB1);

  QString tagToString(const dicomTag& tag);
  void setStringOption(const QString& text, mdm_input_string& option);
  void setStringListOption(const QString& text, mdm_input_strings& option);
  void setIntOption(const QString& text, mdm_input_int& option, QLineEdit* lineEdit);
  void setIntOption(const int value, mdm_input_int& option);
  void setDoubleOption(const QString& text, mdm_input_double& option, QLineEdit* lineEdit);
  void setDoubleOption(const double value, mdm_input_double& option);
  void setBoolOption(const bool flag, mdm_input_bool& option);
  void setRangeOption(const QString& text, mdm_input_ints& option, QLineEdit *lineEdit);
  void setDoubleListOption(const QString& text, mdm_input_doubles& option, QLineEdit* lineEdit);
  void setTagOption(const QString& text, mdm_input_dicom_tag &option, QLineEdit* lineEdit);
  void setRunValid(bool valid, QLineEdit* lineEdit);
  template <class T> void trackChanges(const T& option);
  void trackChangedOption(const std::string& key, const std::string& value);

#ifdef _WIN32
	bool winEvent(MSG * message, long * result);
#endif

private: // Variables

  //: User interface object
  Ui::MadymMainWindow ui;

  //
  madym_gui_processor::RunType runType_;

	//Processor object
	madym_gui_processor processor_;

	//Processor thread
	QThread processor_thread_;

	//Widgets added manually
	QButtonGroup* inputTypeRadioGroup;

  //model so we can configure model params
  std::shared_ptr<mdm_DCEModelBase> model_;

  //Cache data directory
  QString dataDir_;

  //Cache config dir
  QString configDir_;

  //Validator to check range inputs
  QValidator *rangeValidator;

  //Validator to check range inputs
  QValidator *doubleListValidator;

  //Validator to check tag inputs
  QValidator* tagValidator;

  //List of all fields that are invalid
  QList<QLineEdit *> invalidFields_;

  //Flag to track user changes
  bool trackChanges_;

};

#endif // madym_gui_ui_H
