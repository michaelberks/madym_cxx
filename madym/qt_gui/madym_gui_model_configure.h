#ifndef MADYM_GUI_MODEL_CONFIGURE
#define MADYM_GUI_MODEL_CONFIGURE

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include <mdm_DCEModelBase.h>
#include <mdm_OptionsParser.h>
#include "ui_madym_model_configure.h"

struct paramControls {
  paramControls(QLabel *name, QLineEdit *value, 
		QCheckBox *fixed, QCheckBox *maps, QLineEdit *relativeLimit)
    :name_(name), value_(value), fixed_(fixed), maps_(maps), relativeLimit_(relativeLimit)
  {};

  QLabel *name_;
  QLineEdit *value_;
  QCheckBox *fixed_;
	QCheckBox *maps_;
	QLineEdit *relativeLimit_;
};

class madym_gui_model_configure : public QDialog
{
  Q_OBJECT

//  INTERFACE

public:
	madym_gui_model_configure(const mdm_DCEModelBase &model, const QString &modelName,
    mdm_InputOptions &madym_options,
    QWidget *parent = 0);


signals:


public slots:

//  IMPLEMENTATION

private slots: //
  void on_okButton_clicked();
  void on_cancelButton_clicked();

	void on_fixedCheckBox_1_toggled(bool checked);
	void on_fixedCheckBox_2_toggled(bool checked);
	void on_fixedCheckBox_3_toggled(bool checked);
	void on_fixedCheckBox_4_toggled(bool checked);
	void on_fixedCheckBox_5_toggled(bool checked);
	void on_fixedCheckBox_6_toggled(bool checked);
	void on_fixedCheckBox_7_toggled(bool checked);
	void on_fixedCheckBox_8_toggled(bool checked);
	void on_fixedCheckBox_9_toggled(bool checked);
	void on_fixedCheckBox_10_toggled(bool checked);

private: // Variables
  Ui::modelDialog ui;

  mdm_InputOptions &madym_options_;
  const mdm_DCEModelBase &model_;

  std::vector<paramControls> paramControls_;

};

#endif //MADYM_GUI_MODEL_CONFIGURE