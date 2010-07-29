 /*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerMRMLTreeModuleWidget_h
#define __qSlicerMRMLTreeModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerMRMLTreeModuleWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerMRMLTreeModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerMRMLTreeModuleWidget(QWidget *parent=0);

protected:
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerMRMLTreeModuleWidget);
};

#endif
