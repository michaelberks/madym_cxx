#include "madym_gui_model_configure.h"

//  Class that pops an image from the processing queue, performs any
//  necessary processing on the frame, and places it on the save queue for
//  the saver to deal with.
 
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

  paramControls_.push_back(
		paramControls(ui.paramLabel_1, ui.paramLineEdit_1, ui.fixedCheckBox_1,
			ui.mapsCheckBox_1, ui.relLimitLineEdit_1));
  paramControls_.push_back(
		paramControls(ui.paramLabel_2, ui.paramLineEdit_2, ui.fixedCheckBox_2,
			ui.mapsCheckBox_2, ui.relLimitLineEdit_2));
  paramControls_.push_back(
		paramControls(ui.paramLabel_3, ui.paramLineEdit_3, ui.fixedCheckBox_3,
			ui.mapsCheckBox_3, ui.relLimitLineEdit_3));
  paramControls_.push_back(
		paramControls(ui.paramLabel_4, ui.paramLineEdit_4, ui.fixedCheckBox_4,
			ui.mapsCheckBox_4, ui.relLimitLineEdit_4));
  paramControls_.push_back(
		paramControls(ui.paramLabel_5, ui.paramLineEdit_5, ui.fixedCheckBox_5,
			ui.mapsCheckBox_5, ui.relLimitLineEdit_5));
  paramControls_.push_back(
		paramControls(ui.paramLabel_6, ui.paramLineEdit_6, ui.fixedCheckBox_6,
			ui.mapsCheckBox_6, ui.relLimitLineEdit_6));
  paramControls_.push_back(
		paramControls(ui.paramLabel_7, ui.paramLineEdit_7, ui.fixedCheckBox_7,
			ui.mapsCheckBox_7, ui.relLimitLineEdit_7));
  paramControls_.push_back(
		paramControls(ui.paramLabel_8, ui.paramLineEdit_8, ui.fixedCheckBox_8,
			ui.mapsCheckBox_8, ui.relLimitLineEdit_8));
  paramControls_.push_back(
		paramControls(ui.paramLabel_9, ui.paramLineEdit_9, ui.fixedCheckBox_9,
			ui.mapsCheckBox_9, ui.relLimitLineEdit_9));
  paramControls_.push_back(
		paramControls(ui.paramLabel_10, ui.paramLineEdit_10, ui.fixedCheckBox_10,
			ui.mapsCheckBox_10, ui.relLimitLineEdit_10));

  ui.modelName->setText(modelName);
  int nParams = model_.num_params();
  const std::vector<std::string> &params = model_.paramNames();
  const std::vector<bool> &paramFlags = model_.optimisedParamFlags();
	const std::vector<double> &relativeLimits = model_.relativeBounds();

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

			//Set relative limit from model, this option should only be enabled
			//if the parameter is not fixed
			paramControls_[iParam].relativeLimit_->setValidator(new QDoubleValidator(0, 1000, 4, this));
			paramControls_[iParam].relativeLimit_->setText(QString::number(relativeLimits[iParam]));
			
			//this should happen automatically via the toggle signal auto callback on fixed_
			//paramControls_[iParam].relativeLimit_->setEnabled(paramFlags[iParam]);

			paramControls_[iParam].name_->setVisible(true);
      paramControls_[iParam].value_->setVisible(true);
      paramControls_[iParam].fixed_->setVisible(true);
			paramControls_[iParam].maps_->setVisible(true);
			paramControls_[iParam].relativeLimit_->setVisible(true);
    }
    else
    {
      paramControls_[iParam].name_->setVisible(false);
      paramControls_[iParam].value_->setVisible(false);
      paramControls_[iParam].fixed_->setVisible(false);
			paramControls_[iParam].maps_->setVisible(false);
			paramControls_[iParam].relativeLimit_->setVisible(false);
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
  int nParams = model_.num_params();
  
	std::vector<double> initialParams(nParams);
	std::vector<int> fixedParams(0);
	std::vector<int> initMapParams(0);
	std::vector<int> relativeLimitParams(0);
	std::vector<double>relativeLimitValues(0);

  for (int iParam = 0; iParam < 10; iParam++)
  {
    if (nParams >= iParam + 1)
    {
      initialParams[iParam] = 
        paramControls_[iParam].value_->text().toDouble();

      if (paramControls_[iParam].fixed_->isChecked())
        fixedParams.push_back(iParam+1);

			if (paramControls_[iParam].maps_->isChecked())
				initMapParams.push_back(iParam + 1);

			double rl = paramControls_[iParam].relativeLimit_->text().toDouble();
			if (rl)
			{
				relativeLimitParams.push_back(iParam + 1);
				relativeLimitValues.push_back(rl);
			}
    }
  }

	madym_options_.initialParams.set(initialParams);
	madym_options_.initMapParams.set(initMapParams);
	madym_options_.fixedParams.set(fixedParams);
	madym_options_.relativeLimitParams.set(relativeLimitParams);
	madym_options_.relativeLimitValues.set(relativeLimitValues);
  done(0);
}
void madym_gui_model_configure::on_cancelButton_clicked()
{
  done(1);
}

void madym_gui_model_configure::on_fixedCheckBox_1_toggled(bool checked)
{
	ui.relLimitLineEdit_1->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_2_toggled(bool checked)
{
	ui.relLimitLineEdit_2->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_3_toggled(bool checked)
{
	ui.relLimitLineEdit_3->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_4_toggled(bool checked)
{
	ui.relLimitLineEdit_4->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_5_toggled(bool checked)
{
	ui.relLimitLineEdit_5->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_6_toggled(bool checked)
{
	ui.relLimitLineEdit_6->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_7_toggled(bool checked)
{
	ui.relLimitLineEdit_7->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_8_toggled(bool checked)
{
	ui.relLimitLineEdit_8->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_9_toggled(bool checked)
{
	ui.relLimitLineEdit_9->setEnabled(!checked);
}
void madym_gui_model_configure::on_fixedCheckBox_10_toggled(bool checked)
{
	ui.relLimitLineEdit_10->setEnabled(!checked);
}
