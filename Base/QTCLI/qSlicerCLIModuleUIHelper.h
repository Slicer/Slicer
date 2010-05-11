/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerCLIModuleUIHelper_h
#define __qSlicerCLIModuleUIHelper_h

// CTK includes
#include <ctkPimpl.h>

/// ModuleDescriptionParser includes
#include <ModuleDescription.h>

#include "qSlicerBaseQTCLIExport.h"

class QWidget;
class qSlicerCLIModuleWidget;
class vtkMRMLCommandLineModuleNode; 
class qSlicerCLIModuleUIHelperPrivate;

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleUIHelper
{
public:

  qSlicerCLIModuleUIHelper(qSlicerCLIModuleWidget* cliModuleWidget);
  virtual ~qSlicerCLIModuleUIHelper(){}

  /// Create the widget associated with the given \a moduleParameter
  /// The caller is responsible to delete the widget.
  /// Note also that if the widget is added to a layout, Qt will
  /// be responsible to delete the widget.
  QWidget* createTagWidget(const ModuleParameter& moduleParameter);

  /// 
  /// Update \a commandLineModuleNode properties using value entered from the UI
  void updateMRMLCommandLineModuleNode(vtkMRMLCommandLineModuleNode* commandLineModuleNode);

  ///
  /// Update user interface using the given \a commandLineModuleNode parameters
  void updateUi(vtkMRMLCommandLineModuleNode* commandLineModuleNode);

private:
  CTK_DECLARE_PRIVATE(qSlicerCLIModuleUIHelper);
};

#endif
