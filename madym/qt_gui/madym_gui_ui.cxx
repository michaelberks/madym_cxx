/*!
*  @file    madym_gui_ui.cxx
*  @brief   Implementation of madym_gui_ui class
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#include "madym_gui_ui.h"

#include <madym/dce_models/mdm_DCEModelGenerator.h>
#include <madym/run/mdm_RunTools_madym_T1.h>
#include <madym/run/mdm_RunTools_madym_DCE.h>

#include "madym_gui_model_configure.h"
#include <iomanip>

static const std::string DCE_ARGV = "madym_GUI-DCE_fit";
static const std::string T1_ARGV = "madym_GUI-T1";
static const std::string AIF_ARGV = "madym_GUI-AIF";
static const std::string STATS_ARGV = "madym_GUI-stats";
static const QString NONE_SELECTED = "<None selected>";
//
//: Constructor
madym_gui_ui::madym_gui_ui(QWidget *parent)
: QMainWindow(parent),
  model_(NULL)
{
	// setup the UI
  ui.setupUi(this);

	//Set up the radio button group to select the current camera
	inputTypeRadioGroup = new QButtonGroup(this);
  inputTypeRadioGroup->addButton(ui.inputTypeRadioButtonS, 0);
  inputTypeRadioGroup->addButton(ui.inputTypeRadioButtonC, 1);
  inputTypeRadioGroup->setExclusive(true);
  //ui.activeCameraButton1->setChecked(true);
  QObject::connect( inputTypeRadioGroup, SIGNAL(buttonClicked( int )),
                    this, SLOT(change_input_type( int )) );

  initialize_widget_values();

  initialize_processor_thread();
  connect_signals_to_slots();
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

std::vector<std::string> parse_string_list(const QString &text)
{
  QStringList textList = text.split(";");
  std::vector<std::string> strings;
  foreach(QString str, textList) {
		strings.push_back(str.toStdString());
  }
  return strings;
}

QString make_strings_text(const std::vector<std::string> &strings)
{
	QString text;
	if (strings.empty())
		text = "";
	else
	{
		text = strings[0].c_str();

		for (int it = 1; it < strings.size(); it++)
			text.append(";").append(strings[it].c_str());
	}
	return text;
}

std::vector<double> parse_double_list(const QString &text)
{
  QStringList textList = text.split(",");
  std::vector<double> doubles;
  foreach(QString str, textList) {
		doubles.push_back(str.toDouble());
  }
  return doubles;
}

QString make_doubles_text(const std::vector<double> &doubles)
{
	QString text;
	if (doubles.empty())
		text = "";
	else
	{
		text = QString::number(doubles[0]);
		for (int it = 1; it < doubles.size(); it++)
			text.append(",").append(QString::number(doubles[it]));
	}	

	return text;
}

//-------------------------------------------------------------------------
//:Menu file
//-------------------------------------------------------------------------
void madym_gui_ui::on_actionLoadConfigFile_triggered()
{
	//Open file select and get config filename
	QString config_file = QFileDialog::getOpenFileName(this, tr("Select config file"),
		"",
		tr("Config files (*.txt *.cfg);;All files (*.*)"));

	if (config_file.isEmpty())
		return;

	//Call input options to parse madym arguments, this will set all
	//the current madym_options_ fields into the input options
	//variable map and then check the config file	
	madym_options_.configFile.set(config_file.toStdString());
	mdm_RunTools_madym_DCE madym_exe(madym_options_, options_parser_);
	if (madym_exe.parseInputs(DCE_ARGV))
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
void madym_gui_ui::on_actionSaveConfigFileDCE_triggered()
{
	QString config_file = QFileDialog::getSaveFileName(this, tr("Select config file"),
		"",
		tr("Config files (*.txt *.cfg);;All files (*.*)"));
	
	if (config_file.isEmpty())
		return;

	//Make sure options config file is empty, because when we call parse args
	//we don't want to read a config file
	madym_options_.configFile.set("");
	mdm_RunTools_madym_DCE madym_exe(madym_options_, options_parser_);
	madym_exe.parseInputs(DCE_ARGV);
	options_parser_.to_file(config_file.toStdString(), madym_options_);

}
void madym_gui_ui::on_actionSaveConfigFileT1_triggered()
{
	QString config_file = QFileDialog::getSaveFileName(this, tr("Select config file"),
		"",
		tr("Config files (*.txt *.cfg);;All files (*.*)"));

	if (config_file.isEmpty())
		return;

	//Make sure options config file is empty, because when we call parse args
	//we don't want to read a config file
	madym_options_.configFile.set("");
	mdm_RunTools_madym_T1 madym_exe(madym_options_, options_parser_);
	madym_exe.parseInputs(T1_ARGV);
	options_parser_.to_file(config_file.toStdString(), madym_options_);

}
void madym_gui_ui::on_actionSaveConfigFileIF_triggered()
{
	QString config_file = QFileDialog::getSaveFileName(this, tr("Select config file"),
		"",
		tr("Config files (*.txt *.cfg);;All files (*.*)"));

	if (config_file.isEmpty())
		return;

	//Make sure options config file is empty, because when we call parse args
	//we don't want to read a config file
	madym_options_.configFile.set("");
	mdm_RunTools_madym_DCE madym_exe(madym_options_, options_parser_);
	madym_exe.parseInputs(AIF_ARGV);
	options_parser_.to_file(config_file.toStdString(), madym_options_);

}
void madym_gui_ui::on_actionExit_triggered()
{
	close();
}

//-------------------------------------------------------------------------
//:Main functions
//-------------------------------------------------------------------------
void madym_gui_ui::on_computeT1Button_clicked()
{
  //First check the user has actually defineds that are required
  if (madym_options_.outputDir().empty())
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Output folder not selected");
    msgBox.setInformativeText("You must select a folder in which the analysis output will be saved.");
    msgBox.exec();
    return;
  }

	//Make sure options config file is empty, because when we call parse args
	//we don't want to read a config file
	madym_options_.configFile.set("");
	mdm_RunTools_madym_T1 madym_exe(madym_options_, options_parser_);
	madym_exe.parseInputs(T1_ARGV);
	int result = madym_exe.run();
}

void madym_gui_ui::on_computeIFButton_clicked()
{
  //First check the user has actually defineds that are required
  if (!ui.inputTypeRadioButtonC->isChecked() && !ui.inputTypeRadioButtonS->isChecked())
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Input type not selected");
    msgBox.setInformativeText("You must select input type as either signal or concentration.");
    msgBox.exec();
    return;
  }
  //First check the user has actually defineds that are required
  if (madym_options_.outputDir().empty())
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Output folder not selected");
    msgBox.setInformativeText("You must select a folder in which the analysis output will be saved.");
    msgBox.exec();
    return;
  }

	//Make sure options config file is empty, because when we call parse args
	//we don't want to read a config file
	madym_options_.configFile.set("");

	mdm_RunTools_madym_DCE madym_exe(madym_options_, options_parser_);
	madym_exe.parseInputs(AIF_ARGV);
	int result = madym_exe.run();

}
void madym_gui_ui::on_fitModelButton_clicked()
{
  //First check the user has actually defineds that are required
  if (!ui.inputTypeRadioButtonC->isChecked() && !ui.inputTypeRadioButtonS->isChecked())
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Input type not selected");
    msgBox.setInformativeText("You must select input type as either signal or concentration.");
    msgBox.exec();
    return;
  }
  //First check the user has actually defineds that are required
  if (madym_options_.outputDir().empty())
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Output folder not selected");
    msgBox.setInformativeText("You must select a folder in which the analysis output will be saved.");
    msgBox.exec();
    return;
  }

	//Make sure options config file is empty, because when we call parse args
	//we don't want to read a config file
	madym_options_.configFile.set("");
	mdm_RunTools_madym_DCE madym_exe(madym_options_, options_parser_);
	madym_exe.parseInputs(DCE_ARGV);
	int result = madym_exe.run();
}
void madym_gui_ui::on_outputStatsButton_clicked()
{

}

//-------------------------------------------------------------------------
//:DCE data options
void madym_gui_ui::on_dceInputLineEdit_textChanged(const QString &text)
{
	madym_options_.dynDir.set(text.toStdString());
}
void madym_gui_ui::on_dceInputSelect_clicked()
{
  QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Choose DCE input folder"),
    "",
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  if (selectedDir.isEmpty())
    return;

  ui.dceInputLineEdit->setText(selectedDir);
}
void madym_gui_ui::on_dceNameLineEdit_textChanged(const QString &text)
{
	madym_options_.dynName.set(text.toStdString());
}

void madym_gui_ui::on_dceFormatLineEdit_textChanged(const QString &text)
{
	madym_options_.dynFormat.set(text.toStdString());
}

void madym_gui_ui::on_roiPathLineEdit_textChanged(const QString &text)
{
	madym_options_.roiName.set(text.toStdString());
}

void madym_gui_ui::on_roiPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select ROI mask"),
    "",
    tr("Mask files (*.hdr)"));

  if (selectedPath.isEmpty())
    return;

  ui.roiPathLineEdit->setText(selectedPath);
}

void madym_gui_ui::on_nDynSpinBox_valueChanged(int value)
{
	madym_options_.nDyns.set(value);
}

void madym_gui_ui::on_injectionImageSpinBox_valueChanged(int value)
{
	madym_options_.injectionImage.set(value);
}

//-------------------------------------------------------------------------
//:T1 calculation options
void madym_gui_ui::on_t1MethodComboBox_currentIndexChanged(const QString &text)
{
	madym_options_.T1method.set(text.toStdString());
}
void madym_gui_ui::on_t1InputTextEdit_textChanged()
{
	madym_options_.T1inputNames.set(parse_string_list(ui.t1InputTextEdit->toPlainText()));
}
void madym_gui_ui::on_t1InputSelect_clicked()
{
  QStringList selectedMaps = QFileDialog::getOpenFileNames(this, tr("Select input maps for baseline T1 calculation"),
    "",
    tr("Map files (*.hdr)"));

  if (selectedMaps.isEmpty())
    return;

  QString maps = selectedMaps[0];
  for (int i = 1; i < selectedMaps.length(); i++)
    maps.append(";").append(selectedMaps[i]);

  ui.t1InputTextEdit->setText(maps);
}
void madym_gui_ui::on_t1ThresholdLineEdit_textChanged(const QString &text)
{
	madym_options_.T1noiseThresh.set(text.toDouble());
}

//-------------------------------------------------------------------------
//:Signal to concentration_options

void madym_gui_ui::on_s0UseRatioCheckBox_stateChanged(int state)
{
  ui.s0VolLineEdit->setEnabled(!state && ui.t1UsePrecomputedCheckBox->isChecked());
  ui.s0VolPathSelect->setEnabled(!state && ui.t1UsePrecomputedCheckBox->isChecked());
}
void madym_gui_ui::on_t1UsePrecomputedCheckBox_stateChanged(int state)
{
  ui.t1VolLineEdit->setEnabled(state);
  ui.t1VolPathSelect->setEnabled(state);
  ui.s0VolLineEdit->setEnabled(state && !ui.s0UseRatioCheckBox->isChecked());
  ui.s0VolPathSelect->setEnabled(state && !ui.s0UseRatioCheckBox->isChecked());
}
void madym_gui_ui::on_t1VolLineEdit_textChanged(const QString &text)
{
	madym_options_.T1Name.set(text.toStdString());
}

void madym_gui_ui::on_t1VolPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select baseline T1 map"),
    "",
    tr("Map files (*.hdr)"));

  if (selectedPath.isEmpty())
    return;

  ui.t1VolLineEdit->setText(selectedPath);
}
void madym_gui_ui::on_s0VolLineEdit_textChanged(const QString &text)
{
	madym_options_.M0Name.set(text.toStdString());
}

void madym_gui_ui::on_s0VolPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select baseline M0 map"),
    "",
    tr("Map files (*.hdr)"));

  if (selectedPath.isEmpty())
    return;

  ui.s0VolLineEdit->setText(selectedPath);
}

void madym_gui_ui::on_r1LineEdit_textChanged(const QString &text)
{
	madym_options_.r1Const.set(text.toDouble());
}
 
//-------------------------------------------------------------------------
//:Logging options
void madym_gui_ui::on_logNameLineEdit_textChanged(const QString &text)
{
	madym_options_.programLogName.set(text.toStdString());
}
void madym_gui_ui::on_errorCodesLineEdit_textChanged(const QString &text)
{
	madym_options_.errorCodesName.set(text.toStdString());
}
void madym_gui_ui::on_auditNameLineEdit_textChanged(const QString &text)
{
	madym_options_.auditLogBaseName.set(text.toStdString());
}
void madym_gui_ui::on_auditDirLineEdit_textChanged(const QString &text)
{
	madym_options_.auditLogDir.set(text.toStdString());
}
void madym_gui_ui::on_auditDirSelect_clicked()
{
  QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Choose output folder"),
    "",
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  if (selectedDir.isEmpty())
    return;

  ui.auditDirLineEdit->setText(selectedDir);
}

//-------------------------------------------------------------------------
//:AIF options
void madym_gui_ui::on_populationAIFCheckbox_stateChanged(int state)
{
  ui.autoAIFPathLineEdit->setEnabled(!state);
  ui.autoAIFPathSelect->setEnabled(!state);
}
void madym_gui_ui::on_autoAIFPathLineEdit_textChanged(const QString &text)
{
	madym_options_.aifName.set(text.toStdString());
}
void madym_gui_ui::on_autoAIFPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select AIF file"),
    "",
    tr("AIF files (*.txt)"));

  if (selectedPath.isEmpty())
    return;

  ui.autoAIFPathLineEdit->setText(selectedPath);
}
void madym_gui_ui::on_populationPIFCheckbox_stateChanged(int state)
{
  ui.autoPIFPathLineEdit->setEnabled(!state);
  ui.autoPIFPathSelect->setEnabled(!state);
}
void madym_gui_ui::on_autoPIFPathLineEdit_textChanged(const QString &text)
{
	madym_options_.pifName.set(text.toStdString());
}
void madym_gui_ui::on_autoPIFPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select PIF file"),
    "",
    tr("PIF files (*.txt)"));

  if (selectedPath.isEmpty())
    return;

  ui.autoPIFPathLineEdit->setText(selectedPath);
} 

void madym_gui_ui::on_doseLineEdit_textChanged(const QString &text)
{
	madym_options_.dose.set(text.toDouble());
}
void madym_gui_ui::on_hctLineEdit_textChanged(const QString &text)
{
	madym_options_.hct.set(text.toDouble());
}

//-------------------------------------------------------------------------
//:Output options
void madym_gui_ui::on_outputDirLineEdit_textChanged(const QString &text)
{
	madym_options_.outputDir.set(text.toStdString());
}
void madym_gui_ui::on_outputDirSelect_clicked()
{
  QString outputDir = QFileDialog::getExistingDirectory(this, tr("Choose output folder"),
    "",
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  if (outputDir.isEmpty())
    return;

  ui.outputDirLineEdit->setText(outputDir);
}
void madym_gui_ui::on_iaucTimesLineEdit_textChanged(const QString &text)
{
	madym_options_.IAUCTimes.set(parse_double_list(text));
}
void madym_gui_ui::on_initMapsLineEdit_textChanged(const QString &text)
{
	madym_options_.initMapsDir.set(text.toStdString());
}
void madym_gui_ui::on_initMapsDirSelect_clicked()
{
	QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Choose output folder"),
		"",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	if (selectedDir.isEmpty())
		return;

	ui.initMapsLineEdit->setText(selectedDir);
}
void madym_gui_ui::on_overwriteCheckBox_stateChanged(int state)
{
	madym_options_.overwrite.set(state);
}
void madym_gui_ui::on_outputCsCheckBox_stateChanged(int state)
{
	madym_options_.outputCt_sig.set(state);
}
void madym_gui_ui::on_outputCmCheckBox_stateChanged(int state)
{
	madym_options_.outputCt_mod.set(state);
}
void madym_gui_ui::on_sparseCheckBox_stateChanged(int state)
{
	madym_options_.sparseWrite.set(state);
}

//-------------------------------------------------------------------------
//:Model fitting
void madym_gui_ui::on_modelSelectComboBox_currentIndexChanged(const QString &text)
{
	if (text == NONE_SELECTED)
		return;

	mdm_AIF aif;
	auto modelType = mdm_DCEModelGenerator::ParseModelName(text.toStdString());
  model_ = mdm_DCEModelGenerator::createModel(aif,
    modelType, false, false, {},
		{}, {}, {}, {}, {});
	madym_options_.model.set(text.toStdString());
  madym_options_.paramNames.set({});
  madym_options_.initialParams.set({});
  madym_options_.fixedParams.set({});
  madym_options_.fixedValues.set({});
	madym_options_.relativeLimitParams.set({});
	madym_options_.relativeLimitValues.set({});
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
	model_ = mdm_DCEModelGenerator::createModel(aif,
		modelType,
    false, false, madym_options_.paramNames(),
    madym_options_.initialParams(), 
		madym_options_.fixedParams(), madym_options_.fixedValues(),
		madym_options_.relativeLimitParams(), madym_options_.relativeLimitValues());

  madym_gui_model_configure optionsWindow(*model_, modelName, madym_options_, this);
  const int response = optionsWindow.exec();
  /**/
}
void madym_gui_ui::on_firstImageSpinBox_valueChanged(int value)
{
	madym_options_.firstImage.set(value);
}
void madym_gui_ui::on_lastImageSpinBox_valueChanged(int value)
{
	madym_options_.lastImage.set(value);
}
void madym_gui_ui::on_temporalNoiseCheckBox_stateChanged(int state)
{
	madym_options_.dynNoise.set(state);
}
void madym_gui_ui::on_optimiseFitCheckBox_stateChanged(int state)
{
  ui.maxIterationsLineEdit->setEnabled(state);
	madym_options_.noOptimise.set(!state);

}
void madym_gui_ui::on_testEnhancementCheckBox_stateChanged(int state)
{
	madym_options_.testEnhancement.set(state);
}
void madym_gui_ui::on_maxIterationsLineEdit_textChanged(const QString &text)
{
	madym_options_.maxIterations.set(text.toInt());
} 

//-------------------------------------------------------------------------
//: Other slots
void madym_gui_ui::change_input_type(int type)
{
	//Type will be 0 if signal selected, 1 if concentration selected
	madym_options_.inputCt.set(type);
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
  // Signals that trigger slots in the main thread
}

bool madym_gui_ui::winEvent(MSG * message, long * result)
{
	//Placeholder in case we want to intercept keyboard entries etc.
	return false;
}

void madym_gui_ui::initialize_widget_values()
{

  //DCE input options
	ui.inputTypeRadioButtonS->setChecked(!madym_options_.inputCt());
  ui.inputTypeRadioButtonC->setChecked(madym_options_.inputCt());
  ui.dceInputLineEdit->setText(madym_options_.dynDir().c_str());
  ui.dceNameLineEdit->setText(madym_options_.dynName().c_str());
  ui.dceFormatLineEdit->setText(madym_options_.dynFormat().c_str());
	ui.nDynSpinBox->setValue(madym_options_.nDyns());
	ui.injectionImageSpinBox->setValue(madym_options_.injectionImage());
  ui.roiPathLineEdit->setText(madym_options_.roiName().c_str());

  //T1 calculation
  //TODO: set-up T1 methods box method?
  ui.t1MethodComboBox->addItem(madym_options_.T1method().c_str());
  ui.t1ThresholdLineEdit->setValidator(new QDoubleValidator(0, 10000, 2, this));
  ui.t1ThresholdLineEdit->setText(QString::number(madym_options_.T1noiseThresh()));
	ui.t1InputTextEdit->setText(make_strings_text(madym_options_.T1inputNames()));

  //Signal to concentration - TODO constrain inputs and put units on number inputs
	ui.s0UseRatioCheckBox->setChecked(madym_options_.M0Ratio());
	ui.t1VolLineEdit->setText(madym_options_.T1Name().c_str());
	ui.t1UsePrecomputedCheckBox->setChecked(!madym_options_.T1Name().empty());
	ui.s0VolLineEdit->setText(madym_options_.M0Name().c_str());
	ui.t1VolLineEdit->setEnabled(ui.t1UsePrecomputedCheckBox->isChecked());
	ui.t1VolPathSelect->setEnabled(ui.t1UsePrecomputedCheckBox->isChecked());
	ui.s0VolLineEdit->setEnabled(!madym_options_.M0Ratio() &&
		ui.t1UsePrecomputedCheckBox->isChecked());
	ui.s0VolPathSelect->setEnabled(!madym_options_.M0Ratio() &&
		ui.t1UsePrecomputedCheckBox->isChecked());
  ui.r1LineEdit->setText(QString::number(madym_options_.r1Const()));
  
	//AIF options
	ui.populationAIFCheckbox->setChecked(madym_options_.aifName().empty());
	ui.autoAIFPathLineEdit->setText(madym_options_.aifName().c_str());
	ui.populationPIFCheckbox->setChecked(madym_options_.pifName().empty());
	ui.autoPIFPathLineEdit->setText(madym_options_.pifName().c_str());
	ui.doseLineEdit->setText(QString::number(madym_options_.dose()));
	ui.hctLineEdit->setText(QString::number(madym_options_.hct()));
  
	//Ouput options
	ui.outputDirLineEdit->setText(madym_options_.outputDir().c_str());
	ui.overwriteCheckBox->setChecked(madym_options_.overwrite());
	ui.outputCsCheckBox->setChecked(madym_options_.outputCt_sig());
	ui.outputCmCheckBox->setChecked(madym_options_.outputCt_mod());
	ui.sparseCheckBox->setChecked(madym_options_.sparseWrite());
	ui.iaucTimesLineEdit->setText(make_doubles_text(madym_options_.IAUCTimes()));

  //Logging options
  ui.logNameLineEdit->setText(madym_options_.programLogName().c_str());
  ui.errorCodesLineEdit->setText(madym_options_.errorCodesName().c_str());
	ui.configLineEdit->setText(madym_options_.outputConfigFileName().c_str());
  ui.auditNameLineEdit->setText(madym_options_.auditLogBaseName().c_str());
  ui.auditDirLineEdit->setText(madym_options_.auditLogDir().c_str());

  //Model options
  const std::vector<std::string> &models = mdm_DCEModelGenerator::implementedModels();
	int selected_index = 0;
	{
		const QSignalBlocker blocker(ui.modelSelectComboBox);
		// We use a signal blocker here to avoid trying to set an empty model
		//if a config file is loaded an we update the widget values
		ui.modelSelectComboBox->clear();
		ui.modelSelectComboBox->addItem(NONE_SELECTED);
		int index = 0;		
		for (auto model : models)
		{
			ui.modelSelectComboBox->addItem(model.c_str());
			index++;
			if (model == madym_options_.model())
				selected_index = 0;
		}
	}
	ui.modelSelectComboBox->setCurrentIndex(selected_index);
  
  ui.firstImageSpinBox->setValue(madym_options_.firstImage());
  ui.lastImageSpinBox->setValue(madym_options_.lastImage());
  ui.temporalNoiseCheckBox->setChecked(madym_options_.dynNoise());
  ui.optimiseFitCheckBox->setChecked(!madym_options_.noOptimise());
  ui.testEnhancementCheckBox->setChecked(madym_options_.testEnhancement());
  ui.maxIterationsLineEdit->setValidator(new QIntValidator(0, 10000, this));
  ui.maxIterationsLineEdit->setText(QString::number(madym_options_.maxIterations()));
	ui.initMapsLineEdit->setText(madym_options_.initMapsDir().c_str());

}
