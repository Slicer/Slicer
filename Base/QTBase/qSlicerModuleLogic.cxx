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
class qSlicerModuleLogicPrivate: public qCTKPrivate<qSlicerModuleLogic>
{
public:
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic> AppLogic;
};

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerModuleLogic, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_NO_ARG_CXX(qSlicerModuleLogic);

//-----------------------------------------------------------------------------
qSlicerModuleLogic::~qSlicerModuleLogic()
{
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerModuleLogic, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
void qSlicerModuleLogic::setMRMLScene(vtkMRMLScene* scene)
{
  QCTK_D(qSlicerModuleLogic);
  if (scene == d->MRMLScene)
    {
    return;
    }
  if (scene)
    {
    this->registerNodes(scene); 
    }
  d->MRMLScene = scene; 
}

//-----------------------------------------------------------------------------
void qSlicerModuleLogic::initialize(vtkSlicerApplicationLogic* _appLogic)
{
  Q_ASSERT(_appLogic);
  qctk_d()->AppLogic = _appLogic;

  this->setup();
}
