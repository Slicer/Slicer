/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerSettingsUserInformationPanel.h"
#include "ui_qSlicerSettingsUserInformationPanel.h"

// VTK includes
#include <vtkSmartPointer.h>

#include "vtkPersonInformation.h"

#include "qsettings.h"

// --------------------------------------------------------------------------
// qSlicerSettingsUserInformationPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsUserInformationPanelPrivate: public Ui_qSlicerSettingsUserInformationPanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsUserInformationPanel);
protected:
  qSlicerSettingsUserInformationPanel* const q_ptr;

public:
  qSlicerSettingsUserInformationPanelPrivate(qSlicerSettingsUserInformationPanel& object);
  void init();

  vtkSmartPointer<vtkPersonInformation> UserInformation;
};

// --------------------------------------------------------------------------
// qSlicerSettingsUserInformationPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsUserInformationPanelPrivate
::qSlicerSettingsUserInformationPanelPrivate(qSlicerSettingsUserInformationPanel& object)
  :q_ptr(&object)
{
  this->UserInformation = nullptr;
}

// --------------------------------------------------------------------------
void qSlicerSettingsUserInformationPanelPrivate::init()
{
  Q_Q(qSlicerSettingsUserInformationPanel);

  this->setupUi(q);

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->NameLineEdit, SIGNAL(editingFinished()),
                   q, SLOT(onNameChanged()));
  QObject::connect(this->LoginLineEdit, SIGNAL(editingFinished()),
                   q, SLOT(onLoginChanged()));
  QObject::connect(this->EmailLineEdit, SIGNAL(textChanged(QString)),
                   q, SLOT(onEmailChanged(QString)));
  QObject::connect(this->OrganizationLineEdit, SIGNAL(editingFinished()),
                   q, SLOT(onOrganizationChanged()));
  QObject::connect(this->OrganizationRoleLineEdit, SIGNAL(editingFinished()),
                   q, SLOT(onOrganizationRoleChanged()));
  QObject::connect(this->ProcedureRoleLineEdit, SIGNAL(editingFinished()),
                   q, SLOT(onProcedureRoleChanged()));
}

// --------------------------------------------------------------------------
// qSlicerSettingsUserInformationPanel methods

// --------------------------------------------------------------------------
qSlicerSettingsUserInformationPanel::qSlicerSettingsUserInformationPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsUserInformationPanelPrivate(*this))
{
  Q_D(qSlicerSettingsUserInformationPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsUserInformationPanel::~qSlicerSettingsUserInformationPanel() = default;

// --------------------------------------------------------------------------
void qSlicerSettingsUserInformationPanel::setUserInformation(vtkPersonInformation* userInfo)
{
  Q_D(qSlicerSettingsUserInformationPanel);
  if (d->UserInformation == userInfo)
    {
    return;
    }

  this->qvtkReconnect(d->UserInformation, userInfo,
    vtkCommand::ModifiedEvent,
    this, SLOT(updateFromUserInformation()));
  d->UserInformation = userInfo;

  // Default values
  this->updateFromUserInformation();
  this->UserInformationBackup = userInfo->GetAsString();
}

// --------------------------------------------------------------------------
void qSlicerSettingsUserInformationPanel::updateFromUserInformation()
{
  Q_D(qSlicerSettingsUserInformationPanel);

  if (d->UserInformation == nullptr)
    {
    return;
    }

  d->NameLineEdit->setText(d->UserInformation->GetName().c_str());
  d->LoginLineEdit->setText(d->UserInformation->GetLogin().c_str());
  d->EmailLineEdit->setText(d->UserInformation->GetEmail().c_str());
  d->OrganizationLineEdit->setText(d->UserInformation->GetOrganization().c_str());
  d->OrganizationRoleLineEdit->setText(d->UserInformation->GetOrganizationRole().c_str());
  d->ProcedureRoleLineEdit->setText(d->UserInformation->GetProcedureRole().c_str());

  this->resetWarnings();
}

// --------------------------------------------------------------------------
void qSlicerSettingsUserInformationPanel::applySettings()
{
  Q_D(qSlicerSettingsUserInformationPanel);
  this->UserInformationBackup = d->UserInformation->GetAsString();
  this->updateFromUserInformation();
}

// --------------------------------------------------------------------------
void qSlicerSettingsUserInformationPanel::resetSettings()
{
  Q_D(qSlicerSettingsUserInformationPanel);
  d->UserInformation->SetFromString(this->UserInformationBackup);
  updateFromUserInformation();
}

// --------------------------------------------------------------------------
void qSlicerSettingsUserInformationPanel::resetWarnings()
{
  Q_D(qSlicerSettingsUserInformationPanel);
  d->EmailValidationLabel->setText("");
}

// --------------------------------------------------------------------------
void qSlicerSettingsUserInformationPanel::onNameChanged()
{
  Q_D(qSlicerSettingsUserInformationPanel);
  d->UserInformation->SetName(d->NameLineEdit->text().toStdString().c_str());
}

// --------------------------------------------------------------------------
void qSlicerSettingsUserInformationPanel::onLoginChanged()
{
  Q_D(qSlicerSettingsUserInformationPanel);
  d->UserInformation->SetLogin(d->LoginLineEdit->text().toStdString().c_str());
}

// --------------------------------------------------------------------------
void qSlicerSettingsUserInformationPanel::onEmailChanged(const QString& value)
{
  Q_D(qSlicerSettingsUserInformationPanel);
  if(!d->UserInformation->SetEmail(value.toStdString().c_str()))
    {
    d->EmailValidationLabel->setText("Invalid format");
    }
  else
    {
    d->EmailValidationLabel->setText("");
    }
}

// --------------------------------------------------------------------------
void qSlicerSettingsUserInformationPanel::onOrganizationChanged()
{
  Q_D(qSlicerSettingsUserInformationPanel);
  d->UserInformation->SetOrganization(d->OrganizationLineEdit->text().toStdString().c_str());
}

// --------------------------------------------------------------------------
void qSlicerSettingsUserInformationPanel::onOrganizationRoleChanged()
{
  Q_D(qSlicerSettingsUserInformationPanel);
  d->UserInformation->SetOrganizationRole(d->OrganizationRoleLineEdit->text().toStdString().c_str());
}

// --------------------------------------------------------------------------
void qSlicerSettingsUserInformationPanel::onProcedureRoleChanged()
{
  Q_D(qSlicerSettingsUserInformationPanel);
  d->UserInformation->SetProcedureRole(d->ProcedureRoleLineEdit->text().toStdString().c_str());
}
