/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCLIModuleWidget_h
#define __qSlicerCLIModuleWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBaseQTCLIExport.h"

class ModuleDescription;
class vtkMRMLNode; 
class qSlicerCLIModuleWidgetPrivate;

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCLIModuleWidget(ModuleDescription* desc, QWidget *parent=0);
  virtual ~qSlicerCLIModuleWidget(){}

  ///
  /// Set the \a entryPoint associated with the module
  void setModuleEntryPoint(const QString& entryPoint);

public slots:
  ///
  /// Set the current \a commandLineModuleNode
  void setCurrentCommandLineModuleNode(vtkMRMLNode* commandLineModuleNode);
  
protected:
  /// 
  /// \Overload
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerCLIModuleWidget);
};

#endif
