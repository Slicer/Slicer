/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCamerasModuleWidget_h
#define __qSlicerCamerasModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class vtkMRMLNode;
class vtkMRMLViewNode;
class qSlicerCamerasModuleWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerCamerasModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCamerasModuleWidget(QWidget *parent=0);

  virtual QAction* showModuleAction();

public slots:
  /// 
  /// Inherited from qSlicerWidget. Reimplemented for refresh issues.
  virtual void setMRMLScene(vtkMRMLScene*);

protected:
  virtual void setup();
  void synchronizeCameraWithView(vtkMRMLViewNode* currentViewNode);

protected slots:
  void onCurrentViewNodeChanged(vtkMRMLNode*);
  void setCameraToCurrentView(vtkMRMLNode*);
  void onCameraNodeAdded(vtkMRMLNode*);
  void onCameraNodeRemoved(vtkMRMLNode*);
  void synchronizeCameraWithView();

private:
  CTK_DECLARE_PRIVATE(qSlicerCamerasModuleWidget);
};

#endif
