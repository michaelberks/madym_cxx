/*!
*  @file    madym_gui_ui.cxx
*  @brief   Implementation of madym_gui_ui class
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#include "madym_gui_ui.h"

#include <madym/dce_models/mdm_DCEModelGenerator.h>
#include <madym/t1_methods/mdm_T1MethodGenerator.h>
#include <madym/image_io/mdm_ImageIO.h>

#include <madym/mdm_ProgramLogger.h>
#include <mdm_version.h>
#include "madym_gui_model_configure.h"
#include <iomanip>

#include <QScrollBar>
#include <QDesktopServices>

static const QString NONE_SELECTED = "<None selected>";
//
//: Constructor
madym_gui_ui::madym_gui_ui(QWidget *parent)
: QMainWindow(parent),
  model_(NULL),
  dataDir_("")
{
	// setup the UI
  ui.setupUi(this);

	//Set up the radio button group to select DCE input type
	inputTypeRadioGroup = new QButtonGroup(this);
  inputTypeRadioGroup->addButton(ui.inputTypeRadioButtonS, 0);
  inputTypeRadioGroup->addButton(ui.inputTypeRadioButtonC, 1);
  inputTypeRadioGroup->setExclusive(true);
  QObject::connect( inputTypeRadioGroup, SIGNAL(buttonClicked( int )),
                    this, SLOT(change_input_type( int )) );

  initialize_processor_thread();
  connect_signals_to_slots();
  ui.stackedWidget->setCurrentWidget(ui.homePage);
  mdm_ProgramLogger::setQuiet(true);
}

//
//: Destructor
madym_gui_ui::~madym_gui_ui()
{
}

 
// Events
void madym_gui_ui::closeEvent(QCloseEvent *ev)
{
  processor_thread_.quit();

  ev->accept();
}


//-------------------------------------------------------------------------
//:Menu file
//-------------------------------------------------------------------------
void madym_gui_ui::on_actionExit_triggered()
{
	close();
}

void madym_gui_ui::on_actionAbout_triggered()
{
  QMessageBox msgBox;
  msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
  msgBox.setText(tr(
    "Madym Version %1.<br/>"
    "Author: Michael Berks<br/>"
    "Copyright: The University of Manchester<br/>"
    "<a href='%2'>%2</a>")
    .arg(MDM_VERSION)
    .arg(MDM_QBI_WEBSITE));
  msgBox.setWindowTitle("About Madym");
  msgBox.setIcon(QMessageBox::Information);
  msgBox.exec();
}

void madym_gui_ui::on_actionUser_wiki_triggered()
{
  QDesktopServices::openUrl(QUrl(MDM_USER_WIKI));
}

//-------------------------------------------------------------------------
//:Main functions
//-------------------------------------------------------------------------
void madym_gui_ui::on_computeT1Button_clicked()
{
  processor_.set_madym_exe(madym_gui_processor::RunType::T1);
  runType_ = madym_gui_processor::RunType::T1;
  initialize_widget_values();
  ui.stackedWidget->setCurrentWidget(ui.runPage);
}

void madym_gui_ui::on_computeIFButton_clicked()
{
  processor_.set_madym_exe(madym_gui_processor::RunType::AIF);
  runType_ = madym_gui_processor::RunType::AIF;
  initialize_widget_values();
  ui.stackedWidget->setCurrentWidget(ui.runPage);
}

void madym_gui_ui::on_fitModelButton_clicked()
{
  processor_.set_madym_exe(madym_gui_processor::RunType::DCE);
  runType_ = madym_gui_processor::RunType::DCE;
  initialize_widget_values();
  ui.stackedWidget->setCurrentWidget(ui.runPage);
}

//-------------------------------------------------------------------------
//:Buttons in run window
void madym_gui_ui::on_loadConfigButton_clicked()
{
  //Open file select and get config filename
  QString config_file = QFileDialog::getOpenFileName(this, tr("Select config file"),
    dataDir_,
    tr("Config files (*.txt *.cfg);;All files (*.*)"));

  if (config_file.isEmpty())
    return;

  //Call input options to parse madym arguments, this will set all
  //the current processor_.madym_exe().options() fields into the input options
  //variable map and then check the config file	
  processor_.madym_exe().options().configFile.set(config_file.toStdString());
  if (processor_.madym_exe().parseInputs(processor_.madym_exe().who()))
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Error loading config file");
    msgBox.setInformativeText(tr("%1 could not be loaded.").arg(config_file));
    msgBox.exec();
    return;
  }
  else
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("Config file loaded");
    msgBox.setInformativeText(tr("Options successfully loaded from \n%1.").arg(config_file));
    msgBox.exec();
  }
  //Finally update the widget values with the new options
  initialize_widget_values();
}

//
void madym_gui_ui::on_saveConfigButton_clicked()
{
  QString config_file = QFileDialog::getSaveFileName(this, tr("Select config file"),
    dataDir_,
    tr("Config files (*.txt *.cfg);;All files (*.*)"));

  if (config_file.isEmpty())
    return;

  //Make sure options config file is empty, because when we call parse args
  //we don't want to read a config file
  processor_.madym_exe().options().configFile.set("");
  processor_.madym_exe().parseInputs(processor_.madym_exe().who());
  processor_.madym_exe().saveConfigFile(config_file.toStdString());
}

//
void madym_gui_ui::on_homeButton_clicked()
{
  ui.stackedWidget->setCurrentWidget(ui.homePage);
}

//
void madym_gui_ui::on_runButton_clicked()
{
  //Check required inputs are set
  if (!check_required_options())
    return;

  ui.controls->setEnabled(false);
  emit start_processing();
}

//-------------------------------------------------------------------------
//:General input options
void madym_gui_ui::on_dataDirLineEdit_textChanged(const QString &text)
{
  dataDir_ = text;
  processor_.madym_exe().options().dataDir.set(text.toStdString());
}

void madym_gui_ui::on_dataDirSelect_clicked()
{
  QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Choose data folder"),
    dataDir_,
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  if (selectedDir.isEmpty())
    return;

  ui.dataDirLineEdit->setText(selectedDir);
}

void madym_gui_ui::on_roiPathLineEdit_textChanged(const QString &text)
{
  processor_.madym_exe().options().roiName.set(text.toStdString());
}

void madym_gui_ui::on_roiPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select ROI mask"),
    dataDir_,
    tr("Mask files (*.hdr)"));

  if (selectedPath.isEmpty())
    return;

  ui.roiPathLineEdit->setText(selectedPath);
}

void madym_gui_ui::on_errorTrackerLineEdit_textChanged(const QString &text)
{
  processor_.madym_exe().options().errorTrackerName.set(text.toStdString());
}

void madym_gui_ui::on_errorTrackerSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select error tracker"),
    dataDir_,
    tr("Mask files (*.hdr)"));

  if (selectedPath.isEmpty())
    return;

  ui.errorTrackerLineEdit->setText(selectedPath);
}

//-------------------------------------------------------------------------
//:DCE data options
void madym_gui_ui::on_dceInputLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().dynDir.set(text.toStdString());
}
void madym_gui_ui::on_dceInputSelect_clicked()
{
  QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Choose DCE input folder"),
    dataDir_,
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  if (selectedDir.isEmpty())
    return;

  ui.dceInputLineEdit->setText(selectedDir);
}

void madym_gui_ui::on_dceNameLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().dynName.set(text.toStdString());
}

void madym_gui_ui::on_dceFormatLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().dynFormat.set(text.toStdString());
}

void madym_gui_ui::on_nDynSpinBox_valueChanged(int value)
{
	processor_.madym_exe().options().nDyns.set(value);
}

void madym_gui_ui::on_injectionImageSpinBox_valueChanged(int value)
{
	processor_.madym_exe().options().injectionImage.set(value);
}

//-------------------------------------------------------------------------
//:T1 calculation options
void madym_gui_ui::on_t1MethodComboBox_currentIndexChanged(const QString &text)
{
	processor_.madym_exe().options().T1method.set(text.toStdString());
}

//
void madym_gui_ui::on_t1InputTextEdit_textChanged()
{
  QString text = ui.t1InputTextEdit->toPlainText();
  text.replace("\n", ",");
  processor_.madym_exe().options().T1inputNames.value().fromString(text.toStdString());
}

//
void madym_gui_ui::on_t1InputSelect_clicked()
{
  QStringList selectedMaps = QFileDialog::getOpenFileNames(this, tr("Select input maps for baseline T1 calculation"),
    dataDir_,
    tr("Map files (*.hdr)"));

  if (selectedMaps.isEmpty())
    return;

  QString maps = selectedMaps[0];
  for (int i = 1; i < selectedMaps.length(); i++)
    maps.append("\n").append(selectedMaps[i]);

  ui.t1InputTextEdit->setText(maps);
}
void madym_gui_ui::on_t1ThresholdLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().T1noiseThresh.set(text.toDouble());
}

//-------------------------------------------------------------------------
//:Signal to concentration_options

void madym_gui_ui::on_s0UseRatioCheckBox_stateChanged(int state)
{
  ui.s0VolLineEdit->setEnabled(!state && ui.t1UsePrecomputedCheckBox->isChecked());
  ui.s0VolPathSelect->setEnabled(!state && ui.t1UsePrecomputedCheckBox->isChecked());
  if (state)
    ui.t1VolLineEdit->setText("");
}
void madym_gui_ui::on_t1UsePrecomputedCheckBox_stateChanged(int state)
{
  ui.t1VolLineEdit->setEnabled(state);
  ui.t1VolPathSelect->setEnabled(state);
  ui.s0VolLineEdit->setEnabled(state && !ui.s0UseRatioCheckBox->isChecked());
  ui.s0VolPathSelect->setEnabled(state && !ui.s0UseRatioCheckBox->isChecked());

  ui.t1MapTab->setEnabled(!state && ui.inputTypeRadioButtonS->isChecked());
}
void madym_gui_ui::on_t1VolLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().T1Name.set(text.toStdString());
  ui.t1UsePrecomputedCheckBox->setChecked(!text.isEmpty());
}

void madym_gui_ui::on_t1VolPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select baseline T1 map"),
    dataDir_,
    tr("Map files (*.hdr)"));

  if (selectedPath.isEmpty())
    return;

  ui.t1VolLineEdit->setText(selectedPath);
}
void madym_gui_ui::on_s0VolLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().M0Name.set(text.toStdString());
}

void madym_gui_ui::on_s0VolPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select baseline M0 map"),
    dataDir_,
    tr("Map files (*.hdr)"));

  if (selectedPath.isEmpty())
    return;

  ui.s0VolLineEdit->setText(selectedPath);
}

void madym_gui_ui::on_r1LineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().r1Const.set(text.toDouble());
}

//-------------------------------------------------------------------------
//:Image format options
void madym_gui_ui::on_imageReadComboBox_currentIndexChanged(const QString &text)
{
  processor_.madym_exe().options().imageReadFormat.set(text.toStdString());
}

void madym_gui_ui::on_imageWriteComboBox_currentIndexChanged(const QString &text)
{
  processor_.madym_exe().options().imageWriteFormat.set(text.toStdString());
}
 
//-------------------------------------------------------------------------
//:Logging options
void madym_gui_ui::on_logNameLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().programLogName.set(text.toStdString());
}

void madym_gui_ui::on_auditNameLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().auditLogBaseName.set(text.toStdString());
}

void madym_gui_ui::on_auditDirLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().auditLogDir.set(text.toStdString());
}

void madym_gui_ui::on_auditDirSelect_clicked()
{
  QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Choose output folder"),
    dataDir_,
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  if (selectedDir.isEmpty())
    return;

  ui.auditDirLineEdit->setText(selectedDir);
}

//-------------------------------------------------------------------------
//:AIF options
void madym_gui_ui::on_AIFtypeComboBox_currentIndexChanged(const QString &text)
{
  auto type = mdm_AIF::typeFromString(text.toStdString());
  processor_.madym_exe().options().aifType.set(type);
  set_AIF_enabled();
}

void madym_gui_ui::on_AIFfileLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().aifName.set(text.toStdString());
}
void madym_gui_ui::on_AIFfileSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select AIF file"),
    dataDir_,
    tr("AIF files (*.txt)"));

  if (selectedPath.isEmpty())
    return;

  ui.AIFfileLineEdit->setText(selectedPath);
}

//
void madym_gui_ui::on_AIFmapLineEdit_textChanged(const QString &text)
{
  processor_.madym_exe().options().aifMap.set(text.toStdString());
}

//
void madym_gui_ui::on_AIFmapSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select AIF map"),
    dataDir_,
    tr("AIF map files (*.hdr)"));

  if (selectedPath.isEmpty())
    return;

  ui.AIFmapLineEdit->setText(selectedPath);
}

//
void madym_gui_ui::on_populationPIFCheckbox_stateChanged(int state)
{
  ui.autoPIFPathLineEdit->setEnabled(!state);
  ui.autoPIFPathSelect->setEnabled(!state);
}

//
void madym_gui_ui::on_autoPIFPathLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().pifName.set(text.toStdString());
}
void madym_gui_ui::on_autoPIFPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select PIF file"),
    dataDir_,
    tr("PIF files (*.txt)"));

  if (selectedPath.isEmpty())
    return;

  ui.autoPIFPathLineEdit->setText(selectedPath);
} 

void madym_gui_ui::on_doseLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().dose.set(text.toDouble());
}
void madym_gui_ui::on_hctLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().hct.set(text.toDouble());
}

//---------------------------------------------------------------
//AIF detection options
void madym_gui_ui::on_xRangeLineEdit_textChanged(const QString &text)
{
  processor_.madym_exe().options().aifXrange.value().fromString(text.toStdString());
}

//
void madym_gui_ui::on_yRangeLineEdit_textChanged(const QString &text)
{
  processor_.madym_exe().options().aifYrange.value().fromString(text.toStdString());
}

//
void madym_gui_ui::on_slicesLineEdit_textChanged(const QString &text)
{
  processor_.madym_exe().options().aifSlices.value().fromString(text.toStdString());
}

//
void madym_gui_ui::on_minT1lineEdit_textChanged(const QString &text)
{
  processor_.madym_exe().options().minT1Blood.set(text.toDouble());
}

//
void madym_gui_ui::on_peakTimeLineEdit_textChanged(const QString &text)
{
  processor_.madym_exe().options().peakTime.set(text.toDouble());
}

//
void madym_gui_ui::on_prebolusMinSpinBox_valueChanged(int value)
{
  processor_.madym_exe().options().prebolusMinImages.set(value);
}

//
void madym_gui_ui::on_prebolusNoiseLineEdit_textChanged(const QString &text)
{
  processor_.madym_exe().options().prebolusNoise.set(text.toDouble());
}

//
void madym_gui_ui::on_selectPctSpinBox_valueChanged(double value)
{
  processor_.madym_exe().options().selectPct.set(value);
}

//

//-------------------------------------------------------------------------
//:Output options
void madym_gui_ui::on_outputDirLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().outputDir.set(text.toStdString());
}

//
void madym_gui_ui::on_outputDirSelect_clicked()
{
  QString outputDir = QFileDialog::getExistingDirectory(this, tr("Choose output folder"),
    dataDir_,
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  if (outputDir.isEmpty())
    return;

  ui.outputDirLineEdit->setText(outputDir);
}

//
void madym_gui_ui::on_iaucTimesLineEdit_textChanged(const QString &text)
{
  processor_.madym_exe().options().IAUCTimes.value().fromString(text.toStdString());
}

//
void madym_gui_ui::on_initMapsLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().initMapsDir.set(text.toStdString());
}

//
void madym_gui_ui::on_initMapsDirSelect_clicked()
{
	QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Choose folder containing param maps"),
    dataDir_,
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	if (selectedDir.isEmpty())
		return;

	ui.initMapsLineEdit->setText(selectedDir);
}


//
void madym_gui_ui::on_residualsLineEdit_textChanged(const QString &text)
{
  processor_.madym_exe().options().modelResiduals.set(text.toStdString());
}

//
void madym_gui_ui::on_residualsSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select residuals map"),
    dataDir_,
    tr("Residuals map files (*.hdr)"));

  if (selectedPath.isEmpty())
    return;

  ui.residualsLineEdit->setText(selectedPath);
}

//
void madym_gui_ui::on_clearLogButton_clicked()
{
  ui.cmdTextEdit->clear();
}

//
void madym_gui_ui::on_overwriteCheckBox_stateChanged(int state)
{
	processor_.madym_exe().options().overwrite.set(state);
}

//
void madym_gui_ui::on_outputCsCheckBox_stateChanged(int state)
{
	processor_.madym_exe().options().outputCt_sig.set(state);
}

void madym_gui_ui::on_outputCmCheckBox_stateChanged(int state)
{
	processor_.madym_exe().options().outputCt_mod.set(state);
}

void madym_gui_ui::on_logMessageReceived(QString msg)
{
  ui.cmdTextEdit->appendPlainText(msg);
  auto sb = ui.cmdTextEdit->verticalScrollBar();
  sb->setValue(sb->maximum());
}

//-------------------------------------------------------------------------
//:Model fitting
void madym_gui_ui::on_modelSelectComboBox_currentIndexChanged(const QString &text)
{
	if (text == NONE_SELECTED)
		return;

	mdm_AIF aif;
	auto modelType = mdm_DCEModelGenerator::ParseModelName(text.toStdString());
  aif.setAIFType(mdm_AIF::AIF_TYPE::AIF_POP);
  aif.setPIFType(mdm_AIF::PIF_TYPE::PIF_POP);
  model_ = mdm_DCEModelGenerator::createModel(aif,
    modelType, {},
		{}, {}, {}, {}, {});
	processor_.madym_exe().options().model.set(text.toStdString());
  processor_.madym_exe().options().paramNames.set({});
  processor_.madym_exe().options().initialParams.set({});
  processor_.madym_exe().options().fixedParams.set({});
  processor_.madym_exe().options().fixedValues.set({});
	processor_.madym_exe().options().relativeLimitParams.set({});
	processor_.madym_exe().options().relativeLimitValues.set({});
}
void madym_gui_ui::on_configureModelButton_clicked()
{
  //this is a bit clumsy (it will basically delete the old model object
  //and recreate it with potentialy the same parameters, but it won't be
  //expensive because the model is a very lightweight object and it will
  //force an initialisation of the object using any parameters that were
  //previously set (as long as the user hasn't swapped model type, in which
  //case they're wiped. i can't see an easy way round that, as I don't want
  //to have to store objects that provide a memory for each model type)
  
  const QString &modelName = ui.modelSelectComboBox->currentText();
	if (modelName == NONE_SELECTED)
		return;

	mdm_AIF aif;
	auto modelType = mdm_DCEModelGenerator::ParseModelName(modelName.toStdString());
  aif.setAIFType(mdm_AIF::AIF_TYPE::AIF_POP);
  aif.setPIFType(mdm_AIF::PIF_TYPE::PIF_POP);
	model_ = mdm_DCEModelGenerator::createModel(aif,
		modelType,
    processor_.madym_exe().options().paramNames(),
    processor_.madym_exe().options().initialParams(), 
		processor_.madym_exe().options().fixedParams(), processor_.madym_exe().options().fixedValues(),
		processor_.madym_exe().options().relativeLimitParams(), processor_.madym_exe().options().relativeLimitValues());

  madym_gui_model_configure optionsWindow(*model_, modelName, processor_.madym_exe().options(), this);
  const int response = optionsWindow.exec();
  /**/
}
void madym_gui_ui::on_firstImageSpinBox_valueChanged(int value)
{
	processor_.madym_exe().options().firstImage.set(value);
}
void madym_gui_ui::on_lastImageSpinBox_valueChanged(int value)
{
	processor_.madym_exe().options().lastImage.set(value);
}
void madym_gui_ui::on_temporalNoiseCheckBox_stateChanged(int state)
{
	processor_.madym_exe().options().dynNoise.set(state);
}
void madym_gui_ui::on_optimiseFitCheckBox_stateChanged(int state)
{
  ui.maxIterationsLineEdit->setEnabled(state);
	processor_.madym_exe().options().noOptimise.set(!state);

}
void madym_gui_ui::on_testEnhancementCheckBox_stateChanged(int state)
{
	processor_.madym_exe().options().testEnhancement.set(state);
}
void madym_gui_ui::on_maxIterationsLineEdit_textChanged(const QString &text)
{
	processor_.madym_exe().options().maxIterations.set(text.toInt());
} 

//-------------------------------------------------------------------------
//: Other slots
void madym_gui_ui::change_input_type(int type)
{
	//Type will be 0 if signal selected, 1 if concentration selected
	processor_.madym_exe().options().inputCt.set(type);

  ui.concentrationTab->setEnabled(!type);
  ui.t1MapTab->setEnabled(!type && !ui.t1UsePrecomputedCheckBox->isChecked());
  
}

//:
void madym_gui_ui::on_processingFinished(int result)
{
  if (result)
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Errors during processing, check run window.");
    msgBox.setInformativeText("Errors during processing, check run window.");
    msgBox.exec();
  }
  else
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(tr("%1 completed successfully.").arg(processor_.madym_exe().who().c_str()));
    msgBox.setInformativeText("Success!");
    msgBox.exec();
  }
  ui.controls->setEnabled(true);
}


//
//  Private methods
//

//
//: Initialize the processor thread
void madym_gui_ui::initialize_processor_thread()
{
  processor_thread_.setObjectName("processor_thread");

	processor_.moveToThread(&processor_thread_);
	processor_thread_.start();
}

//
//: Connect signals to slots (not surprisingly...)
void madym_gui_ui::connect_signals_to_slots()
{
  //Main thread triggering processor thread
  QObject::connect(this, SIGNAL(start_processing()),
    &processor_, SLOT(start_processing()));

  // Signals that trigger slots in the main thread
  QObject::connect(&processor_, SIGNAL(processing_finished(int)),
    this, SLOT(on_processingFinished(int)));

  //Connect program logger to log window
  QObject::connect(&mdm_ProgramLogger::qLogger(), SIGNAL(log_message(QString)),
    this, SLOT(on_logMessageReceived(QString)));

  
}

#ifdef _WIN32
bool madym_gui_ui::winEvent(MSG * message, long * result)
{
	//Placeholder in case we want to intercept keyboard entries etc.
	return false;
}
#endif

void madym_gui_ui::initialize_widget_values()
{
  auto &options = processor_.madym_exe().options();

  if (!dataDir_.isEmpty())
    options.dataDir.set(dataDir_.toStdString());

  //General input options
  ui.dataDirLineEdit->setText(options.dataDir().c_str());
  ui.roiPathLineEdit->setText(options.roiName().c_str());
  ui.errorTrackerLineEdit->setText(options.errorTrackerName().c_str());

  //Configure tabs only visible for fitting DCE models
  if (runType_ == madym_gui_processor::DCE)
  {
    //AIF options
    if (ui.fittingTabWidget->indexOf(ui.vascularTab) < 0)
      ui.fittingTabWidget->insertTab(0, ui.vascularTab, "Vascular input");

    ui.AIFmapLineEdit->setText(options.aifMap().c_str());
    ui.AIFfileLineEdit->setText(options.aifName().c_str());

    if (!options.aifMap().empty())
      options.aifType.set(mdm_AIF::AIF_TYPE::AIF_MAP);
      
    if (!options.aifName().empty())
      options.aifType.set(mdm_AIF::AIF_TYPE::AIF_FILE);
      
    initialize_AIF_options();

    ui.populationPIFCheckbox->setChecked(options.pifName().empty());
    ui.autoPIFPathLineEdit->setText(options.pifName().c_str());
    ui.doseLineEdit->setText(QString::number(options.dose()));
    ui.hctLineEdit->setText(QString::number(options.hct()));

    //Model options
    if (ui.fittingTabWidget->indexOf(ui.modelTab) < 0)
      ui.fittingTabWidget->insertTab(0, ui.modelTab, "Model fitting");

    initialize_model_options();

    ui.firstImageSpinBox->setValue(options.firstImage());
    ui.lastImageSpinBox->setValue(options.lastImage());
    ui.temporalNoiseCheckBox->setChecked(options.dynNoise());
    ui.optimiseFitCheckBox->setChecked(!options.noOptimise());
    ui.testEnhancementCheckBox->setChecked(options.testEnhancement());
    ui.maxIterationsLineEdit->setValidator(new QIntValidator(0, 10000, this));
    ui.maxIterationsLineEdit->setText(QString::number(options.maxIterations()));
    ui.initMapsLineEdit->setText(options.initMapsDir().c_str());
    ui.residualsLineEdit->setText(options.modelResiduals().c_str());

    //Output options specific to DCE fits
    ui.iaucLabel->show();
    ui.iaucTimesLineEdit->show();
    ui.outputCsCheckBox->setChecked(options.outputCt_sig());
    ui.outputCmCheckBox->setChecked(options.outputCt_mod());

    QString iaucTimes(options.IAUCTimes.value().toString().c_str());
    ui.iaucTimesLineEdit->setText(iaucTimes.replace("[", "").replace("]", ""));

    //Hide the AIF detection tab
    auto idx = ui.fittingTabWidget->indexOf(ui.aifTab);
    if (idx >= 0)
      ui.fittingTabWidget->removeTab(idx);

    //Set the tool label and run button text
    ui.runButton->setText("Run DCE model fitting");
    ui.toolLabel->setText("DCE model fitting");
  }

  //Configure tabs only visible for AIF detection
  if (runType_ == madym_gui_processor::AIF)
  {
    //AIF detection options
    if (ui.fittingTabWidget->indexOf(ui.aifTab) < 0)
      ui.fittingTabWidget->insertTab(0, ui.aifTab, "AIF detection");

    QString xRange(options.aifXrange.value().toString().c_str());
    ui.xRangeLineEdit->setText(xRange.replace("[", "").replace("]", ""));

    QString yRange(options.aifYrange.value().toString().c_str());
    ui.yRangeLineEdit->setText(yRange.replace("[", "").replace("]", ""));

    QString slices(options.aifSlices.value().toString().c_str());
    ui.slicesLineEdit->setText(slices.replace("[", "").replace("]", ""));

    ui.minT1lineEdit->setValidator(new QDoubleValidator(0, 10000, 2, this));
    ui.minT1lineEdit->setText(QString::number(options.minT1Blood()));
    ui.peakTimeLineEdit->setValidator(new QDoubleValidator(0, 10, 3, this));
    ui.peakTimeLineEdit->setText(QString::number(options.peakTime()));

    ui.prebolusMinSpinBox->setRange(0, 100);
    ui.prebolusMinSpinBox->setValue(options.prebolusMinImages());
    ui.prebolusNoiseLineEdit->setValidator(new QDoubleValidator(0, 1000, 2, this));
    ui.prebolusNoiseLineEdit->setText(QString::number(options.prebolusNoise()));

    ui.selectPctSpinBox->setRange(0, 100);
    ui.selectPctSpinBox->setValue(options.selectPct());

    //Hide the vascular input and model fitting tabs
    auto idx = ui.fittingTabWidget->indexOf(ui.modelTab);
    if (idx >= 0)
      ui.fittingTabWidget->removeTab(idx);

    idx = ui.fittingTabWidget->indexOf(ui.vascularTab);
    if (idx >= 0)
      ui.fittingTabWidget->removeTab(idx);

    //Hide the IAUC options from the S(t) to C(t) tab
    ui.iaucLabel->hide();
    ui.iaucTimesLineEdit->hide();

    //Set the tool label and run button text
    ui.runButton->setText("Run AIF detection");
    ui.toolLabel->setText("AIF detection");
  }

  if (runType_ == madym_gui_processor::AIF || runType_ == madym_gui_processor::DCE)
  {
    //DCE input options
    if (ui.inputTabWidget->indexOf(ui.dceTab) < 0)
      ui.inputTabWidget->insertTab(0, ui.dceTab, "DCE data");

    ui.inputTypeRadioButtonS->setChecked(!options.inputCt());
    ui.inputTypeRadioButtonC->setChecked(options.inputCt());
    ui.dceInputLineEdit->setText(options.dynDir().c_str());
    ui.dceNameLineEdit->setText(options.dynName().c_str());
    ui.dceFormatLineEdit->setText(options.dynFormat().c_str());
    ui.nDynSpinBox->setValue(options.nDyns());
    ui.injectionImageSpinBox->setValue(options.injectionImage());

    //Signal to concentration - TODO constrain inputs and put units on number inputs
    if (ui.inputTabWidget->indexOf(ui.concentrationTab) < 0)
      ui.inputTabWidget->insertTab(1, ui.concentrationTab, "Signal to concentration");

    ui.s0UseRatioCheckBox->setChecked(options.M0Ratio());
    ui.t1VolLineEdit->setText(options.T1Name().c_str());
    ui.t1UsePrecomputedCheckBox->setChecked(!options.T1Name().empty());
    ui.s0VolLineEdit->setText(options.M0Name().c_str());
    ui.t1VolLineEdit->setEnabled(ui.t1UsePrecomputedCheckBox->isChecked());
    ui.t1VolPathSelect->setEnabled(ui.t1UsePrecomputedCheckBox->isChecked());
    ui.s0VolLineEdit->setEnabled(!options.M0Ratio() &&
      ui.t1UsePrecomputedCheckBox->isChecked());
    ui.s0VolPathSelect->setEnabled(!options.M0Ratio() &&
      ui.t1UsePrecomputedCheckBox->isChecked());
    ui.r1LineEdit->setText(QString::number(options.r1Const()));

    ui.fittingTabWidget->show();
    ui.fittingTabWidget->setCurrentIndex(0);
  }
  else //T1 tool
  {
    //Hide DCE data, signal to concentration tabs
    auto idx = ui.inputTabWidget->indexOf(ui.dceTab);
    if (idx >= 0)
      ui.inputTabWidget->removeTab(idx);

    idx = ui.inputTabWidget->indexOf(ui.concentrationTab);
    if (idx >= 0)
      ui.inputTabWidget->removeTab(idx);

    ui.fittingTabWidget->hide();

    //Set the tool label and run button text
    ui.runButton->setText("Run T1 mapping");
    ui.toolLabel->setText("T1 mapping");
  }
  
  //T1 calculation - visible for all tools
  initialize_T1_options();
  ui.t1ThresholdLineEdit->setValidator(new QDoubleValidator(0, 10000, 2, this));
  ui.t1ThresholdLineEdit->setText(QString::number(options.T1noiseThresh()));

  QString t1Inputs(options.T1inputNames.value().toString().c_str());
  ui.t1InputTextEdit->setText(t1Inputs.replace("[", "").replace("]", "").replace(",","\n"));//
  ui.inputTabWidget->setCurrentIndex(0);

  //Image format options
  initialize_image_format_options(*ui.imageReadComboBox);
  initialize_image_format_options(*ui.imageWriteComboBox);

	//Ouput options - visible for all tools
	ui.outputDirLineEdit->setText(options.outputDir().c_str());
	ui.overwriteCheckBox->setChecked(options.overwrite());

  //Logging options - visible for all tools
  ui.logNameLineEdit->setText(options.programLogName().c_str());
  ui.configLineEdit->setText(options.outputConfigFileName().c_str());
  ui.auditNameLineEdit->setText(options.auditLogBaseName().c_str());
  ui.auditDirLineEdit->setText(options.auditLogDir().c_str());
  ui.outputTabWidget->setCurrentIndex(0);
}

//
void madym_gui_ui::initialize_model_options()
{
  const auto &models = mdm_DCEModelGenerator::implementedModels();
  int selected_index = 0;
  bool matched = false;
  {
    const QSignalBlocker blocker(ui.modelSelectComboBox);
    // We use a signal blocker here to avoid trying to set an empty model
    //if a config file is loaded an we update the widget values
    ui.modelSelectComboBox->clear();
    for (auto model : models)
    {
      ui.modelSelectComboBox->addItem(model.c_str());
      if (model == processor_.madym_exe().options().model())
        matched = true;
      else if (!matched)
        selected_index++;

    }
    ui.modelSelectComboBox->addItem(NONE_SELECTED);
  }
  ui.modelSelectComboBox->setCurrentIndex(selected_index);
}

//
void madym_gui_ui::initialize_T1_options()
{
  const auto &methods = mdm_T1MethodGenerator::implementedMethods();
  int selected_index = 0;
  bool matched = false;
  {
    const QSignalBlocker blocker(ui.t1MethodComboBox);
    // We use a signal blocker here to avoid trying to set an empty model
    //if a config file is loaded an we update the widget values
    ui.t1MethodComboBox->clear();
    for (auto method : methods)
    {
      ui.t1MethodComboBox->addItem(method.c_str()); 
      if (method == processor_.madym_exe().options().T1method())
        matched = true;
      else if (!matched)
        selected_index++;
    }
    ui.t1MethodComboBox->addItem(NONE_SELECTED);
  }
  ui.t1MethodComboBox->setCurrentIndex(selected_index);
}

//
void madym_gui_ui::initialize_AIF_options()
{
  int selected_index = 0;
  {
    const QSignalBlocker blocker(ui.AIFtypeComboBox);
    ui.AIFtypeComboBox->clear();
    std::vector<mdm_AIF::AIF_TYPE> types = {
      mdm_AIF::AIF_TYPE::AIF_POP,
      mdm_AIF::AIF_TYPE::AIF_FILE,
      mdm_AIF::AIF_TYPE::AIF_MAP,
      mdm_AIF::AIF_TYPE::AIF_STD
    };
    bool matched = false;
    for (const auto type : types)
    {
      ui.AIFtypeComboBox->addItem(mdm_AIF::typeToString(type).c_str());
      if (type == processor_.madym_exe().options().aifType())
        matched = true;
      else if (!matched)
        selected_index++;
    }
    ui.AIFtypeComboBox->addItem(NONE_SELECTED);
  }
  ui.AIFtypeComboBox->setCurrentIndex(selected_index);
  set_AIF_enabled();
}

//
void madym_gui_ui::initialize_image_format_options(QComboBox &b)
{
  const auto &formats = mdm_ImageIO::validFormats();
  int selected_index = 0;
  bool matched = false;
  {
    const QSignalBlocker blocker(&b);
    // We use a signal blocker here to avoid trying to set an empty model
    //if a config file is loaded an we update the widget values
    b.clear();
    for (auto format : formats)
    {
      b.addItem(format.c_str());
      if (format == processor_.madym_exe().options().imageReadFormat())
        matched = true;
      else if (!matched)
        selected_index++;
    }
    b.addItem(NONE_SELECTED);
  }
  b.setCurrentIndex(selected_index);
}

void madym_gui_ui::set_AIF_enabled()
{
  auto type = processor_.madym_exe().options().aifType();
  ui.AIFfileLineEdit->setEnabled(type == mdm_AIF::AIF_FILE);
  ui.AIFfileSelect->setEnabled(type == mdm_AIF::AIF_FILE);
  ui.AIFmapLineEdit->setEnabled(type == mdm_AIF::AIF_MAP);
  ui.AIFmapSelect->setEnabled(type == mdm_AIF::AIF_MAP);
  ui.doseLineEdit->setEnabled(type == mdm_AIF::AIF_POP || type == mdm_AIF::AIF_STD);
}

bool madym_gui_ui::check_required_options()
{
  if (processor_.madym_exe().options().outputDir().empty())
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Output folder not selected");
    msgBox.setInformativeText("You must select a folder in which the analysis output will be saved.");
    msgBox.exec();
    return false;
  }

  switch (runType_)
  {
  case madym_gui_processor::RunType::T1:
  {  
    break;
  }
  case madym_gui_processor::RunType::AIF:
  {
    break;
  }
  case madym_gui_processor::RunType::DCE:
  {
    
    break;
  }
  }
  return true;
}
