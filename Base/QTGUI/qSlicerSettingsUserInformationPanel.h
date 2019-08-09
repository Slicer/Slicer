/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __qqSlicerSettingsUserInformationPanel_h
#define __qqSlicerSettingsUserInformationPanel_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkSettingsPanel.h>
#include <ctkVTKObject.h>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerSettingsUserInformationPanelPrivate;
class vtkPersonInformation;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSettingsUserInformationPanel
  : public ctkSettingsPanel
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Superclass typedef
  typedef ctkSettingsPanel Superclass;

  /// Constructor
  explicit qSlicerSettingsUserInformationPanel(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerSettingsUserInformationPanel() override;

  Q_INVOKABLE virtual void setUserInformation(vtkPersonInformation* userInfo);

public Q_SLOTS:
  void resetSettings() override;
  void applySettings() override;

  void updateFromUserInformation();
  void onNameChanged();
  void onLoginChanged();
  void onEmailChanged(const QString& value);
  void onOrganizationChanged();
  void onOrganizationRoleChanged();
  void onProcedureRoleChanged();

protected:

  void resetWarnings();

  std::string UserInformationBackup;
  QScopedPointer<qSlicerSettingsUserInformationPanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSettingsUserInformationPanel);
  Q_DISABLE_COPY(qSlicerSettingsUserInformationPanel);
};

#endif
