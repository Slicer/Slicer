/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerAbstractModule_h
#define __qSlicerAbstractModule_h

// Qt includes
#include <QIcon>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerBaseQTGUIExport.h"

class QAction;
class qSlicerAbstractModulePrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerAbstractModule : public qSlicerAbstractCoreModule
{
  Q_OBJECT

public:

  typedef qSlicerAbstractCoreModule Superclass;
  qSlicerAbstractModule(QObject *parent=0);

  virtual QIcon icon()const;

  ///
  /// Returns a QAction if the module would like to be added in a toolbar.
  /// By default, returns no QAction.
  QAction * action();
private:
  CTK_DECLARE_PRIVATE(qSlicerAbstractModule);
};

#endif
