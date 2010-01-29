/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerTransformsModuleWidget_h
#define __qSlicerTransformsModuleWidget_h

/// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

/// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCoreModulesExport.h"

class vtkMatrix4x4;
class vtkMRMLNode;
class qSlicerTransformsModuleWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerTransformsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTransformsModuleWidget(QWidget *parent=0);

  virtual QAction* showModuleAction();

public slots:
  void loadTransform();

protected:
  virtual void setup();

protected slots:
  void onCoordinateReferenceButtonPressed(int id);
  void onIdentityButtonPressed();
  void onInvertButtonPressed();
  void onNodeSelected(vtkMRMLNode* node);

  /// 
  /// Triggered upon MRML transform node updates
  void onMRMLTransformNodeModified(vtkObject* caller);

protected:
  /// 
  /// Fill the 'minmax' array with the min/max translation value of the matrix.
  /// Parameter expand allows to specify (using a value between 0 and 1)
  /// which percentage of the found min/max value should be substracted/added
  /// to the min/max value found.
  void extractMinMaxTranslationValue(vtkMatrix4x4 * mat, double& min, double& max);

  /// 
  /// Convenient method to return the coordinate system currently selected
  int coordinateReference();

private:
  QCTK_DECLARE_PRIVATE(qSlicerTransformsModuleWidget);
};

#endif
