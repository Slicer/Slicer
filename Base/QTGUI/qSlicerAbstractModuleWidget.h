/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerAbstractModuleWidget_h
#define __qSlicerAbstractModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleRepresentation.h"
#include "qSlicerWidget.h"

#include "qSlicerBaseQTGUIExport.h"


class Q_SLICER_BASE_QTGUI_EXPORT qSlicerAbstractModuleWidget : public qSlicerWidget, public qSlicerAbstractModuleRepresentation
{
  Q_OBJECT
public:
  qSlicerAbstractModuleWidget(QWidget *parent=0);
  virtual void setName(const QString& moduleName);
};

#endif
