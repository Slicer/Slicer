/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerLoadableModule_h
#define __qSlicerLoadableModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModule.h"

#include "qSlicerBaseQTGUIExport.h"

class qSlicerLoadableModulePrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerLoadableModule : public qSlicerAbstractModule
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerLoadableModule(QObject *parent=0);

  ///
  /// Return help/acknowledgement text
  virtual QString helpText()const;
  virtual QString acknowledgementText()const;

protected:
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerLoadableModule);
};

Q_DECLARE_INTERFACE(qSlicerLoadableModule,
                     "org.slicer.QTModules.qSlicerLoadableModule/1.0");

#endif
