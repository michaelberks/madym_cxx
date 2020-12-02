/*!
*  @file    madym_gui_model_configure.h
*  @brief   QT UI class for configuring tracer-kinetic model parameters in pop-up window of main GUI
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MADYM_GUI_MODEL_CONFIGURE
#define MADYM_GUI_MODEL_CONFIGURE

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include <madym/dce_models/mdm_DCEModelBase.h>
#include <madym/mdm_OptionsParser.h>
#include "ui_madym_model_configure.h"

//! Helper class providing GUI controls to configure each individual model parameter
struct paramControls {

  //! Constructor
  /*!
  \param name of parameter in widget label
  \param value initial value in widget linedit
  \param fixed checkbox to show if parameter fixed
  \param maps checkbox to show if parameter initialised from preloaded maps
  \param relativeLimit value of relative limit in widget linedit
  */
  paramControls(QLabel *name, QLineEdit *value, 
		QCheckBox *fixed, QCheckBox *maps, QLineEdit *relativeLimit)
    :name_(name), value_(value), fixed_(fixed), maps_(maps), relativeLimit_(relativeLimit)
  {};

  QLabel *name_; //!<name of parameter in widget label
  QLineEdit *value_; //!< initial value in widget linedit
  QCheckBox *fixed_; //!< checkbox to show if parameter fixed
	QCheckBox *maps_; //!< checkbox to show if parameter initialised from preloaded maps
	QLineEdit *relativeLimit_; //!< relativeLimit value of relative limit in widget linedit
};

//! QT UI class for configuring tracer-kinetic model parameters in pop-up window of main GUI
class madym_gui_model_configure : public QDialog
{
  Q_OBJECT

//  INTERFACE

public:

	//!Constructor
	/*!
	\param model tracer-kinetic model of specific type (set by main GUI), used to configure what parameter controls are visble and to label them appropriately
	\param modelName to label pop-up window title bar
	\param madym_options reference to madym_options from main GUI, supplied so configured changes persist when pop-up window is closed and this GUI object destroyed
	\param parent QT default input
	*/
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