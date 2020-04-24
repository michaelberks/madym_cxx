#include "madym_gui_ui.h"

#include <mdm_DCEModelGenerator.h>
#include "madym_gui_model_configure.h"
#include <iomanip>

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

std::vector<std::string> parse_FANames(const QString &FANames)
{
  QStringList FANamesList = FANames.split(",");
  std::vector<std::string> FANamesStd;
  foreach(QString str, FANamesList) {
    FANamesStd.push_back(str.toStdString());
  }
  return FANamesStd;
}

std::vector<double> parse_IAUCTimes(const QString &IAUCTimes)
{
  QStringList IAUCTimesList = IAUCTimes.split(",");
  std::vector<double> times;
  foreach(QString str, IAUCTimesList) {
    times.push_back(str.toDouble());
  }
  return times;
}

//-------------------------------------------------------------------------
//:Main functions
//-------------------------------------------------------------------------
void madym_gui_ui::on_computeT1Button_clicked()
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
  if (ui.outputDirLineEdit->text().isEmpty())
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Output folder not selected");
    msgBox.setInformativeText("You must select a folder in which the analysis output will be saved.");
    msgBox.exec();
    return;
  }

  mdm_ToolsOptions madym_options_;

  madym_options_.outputDir = ui.outputDirLineEdit->text().toStdString();

  madym_options_.T1method = ui.t1MethodComboBox->currentText().toStdString();
  madym_options_.T1inputNames = parse_FANames(ui.t1InputTextEdit->toPlainText());
  madym_options_.T1noiseThresh = ui.t1ThresholdLineEdit->text().toDouble();

  madym_options_.roiName = ui.roiPathLineEdit->text().toStdString();

  madym_options_.programLogName = ui.logNameLineEdit->text().toStdString();
  madym_options_.errorCodesName = ui.errorCodesLineEdit->text().toStdString();
  madym_options_.auditLogBaseName = ui.auditNameLineEdit->text().toStdString();
  madym_options_.auditLogDir = ui.auditDirLineEdit->text().toStdString();

  madym_options_.overwrite = ui.overwriteCheckBox->isChecked();
  madym_options_.help = false;
  madym_options_.version = false;

  //Instantiate new madym_exe object with these options and run
  mdm_RunTools madym_exe(madym_options_);

  std::string exeArgs;
  int result = madym_exe.run_CalculateT1(exeArgs, "madym_GUI:calculate_T1");
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
  if (ui.outputDirLineEdit->text().isEmpty())
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Output folder not selected");
    msgBox.setInformativeText("You must select a folder in which the analysis output will be saved.");
    msgBox.exec();
    return;
  }

  mdm_ToolsOptions madym_options_;

  //DCE input options
  madym_options_.inputCt = ui.inputTypeRadioButtonC->isChecked();
  madym_options_.dynDir = ui.dceInputLineEdit->text().toStdString();
  madym_options_.dynName = ui.dceNameLineEdit->text().toStdString();
  //TODO: set indexing format

  //T1 calculation
  madym_options_.T1method = ui.t1MethodComboBox->currentText().toStdString();
  madym_options_.T1inputNames = parse_FANames(ui.t1InputTextEdit->toPlainText());
  madym_options_.T1noiseThresh = ui.t1ThresholdLineEdit->text().toDouble();

  //Signal to concentration
  madym_options_.r1Const = ui.r1LineEdit->text().toDouble();
  madym_options_.injectionImage = ui.injectionImageSpinBox->value();
  madym_options_.hct = ui.hctLineEdit->text().toDouble();

  //Baseline T1
  madym_options_.useRatio = ui.s0UseRatioCheckBox->isChecked();
  if (ui.t1UsePrecomputedCheckBox->isChecked())
  {
    madym_options_.T1Name = ui.t1VolLineEdit->text().toStdString();

    if (!ui.s0UseRatioCheckBox->isChecked())
      madym_options_.S0Name = ui.s0VolLineEdit->text().toStdString();
  }

  //Logging options
  madym_options_.programLogName = ui.logNameLineEdit->text().toStdString();
  madym_options_.errorCodesName = ui.errorCodesLineEdit->text().toStdString();
  madym_options_.auditLogBaseName = ui.auditNameLineEdit->text().toStdString();
  madym_options_.auditLogDir = ui.auditDirLineEdit->text().toStdString();

  //AIF options
  if (ui.populationAIFCheckbox->isChecked())
    madym_options_.aifName = "";
  else
    madym_options_.aifName = ui.autoAIFPathLineEdit->text().toStdString();

  //Ouput options
  madym_options_.outputDir = ui.outputDirLineEdit->text().toStdString();
  madym_options_.overwrite = ui.overwriteCheckBox->isChecked();

  //Model options
  madym_options_.firstImage = ui.firstImageSpinBox->value();
  madym_options_.lastImage = ui.lastImageSpinBox->value();

  //Always leave help and version false as there are other mechanisms for displaying
  //these in the GUI
  madym_options_.help = false;
  madym_options_.version = false;

  //Instantiate new madym_exe object with these options and run
  mdm_RunTools madym_exe(madym_options_);

  //TODO - compile options string from settings
  std::string exeArgs;
  int result = madym_exe.run_AIFFit(exeArgs, "madym_GUI:AIF fit");
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
  if (ui.outputDirLineEdit->text().isEmpty())
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Output folder not selected");
    msgBox.setInformativeText("You must select a folder in which the analysis output will be saved.");
    msgBox.exec();
    return;
  }

  mdm_ToolsOptions madym_options_;

  //DCE input options
  madym_options_.inputCt = ui.inputTypeRadioButtonC->isChecked();
  madym_options_.dynDir = ui.dceInputLineEdit->text().toStdString();
  madym_options_.dynName = ui.dceNameLineEdit->text().toStdString();
  madym_options_.roiName = ui.roiPathLineEdit->text().toStdString();
  //TODO: set indexing format

  //T1 calculation
  madym_options_.T1method = ui.t1MethodComboBox->currentText().toStdString();
  madym_options_.T1inputNames = parse_FANames(ui.t1InputTextEdit->toPlainText());
  madym_options_.T1noiseThresh = ui.t1ThresholdLineEdit->text().toDouble();

  //Signal to concentration
  madym_options_.r1Const = ui.r1LineEdit->text().toDouble();
  madym_options_.injectionImage = ui.injectionImageSpinBox->value();
  madym_options_.dose = ui.doseLineEdit->text().toDouble();
  madym_options_.hct = ui.hctLineEdit->text().toDouble();

  //Baseline T1
  madym_options_.useRatio = ui.s0UseRatioCheckBox->isChecked();
  if (ui.t1UsePrecomputedCheckBox->isChecked())
  {
    madym_options_.T1Name = ui.t1VolLineEdit->text().toStdString();

    if (!ui.s0UseRatioCheckBox->isChecked())
      madym_options_.S0Name = ui.s0VolLineEdit->text().toStdString();
  }

  //Logging options
  madym_options_.programLogName = ui.logNameLineEdit->text().toStdString();
  madym_options_.errorCodesName = ui.errorCodesLineEdit->text().toStdString();
  madym_options_.auditLogBaseName = ui.auditNameLineEdit->text().toStdString();
  madym_options_.auditLogDir = ui.auditDirLineEdit->text().toStdString();

  //AIF options
  if (ui.populationAIFCheckbox->isChecked())
    madym_options_.aifName = "";
  else
    madym_options_.aifName = ui.autoAIFPathLineEdit->text().toStdString();

  if (ui.populationPIFCheckbox->isChecked())
    madym_options_.pifName = "";
  else
    madym_options_.pifName = ui.autoPIFPathLineEdit->text().toStdString();

  //Ouput options
  madym_options_.outputDir = ui.outputDirLineEdit->text().toStdString();
  madym_options_.overwrite = ui.overwriteCheckBox->isChecked();
  madym_options_.outputCt = ui.outputCsCheckBox->isChecked();
  madym_options_.outputCm = ui.outputCmCheckBox->isChecked();
  madym_options_.IAUCTimes = parse_IAUCTimes(ui.iaucTimesLineEdit->text());

  //Model options
  madym_options_.model = ui.modelSelectComboBox->currentText().toStdString();
  madym_options_.dynNoise = ui.temporalNoiseCheckBox->isChecked();
  madym_options_.noOptimise = !ui.optimiseFitCheckBox->isChecked();
  madym_options_.noEnhFlag = ui.testEnhancementCheckBox->isChecked();
  madym_options_.firstImage = ui.firstImageSpinBox->value();
  madym_options_.lastImage = ui.lastImageSpinBox->value();
  //Model configurations - if changed from defaults - will be set by the model configurer GUI
  madym_options_.maxIterations = ui.maxIterationsLineEdit->text().toInt();

  //Always leave help and version false as there are other mechanisms for displaying
  //these in the GUI
  madym_options_.help = false;
  madym_options_.version = false;

  //Instantiate new madym_exe object with these options and run
  mdm_RunTools madym_exe(madym_options_);

  //TODO - compile options string from settings
  std::string exeArgs;
  int result = madym_exe.run_DCEFit(exeArgs, "madym_GUI:DCE fit");
}
void madym_gui_ui::on_outputStatsButton_clicked()
{

}
//-------------------------------------------------------------------------
//:DCE data options
void madym_gui_ui::on_dceInputLineEdit_textChanged()
{

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
void madym_gui_ui::on_dceNameLineEdit_textChanged()
{

}
void madym_gui_ui::on_dceFormatLineEdit_textChanged()
{

}
void madym_gui_ui::on_roiPathLineEdit_textChanged()
{

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

//-------------------------------------------------------------------------
//:T1 calculation options
void madym_gui_ui::on_t1MethodComboBox_currentIndexChanged(const QString &text)
{

}
void madym_gui_ui::on_t1InputTextEdit_textChanged()
{

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
void madym_gui_ui::on_t1ThresholdLineEdit_textChanged()
{

}

//-------------------------------------------------------------------------
//:Signal to concentration_options
void madym_gui_ui::on_r1LineEdit_textChanged()
{

}
void madym_gui_ui::on_injectionImageSpinBox_valueChanged(int value)
{

}
void madym_gui_ui::on_doseLineEdit_textChanged()
{

}
void madym_gui_ui::on_hctLineEdit_textChanged()
{

}

//-------------------------------------------------------------------------
//:Baseline T1 options
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
void madym_gui_ui::on_t1VolLineEdit_textChanged()
{
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
void madym_gui_ui::on_s0VolLineEdit_textChanged()
{

}
void madym_gui_ui::on_s0VolPathSelect_clicked()
{
  QString selectedPath = QFileDialog::getOpenFileName(this, tr("Select baseline S0 map"),
    "",
    tr("Map files (*.hdr)"));

  if (selectedPath.isEmpty())
    return;

  ui.s0VolLineEdit->setText(selectedPath);
}
 
//-------------------------------------------------------------------------
//:Logging options
void madym_gui_ui::on_logNameLineEdit_textChanged()
{

}
void madym_gui_ui::on_errorCodesLineEdit_textChanged()
{

}
void madym_gui_ui::on_auditNameLineEdit_textChanged()
{

}
void madym_gui_ui::on_auditDirLineEdit_textChanged()
{

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
void madym_gui_ui::on_autoAIFPathLineEdit_textChanged()
{

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
void madym_gui_ui::on_autoPIFPathLineEdit_textChanged()
{

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

//-------------------------------------------------------------------------
//:Output options
void madym_gui_ui::on_outputDirLineEdit_textChanged()
{
  //Doesn't need to be implemented
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
void madym_gui_ui::on_iaucTimesLineEdit_textChanged()
{

} 

//-------------------------------------------------------------------------
//:Model fitting
void madym_gui_ui::on_modelSelectComboBox_currentIndexChanged(const QString &text)
{
  mdm_AIF aif;
  mdm_DCEModelGenerator::setModel(model_, aif,
    text.toStdString(), false, false, {},
		{}, {}, {}, {}, {});
  madym_options_.paramNames = {};
  madym_options_.initParams = {}; 
  madym_options_.fixedParams = {};
  madym_options_.fixedValues = {};
	madym_options_.relativeLimitParams = {};
	madym_options_.relativeLimitValues = {};
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
  mdm_AIF aif;
  const QString &modelName = ui.modelSelectComboBox->currentText();
  mdm_DCEModelGenerator::setModel(model_, aif,
    modelName.toStdString(),
    false, false, madym_options_.paramNames,
    madym_options_.initParams, 
		madym_options_.fixedParams, madym_options_.fixedValues,
		madym_options_.relativeLimitParams, madym_options_.relativeLimitValues);

  madym_gui_model_configure optionsWindow(*model_, modelName, madym_options_, this);
  const int response = optionsWindow.exec();
  /**/
}
void madym_gui_ui::on_firstImageSpinBox_valueChanged(int value)
{

}
void madym_gui_ui::on_lastImageSpinBox_valueChanged(int value)
{

}
void madym_gui_ui::on_optimiseFitCheckBox_stateChanged(int state)
{
  ui.testEnhancementCheckBox->setEnabled(state);
  ui.maxIterationsLineEdit->setEnabled(state);


}
void madym_gui_ui::on_maxIterationsLineEdit_textChanged()
{

} 

//-------------------------------------------------------------------------
//: Other slots
void madym_gui_ui::change_input_type(int type)
{

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
	
  // Connect signals to slots that will run in the thread
  /*
	QObject::connect( this, SIGNAL(frame_tagged(int, bool, bool, bool)),
                    &processor_, SLOT(process_frame(int, bool, bool, bool)) );

	//Processor tells GUI there's a frame ready to draw
  QObject::connect( &processor_, SIGNAL(frame_to_draw( int )),
                    this, SLOT(redraw_scene( int )) );*/

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
  ui.inputTypeRadioButtonC->setChecked(madym_options_.inputCt);
  ui.dceInputLineEdit->setText(madym_options_.dynDir.c_str());
  ui.dceNameLineEdit->setText(madym_options_.dynName.c_str());
  ui.dceFormatLineEdit->setText(madym_options_.dynFormat.c_str());
  ui.roiPathLineEdit->setText(madym_options_.roiName.c_str());

  //T1 calculation
  //TODO: set-up T1 methods box method?
  ui.t1MethodComboBox->addItem(madym_options_.T1method.c_str());
  ui.t1ThresholdLineEdit->setValidator(new QDoubleValidator(0, 10000, 2, this));
  ui.t1ThresholdLineEdit->setText(QString::number(madym_options_.T1noiseThresh));

  //Signal to concentration - TODO constrain inputs and put units on number inputs
  ui.r1LineEdit->setText(QString::number(madym_options_.r1Const));
  ui.injectionImageSpinBox->setValue(madym_options_.injectionImage);
  ui.doseLineEdit->setText(QString::number(madym_options_.dose));
  ui.hctLineEdit->setText(QString::number(madym_options_.hct));

  //Baseline T1
  ui.s0UseRatioCheckBox->setChecked(madym_options_.useRatio);
  ui.t1VolLineEdit->setText(madym_options_.T1Name.c_str());
  ui.t1UsePrecomputedCheckBox->setChecked(!madym_options_.T1Name.empty());
  ui.s0VolLineEdit->setText(madym_options_.S0Name.c_str());
  ui.t1VolLineEdit->setEnabled(ui.t1UsePrecomputedCheckBox->isChecked());
  ui.t1VolPathSelect->setEnabled(ui.t1UsePrecomputedCheckBox->isChecked());
  ui.s0VolLineEdit->setEnabled(!madym_options_.useRatio &&
    ui.t1UsePrecomputedCheckBox->isChecked());
  ui.s0VolPathSelect->setEnabled(!madym_options_.useRatio && 
    ui.t1UsePrecomputedCheckBox->isChecked());

  //Logging options
  ui.logNameLineEdit->setText(madym_options_.programLogName.c_str());
  ui.errorCodesLineEdit->setText(madym_options_.errorCodesName.c_str());
  ui.auditNameLineEdit->setText(madym_options_.auditLogBaseName.c_str());
  ui.auditDirLineEdit->setText(madym_options_.auditLogDir.c_str());

  //AIF options
  ui.populationAIFCheckbox->setChecked(madym_options_.aifName.empty());
  ui.autoAIFPathLineEdit->setText(madym_options_.aifName.c_str());
  ui.populationPIFCheckbox->setChecked(madym_options_.pifName.empty());
  ui.autoPIFPathLineEdit->setText(madym_options_.pifName.c_str());

  //Ouput options
  ui.outputDirLineEdit->setText(madym_options_.outputDir.c_str());
  ui.overwriteCheckBox->setChecked(madym_options_.overwrite);
  ui.outputCsCheckBox->setChecked(madym_options_.outputCt);
  ui.outputCmCheckBox->setChecked(madym_options_.outputCm);
  //TODO write IAUC times parser
  QString iauc_str = QString::number(madym_options_.IAUCTimes[0]);
  for (int i = 1; i < madym_options_.IAUCTimes.size(); i++)
    iauc_str.append(",").append(QString::number(madym_options_.IAUCTimes[i]));
  ui.iaucTimesLineEdit->setText(iauc_str);

  //Model options
  const std::vector<std::string> &models = mdm_DCEModelGenerator::implementedModels();
  for (auto model : models)
    ui.modelSelectComboBox->addItem(model.c_str());
  
  ui.temporalNoiseCheckBox->setChecked(madym_options_.dynNoise);
  ui.optimiseFitCheckBox->setChecked(!madym_options_.noOptimise);
  ui.testEnhancementCheckBox->setChecked(madym_options_.noEnhFlag);
  ui.firstImageSpinBox->setValue(0);
  ui.lastImageSpinBox->setValue(0);
  ui.maxIterationsLineEdit->setValidator(new QIntValidator(0, 10000, this));
  ui.maxIterationsLineEdit->setText(QString::number(madym_options_.maxIterations));
  
  //TODO - how to configure model in GUI?
  /*madym_options_.initParams = initParams();
  madym_options_.initMapsDir = initMapsDir();
  madym_options_.paramNames = paramNames();
  madym_options_.fixedParams = fixedParams();
  madym_options_.fixedValues = fixedValues();*/

  //Always leave help and version false as there are other mechanisms for displaying
  //these in the GUI
  madym_options_.help = false;
  madym_options_.version = false;
}
