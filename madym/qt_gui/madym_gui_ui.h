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

#include <madym/mdm_OptionsParser.h>
#include <madym/dce_models/mdm_DCEModelBase.h>
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

  //Buttons in run window
  void on_loadConfigButton_clicked();
  void on_saveConfigButton_clicked();
  void on_homeButton_clicked();
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

  //Logging options
  void on_logNameLineEdit_textChanged(const QString &text);
  void on_auditNameLineEdit_textChanged(const QString &text);
  void on_auditDirLineEdit_textChanged(const QString &text);
  void on_auditDirSelect_clicked();

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
  void on_maxIterationsLineEdit_textChanged(const QString &text);
	void on_initMapsLineEdit_textChanged(const QString &text);
	void on_initMapsDirSelect_clicked();
  void on_residualsLineEdit_textChanged(const QString &text);
  void on_residualsSelect_clicked();

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
  void initialize_widget_values();
  void initialize_model_options();
  void initialize_T1_options();
  void initialize_AIF_options();
  void initialize_image_format_options(QComboBox &b);
  void set_AIF_enabled();
  bool check_required_options();
  void setB1Name(const QString &text);
  void makeB1Consistent(bool useB1);

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

};

#endif // madym_gui_ui_H
