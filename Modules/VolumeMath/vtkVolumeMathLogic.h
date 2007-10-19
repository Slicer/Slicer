/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkVolumeMathLogic_h
#define __vtkVolumeMathLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"

#include "vtkVolumeMath.h"
#include "vtkMRMLVolumeMathNode.h"


class VTK_VOLUMEMATH_EXPORT vtkVolumeMathLogic : public vtkSlicerModuleLogic
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
  static vtkVolumeMathLogic *New();
  vtkTypeMacro(vtkVolumeMathLogic,vtkSlicerModuleLogic);

  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  // Description: Get/Set MRML node storing parameter values
  vtkGetObjectMacro (VolumeMathNode, vtkMRMLVolumeMathNode);
  void SetAndObserveVolumeMathNode(vtkMRMLVolumeMathNode *n) 
  {
    vtkSetAndObserveMRMLNodeMacro( this->VolumeMathNode, n);
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
  vtkVolumeMathLogic();
  virtual ~vtkVolumeMathLogic();
  vtkVolumeMathLogic(const vtkVolumeMathLogic&);
  void operator=(const vtkVolumeMathLogic&);
  
  vtkMRMLVolumeMathNode* VolumeMathNode;
  float Progress; //progress of labelStats processing in percent
  char* Res;
  float TestFloat;
 };

#endif

