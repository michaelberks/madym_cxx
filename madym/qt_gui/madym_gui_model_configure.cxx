#include "madym_gui_model_configure.h"

//  Class that pops an image from the processing queue, performs any
//  necessary processing on the frame, and places it on the save queue for
//  the saver to deal with.
 
//
// Public methods
//

madym_gui_model_configure::madym_gui_model_configure(const mdm_DCEModelBase &model, const QString &modelName,
  mdm_ToolsOptions &madym_options,
  QWidget *parent)
  :QDialog(parent),
  madym_options_(madym_options),
  model_(model)
{
  // setup the UI
  ui.setupUi(this);

  paramControls_.push_back(paramControls(ui.paramLabel_1, ui.paramLineEdit_1, ui.paramCheckBox_1));
  paramControls_.push_back(paramControls(ui.paramLabel_2, ui.paramLineEdit_2, ui.paramCheckBox_2));
  paramControls_.push_back(paramControls(ui.paramLabel_3, ui.paramLineEdit_3, ui.paramCheckBox_3));
  paramControls_.push_back(paramControls(ui.paramLabel_4, ui.paramLineEdit_4, ui.paramCheckBox_4));
  paramControls_.push_back(paramControls(ui.paramLabel_5, ui.paramLineEdit_5, ui.paramCheckBox_5));
  paramControls_.push_back(paramControls(ui.paramLabel_6, ui.paramLineEdit_6, ui.paramCheckBox_6));
  paramControls_.push_back(paramControls(ui.paramLabel_7, ui.paramLineEdit_7, ui.paramCheckBox_7));
  paramControls_.push_back(paramControls(ui.paramLabel_8, ui.paramLineEdit_8, ui.paramCheckBox_8));
  paramControls_.push_back(paramControls(ui.paramLabel_9, ui.paramLineEdit_9, ui.paramCheckBox_9));
  paramControls_.push_back(paramControls(ui.paramLabel_10, ui.paramLineEdit_10, ui.paramCheckBox_10));

  ui.modelName->setText(modelName);
  int nParams = model_.num_dims();
  const std::vector<std::string> &params = model_.pkParamNames();
  const std::vector<bool> &paramFlags = model_.optParamFlags();

  for (int iParam = 0; iParam < 10; iParam++)
  {
    if (nParams >= iParam+1)
    {
      paramControls_[iParam].name_->setText(params[iParam].c_str());
      paramControls_[iParam].value_->setValidator(new QDoubleValidator(0, 1000, 4, this));
      paramControls_[iParam].value_->setText(QString::number(model_.pkInitParams(iParam)));
      paramControls_[iParam].fixed_->setChecked(!paramFlags[iParam]);
      paramControls_[iParam].name_->setVisible(true);
      paramControls_[iParam].value_->setVisible(true);
      paramControls_[iParam].fixed_->setVisible(true);
    }
    else
    {
      paramControls_[iParam].name_->setVisible(false);
      paramControls_[iParam].value_->setVisible(false);
      paramControls_[iParam].fixed_->setVisible(false);
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
  int nParams = model_.num_dims();
  madym_options_.initParams.resize(nParams);
  madym_options_.fixedParams.clear();

  for (int iParam = 0; iParam < 10; iParam++)
  {
    if (nParams >= iParam + 1)
    {
      madym_options_.initParams[iParam] = 
        paramControls_[iParam].value_->text().toDouble();
      if (paramControls_[iParam].fixed_->isChecked())
        madym_options_.fixedParams.push_back(iParam+1);
    }
  }
  done(0);
}
void madym_gui_model_configure::on_cancelButton_clicked()
{
  done(1);
}