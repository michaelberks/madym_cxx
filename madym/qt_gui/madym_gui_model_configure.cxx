/*!
*  @file    madym_gui_model_configure.cxx
*  @brief   Implementation of madym_gui_model_configure class
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#include "madym_gui_model_configure.h"
#include <QDoubleValidator>
 
//
// Public methods
//

madym_gui_model_configure::madym_gui_model_configure(const mdm_DCEModelBase &model, const QString &modelName,
	mdm_InputOptions &madym_options,
  QWidget *parent)
  :QDialog(parent),
  madym_options_(madym_options),
  model_(model)
{
  // setup the UI
  ui.setupUi(this);

	QRegExp doubleListREX("^[0-9]+(\\.[0-9]+)?(?:,([0-9]+(\\.[0-9]+)?))*$");
	doubleListREX.setPatternSyntax(QRegExp::RegExp);
	doubleListValidator = new QRegExpValidator(doubleListREX);

  paramControls_.push_back(
		paramControls(ui.paramLabel_1, ui.paramLineEdit_1, ui.fixedCheckBox_1,
			ui.mapsCheckBox_1, ui.lowerBoundLineEdit_1, ui.upperBoundLineEdit_1, ui.relLimitLineEdit_1, ui.rptValuesLineEdit_1));
  paramControls_.push_back(
		paramControls(ui.paramLabel_2, ui.paramLineEdit_2, ui.fixedCheckBox_2,
			ui.mapsCheckBox_2, ui.lowerBoundLineEdit_2, ui.upperBoundLineEdit_2, ui.relLimitLineEdit_2, ui.rptValuesLineEdit_2));
  paramControls_.push_back(
		paramControls(ui.paramLabel_3, ui.paramLineEdit_3, ui.fixedCheckBox_3,
			ui.mapsCheckBox_3, ui.lowerBoundLineEdit_3, ui.upperBoundLineEdit_3, ui.relLimitLineEdit_3, ui.rptValuesLineEdit_3));
  paramControls_.push_back(
		paramControls(ui.paramLabel_4, ui.paramLineEdit_4, ui.fixedCheckBox_4,
			ui.mapsCheckBox_4, ui.lowerBoundLineEdit_4, ui.upperBoundLineEdit_4, ui.relLimitLineEdit_4, ui.rptValuesLineEdit_4));
  paramControls_.push_back(
		paramControls(ui.paramLabel_5, ui.paramLineEdit_5, ui.fixedCheckBox_5,
			ui.mapsCheckBox_5, ui.lowerBoundLineEdit_5, ui.upperBoundLineEdit_5, ui.relLimitLineEdit_5, ui.rptValuesLineEdit_5));
  paramControls_.push_back(
		paramControls(ui.paramLabel_6, ui.paramLineEdit_6, ui.fixedCheckBox_6,
			ui.mapsCheckBox_6, ui.lowerBoundLineEdit_6, ui.upperBoundLineEdit_6, ui.relLimitLineEdit_6, ui.rptValuesLineEdit_6));
  paramControls_.push_back(
		paramControls(ui.paramLabel_7, ui.paramLineEdit_7, ui.fixedCheckBox_7,
			ui.mapsCheckBox_7, ui.lowerBoundLineEdit_7, ui.upperBoundLineEdit_7, ui.relLimitLineEdit_7, ui.rptValuesLineEdit_7));
  paramControls_.push_back(
		paramControls(ui.paramLabel_8, ui.paramLineEdit_8, ui.fixedCheckBox_8,
			ui.mapsCheckBox_8, ui.lowerBoundLineEdit_8, ui.upperBoundLineEdit_8, ui.relLimitLineEdit_8, ui.rptValuesLineEdit_8));
  paramControls_.push_back(
		paramControls(ui.paramLabel_9, ui.paramLineEdit_9, ui.fixedCheckBox_9,
			ui.mapsCheckBox_9, ui.lowerBoundLineEdit_9, ui.upperBoundLineEdit_9, ui.relLimitLineEdit_9, ui.rptValuesLineEdit_9));
  paramControls_.push_back(
		paramControls(ui.paramLabel_10, ui.paramLineEdit_10, ui.fixedCheckBox_10,
			ui.mapsCheckBox_10, ui.lowerBoundLineEdit_10, ui.upperBoundLineEdit_10, ui.relLimitLineEdit_10, ui.rptValuesLineEdit_10));

  ui.modelName->setText(modelName);
  int nParams = model_.numParams();
  const auto& params = model_.paramNames();
  const auto& paramFlags = model_.optimisedParamFlags();
	const auto& lowerBounds = model_.lowerBounds();
	const auto& upperBounds = model_.upperBounds();
	const auto& relativeLimits = model_.relativeBounds();
	const auto& repeatParam = model_.repeatParam();
	const auto& repeatValues = model_.repeatValues();

  for (int iParam = 0; iParam < 10; iParam++)
  {
    if (nParams >= iParam+1)
    {
			//Set name, value and fixed flag from model settings
      paramControls_[iParam].name_->setText(params[iParam].c_str());
      paramControls_[iParam].value_->setValidator(new QDoubleValidator(0, 1000, 4, this));
      paramControls_[iParam].value_->setText(QString::number(model_.initialParams(iParam)));

			paramControls_[iParam].fixed_->setChecked(!paramFlags[iParam]);

			//Initialise from maps should be set if
			//a) init_maps_dir is set in options AND 
			//b)	init_map_params is empty OR iParam+1 is a member of init_map_params
			// If NOT (a) then the option should not be enabled
			bool maps = false;
			paramControls_[iParam].maps_->setEnabled(false);
			if (!madym_options_.initMapsDir().empty())
			{
				paramControls_[iParam].maps_->setEnabled(true);
				if (madym_options_.initMapParams().empty())
					maps = true;
				else {
					for (const int i : madym_options_.initMapParams())
					{
						if (i == iParam + 1)
						{
							maps = true; break;
						}
					}
				}
			}
			paramControls_[iParam].maps_->setChecked(maps); //this should trigger toggle auto callback

			//Set lower/upper bounds from model, this option should only be enabled
			//if the parameter is not fixed
			paramControls_[iParam].lowerBound_->setValidator(new QDoubleValidator(-1e6, 1e6, 8, this));
			paramControls_[iParam].lowerBound_->setText(QString::number(lowerBounds[iParam]));
			paramControls_[iParam].upperBound_->setValidator(new QDoubleValidator(-1e6, 1e6, 8, this));
			paramControls_[iParam].upperBound_->setText(QString::number(upperBounds[iParam]));

			//Set relative limit from model, this option should only be enabled
			//if the parameter is not fixed
			paramControls_[iParam].relativeLimit_->setValidator(new QDoubleValidator(0, 1000, 4, this));
			paramControls_[iParam].relativeLimit_->setText(QString::number(relativeLimits[iParam]));

			//Set repeat params
			paramControls_[iParam].repeatValues_->setValidator(doubleListValidator);
			if (iParam == repeatParam)
			{
				QString repeatValuesStr;
				for (const auto value : repeatValues)
				{
					if (!repeatValuesStr.isEmpty())
						repeatValuesStr += ",";

					repeatValuesStr += QString::number(value);
				}
				paramControls_[iParam].repeatValues_->setText(repeatValuesStr);
			}
			
			//this should happen automatically via the toggle signal auto callback on fixed_
			//paramControls_[iParam].relativeLimit_->setEnabled(paramFlags[iParam]);

			paramControls_[iParam].name_->setVisible(true);
      paramControls_[iParam].value_->setVisible(true);
      paramControls_[iParam].fixed_->setVisible(true);
			paramControls_[iParam].maps_->setVisible(true);
			paramControls_[iParam].lowerBound_->setVisible(true);
			paramControls_[iParam].upperBound_->setVisible(true);
			paramControls_[iParam].relativeLimit_->setVisible(true);
			paramControls_[iParam].repeatValues_->setVisible(true);
    }
    else
    {
      paramControls_[iParam].name_->setVisible(false);
      paramControls_[iParam].value_->setVisible(false);
      paramControls_[iParam].fixed_->setVisible(false);
			paramControls_[iParam].maps_->setVisible(false);
			paramControls_[iParam].lowerBound_->setVisible(false);
			paramControls_[iParam].upperBound_->setVisible(false);
			paramControls_[iParam].relativeLimit_->setVisible(false);
			paramControls_[iParam].repeatValues_->setVisible(false);
    }
  }
}
 
//
// Public slots
//

//
// Private slots
//
void madym_gui_model_configure::on_okButton_clicked()
{
  //Set init params and fixed params in madym options
  int nParams = model_.numParams();
  
	std::vector<double> initialParams(nParams);
	std::vector<int> fixedParams(0);
	std::vector<int> initMapParams(0);
	std::vector<double> lowerBounds(nParams);
	std::vector<double> upperBounds(nParams);
	std::vector<int> relativeLimitParams(0);
	std::vector<double>relativeLimitValues(0);

  for (int iParam = 0; iParam < nParams; iParam++)
  {
    
    initialParams[iParam] = 
      paramControls_[iParam].value_->text().toDouble();

    if (paramControls_[iParam].fixed_->isChecked())
      fixedParams.push_back(iParam+1);

		if (paramControls_[iParam].maps_->isChecked())
			initMapParams.push_back(iParam + 1);

		lowerBounds[iParam] =
			paramControls_[iParam].lowerBound_->text().toDouble();

		upperBounds[iParam] =
			paramControls_[iParam].upperBound_->text().toDouble();

		double rl = paramControls_[iParam].relativeLimit_->text().toDouble();
		if (rl)
		{
			relativeLimitParams.push_back(iParam + 1);
			relativeLimitValues.push_back(rl);
		}

		const auto repeatValues = paramControls_[iParam].repeatValues_->text();
		if (!repeatValues.isEmpty())
		{
			madym_options_.repeatParam.set(iParam + 1);
			madym_options_.repeatValues.value().fromString(repeatValues.toStdString());
		}
  }

	setDoubleListOption(initialParams, madym_options_.initialParams);
	setIntListOption(initMapParams, madym_options_.initMapParams);
	setIntListOption(fixedParams, madym_options_.fixedParams);
	setDoubleListOption(lowerBounds, madym_options_.lowerBounds);
	setDoubleListOption(upperBounds, madym_options_.upperBounds);
	setIntListOption(relativeLimitParams, madym_options_.relativeLimitParams);
	setDoubleListOption(relativeLimitValues, madym_options_.relativeLimitValues);
  done(0);
}
void madym_gui_model_configure::on_cancelButton_clicked()
{
  done(1);
}

void madym_gui_model_configure::on_fixedCheckBox_1_toggled(bool checked)
{
	ui.lowerBoundLineEdit_1->setEnabled(!checked);
	ui.upperBoundLineEdit_1->setEnabled(!checked);
	ui.relLimitLineEdit_1->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_2_toggled(bool checked)
{
	ui.lowerBoundLineEdit_2->setEnabled(!checked);
	ui.upperBoundLineEdit_2->setEnabled(!checked);
	ui.relLimitLineEdit_2->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_3_toggled(bool checked)
{
	ui.lowerBoundLineEdit_3->setEnabled(!checked);
	ui.upperBoundLineEdit_3->setEnabled(!checked);
	ui.relLimitLineEdit_3->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_4_toggled(bool checked)
{
	ui.lowerBoundLineEdit_4->setEnabled(!checked);
	ui.upperBoundLineEdit_4->setEnabled(!checked);
	ui.relLimitLineEdit_4->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_5_toggled(bool checked)
{
	ui.lowerBoundLineEdit_5->setEnabled(!checked);
	ui.upperBoundLineEdit_5->setEnabled(!checked);
	ui.relLimitLineEdit_5->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_6_toggled(bool checked)
{
	ui.lowerBoundLineEdit_6->setEnabled(!checked);
	ui.upperBoundLineEdit_6->setEnabled(!checked);
	ui.relLimitLineEdit_6->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_7_toggled(bool checked)
{
	ui.lowerBoundLineEdit_7->setEnabled(!checked);
	ui.upperBoundLineEdit_7->setEnabled(!checked);
	ui.relLimitLineEdit_7->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_8_toggled(bool checked)
{
	ui.lowerBoundLineEdit_8->setEnabled(!checked);
	ui.upperBoundLineEdit_8->setEnabled(!checked);
	ui.relLimitLineEdit_8->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_9_toggled(bool checked)
{
	ui.lowerBoundLineEdit_9->setEnabled(!checked);
	ui.upperBoundLineEdit_9->setEnabled(!checked);
	ui.relLimitLineEdit_9->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_10_toggled(bool checked)
{
	ui.lowerBoundLineEdit_10->setEnabled(!checked);
	ui.upperBoundLineEdit_10->setEnabled(!checked);
	ui.relLimitLineEdit_10->setEnabled(!checked);
}

void madym_gui_model_configure::on_rptValuesLineEdit_1_textChanged(const QString& text)
{
	validateRepeatValues(text, 0);
}

void madym_gui_model_configure::on_rptValuesLineEdit_2_textChanged(const QString& text)
{
	validateRepeatValues(text, 1);
}

void madym_gui_model_configure::on_rptValuesLineEdit_3_textChanged(const QString& text)
{
	validateRepeatValues(text, 2);
}

void madym_gui_model_configure::on_rptValuesLineEdit_4_textChanged(const QString& text)
{
	validateRepeatValues(text, 3);
}

void madym_gui_model_configure::on_rptValuesLineEdit_5_textChanged(const QString& text)
{
	validateRepeatValues(text, 4);
}

void madym_gui_model_configure::on_rptValuesLineEdit_6_textChanged(const QString& text)
{
	validateRepeatValues(text, 5);
}

void madym_gui_model_configure::on_rptValuesLineEdit_7_textChanged(const QString& text)
{
	validateRepeatValues(text, 6);
}

void madym_gui_model_configure::on_rptValuesLineEdit_8_textChanged(const QString& text)
{
	validateRepeatValues(text, 7);
}

void madym_gui_model_configure::on_rptValuesLineEdit_9_textChanged(const QString& text)
{
	validateRepeatValues(text, 8);
}

void madym_gui_model_configure::on_rptValuesLineEdit_10_textChanged(const QString& text)
{
	validateRepeatValues(text, 9);
}


void madym_gui_model_configure::setIntListOption(const std::vector<int>& values, mdm_input_ints& option)
{
	option.set(values);
	madym_options_.trackGuiOptions(option.key(), option.value().toString());
}

void madym_gui_model_configure::setDoubleListOption(const std::vector<double>& values, mdm_input_doubles& option)
{
	option.set(values);
	madym_options_.trackGuiOptions(option.key(), option.value().toString());
}

void madym_gui_model_configure::validateRepeatValues(const QString& text, const size_t param)
{
	for (int iParam = 0; iParam < model_.numParams(); iParam++)
		paramControls_[iParam].repeatValues_->setEnabled(
			text.isEmpty() || (iParam == param)
		);

	int pos = 0;
	QString str(text);
	str.replace(" ", "");
	ui.okButton->setEnabled(
		doubleListValidator->validate(str, pos) == QValidator::Acceptable || text.isEmpty());
}