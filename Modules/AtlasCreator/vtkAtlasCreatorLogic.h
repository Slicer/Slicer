/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkAtlasCreatorLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $
  Author:    $Sylvain Jaume (MIT)$

=======================================================================auto=*/

#ifndef __vtkAtlasCreatorLogic_h
#define __vtkAtlasCreatorLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkMRMLScene.h"

#include "vtkAtlasCreator.h"
#include "vtkMRMLAtlasCreatorNode.h"

class vtkImageData;

class VTK_AtlasCreator_EXPORT vtkAtlasCreatorLogic :
  public vtkSlicerModuleLogic
{
  public:
  static vtkAtlasCreatorLogic *New();
  vtkTypeMacro(vtkAtlasCreatorLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents( vtkObject *caller, unsigned long event,
    void *callData ){};

  // Description: Get/Set MRML node storing parameter values
  vtkGetObjectMacro( AtlasCreatorNode,
    vtkMRMLAtlasCreatorNode);
  void SetAndObserveAtlasCreatorNode(
    vtkMRMLAtlasCreatorNode *node)
    {
    vtkSetAndObserveMRMLNodeMacro(this->AtlasCreatorNode, node);
    }

  // The method that creates and runs VTK pipeline
  void SliceProcess( vtkTransform* xyToijk, double dim0, double dim1 );

  void Apply();
  void ThirdLabelMap();

  double InitMaxThreshold();
  double InitMinThreshold();
  double AxialMax();
  double AxialMin();
  double SagittalMax();
  double CoronalMax();

protected:
  vtkAtlasCreatorLogic();
  virtual ~vtkAtlasCreatorLogic();

  vtkAtlasCreatorLogic(const vtkAtlasCreatorLogic&);
  void operator=(const vtkAtlasCreatorLogic&);

  vtkMRMLAtlasCreatorNode* AtlasCreatorNode;

  vtkImageData *ThirdLabelMapImage;
  vtkImageData *SecondLabelMap;
};

#endif

