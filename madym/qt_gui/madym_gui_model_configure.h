#ifndef MADYM_GUI_MODEL_CONFIGURE
#define MADYM_GUI_MODEL_CONFIGURE

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include <mdm_DCEModelBase.h>
#include <mdm_RunTools.h>
#include "ui_madym_model_configure.h"

struct paramControls {
  paramControls(QLabel *name, QLineEdit *value, QCheckBox *fixed)
    :name_(name), value_(value), fixed_(fixed)
  {};

  QLabel *name_;
  QLineEdit *value_;
  QCheckBox *fixed_;
};

class madym_gui_model_configure : public QDialog
{
  Q_OBJECT

//  INTERFACE

public:
	madym_gui_model_configure(const mdm_DCEModelBase &model, const QString &modelName,
    mdm_ToolsOptions &madym_options,
    QWidget *parent = 0);


signals:


public slots:

//  IMPLEMENTATION

private slots: //
  void on_okButton_clicked();
  void on_cancelButton_clicked();

private: // Variables
  Ui::modelDialog ui;

  mdm_ToolsOptions &madym_options_;
  const mdm_DCEModelBase &model_;

  std::vector<paramControls> paramControls_;

};

#endif //MADYM_GUI_MODEL_CONFIGURE