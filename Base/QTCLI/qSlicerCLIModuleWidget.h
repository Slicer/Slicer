/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCLIModuleWidget_h
#define __qSlicerCLIModuleWidget_h

/// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

/// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCLIExport.h"

class ModuleDescription; 
class qSlicerCLIModuleWidgetPrivate;

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCLIModuleWidget(ModuleDescription* desc, QWidget *parent=0);
  virtual ~qSlicerCLIModuleWidget(){}

protected:
  /// 
  /// Overloaded
  virtual void setup();

private:
  QCTK_DECLARE_PRIVATE(qSlicerCLIModuleWidget);
};

#endif
