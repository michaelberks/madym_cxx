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
  void series_to_process(int, bool);
  
private slots:
	//Private QT callbacks for all the widgets in the GUI

	// Menu file
	void on_actionLoadConfigFile_triggered();
	void on_actionSaveConfigFileDCE_triggered();
	void on_actionSaveConfigFileT1_triggered();
	void on_actionSaveConfigFileIF_triggered();
	void on_actionExit_triggered();

	//Main functions
	void on_computeT1Button_clicked();
	void on_computeIFButton_clicked();
  void on_fitModelButton_clicked();
  void on_outputStatsButton_clicked();

	//DCE data options
  void on_dceInputLineEdit_textChanged(const QString &text);
  void on_dceInputSelect_clicked();
  void on_dceNameLineEdit_textChanged(const QString &text);
  void on_dceFormatLineEdit_textChanged(const QString &text);
  void on_roiPathLineEdit_textChanged(const QString &text);
  void on_roiPathSelect_clicked();
	void on_nDynSpinBox_valueChanged(int value);
	void on_injectionImageSpinBox_valueChanged(int value);

  //T1 calculation options
  void on_t1MethodComboBox_currentIndexChanged(const QString &text);
  void on_t1InputTextEdit_textChanged();
  void on_t1InputSelect_clicked();
  void on_t1ThresholdLineEdit_textChanged(const QString &text);

  //Signal to concentration_options
  void on_s0UseRatioCheckBox_stateChanged(int state);
  void on_t1UsePrecomputedCheckBox_stateChanged(int state);
  void on_t1VolLineEdit_textChanged(const QString &text);
  void on_t1VolPathSelect_clicked();
  void on_s0VolLineEdit_textChanged(const QString &text);
  void on_s0VolPathSelect_clicked();
	void on_r1LineEdit_textChanged(const QString &text);

  //Logging options
  void on_logNameLineEdit_textChanged(const QString &text);
  void on_errorTrackerLineEdit_textChanged(const QString &text);
  void on_auditNameLineEdit_textChanged(const QString &text);
  void on_auditDirLineEdit_textChanged(const QString &text);
  void on_auditDirSelect_clicked();

  //AIF options
  void on_populationAIFCheckbox_stateChanged(int state);
  void on_autoAIFPathLineEdit_textChanged(const QString &text);
  void on_autoAIFPathSelect_clicked();
  void on_populationPIFCheckbox_stateChanged(int state);
  void on_autoPIFPathLineEdit_textChanged(const QString &text);
  void on_autoPIFPathSelect_clicked();
	void on_doseLineEdit_textChanged(const QString &text);
	void on_hctLineEdit_textChanged(const QString &text);

  //Output options
  void on_outputDirLineEdit_textChanged(const QString &text);
  void on_outputDirSelect_clicked();
  void on_iaucTimesLineEdit_textChanged(const QString &text);
	void on_overwriteCheckBox_stateChanged(int state);
	void on_outputCsCheckBox_stateChanged(int state);
	void on_outputCmCheckBox_stateChanged(int state);
	void on_sparseCheckBox_stateChanged(int state);

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

  //: Other slots
  void change_input_type(int type);

private: // Methods

  void initialize_processor_thread();
  void connect_signals_to_slots();
  void initialize_widget_values();

#ifdef _WIN32
	bool winEvent(MSG * message, long * result);
#endif

private: // Variables

  //: User interface object
  Ui::MadymMainWindow ui;

	//Processor object
	madym_gui_processor processor_;

	//Processor thread
	QThread processor_thread_;

	//Widgets added manually
	QButtonGroup* inputTypeRadioGroup;

  //Options list
  mdm_InputOptions madym_options_;
	mdm_OptionsParser options_parser_;

  //model so we can configure model params
  std::shared_ptr<mdm_DCEModelBase> model_;

};

#endif // madym_gui_ui_H
