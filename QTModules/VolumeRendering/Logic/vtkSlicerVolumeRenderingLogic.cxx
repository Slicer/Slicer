/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumeRenderingLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// 
#include "vtkSlicerVolumeRenderingLogic.h"
#include "vtkSlicerColorLogic.h"

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVolumeArchetypeStorageNode.h>
#include <vtkMRMLVolumeHeaderlessStorageNode.h>
#include <vtkMRMLNRRDStorageNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLDiffusionWeightedVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLDiffusionTensorVolumeDisplayNode.h>
#include <vtkMRMLDiffusionWeightedVolumeDisplayNode.h>
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>
#include <vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx> 

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkImageThreshold.h>
#include <vtkImageAccumulateDiscrete.h>
#include <vtkImageBimodalAnalysis.h>
#include <vtkImageExtractComponents.h>
#include <vtkDiffusionTensorMathematics.h>
#include <vtkAssignAttribute.h>
#include <vtkStringArray.h>

#include "MRML/vtkMRMLVolumeRenderingScenarioNode.h"
#include "MRML/vtkMRMLVolumeRenderingParametersNode.h"

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerVolumeRenderingLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerVolumeRenderingLogic);

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic::vtkSlicerVolumeRenderingLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic::~vtkSlicerVolumeRenderingLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::ProcessMRMLEvents(vtkObject *vtkNotUsed(caller),
                                              unsigned long vtkNotUsed(event),
                                              void *vtkNotUsed(callData))
{
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::ProcessLogicEvents(vtkObject *vtkNotUsed(caller), 
                                            unsigned long event, 
                                            void *callData)
{
  if (event ==  vtkCommand::ProgressEvent) 
    {
    this->InvokeEvent(vtkCommand::ProgressEvent,callData);
    }
}

void vtkSlicerVolumeRenderingLogic::RegisterNodes()
{
  if (this->GetMRMLScene())
  {
    // :NOTE: 20050513 tgl: Guard this so it is only registered once.
    vtkMRMLVolumeRenderingScenarioNode *vrsNode = vtkMRMLVolumeRenderingScenarioNode::New();
    this->GetMRMLScene()->RegisterNodeClass(vrsNode);
    vrsNode->Delete();

    vtkMRMLVolumeRenderingParametersNode *vrpNode = vtkMRMLVolumeRenderingParametersNode::New();
    this->GetMRMLScene()->RegisterNodeClass(vrpNode);
    vrpNode->Delete();
  }
}
