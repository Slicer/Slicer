/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerAbstractModuleWidget_h
#define __qSlicerAbstractModuleWidget_h

#if defined(_MSC_VER)
#pragma warning( disable:4250 )
#endif

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleRepresentation.h"
#include "qSlicerWidget.h"

#include "qSlicerBaseQTGUIExport.h"

///
/// Base class of all the Slicer module widgets. The widget is added in the module panels.
/// Deriving from qSlicerWidget, it inherits the mrmlScene()/setMRMLScene() methods.
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerAbstractModuleWidget
  :public qSlicerWidget, public qSlicerAbstractModuleRepresentation
{
  Q_OBJECT
public:
  /// Constructor
  /// \sa QWidget
  qSlicerAbstractModuleWidget(QWidget *parent=0);

  ///
  /// Reimplemented from qSlicerAbstractModuleRepresentation
  /// Set the module unique name to the window title of the widget
  virtual void setName(const QString& moduleName);

  ///
  /// Returns a QAction if the module would like to be added in a toolbar.
  /// By default, returns no QAction.
  virtual QAction * showModuleAction();
};

#endif
