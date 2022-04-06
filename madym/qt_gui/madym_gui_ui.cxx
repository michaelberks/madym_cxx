/*!
*  @file    madym_gui_ui.cxx
*  @brief   Implementation of madym_gui_ui class
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#include "madym_gui_ui.h"

#include <madym/dce/mdm_DCEModelGenerator.h>
#include <madym/t1/mdm_T1MethodGenerator.h>
#include <madym/dwi/mdm_DWIModelGenerator.h>
#include <madym/image_io/mdm_ImageIO.h>
#include <madym/image_io/mdm_ImageDatatypes.h>

#include <madym/utils/mdm_ProgramLogger.h>
#include <mdm_version.h>
#include "madym_gui_model_configure.h"
#include <iomanip>

#include <QScrollBar>
#include <QDesktopServices>
#include <QRegExp>

static const QString NONE_SELECTED = "<None selected>";
static const QString IMAGE_FILE_FILTER =
  "NIFTI images (*.nii *.hdr *.nii.gz *.hdr.gz);;Analyze images (*.hdr);;All files (*.*)";

//
//: Constructor
madym_gui_ui::madym_gui_ui(QWidget *parent)
: QMainWindow(parent),
  model_(NULL),
  dataDir_(""),
  trackChanges_(true)
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

  //Set up some validators for entry boxes
  QRegExp rangeREX("^[0-9]+(?:(,|-)[0-9]+)*$");
  rangeREX.setPatternSyntax(QRegExp::RegExp);
  rangeValidator = new QRegExpValidator(rangeREX);

  QRegExp doubleListREX("^[0-9]+(\\.[0-9]+)?(?:,([0-9]+(\\.[0-9]+)?))*$");
  doubleListREX.setPatternSyntax(QRegExp::RegExp);
  doubleListValidator = new QRegExpValidator(doubleListREX);

  //Set up validator for tags
  QRegExp dicomTagREX("[0-9a-f]{4},[0-9a-f]{4}", Qt::CaseInsensitive, QRegExp::RegExp);
  tagValidator = new QRegExpValidator(dicomTagREX);

  initialize_processor_thread();
  connect_signals_to_slots();
  ui.stackedWidget->setCurrentWidget(ui.homePage);
  mdm_ProgramLogger::setQuiet(true);
  ui.invalidLabel->setVisible(false);
  ui.invalidLabel->setStyleSheet("color: red;");

  //See if config dir and data are set
  if (const char* env_c = std::getenv("MADYM_CONFIG_DIR"))
    configDir_ = QString(env_c);

  if (const char* env_d = std::getenv("MADYM_DATA_DIR"))
    dataDir_ = QString(env_d);
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

void madym_gui_ui::on_dwiModelButton_clicked()
{
  processor_.set_madym_exe(madym_gui_processor::RunType::DWI);
  runType_ = madym_gui_processor::RunType::DWI;
  initialize_widget_values();
  ui.stackedWidget->setCurrentWidget(ui.runPage);
}

void madym_gui_ui::on_dicomButton_clicked()
{
  processor_.set_madym_exe(madym_gui_processor::RunType::DICOM);
  runType_ = madym_gui_processor::RunType::DICOM;
  initialize_widget_values();
  ui.stackedWidget->setCurrentWidget(ui.runPage);
}

void madym_gui_ui::on_xtrButton_clicked()
{
  processor_.set_madym_exe(madym_gui_processor::RunType::XTR);
  runType_ = madym_gui_processor::RunType::XTR;
  initialize_widget_values();
  ui.stackedWidget->setCurrentWidget(ui.runPage);
}

//-------------------------------------------------------------------------
//:Buttons in run window
void madym_gui_ui::on_loadConfigButton_clicked()
{
  //Open file select and get config filename
  QString config_file = QFileDialog::getOpenFileName(this, tr("Select config file"),
    configDir_.isEmpty() ? dataDir_ : configDir_,
    tr("Config files (*.txt *.cfg);;All files (*.*)"));

  if (config_file.isEmpty())
    return;

  configDir_ = QFileInfo(config_file).absolutePath();

  //Call input options to parse madym arguments, this will set all
  //the current processor_.madym_exe().options() fields into the input options
  //variable map and then check the config file	
  //Need to reset madym_exe or we get confusing mix of old and new options
  processor_.set_madym_exe(runType_);
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

    on_logMessageReceived(tr("Options successfully loaded from \n%1.\n").arg(config_file));
  }
  //Finally update the widget values with the new options
  initialize_widget_values();
}

//
void madym_gui_ui::on_saveConfigButton_clicked()
{
  QString config_file = QFileDialog::getSaveFileName(this, tr("Select config file"),
    configDir_.isEmpty() ? dataDir_ : configDir_,
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
void madym_gui_ui::on_helpButton_clicked()
{
  processor_.madym_exe().options().help.set(true);
  processor_.madym_exe().parseInputs(processor_.madym_exe().who());
  processor_.madym_exe().options().help.set(false);
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
  setStringOption(text, processor_.madym_exe().options().roiName);
}

void madym_gui_ui::on_roiPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select ROI mask"),
    dataDir_,
    IMAGE_FILE_FILTER);

  if (selectedPath.isEmpty())
    return;

  ui.roiPathLineEdit->setText(selectedPath);
}

void madym_gui_ui::on_errorTrackerLineEdit_textChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().errorTrackerName);
}

void madym_gui_ui::on_errorTrackerSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select error tracker"),
    dataDir_,
    IMAGE_FILE_FILTER);

  if (selectedPath.isEmpty())
    return;

  ui.errorTrackerLineEdit->setText(selectedPath);
}

//-------------------------------------------------------------------------
//:DCE data options
void madym_gui_ui::on_dceInputLineEdit_textChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().dynDir);
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
  setStringOption(text, processor_.madym_exe().options().dynName);
}

void madym_gui_ui::on_dceFormatLineEdit_textChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().sequenceFormat);
}

void madym_gui_ui::on_dceStartSpinBox_valueChanged(int value)
{
  setIntOption(value, processor_.madym_exe().options().sequenceStart);
}

void madym_gui_ui::on_dceStepSpinBox_valueChanged(int value)
{
  setIntOption(value, processor_.madym_exe().options().sequenceStep);
}

void madym_gui_ui::on_nDynSpinBox_valueChanged(int value)
{
  setIntOption(value, processor_.madym_exe().options().nDyns);
}

void madym_gui_ui::on_injectionImageSpinBox_valueChanged(int value)
{
  setIntOption(value, processor_.madym_exe().options().injectionImage);
}

//-------------------------------------------------------------------------
//:T1 calculation options
void madym_gui_ui::on_t1MethodComboBox_currentIndexChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().T1method);
  makeB1Consistent(text.toStdString() == mdm_T1MethodGenerator::toString(mdm_T1MethodGenerator::VFA_B1));
}

//
void madym_gui_ui::on_t1InputTextEdit_textChanged()
{
  QString text = ui.t1InputTextEdit->toPlainText();
  text.replace("\n", ",");
  setStringListOption(text, processor_.madym_exe().options().T1inputNames);
}

//
void madym_gui_ui::on_t1InputSelect_clicked()
{
  QStringList selectedMaps = QFileDialog::getOpenFileNames(this, tr("Select input maps for baseline T1 calculation"),
    dataDir_,
    IMAGE_FILE_FILTER);

  if (selectedMaps.isEmpty())
    return;

  QString maps = selectedMaps[0];
  for (int i = 1; i < selectedMaps.length(); i++)
    maps.append("\n").append(selectedMaps[i]);

  ui.t1InputTextEdit->setText(maps);
}

void madym_gui_ui::on_t1ThresholdLineEdit_textChanged(const QString &text)
{
  setDoubleOption(text, processor_.madym_exe().options().T1noiseThresh, ui.t1ThresholdLineEdit);
}

void madym_gui_ui::on_b1MapLineEdit2_textChanged(const QString &text)
{
  on_b1MapLineEdit_textChanged(text);
}

void madym_gui_ui::on_b1MapPathSelect2_clicked()
{
  on_b1MapPathSelect_clicked();
}

void madym_gui_ui::on_b1ScalingSpinBox2_valueChanged(double value)
{
  setDoubleOption(value, processor_.madym_exe().options().B1Scaling);
  QSignalBlocker(ui.b1ScalingSpinBox);
  ui.b1ScalingSpinBox->setValue(value);
}

//-------------------------------------------------------------------------
//:Signal to concentration_options

void madym_gui_ui::on_m0RatioCheckBox_stateChanged(int state)
{
  ui.m0MapLineEdit->setEnabled(!state && ui.t1UsePrecomputedCheckBox->isChecked());
  ui.m0MapPathSelect->setEnabled(!state && ui.t1UsePrecomputedCheckBox->isChecked());
  if (state)
    ui.t1MapLineEdit->setText("");
  setBoolOption(state, processor_.madym_exe().options().M0Ratio);
}
void madym_gui_ui::on_t1UsePrecomputedCheckBox_stateChanged(int state)
{
  ui.t1MapLineEdit->setEnabled(state);
  ui.t1MapPathSelect->setEnabled(state);
  ui.m0MapLineEdit->setEnabled(state && !ui.m0RatioCheckBox->isChecked());
  ui.m0MapPathSelect->setEnabled(state && !ui.m0RatioCheckBox->isChecked());

  ui.t1MapTab->setEnabled(!state && ui.inputTypeRadioButtonS->isChecked());
}
void madym_gui_ui::on_t1MapLineEdit_textChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().T1Name);
  ui.t1UsePrecomputedCheckBox->setChecked(!text.isEmpty());
}

void madym_gui_ui::on_t1MapPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select baseline T1 map"),
    dataDir_,
    IMAGE_FILE_FILTER);

  if (selectedPath.isEmpty())
    return;

  ui.t1MapLineEdit->setText(selectedPath);
}
void madym_gui_ui::on_m0MapLineEdit_textChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().M0Name);
}

void madym_gui_ui::on_m0MapPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select baseline M0 map"),
    dataDir_,
    IMAGE_FILE_FILTER);

  if (selectedPath.isEmpty())
    return;

  ui.m0MapLineEdit->setText(selectedPath);
}

void madym_gui_ui::on_r1LineEdit_textChanged(const QString &text)
{
  setDoubleOption(text, processor_.madym_exe().options().r1Const, ui.r1LineEdit);
}

void madym_gui_ui::on_b1CorrectionCheckBox_stateChanged(int state)
{
 makeB1Consistent(state);
}

void madym_gui_ui::on_b1MapLineEdit_textChanged(const QString &text)
{
  setB1Name(text);
  makeB1Consistent(!text.isEmpty());
}

void madym_gui_ui::on_b1MapPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select B1 correction map"),
    dataDir_,
    IMAGE_FILE_FILTER);

  if (selectedPath.isEmpty())
    return;

  ui.b1MapLineEdit->setText(selectedPath); //This will trigger setB1Name and makeB1Consistent
}

void madym_gui_ui::on_b1ScalingSpinBox_valueChanged(double value)
{
  setDoubleOption(value, processor_.madym_exe().options().B1Scaling);
  QSignalBlocker(ui.b1ScalingSpinBox2);
  ui.b1ScalingSpinBox2->setValue(value);
}

//-------------------------------------------------------------------------
//:Image format options
void madym_gui_ui::on_imageReadComboBox_currentIndexChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().imageReadFormat);
}

void madym_gui_ui::on_imageWriteComboBox_currentIndexChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().imageWriteFormat);
}
 
//-------------------------------------------------------------------------
//:Logging options
void madym_gui_ui::on_logNameLineEdit_textChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().programLogName);
}

void madym_gui_ui::on_auditNameLineEdit_textChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().auditLogBaseName);
}

void madym_gui_ui::on_auditDirLineEdit_textChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().auditLogDir);
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
  setIntOption(type, processor_.madym_exe().options().aifType);
  set_AIF_enabled();
}

void madym_gui_ui::on_AIFfileLineEdit_textChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().aifName);
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
  setStringOption(text, processor_.madym_exe().options().aifMap);
}

//
void madym_gui_ui::on_AIFmapSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select AIF map"),
    dataDir_,
    IMAGE_FILE_FILTER);

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
  setStringOption(text, processor_.madym_exe().options().pifName);
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
  setDoubleOption(text, processor_.madym_exe().options().dose, ui.doseLineEdit);
}
void madym_gui_ui::on_hctLineEdit_textChanged(const QString &text)
{
  setDoubleOption(text, processor_.madym_exe().options().hct, ui.hctLineEdit);
}

//---------------------------------------------------------------
//AIF detection options
void madym_gui_ui::on_xRangeLineEdit_textChanged(const QString &text)
{
  setRangeOption(text, processor_.madym_exe().options().aifXrange, ui.xRangeLineEdit);
}

//
void madym_gui_ui::on_yRangeLineEdit_textChanged(const QString &text)
{
  setRangeOption(text, processor_.madym_exe().options().aifYrange, ui.yRangeLineEdit);
}

//
void madym_gui_ui::on_slicesLineEdit_textChanged(const QString &text)
{
  setRangeOption(text, processor_.madym_exe().options().aifSlices, ui.slicesLineEdit);
}

//
void madym_gui_ui::on_minT1lineEdit_textChanged(const QString &text)
{
  setDoubleOption(text, processor_.madym_exe().options().minT1Blood, ui.minT1lineEdit);
}

//
void madym_gui_ui::on_peakTimeLineEdit_textChanged(const QString &text)
{
  setDoubleOption(text, processor_.madym_exe().options().peakTime, ui.peakTimeLineEdit);
}

//
void madym_gui_ui::on_prebolusMinSpinBox_valueChanged(int value)
{
  setIntOption(value, processor_.madym_exe().options().prebolusMinImages);
}

//
void madym_gui_ui::on_prebolusNoiseLineEdit_textChanged(const QString &text)
{
  setDoubleOption(text, processor_.madym_exe().options().prebolusNoise, ui.prebolusNoiseLineEdit);
}

//
void madym_gui_ui::on_selectPctSpinBox_valueChanged(double value)
{
  setDoubleOption(value, processor_.madym_exe().options().selectPct);
}

//

//-------------------------------------------------------------------------
//:Output options
void madym_gui_ui::on_outputDirLineEdit_textChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().outputDir);
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
  setDoubleListOption(text, processor_.madym_exe().options().IAUCTimes, ui.iaucTimesLineEdit);
}

//
void madym_gui_ui::on_initMapsLineEdit_textChanged(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().initMapsDir);
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
  setStringOption(text, processor_.madym_exe().options().modelResiduals);
}

//
void madym_gui_ui::on_residualsSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select residuals map"),
    dataDir_,
    IMAGE_FILE_FILTER);

  if (selectedPath.isEmpty())
    return;

  ui.residualsLineEdit->setText(selectedPath);
}


//-----------------------------------------------------------------------------------
// Dicom image formats
//
void madym_gui_ui::on_dicomImageWriteComboBox_currentIndexChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().imageWriteFormat);
}

//
void madym_gui_ui::on_dicomDataTypeComboBox_currentIndexChanged(const QString& text)
{
  auto type = mdm_ImageDatatypes::typeFromString(text.toStdString());
  setIntOption(type, processor_.madym_exe().options().imageDataType);
}

//
void madym_gui_ui::on_flipXCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().flipX);
}

//
void madym_gui_ui::on_flipYCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().flipY);
}

//
void madym_gui_ui::on_flipZCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().flipZ);
}

//
void madym_gui_ui::on_dicomScaleSpinBox_valueChanged(double value)
{
  setDoubleOption(value, processor_.madym_exe().options().dicomScale);
}

//
void madym_gui_ui::on_dicomOffsetSpinBox_valueChanged(double value)
{
  setDoubleOption(value, processor_.madym_exe().options().dicomOffset);
}

//
void madym_gui_ui::on_autoScaleTagLineEdit_textChanged(const QString& text)
{
  setTagOption(text, processor_.madym_exe().options().autoScaleTag, ui.autoScaleTagLineEdit);
}

//
void madym_gui_ui::on_autoOffsetTagLineEdit_textChanged(const QString& text)
{
  setTagOption(text, processor_.madym_exe().options().autoOffsetTag, ui.autoOffsetTagLineEdit);
}

//-----------------------------------------------------------------------------------
// Dicom sequence naming
//
void madym_gui_ui::on_sequenceFormatLineEdit_textChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().sequenceFormat);
}

void madym_gui_ui::on_sequenceStartSpinBox_valueChanged(int value)
{
  setIntOption(value, processor_.madym_exe().options().sequenceStart);
}

void madym_gui_ui::on_sequenceStepSpinBox_valueChanged(int value)
{
  setIntOption(value, processor_.madym_exe().options().sequenceStep);
}

void madym_gui_ui::on_meanSuffixLineEdit_textChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().meanSuffix);
}

void madym_gui_ui::on_repeatPrefixLineEdit_textChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().repeatPrefix);
}
//-----------------------------------------------------------------------------------
//Dicom main options
//
void madym_gui_ui::on_dicomDirLineEdit_textChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().dicomDir);
}

void madym_gui_ui::on_dicomDirSelect_clicked()
{
  QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Choose dicom folder"),
    dataDir_,
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  if (selectedDir.isEmpty())
    return;

  ui.dicomDirLineEdit->setText(selectedDir);
}

void madym_gui_ui::on_seriesNameLineEdit_textChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().dicomSeriesFile);
}

void madym_gui_ui::on_sortCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().dicomSort);
}

void madym_gui_ui::on_makeDynCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().makeDyn);
}

void madym_gui_ui::on_makeT1InputsCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().makeT1Inputs);
}

void madym_gui_ui::on_makeDWIInputsCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().makeDWIInputs);
}

void madym_gui_ui::on_makeSingleCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().makeSingle);
}

//-----------------------------------------------------------------------------------
//Dicom sort options
//
void madym_gui_ui::on_dicomFileFilterLineEdit_textChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().dicomFileFilter);
}

void madym_gui_ui::on_sliceFilterTagLineEdit_textChanged(const QString& text)
{
  setTagOption(text, processor_.madym_exe().options().sliceFilterTag, ui.sliceFilterTagLineEdit);
}

void madym_gui_ui::on_sliceFilterMatchValueLineEdit_textChanged(const QString& text)
{
  setStringListOption(text, processor_.madym_exe().options().sliceFilterMatchValue);
}

//-----------------------------------------------------------------------------------
////Dicom make dynamic options
//
void madym_gui_ui::on_dicomDynSeriesLineEdit_textChanged(const QString& text)
{
  setIntOption(text, processor_.madym_exe().options().dynSeries, ui.dicomDynSeriesLineEdit);
}

void madym_gui_ui::on_makeDynMeanCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().makeDynMean);
}

void madym_gui_ui::on_dynamicDirLineEdit_textChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().dynDir);
}

void madym_gui_ui::on_dynamicDirSelect_clicked()
{
  QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Choose output folder for dynamic series"),
    dataDir_,
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  if (selectedDir.isEmpty())
    return;

  ui.dynamicDirLineEdit->setText(selectedDir);
}

void madym_gui_ui::on_dynamicNameLineEdit_textChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().dynName);
}

void madym_gui_ui::on_dicomNDynSpinBox_valueChanged(int value)
{
  setIntOption(value, processor_.madym_exe().options().nDyns);
}

void madym_gui_ui::on_temporalResolutionSpinBox_valueChanged(double value)
{
  setDoubleOption(value, processor_.madym_exe().options().temporalResolution);
}

//-----------------------------------------------------------------------------------
//Dicom T1 inputs
//
void madym_gui_ui::on_dicomT1InputSeriesLineEdit_textChanged(const QString& text)
{
  setRangeOption(text, processor_.madym_exe().options().T1inputSeries, ui.dicomT1InputSeriesLineEdit);
}

void madym_gui_ui::on_makeT1MeansCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().makeT1Means);
}

void madym_gui_ui::on_T1DirLineEdit_textChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().T1Dir);
}

void madym_gui_ui::on_T1DirSelect_clicked()
{
  QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Choose output folder for T1 inputs"),
    dataDir_,
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  if (selectedDir.isEmpty())
    return;

  ui.T1DirLineEdit->setText(selectedDir);
}

void madym_gui_ui::on_dicomT1InputTextEdit_textChanged()
{
  QString text = ui.dicomT1InputTextEdit->toPlainText();
  text.replace("\n", ",");
  setStringListOption(text, processor_.madym_exe().options().T1inputNames);
}

void madym_gui_ui::on_xtrT1MethodComboBox_currentIndexChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().T1method);
}

//-----------------------------------------------------------------------------------
//Dicom DWI inputs
//
void madym_gui_ui::on_dicomDWIInputSeriesLineEdit_textChanged(const QString& text)
{
  setRangeOption(text, processor_.madym_exe().options().DWIinputSeries, ui.dicomDWIInputSeriesLineEdit);
}

void madym_gui_ui::on_makeBvalueMeansCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().makeBvalueMeans);
}

void madym_gui_ui::on_DWIDirLineEdit_textChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().DWIDir);
}

void madym_gui_ui::on_DWIDirSelect_clicked()
{
  QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Choose output folder for DWI inputs"),
    dataDir_,
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  if (selectedDir.isEmpty())
    return;

  ui.DWIDirLineEdit->setText(selectedDir);
}

void madym_gui_ui::on_dicomDWIInputTextEdit_textChanged()
{
  QString text = ui.dicomDWIInputTextEdit->toPlainText();
  text.replace("\n", ",");
  setStringListOption(text, processor_.madym_exe().options().DWIinputNames);
}

//-----------------------------------------------------------------------------------
//Dicom single volume inputs
void madym_gui_ui::on_dicomSingleSeriesLineEdit_textChanged(const QString& text)
{
  setRangeOption(text, processor_.madym_exe().options().singleSeries, ui.dicomSingleSeriesLineEdit);
}

void madym_gui_ui::on_dicomSingleVolNamesTextEdit_textChanged()
{
  QString text = ui.dicomSingleVolNamesTextEdit->toPlainText();
  text.replace("\n", ",");
  setStringListOption(text, processor_.madym_exe().options().singleVolNames);
}

//-----------------------------------------------------------------------------------
//Dicom scanner attributes
void madym_gui_ui::on_dynTimeTagLineEdit_textChanged(const QString& text)
{
  setTagOption(text, processor_.madym_exe().options().dynTimeTag, ui.dynTimeTagLineEdit);
}

void madym_gui_ui::on_dynTimeRequiredCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().dynTimeRequired);
}

void madym_gui_ui::on_FATagLineEdit_textChanged(const QString& text)
{
  setTagOption(text, processor_.madym_exe().options().FATag, ui.FATagLineEdit);
}

void madym_gui_ui::on_FARequiredCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().FARequired);
}

void madym_gui_ui::on_TRTagLineEdit_textChanged(const QString& text)
{
  setTagOption(text, processor_.madym_exe().options().TRTag, ui.TRTagLineEdit);
}

void madym_gui_ui::on_TRRequiredCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().TRRequired);
}

void madym_gui_ui::on_TITagLineEdit_textChanged(const QString& text)
{
  setTagOption(text, processor_.madym_exe().options().TITag, ui.TITagLineEdit);
}

void madym_gui_ui::on_TIRequiredCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().TIRequired);
}

void madym_gui_ui::on_TETagLineEdit_textChanged(const QString& text)
{
  setTagOption(text, processor_.madym_exe().options().TETag, ui.TETagLineEdit);
}

void madym_gui_ui::on_TERequiredCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().TERequired);
}

void madym_gui_ui::on_BTagLineEdit_textChanged(const QString& text)
{
  setTagOption(text, processor_.madym_exe().options().BTag, ui.BTagLineEdit);
}

void madym_gui_ui::on_BRequiredCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().BRequired);
}

void madym_gui_ui::on_gradOriTagLineEdit_textChanged(const QString& text)
{
  setTagOption(text, processor_.madym_exe().options().gradOriTag, ui.gradOriTagLineEdit);
}

void madym_gui_ui::on_gradOriRequiredCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().gradOriRequired);
}

//-----------------------------------------------------------------------------------
//XTR tabs
void madym_gui_ui::on_xtrSequenceFormatLineEdit_textChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().sequenceFormat);
}

void madym_gui_ui::on_xtrSequenceStartSpinBox_valueChanged(int value)
{
  setIntOption(value, processor_.madym_exe().options().sequenceStart);
}

void madym_gui_ui::on_xtrSequenceStepSpinBox_valueChanged(int value)
{
  setIntOption(value, processor_.madym_exe().options().sequenceStep);
}

void madym_gui_ui::on_xtrFALineEdit_textChanged(const QString& text)
{
  setDoubleOption(text, processor_.madym_exe().options().FA, ui.xtrFALineEdit);
}

void madym_gui_ui::on_xtrVFAsLineEdit_textChanged(const QString& text)
{
  setDoubleListOption(text, processor_.madym_exe().options().VFAs, ui.xtrVFAsLineEdit);
}

void madym_gui_ui::on_xtrTIsLineEdit_textChanged(const QString& text)
{
  setDoubleListOption(text, processor_.madym_exe().options().TIs, ui.xtrTIsLineEdit);
}

void madym_gui_ui::on_xtrBvaluesLineEdit_textChanged(const QString& text)
{
  setDoubleListOption(text, processor_.madym_exe().options().Bvalues, ui.xtrBvaluesLineEdit);
}

void madym_gui_ui::on_xtrTRLineEdit_textChanged(const QString& text)
{
  setDoubleOption(text, processor_.madym_exe().options().TR, ui.xtrTRLineEdit);
}

void madym_gui_ui::on_dynamicTimesFileLineEdit_textChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().dynTimesFile);
}

void madym_gui_ui::on_dynamicTimesFileSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select dynamic times mask"),
    dataDir_,
    tr("Config files (*.txt *.cfg);;All files (*.*)"));

  if (selectedPath.isEmpty())
    return;

  ui.dynamicTimesFileLineEdit->setText(selectedPath);
}

//-----------------------------------------------------------------------------------
// DWI
//
void madym_gui_ui::on_dwiModelComboBox_currentIndexChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().DWImodel);
  set_BValsThresholds_enabled();
}

//
void madym_gui_ui::on_dwiInputTextEdit_textChanged()
{
  QString text = ui.dwiInputTextEdit->toPlainText();
  text.replace("\n", ",");
  setStringListOption(text, processor_.madym_exe().options().DWIinputNames);
}

//
void madym_gui_ui::on_bThresholdsLineEdit_textChanged(const QString& text)
{
  setDoubleListOption(text, processor_.madym_exe().options().BvalsThresh, ui.bThresholdsLineEdit);
}

//
void madym_gui_ui::on_dwiInputSelect_clicked()
{
  QStringList selectedMaps = QFileDialog::getOpenFileNames(this, tr("Select input maps for DWI models"),
    dataDir_,
    IMAGE_FILE_FILTER);

  if (selectedMaps.isEmpty())
    return;

  QString maps = selectedMaps[0];
  for (int i = 1; i < selectedMaps.length(); i++)
    maps.append("\n").append(selectedMaps[i]);

  ui.dwiInputTextEdit->setText(maps);
}

//
void madym_gui_ui::on_clearLogButton_clicked()
{
  ui.cmdTextEdit->clear();
}

//
void madym_gui_ui::on_overwriteCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().overwrite);
}

//
void madym_gui_ui::on_outputCsCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().outputCt_sig);
}

void madym_gui_ui::on_outputCmCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().outputCt_mod);
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
    {}, {}, {}, {}, {}, {}, {});
	setStringOption(text, processor_.madym_exe().options().model);
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
    processor_.madym_exe().options().lowerBounds(), processor_.madym_exe().options().upperBounds(),
		processor_.madym_exe().options().relativeLimitParams(), processor_.madym_exe().options().relativeLimitValues());

  madym_gui_model_configure optionsWindow(
    *model_, modelName, processor_.madym_exe().options(), this);
  const int response = optionsWindow.exec();
  /**/
}

void madym_gui_ui::on_firstImageSpinBox_valueChanged(int value)
{
  setIntOption(value, processor_.madym_exe().options().firstImage);
}

void madym_gui_ui::on_lastImageSpinBox_valueChanged(int value)
{
  setIntOption(value, processor_.madym_exe().options().lastImage);
}

void madym_gui_ui::on_temporalNoiseCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().dynNoise);
}

void madym_gui_ui::on_optimiseFitCheckBox_stateChanged(int state)
{
  ui.maxIterationsLineEdit->setEnabled(state);
  ui.optTypeComboBox->setEnabled(state);
  setBoolOption(!state, processor_.madym_exe().options().noOptimise);

}
void madym_gui_ui::on_testEnhancementCheckBox_stateChanged(int state)
{
  setBoolOption(state, processor_.madym_exe().options().testEnhancement);
}

void madym_gui_ui::on_optTypeComboBox_currentIndexChanged(const QString& text)
{
  setStringOption(text, processor_.madym_exe().options().optimisationType);
}

void madym_gui_ui::on_maxIterationsLineEdit_textChanged(const QString &text)
{
  setIntOption(text, processor_.madym_exe().options().maxIterations, ui.maxIterationsLineEdit);
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

void madym_gui_ui::setup_general_tab(bool show)
{
  if (show)
  {
    if (ui.outputTabWidget->indexOf(ui.outputTab) < 0)
      ui.outputTabWidget->insertTab(0, ui.outputTab, "General options");

    auto& options = processor_.madym_exe().options();
    if (!dataDir_.isEmpty())
      options.dataDir.set(dataDir_.toStdString());

    //General input options
    ui.dataDirLineEdit->setText(options.dataDir().c_str());
    ui.roiPathLineEdit->setText(options.roiName().c_str());
    ui.errorTrackerLineEdit->setText(options.errorTrackerName().c_str());

    //Image format options
    initialize_image_format_options(*ui.imageReadComboBox, options.imageReadFormat);
    initialize_image_format_options(*ui.imageWriteComboBox, options.imageWriteFormat);

    //Ouput options - visible for all tools
    ui.outputDirLineEdit->setText(options.outputDir().c_str());
    ui.overwriteCheckBox->setChecked(options.overwrite());

    //Show/hide widgets on general tab not needed for DICOM/XTR modes
    auto dicom = runType_ == madym_gui_processor::RunType::DICOM;
    auto xtr = runType_ == madym_gui_processor::RunType::XTR;
    ui.outputDirLabel->setVisible(!xtr);
    ui.outputDirLineEdit->setVisible(!xtr);
    ui.outputDirSelect->setVisible(!xtr);
    ui.roiPathLabel->setVisible(!xtr && !dicom);
    ui.roiPathLineEdit->setVisible(!xtr && !dicom);
    ui.roiPathSelect->setVisible(!xtr && !dicom);
    ui.errorTrackerLabel->setVisible(!xtr && !dicom);
    ui.errorTrackerLineEdit->setVisible(!xtr && !dicom);
    ui.errorTrackerSelect->setVisible(!xtr && !dicom);
    ui.imageReadLabel->setVisible(!xtr && !dicom);
    ui.imageReadComboBox->setVisible(!xtr && !dicom);
    ui.imageWriteLabel->setVisible(!xtr && !dicom);
    ui.imageWriteComboBox->setVisible(!xtr && !dicom);
    ui.overwriteCheckBox->setVisible(!xtr);
  }
  else
  {
    auto idx = ui.outputTabWidget->indexOf(ui.outputTab);
    if (idx >= 0)
      ui.outputTabWidget->removeTab(idx);
  }
  
}

void madym_gui_ui::setup_logging_tab(bool show)
{
  if (show)
  {
    if (ui.outputTabWidget->indexOf(ui.loggingTab) < 0)
      ui.outputTabWidget->insertTab(1, ui.loggingTab, "Logging");

    auto& options = processor_.madym_exe().options();
    //Logging options - visible for all tools
    ui.logNameLineEdit->setText(options.programLogName().c_str());
    ui.configLineEdit->setText(options.outputConfigFileName().c_str());
    ui.auditNameLineEdit->setText(options.auditLogBaseName().c_str());
    ui.auditDirLineEdit->setText(options.auditLogDir().c_str());
    ui.outputTabWidget->setCurrentIndex(0);
  }
  else
  {
    auto idx = ui.outputTabWidget->indexOf(ui.loggingTab);
    if (idx >= 0)
      ui.outputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_DCE_data_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();

    //DCE input options
    if (ui.inputTabWidget->indexOf(ui.dceTab) < 0)
      ui.inputTabWidget->insertTab(0, ui.dceTab, "DCE data");

    ui.inputTypeRadioButtonS->setChecked(!options.inputCt());
    ui.inputTypeRadioButtonC->setChecked(options.inputCt());
    ui.dceInputLineEdit->setText(options.dynDir().c_str());
    ui.dceNameLineEdit->setText(options.dynName().c_str());
    ui.dceFormatLineEdit->setText(options.sequenceFormat().c_str());
    ui.dceStartSpinBox->setValue(options.sequenceStart());
    ui.dceStepSpinBox->setValue(options.sequenceStep());
    ui.nDynSpinBox->setValue(options.nDyns());
    ui.injectionImageSpinBox->setValue(options.injectionImage());
  }
  else
  {
    auto idx = ui.inputTabWidget->indexOf(ui.dceTab);
    if (idx >= 0)
      ui.inputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_conc_tab(bool show)
{
  if (show)
  {
    if (ui.inputTabWidget->indexOf(ui.concentrationTab) < 0)
      ui.inputTabWidget->insertTab(1, ui.concentrationTab, "Signal to concentration");

    auto& options = processor_.madym_exe().options();

    //Signal to concentration - TODO constrain inputs and put units on number inputs
    ui.m0RatioCheckBox->setChecked(options.M0Ratio());
    ui.t1MapLineEdit->setText(options.T1Name().c_str());
    ui.t1UsePrecomputedCheckBox->setChecked(!options.T1Name().empty());
    ui.m0MapLineEdit->setText(options.M0Name().c_str());
    ui.t1MapLineEdit->setEnabled(ui.t1UsePrecomputedCheckBox->isChecked());
    ui.t1MapPathSelect->setEnabled(ui.t1UsePrecomputedCheckBox->isChecked());
    ui.m0MapLineEdit->setEnabled(!options.M0Ratio() &&
      ui.t1UsePrecomputedCheckBox->isChecked());
    ui.m0MapPathSelect->setEnabled(!options.M0Ratio() &&
      ui.t1UsePrecomputedCheckBox->isChecked());
    ui.r1LineEdit->setValidator(new QDoubleValidator(0, 10000, 5, this));
  ui.r1LineEdit->setText(QString::number(options.r1Const()));
    

    //For AIF hide the IAUC
    if (runType_ == madym_gui_processor::RunType::AIF)
    {
      //Hide the IAUC options from the S(t) to C(t) tab
      ui.iaucLabel->hide();
      ui.iaucTimesLineEdit->hide();
    }
    else if (runType_ == madym_gui_processor::RunType::DCE)
    {
      ui.iaucLabel->show();
      ui.iaucTimesLineEdit->show();
      QString iaucTimes(options.IAUCTimes.value().toString().c_str());
      ui.iaucTimesLineEdit->setText(iaucTimes.replace("[", "").replace("]", ""));
      ui.iaucTimesLineEdit->setValidator(doubleListValidator);
    }
  }
  else
  {
    auto idx = ui.inputTabWidget->indexOf(ui.concentrationTab);
    if (idx >= 0)
      ui.inputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_t1_mapping_tab(bool show)
{
  if (show)
  {
    if (ui.inputTabWidget->indexOf(ui.t1MapTab) < 0)
      ui.inputTabWidget->insertTab(2, ui.t1MapTab, "T1 mapping");

    auto& options = processor_.madym_exe().options();
    
    initialize_T1_options(*ui.t1MethodComboBox);
    ui.t1ThresholdLineEdit->setValidator(new QDoubleValidator(0, 10000, 5, this));
    ui.t1ThresholdLineEdit->setText(QString::number(options.T1noiseThresh()));

    QString t1Inputs(options.T1inputNames.value().toString().c_str());
    ui.t1InputTextEdit->setText(t1Inputs.replace("[", "").replace("]", "").replace(",", "\n"));//
    ui.inputTabWidget->setCurrentIndex(0);

    //Set B1 options
    makeB1Consistent(
      options.B1Correction() ||
      options.T1method() == mdm_T1MethodGenerator::toString(mdm_T1MethodGenerator::VFA_B1)
    );
    ui.b1ScalingSpinBox->setValue(options.B1Scaling()); //Also sets spinbox in T1 mapping tab
  }
  else
  {
    auto idx = ui.inputTabWidget->indexOf(ui.t1MapTab);
    if (idx >= 0)
      ui.inputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_DWI_model_tab(bool show)
{
  if (show)
  {
    if (ui.inputTabWidget->indexOf(ui.dwiTab) < 0)
      ui.inputTabWidget->insertTab(3, ui.dwiTab, "DWI modelling");

    auto& options = processor_.madym_exe().options();

    initialize_DWI_options();

    QString dwiInputs(options.DWIinputNames.value().toString().c_str());
    ui.dwiInputTextEdit->setText(dwiInputs.replace("[", "").replace("]", "").replace(",", "\n"));//
    ui.inputTabWidget->setCurrentIndex(0);

    QString bThresholds(options.BvalsThresh.value().toString().c_str());
    ui.bThresholdsLineEdit->setText(bThresholds.replace("[", "").replace("]", ""));
  }
  else
  {
    auto idx = ui.inputTabWidget->indexOf(ui.dwiTab);
    if (idx >= 0)
      ui.inputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_DCE_model_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();
    
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
    initialize_optimisation_options();

    //Output options specific to DCE fits
    ui.outputCsCheckBox->setChecked(options.outputCt_sig());
    ui.outputCmCheckBox->setChecked(options.outputCt_mod());
  }
  else
  {
    auto idx = ui.fittingTabWidget->indexOf(ui.modelTab);
    if (idx >= 0)
      ui.fittingTabWidget->removeTab(idx);
  }
  
}

void madym_gui_ui::setup_IF_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();

    //AIF options
    if (ui.fittingTabWidget->indexOf(ui.vascularTab) < 0)
      ui.fittingTabWidget->insertTab(1, ui.vascularTab, "Vascular input");

    ui.AIFmapLineEdit->setText(options.aifMap().c_str());
    ui.AIFfileLineEdit->setText(options.aifName().c_str());

    if (!options.aifMap().empty())
      options.aifType.set(mdm_AIF::AIF_TYPE::AIF_MAP);

    if (!options.aifName().empty())
      options.aifType.set(mdm_AIF::AIF_TYPE::AIF_FILE);

    initialize_AIF_options();

    ui.populationPIFCheckbox->setChecked(options.pifName().empty());
    ui.autoPIFPathLineEdit->setText(options.pifName().c_str());
    ui.doseLineEdit->setValidator(new QDoubleValidator(0, 10000, 5, this));
    ui.doseLineEdit->setText(QString::number(options.dose()));
    ui.hctLineEdit->setValidator(new QDoubleValidator(0, 1, 5, this));
    ui.hctLineEdit->setText(QString::number(options.hct()));
    
  }
  else
  {
    auto idx = ui.fittingTabWidget->indexOf(ui.vascularTab);
    if (idx >= 0)
      ui.fittingTabWidget->removeTab(idx);
  }
  
}

void madym_gui_ui::setup_AIF_detection_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();
    //AIF detection options
    if (ui.fittingTabWidget->indexOf(ui.aifTab) < 0)
      ui.fittingTabWidget->insertTab(2, ui.aifTab, "AIF detection");

    QString xRange(options.aifXrange.value().toString().c_str());
    ui.xRangeLineEdit->setText(xRange.replace("[", "").replace("]", ""));
    ui.xRangeLineEdit->setValidator(rangeValidator);

    QString yRange(options.aifYrange.value().toString().c_str());
    ui.yRangeLineEdit->setText(yRange.replace("[", "").replace("]", ""));
    ui.yRangeLineEdit->setValidator(rangeValidator);

    QString slices(options.aifSlices.value().toString().c_str());
    ui.slicesLineEdit->setText(slices.replace("[", "").replace("]", ""));
    ui.slicesLineEdit->setValidator(rangeValidator);

    ui.minT1lineEdit->setValidator(new QDoubleValidator(0, 10000, 5, this));
    ui.minT1lineEdit->setText(QString::number(options.minT1Blood()));
    ui.peakTimeLineEdit->setValidator(new QDoubleValidator(0, 100, 5, this));
    ui.peakTimeLineEdit->setText(QString::number(options.peakTime()));

    ui.prebolusMinSpinBox->setRange(0, 100);
    ui.prebolusMinSpinBox->setValue(options.prebolusMinImages());
    ui.prebolusNoiseLineEdit->setValidator(new QDoubleValidator(0, 1000, 5, this));
    ui.prebolusNoiseLineEdit->setText(QString::number(options.prebolusNoise()));

    ui.selectPctSpinBox->setRange(0, 100);
    ui.selectPctSpinBox->setValue(options.selectPct());
  }
  else
  {
    //Hide the AIF detection tab
    auto idx = ui.fittingTabWidget->indexOf(ui.aifTab);
    if (idx >= 0)
      ui.fittingTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_fitting_tab(bool show)
{
  if (show)
  {
    ui.fittingTabWidget->show();
    ui.fittingTabWidget->setCurrentIndex(0);
  }
  else
    ui.fittingTabWidget->hide();
}

void madym_gui_ui::setup_dicom_tabs(bool show)
{
  auto dicom = runType_ == madym_gui_processor::RunType::DICOM;
  
  setup_dicom_format_tab(show && dicom);
  setup_dicom_sequence_tab(show && dicom);
  setup_dicom_options_tab(show && dicom);
  setup_dicom_sort_tab(show && dicom);
  setup_dicom_dynamic_tab(show);
  setup_dicom_t1_tab(show);
  setup_dicom_DWI_tab(show);
  setup_dicom_single_tab(show && dicom);
  setup_dicom_scanner_tab(show && dicom);
  setup_xtr_scanner_tab(show && !dicom);
}

void madym_gui_ui::setup_dicom_format_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();
    
    //DICOM convert format options
    if (ui.outputTabWidget->indexOf(ui.formatTab) < 0)
      ui.outputTabWidget->insertTab(0, ui.formatTab, "Image formats");

    //Set up GUI widgets
    initialize_image_format_options(*ui.dicomImageWriteComboBox, options.imageWriteFormat);
    initialize_image_datatype_options(*ui.dicomDataTypeComboBox);

    ui.flipXCheckBox->setChecked(options.flipX());
    ui.flipYCheckBox->setChecked(options.flipY());
    ui.flipZCheckBox->setChecked(options.flipZ());

    ui.dicomScaleSpinBox->setValue(options.dicomScale());
    ui.dicomOffsetSpinBox->setValue(options.dicomOffset());

    ui.autoScaleTagLineEdit->setText(tagToString(options.autoScaleTag()));
    ui.autoOffsetTagLineEdit->setText(tagToString(options.autoOffsetTag()));

    //Set validators for tags
    ui.autoScaleTagLineEdit->setValidator(tagValidator);
    ui.autoOffsetTagLineEdit->setValidator(tagValidator);

  }
  else
  {
    //Hide the tab
    auto idx = ui.outputTabWidget->indexOf(ui.formatTab);
    if (idx >= 0)
      ui.outputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_dicom_sequence_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();

    //DICOM convert format options
    if (ui.outputTabWidget->indexOf(ui.sequenceTab) < 0)
      ui.outputTabWidget->insertTab(0, ui.sequenceTab, "Sequence naming");

    //Set up GUI widgets
    ui.sequenceFormatLineEdit->setText(options.sequenceFormat().c_str());
    ui.sequenceStartSpinBox->setValue(options.sequenceStart());
    ui.sequenceStepSpinBox->setValue(options.sequenceStep());

    ui.meanSuffixLineEdit->setText(options.meanSuffix().c_str());
    ui.repeatPrefixLineEdit->setText(options.repeatPrefix().c_str());
  }
  else
  {
    //Hide the tab
    auto idx = ui.outputTabWidget->indexOf(ui.sequenceTab);
    if (idx >= 0)
      ui.outputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_dicom_options_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();

    //DICOM convert format options
    if (ui.inputTabWidget->indexOf(ui.dicomTab) < 0)
      ui.inputTabWidget->insertTab(0, ui.dicomTab, "DICOM");

    //Set up GUI widgets
    ui.dicomDirLineEdit->setText(options.dicomDir().c_str());
    ui.seriesNameLineEdit->setText(options.dicomSeriesFile().c_str());

    //Flags
    ui.sortCheckBox->setChecked(options.dicomSort());
    ui.makeDynCheckBox->setChecked(options.makeDyn());
    ui.makeT1InputsCheckBox->setChecked(options.makeT1Inputs());
    ui.makeDWIInputsCheckBox->setChecked(options.makeDWIInputs());
    ui.makeSingleCheckBox->setChecked(options.makeSingle());

  }
  else
  {
    //Hide the tab
    auto idx = ui.inputTabWidget->indexOf(ui.dicomTab);
    if (idx >= 0)
      ui.inputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_dicom_sort_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();

    //DICOM convert format options
    if (ui.inputTabWidget->indexOf(ui.sortTab) < 0)
      ui.inputTabWidget->insertTab(0, ui.sortTab, "Sort");

    //Set GUI widgets
    ui.dicomFileFilterLineEdit->setText(options.dicomFileFilter().c_str());
    ui.sliceFilterTagLineEdit->setText(tagToString(options.sliceFilterTag()));
    ui.sliceFilterTagLineEdit->setValidator(tagValidator);
    QString matches(options.sliceFilterMatchValue.value().toString().c_str());
    ui.sliceFilterMatchValueLineEdit->setText(matches.replace("[", "").replace("]",""));

  }
  else
  {
    //Hide the tab
    auto idx = ui.inputTabWidget->indexOf(ui.sortTab);
    if (idx >= 0)
      ui.inputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_dicom_dynamic_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();

    //DICOM convert format options
    if (ui.inputTabWidget->indexOf(ui.dynamicTab) < 0)
      ui.inputTabWidget->insertTab(0, ui.dynamicTab, "Dynamic");

    //Set-up GUI widgets
    if (runType_ == madym_gui_processor::RunType::DICOM)
    {
      ui.dynSeriesLabel->show();
      ui.dicomDynSeriesLineEdit->show();
      ui.makeDynMeanCheckBox->show();
      ui.dicomDynSeriesLineEdit->setValidator(new QIntValidator(1, 1000));
      if (options.dynSeries())
        ui.dicomDynSeriesLineEdit->setText(QString::number(options.dynSeries()));
      
      ui.makeDynMeanCheckBox->setChecked(options.makeDynMean());
    }
    else
    {
      ui.dynSeriesLabel->hide();
      ui.dicomDynSeriesLineEdit->hide();
      ui.makeDynMeanCheckBox->hide();
    }

    ui.dynamicDirLineEdit->setText(options.dynDir().c_str());

    ui.dynamicNameLineEdit->setText(options.dynName().c_str());

    ui.dicomNDynSpinBox->setValue(options.nDyns());

    ui.temporalResolutionSpinBox->setValue(options.temporalResolution());

  }
  else
  {
    //Hide the tab
    auto idx = ui.inputTabWidget->indexOf(ui.dynamicTab);
    if (idx >= 0)
      ui.inputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_dicom_t1_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();

    //DICOM convert format options
    if (ui.inputTabWidget->indexOf(ui.T1InputTab) < 0)
      ui.inputTabWidget->insertTab(0, ui.T1InputTab, "T1 inputs");

    //Set-up GUI widgets
    if (runType_ == madym_gui_processor::RunType::DICOM)
    {
      ui.T1InputSeriesLabel->show();
      ui.dicomT1InputSeriesLineEdit->show();
      ui.makeT1MeansCheckBox->show();
      QString series(options.T1inputSeries.value().toString().c_str());
      ui.dicomT1InputSeriesLineEdit->setText(series.replace("[", "").replace("]", ""));
      ui.dicomT1InputSeriesLineEdit->setValidator(rangeValidator);

      ui.makeT1MeansCheckBox->setChecked(options.makeT1Means());

      ui.xtrT1MethodLabel->hide();
      ui.xtrT1MethodComboBox->hide();  
    }
    else
    {
      ui.T1InputSeriesLabel->hide();
      ui.dicomT1InputSeriesLineEdit->hide();
      ui.makeT1MeansCheckBox->hide();

      ui.xtrT1MethodLabel->show();
      ui.xtrT1MethodComboBox->show();
      initialize_T1_options(*ui.xtrT1MethodComboBox);
    }

    ui.T1DirLineEdit->setText(options.T1Dir().c_str());

    QString t1Inputs(options.T1inputNames.value().toString().c_str());
    ui.dicomT1InputTextEdit->setText(t1Inputs.replace("[", "").replace("]", "").replace(",", "\n"));//

  }
  else
  {
    //Hide the tab
    auto idx = ui.inputTabWidget->indexOf(ui.T1InputTab);
    if (idx >= 0)
      ui.inputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_dicom_DWI_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();

    //DICOM convert format options
    if (ui.inputTabWidget->indexOf(ui.DWIInputTab) < 0)
      ui.inputTabWidget->insertTab(0, ui.DWIInputTab, "DWI");

    //Set-up GUI widgets
    if (runType_ == madym_gui_processor::RunType::DICOM)
    {
      ui.DWIInputSeriesLabel->show();
      ui.dicomDWIInputSeriesLineEdit->show();
      ui.makeBvalueMeansCheckBox->show();
      QString series(options.DWIinputSeries.value().toString().c_str());
      ui.dicomDWIInputSeriesLineEdit->setText(series.replace("[", "").replace("]", ""));
      ui.dicomDWIInputSeriesLineEdit->setValidator(rangeValidator);

      ui.makeBvalueMeansCheckBox->setChecked(options.makeBvalueMeans());
    }
    else
    {
      ui.DWIInputSeriesLabel->hide();
      ui.dicomDWIInputSeriesLineEdit->hide();
      ui.makeBvalueMeansCheckBox->hide();
    }

    ui.DWIDirLineEdit->setText(options.DWIDir().c_str());

    QString dwiInputs(options.DWIinputNames.value().toString().c_str());
    ui.dicomDWIInputTextEdit->setText(dwiInputs.replace("[", "").replace("]", "").replace(",", "\n"));//

  }
  else
  {
    //Hide the tab
    auto idx = ui.inputTabWidget->indexOf(ui.DWIInputTab);
    if (idx >= 0)
      ui.inputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_dicom_single_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();

    //DICOM convert format options
    if (ui.inputTabWidget->indexOf(ui.singlesTab) < 0)
      ui.inputTabWidget->insertTab(0, ui.singlesTab, "Single volumes");

    //Set-up GUI widgets
    QString series(options.singleSeries.value().toString().c_str());
    ui.dicomSingleSeriesLineEdit->setText(series.replace("[", "").replace("]", ""));
    ui.dicomSingleSeriesLineEdit->setValidator(rangeValidator);

    QString singles(options.singleVolNames.value().toString().c_str());
    ui.dicomSingleVolNamesTextEdit->setText(singles.replace("[", "").replace("]", "").replace(",", "\n"));//

  }
  else
  {
    //Hide the tab
    auto idx = ui.inputTabWidget->indexOf(ui.singlesTab);
    if (idx >= 0)
      ui.inputTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_dicom_scanner_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();

    //DICOM convert format options
    if (ui.fittingTabWidget->indexOf(ui.scannerTab) < 0)
      ui.fittingTabWidget->insertTab(0, ui.scannerTab, "Scanner attributes");

    //Set up GUI widgets
    ui.dynTimeTagLineEdit->setText(tagToString(options.dynTimeTag()));
    ui.dynTimeTagLineEdit->setValidator(tagValidator);
    ui.dynTimeRequiredCheckBox->setChecked(options.dynTimeRequired());

    ui.FATagLineEdit->setText(tagToString(options.FATag()));
    ui.FATagLineEdit->setValidator(tagValidator);
    ui.FARequiredCheckBox->setChecked(options.FARequired());
    ui.FATagLineEdit->setWhatsThis(options.FATag.info());
    ui.FATagLineEdit->setToolTip(options.FATag.info());

    ui.TRTagLineEdit->setText(tagToString(options.TRTag()));
    ui.TRTagLineEdit->setValidator(tagValidator);
    ui.TRRequiredCheckBox->setChecked(options.TRRequired());

    ui.TITagLineEdit->setText(tagToString(options.TITag()));
    ui.TITagLineEdit->setValidator(tagValidator);
    ui.TIRequiredCheckBox->setChecked(options.TIRequired());

    ui.TETagLineEdit->setText(tagToString(options.TETag()));
    ui.TETagLineEdit->setValidator(tagValidator);
    ui.TERequiredCheckBox->setChecked(options.TERequired());

    ui.BTagLineEdit->setText(tagToString(options.BTag()));
    ui.BTagLineEdit->setValidator(tagValidator);
    ui.BRequiredCheckBox->setChecked(options.BRequired());

    ui.gradOriTagLineEdit->setText(tagToString(options.gradOriTag()));
    ui.gradOriTagLineEdit->setValidator(tagValidator);
    ui.gradOriRequiredCheckBox->setChecked(options.gradOriRequired());

  }
  else
  {
    //Hide the tab
    auto idx = ui.fittingTabWidget->indexOf(ui.scannerTab);
    if (idx >= 0)
      ui.fittingTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::setup_xtr_scanner_tab(bool show)
{
  if (show)
  {
    auto& options = processor_.madym_exe().options();

    //DICOM convert format options
    if (ui.fittingTabWidget->indexOf(ui.xtrScannerTab) < 0)
      ui.fittingTabWidget->insertTab(0, ui.xtrScannerTab, "Scanner settings");

    //Set up GUI widgets
    ui.xtrSequenceFormatLineEdit->setText(options.sequenceFormat().c_str());
    ui.xtrSequenceStartSpinBox->setValue(options.sequenceStart());
    ui.xtrSequenceStepSpinBox->setValue(options.sequenceStep());

    ui.xtrFALineEdit->setValidator(new QDoubleValidator(0, 10000, 5, this));
    ui.xtrFALineEdit->setText(QString::number(options.FA()));
    
    QString VFAs(options.VFAs.value().toString().c_str());
    ui.xtrVFAsLineEdit->setText(VFAs.replace("[", "").replace("]", ""));
    ui.xtrVFAsLineEdit->setValidator(doubleListValidator);

    QString TIs(options.TIs.value().toString().c_str());
    ui.xtrTIsLineEdit->setText(TIs.replace("[", "").replace("]", ""));
    ui.xtrTIsLineEdit->setValidator(doubleListValidator);

    QString Bvalues(options.Bvalues.value().toString().c_str());
    ui.xtrBvaluesLineEdit->setText(Bvalues.replace("[", "").replace("]", ""));
    ui.xtrBvaluesLineEdit->setValidator(doubleListValidator);

    ui.xtrTRLineEdit->setValidator(new QDoubleValidator(0, 10000, 5, this));
    ui.xtrTRLineEdit->setText(QString::number(options.TR()));
    
    ui.dynamicTimesFileLineEdit->setText(options.dynTimesFile().c_str());

  }
  else
  {
    //Hide the tab
    auto idx = ui.fittingTabWidget->indexOf(ui.xtrScannerTab);
    if (idx >= 0)
      ui.fittingTabWidget->removeTab(idx);
  }
}

void madym_gui_ui::initialize_widget_values()
{
  //We don't want to track changes while setting up widgets
  reset_user_set_options();

  //General and logging tabs used by all
  setup_general_tab(true);
  setup_logging_tab(runType_ != madym_gui_processor::XTR);

  //Configure tabs for different run types
  switch (runType_)
  {
  case madym_gui_processor::DCE:
    //Tabs to setup and show
    setup_t1_mapping_tab(true);
    setup_IF_tab(true);
    setup_DCE_model_tab(true);
    setup_DCE_data_tab(true);
    setup_conc_tab(true);
    setup_fitting_tab(true);

    //Tabs to hide
    setup_AIF_detection_tab(false);
    setup_DWI_model_tab(false);
    setup_dicom_tabs(false);

    //Set the tool label and run button text
    ui.runButton->setText("Run DCE model fitting");
    ui.toolLabel->setText("DCE model fitting");
    break;

  case madym_gui_processor::AIF:
    //Tabs to setup and show
    setup_t1_mapping_tab(true);
    setup_AIF_detection_tab(true);
    setup_DCE_data_tab(true);
    setup_conc_tab(true);
    setup_fitting_tab(true);

    //Tabs to hide
    setup_DCE_model_tab(false);
    setup_IF_tab(false);
    setup_dicom_tabs(false);

    //Set the tool label and run button text
    ui.runButton->setText("Run AIF detection");
    ui.toolLabel->setText("AIF detection");
    break;

  case madym_gui_processor::T1:
    //Tabs to setup and show
    setup_t1_mapping_tab(true);

    //Tabs to hide
    setup_DCE_data_tab(false);
    setup_conc_tab(false);
    setup_DWI_model_tab(false);
    setup_fitting_tab(false);
    setup_dicom_tabs(false);

    //Set the tool label and run button text
    ui.runButton->setText("Run T1 mapping");
    ui.toolLabel->setText("T1 mapping");
    break;

  case madym_gui_processor::DWI:
    //Tabs to setup and show
    setup_DWI_model_tab(true);

    //Tabs to hide
    setup_t1_mapping_tab(false); 
    setup_DCE_data_tab(false);
    setup_conc_tab(false);
    setup_fitting_tab(false);
    setup_dicom_tabs(false);

    //Set the tool label and run button text
    ui.runButton->setText("Run DWI modelling");
    ui.toolLabel->setText("DWI modelling");
    break;

  case madym_gui_processor::DICOM:
    //Tabs to setup and show
    setup_dicom_tabs(true);
    setup_fitting_tab(true);

    //Tabs to hide
    setup_DWI_model_tab(false);
    setup_t1_mapping_tab(false);
    setup_DCE_data_tab(false);
    setup_conc_tab(false);
    setup_IF_tab(false);
    setup_DCE_model_tab(false);
    setup_AIF_detection_tab(false);
    

    //Set the tool label and run button text
    ui.runButton->setText("Convert DICOM files");
    ui.toolLabel->setText("Dicom conversion");

    break;

  case madym_gui_processor::XTR:
    //Tabs to setup and show
    setup_fitting_tab(true);
    setup_dicom_tabs(true);

    //Tabs to hide
    setup_DWI_model_tab(false);
    setup_t1_mapping_tab(false);
    setup_DCE_data_tab(false);
    setup_conc_tab(false);
    setup_IF_tab(false);
    setup_DCE_model_tab(false);
    setup_AIF_detection_tab(false);

    //Set the tool label and run button text
    ui.runButton->setText("Make XTR files");
    ui.toolLabel->setText("Make XTR files");

    break;

  default:
    ;
  }
  //Make sure the tabs rae on their first tab
  ui.outputTabWidget->setCurrentIndex(0);
  ui.inputTabWidget->setCurrentIndex(0);
  ui.fittingTabWidget->setCurrentIndex(0);

  //Start tracking changes
  trackChanges_ = true;
}

//
void madym_gui_ui::initialize_model_options()
{
  const auto &models = mdm_DCEModelGenerator::models();
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
void madym_gui_ui::initialize_T1_options(QComboBox& b)
{
  const auto &methods = mdm_T1MethodGenerator::methods();
  int selected_index = 0;
  bool matched = false;
  {
    const QSignalBlocker blocker(&b);
    // We use a signal blocker here to avoid trying to set an empty model
    //if a config file is loaded an we update the widget values
    b.clear();
    for (auto method : methods)
    {
      b.addItem(method.c_str());
      if (method == processor_.madym_exe().options().T1method())
        matched = true;
      else if (!matched)
        selected_index++;
    }
    b.addItem(NONE_SELECTED);
  }
  b.setCurrentIndex(selected_index);
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

void madym_gui_ui::initialize_DWI_options()
{
  const auto& models = mdm_DWIModelGenerator::models();
  int selected_index = 0;
  bool matched = false;
  {
    const QSignalBlocker blocker(ui.dwiModelComboBox);
    // We use a signal blocker here to avoid trying to set an empty model
    //if a config file is loaded an we update the widget values
    ui.dwiModelComboBox->clear();
    for (auto model : models)
    {
      ui.dwiModelComboBox->addItem(model.c_str());
      if (model == processor_.madym_exe().options().DWImodel())
        matched = true;
      else if (!matched)
        selected_index++;
    }
    ui.dwiModelComboBox->addItem(NONE_SELECTED);
  }
  ui.dwiModelComboBox->setCurrentIndex(selected_index);
  set_BValsThresholds_enabled();
}

//
void madym_gui_ui::initialize_image_format_options(QComboBox &b, const mdm_input_string& option)
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
      if (format == option())
        matched = true;
      else if (!matched)
        selected_index++;
    }
    b.addItem(NONE_SELECTED);
  }
  b.setCurrentIndex(selected_index);
}

void madym_gui_ui::initialize_image_datatype_options(QComboBox& b)
{
  const auto& types = mdm_ImageDatatypes::validTypes();
  int selected_index = 0;
  bool matched = false;
  {
    const QSignalBlocker blocker(&b);
    // We use a signal blocker here to avoid trying to set an empty model
    //if a config file is loaded an we update the widget values
    b.clear();
    for (auto type : types)
    {
      b.addItem(type.c_str());
      if (mdm_ImageDatatypes::typeFromString(type) == processor_.madym_exe().options().imageDataType())
        matched = true;
      else if (!matched)
        selected_index++;
    }
    b.addItem(NONE_SELECTED);
  }
  b.setCurrentIndex(selected_index);
}

//
void madym_gui_ui::initialize_optimisation_options()
{
  const auto& types = mdm_DCEModelFitter::validTypes();
  int selected_index = 0;
  bool matched = false;
  auto& b = ui.optTypeComboBox;
  {
    const QSignalBlocker blocker(b);
    // We use a signal blocker here to avoid trying to set an empty model
    //if a config file is loaded an we update the widget values
    b->clear();
    for (auto type : types)
    {
      b->addItem(type.c_str());
      if (type == processor_.madym_exe().options().optimisationType())
        matched = true;
      else if (!matched)
        selected_index++;
    }
    b->addItem(NONE_SELECTED);
  }
  b->setCurrentIndex(selected_index);
}

//
void madym_gui_ui::set_AIF_enabled()
{
  auto type = processor_.madym_exe().options().aifType();
  ui.AIFfileLineEdit->setEnabled(type == mdm_AIF::AIF_FILE);
  ui.AIFfileSelect->setEnabled(type == mdm_AIF::AIF_FILE);
  ui.AIFmapLineEdit->setEnabled(type == mdm_AIF::AIF_MAP);
  ui.AIFmapSelect->setEnabled(type == mdm_AIF::AIF_MAP);
  ui.doseLineEdit->setEnabled(type == mdm_AIF::AIF_POP || type == mdm_AIF::AIF_STD);
}

//
void madym_gui_ui::set_BValsThresholds_enabled()
{
  auto model_str = ui.dwiModelComboBox->currentText().toStdString();
  auto model = mdm_DWIModelGenerator::parseModelName(model_str);
  bool IVIM = (model == mdm_DWIModelGenerator::IVIM) || (model == mdm_DWIModelGenerator::IVIM_simple);
  ui.bThresholdsLineEdit->setEnabled(IVIM);
}

bool madym_gui_ui::check_required_options()
{
  if (runType_ != madym_gui_processor::RunType::XTR &&
    processor_.madym_exe().options().outputDir().empty())
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

void madym_gui_ui::reset_user_set_options()
{
  processor_.madym_exe().options().resetGuiOptions();
  trackChanges_ = false;
}

void madym_gui_ui::setB1Name(const QString &text)
{
  setStringOption(text, processor_.madym_exe().options().B1Name);
}

void madym_gui_ui::makeB1Consistent(bool useB1)
{
  auto & options = processor_.madym_exe().options();
  const auto vfaB1 = mdm_T1MethodGenerator::toString(mdm_T1MethodGenerator::VFA_B1);
  const auto vfa = mdm_T1MethodGenerator::toString(mdm_T1MethodGenerator::VFA);

  //Use signal blocker so we can set GUI elements without triggering callbacks
  QSignalBlocker(ui.t1MethodComboBox);
  QSignalBlocker(ui.b1CorrectionCheckBox);
  QSignalBlocker(ui.b1MapLineEdit);
  QSignalBlocker(ui.b1MapLineEdit2);

  //Set options B1 flag
  setBoolOption(useB1, options.B1Correction);
  ui.b1CorrectionCheckBox->setChecked(useB1);

  //Now make sure T1 method is correct - if using B1, method should be VFA_B1
  //If it was VFA_B1 and useB1 switched off. Set to VFA
  if (options.T1method() == vfaB1 && !useB1)
    setStringOption(vfa.c_str(), options.T1method);
    
  if (options.T1method() != vfaB1 && useB1)
    setStringOption(vfaB1.c_str(), options.T1method);
    

  //Now make sure GUI elements match
  ui.b1CorrectionCheckBox->setChecked(options.B1Correction());
  if (options.T1method() != ui.t1MethodComboBox->currentText().toStdString())
    ui.t1MethodComboBox->setCurrentText(options.T1method().c_str());
  ui.b1MapLineEdit->setText(options.B1Name().c_str());
  ui.b1MapLineEdit2->setText(options.B1Name().c_str());

  //Finally, make sure everything is correctly enabled
  ui.b1MapLineEdit->setEnabled(options.B1Correction());
  ui.b1MapLineEdit2->setEnabled(options.B1Correction());
  ui.b1MapPathSelect->setEnabled(options.B1Correction());
  ui.b1MapPathSelect2->setEnabled(options.B1Correction());
  ui.b1ScalingSpinBox->setEnabled(options.B1Correction());
  ui.b1ScalingSpinBox2->setEnabled(options.B1Correction());
}

QString madym_gui_ui::tagToString(const dicomTag& tag)
{
  if (tag.first.empty())
    return "";
  else
    return tr("%1,%2").arg(tag.first.c_str()).arg(tag.second.c_str());
}

void madym_gui_ui::setStringOption(const QString& text, mdm_input_string& option)
{
  option.set(text.toStdString());
  trackChanges(option);
}

void madym_gui_ui::setStringListOption(const QString& text, mdm_input_strings& option)
{
  option.value().fromString(text.toStdString());
  trackChanges(option);
}

void madym_gui_ui::setIntOption(const QString& text, mdm_input_int& option, QLineEdit* lineEdit)
{
  auto validator = lineEdit->validator();
  int pos = 0;
  QString str(text);
  str.replace(" ", "");
  if (validator->validate(str, pos) == QValidator::Acceptable || text.isEmpty())
  {
    option.set(text.toInt());
    lineEdit->setStyleSheet("color: black;");
    setRunValid(true, lineEdit);
  }
  else
  {
    lineEdit->setStyleSheet("color: red;");
    setRunValid(false, lineEdit);
  }
}

void madym_gui_ui::setIntOption(const int value, mdm_input_int& option)
{
  option.set(value);
  trackChanges(option);
}

void madym_gui_ui::setDoubleOption(const QString& text, mdm_input_double& option, QLineEdit* lineEdit)
{
  auto validator = lineEdit->validator();
  int pos = 0;
  QString str(text);
  str.replace(" ", "");
  if ((validator->validate(str, pos) == QValidator::Acceptable || text.isEmpty()) &&
    !str.contains(","))
  {
    option.set(text.toDouble());
    trackChanges(option);
    lineEdit->setStyleSheet("color: black;");
    setRunValid(true, lineEdit);
  }
  else
  {
    lineEdit->setStyleSheet("color: red;");
    setRunValid(false, lineEdit);
  }
}

void madym_gui_ui::setDoubleOption(const double value, mdm_input_double& option)
{
  option.set(value);
  trackChanges(option);
}

void madym_gui_ui::setBoolOption(const bool flag, mdm_input_bool& option)
{
  option.set(flag);
  trackChanges(option);
}

void madym_gui_ui::setRangeOption(const QString& text, mdm_input_ints& option, QLineEdit* lineEdit)
{
  int pos = 0;
  QString str(text);
  str.replace(" ", "");
  if (rangeValidator->validate(str, pos) == QValidator::Acceptable || text.isEmpty())
  {
    option.value().fromString(text.toStdString());
    trackChanges(option);
    lineEdit->setStyleSheet("color: black;");
    setRunValid(true, lineEdit);
  }
  else
  {
    lineEdit->setStyleSheet("color: red;");
    setRunValid(false, lineEdit);
  }
    
}

void madym_gui_ui::setDoubleListOption(const QString& text, mdm_input_doubles& option, QLineEdit* lineEdit)
{
  int pos = 0;
  QString str(text);
  str.replace(" ", "");
  if (doubleListValidator->validate(str, pos) == QValidator::Acceptable || text.isEmpty())
  {
    option.value().fromString(text.toStdString());
    trackChanges(option);
    lineEdit->setStyleSheet("color: black;");
    setRunValid(true, lineEdit);
  }
  else
  {
    lineEdit->setStyleSheet("color: red;");
    setRunValid(false, lineEdit);
  }
}

void madym_gui_ui::setTagOption(const QString& text, mdm_input_dicom_tag& option, QLineEdit* lineEdit)
{
  int pos = 0;
  QString str(text);
  str.replace(" ", "");
  if (tagValidator->validate(str, pos) == QValidator::Acceptable || text.isEmpty())
  {
    if (text.isEmpty())
      option.value().fromString(mdm_input_str::EMPTY_STR);
    else
      option.value().fromString(text.toStdString());
    trackChanges(option);

    lineEdit->setStyleSheet("color: black;");
    setRunValid(true, lineEdit);
  }
  else
  {
    lineEdit->setStyleSheet("color: red;");
    setRunValid(false, lineEdit);
  }
}

void madym_gui_ui::setRunValid(bool valid, QLineEdit* lineEdit)
{
  //Check to see if this lineEdit is on the invalid list.
  auto idx = invalidFields_.indexOf(lineEdit);
  if (valid)
  {
    //If lineEdit on list, remove it
    if (idx >= 0)
      invalidFields_.removeAt(idx);

    //Check to see if the list is now empty, if so, we're valid
    auto valid = invalidFields_.empty();
    ui.runButton->setEnabled(valid);
    ui.invalidLabel->setVisible(!valid);
  }
  else
  {
    //If lineEdit not on list add it
    if (idx < 0)
      invalidFields_.append(lineEdit);

    //We can't be valid, so disable the runButton
    ui.runButton->setEnabled(false);
    ui.invalidLabel->setVisible(true);
  }
}

template <class T> void madym_gui_ui::trackChanges(const T& option)
{
  std::stringstream ss;
  ss << option.value();
  trackChangedOption(option.key(), ss.str());
}

template void madym_gui_ui::trackChanges<mdm_input_bool>(const mdm_input_bool& option);
template void madym_gui_ui::trackChanges<mdm_input_int>(const mdm_input_int& option);
template void madym_gui_ui::trackChanges<mdm_input_double>(const mdm_input_double& option);
template void madym_gui_ui::trackChanges<mdm_input_string>(const mdm_input_string& option);
template void madym_gui_ui::trackChanges<mdm_input_ints>(const mdm_input_ints& option);
template void madym_gui_ui::trackChanges<mdm_input_doubles>(const mdm_input_doubles& option);
template void madym_gui_ui::trackChanges<mdm_input_strings>(const mdm_input_strings& option);
template void madym_gui_ui::trackChanges<mdm_input_dicom_tag>(const mdm_input_dicom_tag& option);

void madym_gui_ui::trackChangedOption(const std::string& key, const std::string& value)
{
  //If track changes flag is false, don't need to do anything
  if (!trackChanges_)
    return;

  processor_.madym_exe().options().trackGuiOptions(key, value);
}