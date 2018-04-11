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
  explicit qSlicerSettingsUserInformationPanel(QWidget* parent = 0);

  /// Destructor
  virtual ~qSlicerSettingsUserInformationPanel();

  virtual void setUserInformation(vtkPersonInformation* userInfo);

public Q_SLOTS:
  virtual void resetSettings();
  virtual void applySettings();

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
