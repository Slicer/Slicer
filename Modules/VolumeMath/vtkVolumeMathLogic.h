/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkSlicerVolumeMathLogic_h
#define __vtkSlicerVolumeMathLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"

#include "vtkSlicerVolumeMath.h"
#include "vtkMRMLVolumeMathNode.h"


class VTK_SLICERVOLUMEMATH_EXPORT vtkSlicerVolumeMathLogic : public vtkSlicerModuleLogic
{
  public:
  //BTX
  enum VolumeLogicEventIDs {
    StartLabelStats,
    EndLabelStats,
    LabelStatsOuterLoop,
    LabelStatsInnerLoop
  };
  //ETX
  static vtkSlicerVolumeMathLogic *New();
  vtkTypeMacro(vtkSlicerVolumeMathLogic,vtkSlicerModuleLogic);

  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  // Description: Get/Set MRML node storing parameter values
  vtkGetObjectMacro (SlicerVolumeMathNode, vtkMRMLVolumeMathNode);
  void SetAndObserveSlicerVolumeMathNode(vtkMRMLVolumeMathNode *n) 
  {
    vtkSetAndObserveMRMLNodeMacro( this->SlicerVolumeMathNode, n);
  }
  
  // The method that creates and runs VTK or ITK pipeline
  void Apply();
  
  float GetProgress();
  void SetProgress(float p);
  //vtkGetFloatMacro(Progress);
  //vtkSetFloatMacro(Progress);
  vtkGetStringMacro(Res);
  vtkSetStringMacro(Res);
  // vtkGetMacro(TestFloat);
  //vtkSetMacro(TestFloat);
 
 protected:
  vtkSlicerVolumeMathLogic();
  virtual ~vtkSlicerVolumeMathLogic();
  vtkSlicerVolumeMathLogic(const vtkSlicerVolumeMathLogic&);
  void operator=(const vtkSlicerVolumeMathLogic&);
  
  vtkMRMLVolumeMathNode* SlicerVolumeMathNode;
  float Progress; //progress of labelStats processing in percent
  char* Res;
  float TestFloat;
 };

#endif

