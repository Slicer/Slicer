/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerModuleLogic.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include "vtkMRMLScene.h"

// QT includes
#include <QDebug>

// VTK includes
#include "vtkSmartPointer.h"

//-----------------------------------------------------------------------------
struct qSlicerModuleLogicPrivate: public qCTKPrivate<qSlicerModuleLogic>
{
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic> AppLogic;
};

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerModuleLogic, vtkMRMLScene*, mrmlScene, MRMLScene);
QCTK_SET_CXX(qSlicerModuleLogic, vtkMRMLScene*, setMRMLScene, MRMLScene);

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerModuleLogic, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_NO_ARG_CXX(qSlicerModuleLogic);

//-----------------------------------------------------------------------------
qSlicerModuleLogic::~qSlicerModuleLogic()
{
}

//-----------------------------------------------------------------------------
void qSlicerModuleLogic::initialize(vtkSlicerApplicationLogic* appLogic)
{
  Q_ASSERT(appLogic);
  qctk_d()->AppLogic = appLogic;

  this->setup();
}
