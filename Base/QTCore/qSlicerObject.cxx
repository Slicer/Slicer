/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


// Qt includes
#include <QDebug>

// QTCore includes
#include "qSlicerObject.h"

// VTK includes
#include "vtkMRMLScene.h"
#include "vtkSmartPointer.h"

//-----------------------------------------------------------------------------
class qSlicerObjectPrivate: public ctkPrivate<qSlicerObject>
{
public:
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
};

//-----------------------------------------------------------------------------
qSlicerObject::qSlicerObject()
{
  CTK_INIT_PRIVATE(qSlicerObject);
}

//-----------------------------------------------------------------------------
qSlicerObject::~qSlicerObject()
{
}

//-----------------------------------------------------------------------------
void qSlicerObject::setMRMLScene(vtkMRMLScene* scene)
{
  if (scene == ctk_d()->MRMLScene)
    {
    return ;
    }
  ctk_d()->MRMLScene = scene;
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerObject, vtkMRMLScene*, mrmlScene, MRMLScene);
