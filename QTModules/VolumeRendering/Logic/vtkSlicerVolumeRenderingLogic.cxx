/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumeRenderingLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// 
#include "vtkSlicerVolumeRenderingLogic.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLSliceLogic.h"

// MRML includes
#include <vtkCacheManager.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVolumeArchetypeStorageNode.h>
#include <vtkMRMLVolumeHeaderlessStorageNode.h>
#include <vtkMRMLNRRDStorageNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLVolumePropertyStorageNode.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx> 
#include <itksys/SystemTools.hxx> 
#include <itksys/Directory.hxx> 

// VTK includes
#include <vtkAssignAttribute.h>
#include <vtkDiffusionTensorMathematics.h>
#include <vtkImageThreshold.h>
#include <vtkImageAccumulateDiscrete.h>
#include <vtkImageBimodalAnalysis.h>
#include <vtkImageExtractComponents.h>
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>

#include "vtkMRMLVolumeRenderingScenarioNode.h"
#include "vtkMRMLVolumeRenderingDisplayNode.h"

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerVolumeRenderingLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerVolumeRenderingLogic);

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic::vtkSlicerVolumeRenderingLogic()
{
  this->PresetsScene = 0;
}

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic::~vtkSlicerVolumeRenderingLogic()
{
  if (this->PresetsScene)
    {
    this->PresetsScene->Delete();
    }
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

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::RegisterNodes()
{
  if (this->GetMRMLScene())
  {
    // :NOTE: 20050513 tgl: Guard this so it is only registered once.
    vtkMRMLVolumeRenderingScenarioNode *vrsNode = vtkMRMLVolumeRenderingScenarioNode::New();
    this->GetMRMLScene()->RegisterNodeClass(vrsNode);
    vrsNode->Delete();

    vtkMRMLVolumeRenderingDisplayNode *vrpNode = vtkMRMLVolumeRenderingDisplayNode::New();
    this->GetMRMLScene()->RegisterNodeClass(vrpNode);
    vrpNode->Delete();
  }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

void vtkSlicerVolumeRenderingLogic::UpdateTranferFunctionRangeFromImage(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode == 0 || vspNode->GetVolumeNode() == 0 || vspNode->GetVolumePropertyNode() == 0)
  {
    return;
  }
  vtkImageData *input = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData();
  vtkVolumeProperty *prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();
  if (input == NULL || prop == NULL)
    {
    return;
    }

  //update scalar range
  vtkColorTransferFunction *functionColor = prop->GetRGBTransferFunction();

  vtkDataArray* scalars = input->GetPointData()->GetScalars();
  if (!scalars)
    {
    return;
    }

  double rangeNew[2];
  scalars->GetRange(rangeNew);
  functionColor->AdjustRange(rangeNew);
  
  vtkPiecewiseFunction *functionOpacity = prop->GetScalarOpacity();
  functionOpacity->AdjustRange(rangeNew);

  rangeNew[1] = (rangeNew[1] - rangeNew[0])*0.25;
  rangeNew[0] = 0;

  functionOpacity = prop->GetGradientOpacity();
  functionOpacity->RemovePoint(255);//Remove the standard value
  functionOpacity->AdjustRange(rangeNew);
}

void vtkSlicerVolumeRenderingLogic::UpdateFgTranferFunctionRangeFromImage(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  vtkImageData *input = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetFgVolumeNode())->GetImageData();
  vtkVolumeProperty *prop = vspNode->GetFgVolumePropertyNode()->GetVolumeProperty();

  //update scalar range
  vtkColorTransferFunction *functionColor = prop->GetRGBTransferFunction();

  double rangeNew[2];
  input->GetPointData()->GetScalars()->GetRange(rangeNew);
  functionColor->AdjustRange(rangeNew);

  vtkPiecewiseFunction *functionOpacity = prop->GetScalarOpacity();
  functionOpacity->AdjustRange(rangeNew);

  rangeNew[1] = (rangeNew[1] - rangeNew[0])*0.25;
  rangeNew[0] = 0;

  functionOpacity = prop->GetGradientOpacity();
  functionOpacity->RemovePoint(255);//Remove the standard value
  functionOpacity->AdjustRange(rangeNew);
}

void vtkSlicerVolumeRenderingLogic::UpdateVolumePropertyFromDisplayNode(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode == NULL || (!vspNode->GetFollowVolumeDisplayNode()) )
  {
    return;
  }

  int disabledModify = vspNode->StartModify();

  double range[2];
  vtkImageData *input = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData();
  input->GetScalarRange(range);
  /***
  char histogramName[1024];
  this->Histograms->ComputeHistogramName(input->GetPointData()->GetScalars()->GetName(), 0, 0, histogramName);

  //add points into transfer functions
  vtkKWHistogram *histogram = this->Histograms->GetHistogramWithName(histogramName);

  histogram->GetRange(range);
  ***/
  vtkMRMLScalarVolumeDisplayNode *vpNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(vspNode->GetVolumeNode()->GetDisplayNode());
  
  double windowLevel[2];
  windowLevel[0] = vpNode->GetWindow();
  windowLevel[1] = vpNode->GetLevel();

  vspNode->SetWindowLevel(windowLevel);

  double threshold[2];
  threshold[0] = vpNode->GetLowerThreshold();
  threshold[1] = vpNode->GetUpperThreshold();

  //when volumes module is in auto threshold mode, the value of threshold is invalid
  threshold[0] = threshold[0] < range[0] ? range[0] : threshold[0];
  threshold[1] = threshold[1] > range[1] ? range[1] : threshold[1];
  
  vspNode->SetThreshold(threshold);
  
  vtkVolumeProperty *prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();
  prop->SetInterpolationTypeToLinear();

  vtkPiecewiseFunction *opacity = prop->GetScalarOpacity();

  double step = (range[1] - range[0]) * 0.001;
  
  opacity->RemoveAllPoints();
  opacity->AddPoint(range[0], 0.0);
  opacity->AddPoint(threshold[0], 0.0);
  opacity->AddPoint(threshold[0] + step, 1.0);
  opacity->AddPoint(threshold[1] - step, 1.0);
  opacity->AddPoint(threshold[1], 0.0);
  opacity->AddPoint(range[1], 0.0);
    
  vtkColorTransferFunction *colorTransfer = prop->GetRGBTransferFunction();

  colorTransfer->RemoveAllPoints();

  vtkLookupTable* pLut = vpNode->GetColorNode()->GetLookupTable();

  if (pLut == NULL)
  {
    colorTransfer->AddRGBPoint(range[0], 0.0, 0.0, 0.0);
    colorTransfer->AddRGBPoint(windowLevel[1] - windowLevel[0]*0.5, 0.0, 0.0, 0.0);
    colorTransfer->AddRGBPoint(windowLevel[1] + windowLevel[0]*0.5, 1.0, 1.0, 1.0);
    colorTransfer->AddRGBPoint(range[1], 1.0, 1.0, 1.0);
  }
  else
  {
    int size = pLut->GetNumberOfTableValues();

    double color[4];
    pLut->GetTableValue(0, color);
  
    if (size == 1)
    {
      colorTransfer->AddRGBPoint(range[0], color[0], color[1], color[2]);
      colorTransfer->AddRGBPoint(windowLevel[1] - windowLevel[0]*0.5, color[0], color[1], color[2]);
      colorTransfer->AddRGBPoint(windowLevel[1] + windowLevel[0]*0.5, color[0], color[1], color[2]);
      colorTransfer->AddRGBPoint(range[1], color[0], color[1], color[2]);
    }
    else
    {
      colorTransfer->AddRGBPoint(range[0], color[0], color[1], color[2]);
    
      double value = windowLevel[1] - windowLevel[0]*0.5;

      step = windowLevel[0] / (size - 1);

      int downSamplingFactor = 64;
      
      for (int i = 0; i < size; i += downSamplingFactor, value += downSamplingFactor*step)
      {
        pLut->GetTableValue(i, color);
        colorTransfer->AddRGBPoint(value, color[0], color[1], color[2]);
      }

      pLut->GetTableValue(size - 1, color);
      colorTransfer->AddRGBPoint(windowLevel[1] + windowLevel[0]*0.5, color[0], color[1], color[2]);
      colorTransfer->AddRGBPoint(range[1], color[0], color[1], color[2]);
    }
  }
  
  prop->ShadeOn();
  prop->SetAmbient(0.30);
  prop->SetDiffuse(0.60);
  prop->SetSpecular(0.50);
  prop->SetSpecularPower(40);

  vspNode->EndModify(disabledModify);
}

void vtkSlicerVolumeRenderingLogic::UpdateVolumePropertyFromImageData(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  int disabledModify = vspNode->StartModify();

  this->UpdateTranferFunctionRangeFromImage(vspNode);
  //this->SetupHistograms(vspNode);

  if (vspNode->GetFollowVolumeDisplayNode())
  {
    this->UpdateVolumePropertyFromDisplayNode(vspNode);
  }
  else if (vspNode->GetVolumeNode())
  {
    vtkMRMLScalarVolumeDisplayNode *vpNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(vspNode->GetVolumeNode()->GetDisplayNode());
    vtkMRMLLabelMapVolumeDisplayNode *vlNode = vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(vspNode->GetVolumeNode()->GetDisplayNode());
    
    if (vpNode && vspNode->GetVolumeNode()->GetImageData())
      {
      double range[2];
      vtkImageData *input = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData();
      input->GetScalarRange(range);
      /***
      char histogramName[1024];
      this->Histograms->ComputeHistogramName(input->GetPointData()->GetScalars()->GetName(), 0, 0, histogramName);

      //add points into transfer functions
      vtkKWHistogram *histogram = this->Histograms->GetHistogramWithName(histogramName);

      histogram->GetRange(range);
      ***/

      double windowLevel[2];
      windowLevel[0] = vpNode->GetWindow();
      windowLevel[1] = vpNode->GetLevel();

      vspNode->SetWindowLevel(windowLevel);
      
      vtkVolumeProperty *prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();
      prop->SetInterpolationTypeToLinear();

      vtkPiecewiseFunction *opacity = prop->GetScalarOpacity();
      
      opacity->RemoveAllPoints();
      opacity->AddPoint(range[0], 0.0);
      opacity->AddPoint(windowLevel[1] - windowLevel[0]*0.5, 0.0);
      opacity->AddPoint(windowLevel[1] + windowLevel[0]*0.5, 1.0);
      opacity->AddPoint(range[1], 1.0);
        
      vtkColorTransferFunction *colorTransfer = prop->GetRGBTransferFunction();

      colorTransfer->RemoveAllPoints();

      vtkMRMLColorNode* colorNode = vpNode->GetColorNode();
      vtkLookupTable* pLut = colorNode ? colorNode->GetLookupTable() : 0;

      if (pLut == NULL)
      {
        colorTransfer->AddRGBPoint(range[0], 0.0, 0.0, 0.0);
        colorTransfer->AddRGBPoint(windowLevel[1] - windowLevel[0]*0.5, 0.0, 0.0, 0.0);
        colorTransfer->AddRGBPoint(windowLevel[1] + windowLevel[0]*0.5, 1.0, 1.0, 1.0);
        colorTransfer->AddRGBPoint(range[1], 1.0, 1.0, 1.0);
      }
      else
      {
        int size = pLut->GetNumberOfTableValues();

        double color[4];
        pLut->GetTableValue(0, color);
      
        if (size == 1)
        {
          colorTransfer->AddRGBPoint(range[0], color[0], color[1], color[2]);
          colorTransfer->AddRGBPoint(windowLevel[1] - windowLevel[0]*0.5, color[0], color[1], color[2]);
          colorTransfer->AddRGBPoint(windowLevel[1] + windowLevel[0]*0.5, color[0], color[1], color[2]);
          colorTransfer->AddRGBPoint(range[1], color[0], color[1], color[2]);
        }
        else
        {
          colorTransfer->AddRGBPoint(range[0], color[0], color[1], color[2]);
        
          double value = windowLevel[1] - windowLevel[0]*0.5;
          double step;

          step = windowLevel[0] / (size - 1);

          int downSamplingFactor = 64;
          
          for (int i = 0; i < size; i += downSamplingFactor, value += downSamplingFactor*step)
          {
            pLut->GetTableValue(i, color);
            colorTransfer->AddRGBPoint(value, color[0], color[1], color[2]);
          }

          pLut->GetTableValue(size - 1, color);
          colorTransfer->AddRGBPoint(windowLevel[1] + windowLevel[0]*0.5, color[0], color[1], color[2]);
          colorTransfer->AddRGBPoint(range[1], color[0], color[1], color[2]);
        }
      }
      
      prop->ShadeOn();
      prop->SetAmbient(0.30);
      prop->SetDiffuse(0.60);
      prop->SetSpecular(0.50);
      prop->SetSpecularPower(40);
    }
  else if (vlNode)
    {
    //TODO label map
    } 

  vtkImageData* imageData = vspNode->GetVolumeNode() ? vtkMRMLScalarVolumeNode::SafeDownCast(
    vspNode->GetVolumeNode())->GetImageData() : 0;
  vtkDataArray* scalars = (imageData && imageData->GetPointData()) ? imageData->GetPointData()->GetScalars() : 0;
  if (scalars)
    {
    double scalarRange[2];
    scalars->GetRange(scalarRange, 0);
    vspNode->SetDepthPeelingThreshold(scalarRange[0]);

    vspNode->SetThreshold(scalarRange);
    }
  }

  vspNode->EndModify(disabledModify);
}

void vtkSlicerVolumeRenderingLogic::SetupFgVolumePropertyFromImageData(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  int disabledModify = vspNode->StartModify();

  this->UpdateFgTranferFunctionRangeFromImage(vspNode);
  //this->SetupHistogramsFg(vspNode);

  if (vspNode->GetFgVolumeNode())
  {
    vtkMRMLScalarVolumeDisplayNode *vpNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(vspNode->GetFgVolumeNode()->GetDisplayNode());
    vtkMRMLLabelMapVolumeDisplayNode *vlNode = vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(vspNode->GetFgVolumeNode()->GetDisplayNode());
    
    if (vpNode)
    {
      vtkImageData *input = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData();
      double range[2];
      input->GetScalarRange(range);

      /***
      char histogramName[1024];
      this->Histograms->ComputeHistogramName(input->GetPointData()->GetScalars()->GetName(), 0, 0, histogramName);

      //add points into transfer functions
      vtkKWHistogram *histogram = this->Histograms->GetHistogramWithName(histogramName);

      histogram->GetRange(range);
      ***/

      double windowLevel[2];
      windowLevel[0] = vpNode->GetWindow();
      windowLevel[1] = vpNode->GetLevel();

      vspNode->SetWindowLevelFg(windowLevel);
      
      vtkVolumeProperty *prop = vspNode->GetFgVolumePropertyNode()->GetVolumeProperty();
      prop->SetInterpolationTypeToLinear();

      vtkPiecewiseFunction *opacity = prop->GetScalarOpacity();
      
      opacity->RemoveAllPoints();
      opacity->AddPoint(range[0], 0.0);
      opacity->AddPoint(windowLevel[1] - windowLevel[0]*0.5, 0.0);
      opacity->AddPoint(windowLevel[1] + windowLevel[0]*0.5, 1.0);
      opacity->AddPoint(range[1], 1.0);
        
      vtkColorTransferFunction *colorTransfer = prop->GetRGBTransferFunction();

      colorTransfer->RemoveAllPoints();

      vtkLookupTable* pLut = vpNode->GetColorNode()->GetLookupTable();

      if (pLut == NULL)
      {
        colorTransfer->AddRGBPoint(range[0], 0.0, 0.0, 0.0);
        colorTransfer->AddRGBPoint(windowLevel[1] - windowLevel[0]*0.5, 0.0, 0.0, 0.0);
        colorTransfer->AddRGBPoint(windowLevel[1] + windowLevel[0]*0.5, 1.0, 1.0, 1.0);
        colorTransfer->AddRGBPoint(range[1], 1.0, 1.0, 1.0);
      }
      else
      {
        int size = pLut->GetNumberOfTableValues();

        double color[4];
        pLut->GetTableValue(0, color);
      
        if (size == 1)
        {
          colorTransfer->AddRGBPoint(range[0], color[0], color[1], color[2]);
          colorTransfer->AddRGBPoint(windowLevel[1] - windowLevel[0]*0.5, color[0], color[1], color[2]);
          colorTransfer->AddRGBPoint(windowLevel[1] + windowLevel[0]*0.5, color[0], color[1], color[2]);
          colorTransfer->AddRGBPoint(range[1], color[0], color[1], color[2]);
        }
        else
        {
          colorTransfer->AddRGBPoint(range[0], color[0], color[1], color[2]);
        
          double value = windowLevel[1] - windowLevel[0]*0.5;
          double step;

          step = windowLevel[0] / (size - 1);

          int downSamplingFactor = 64;
          
          for (int i = 0; i < size; i += downSamplingFactor, value += downSamplingFactor*step)
          {
            pLut->GetTableValue(i, color);
            colorTransfer->AddRGBPoint(value, color[0], color[1], color[2]);
          }

          pLut->GetTableValue(size - 1, color);
          colorTransfer->AddRGBPoint(windowLevel[1] + windowLevel[0]*0.5, color[0], color[1], color[2]);
          colorTransfer->AddRGBPoint(range[1], color[0], color[1], color[2]);
        }
      }
      
      prop->ShadeOn();
      prop->SetAmbient(0.30);
      prop->SetDiffuse(0.60);
      prop->SetSpecular(0.50);
      prop->SetSpecularPower(40);
    }
    else if (vlNode)
    {
    //TODO label map
    }

    if (vspNode->GetVolumeNode() && vspNode->GetVolumeNode()->GetImageData())
    {
        double scalarRange[2];
        vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData()->GetPointData()->GetScalars()->GetRange(scalarRange, 0);
        vspNode->SetDepthPeelingThreshold(scalarRange[0]);

        vspNode->SetThreshold(scalarRange);
    }
  }
  vspNode->EndModify(disabledModify);

}
void vtkSlicerVolumeRenderingLogic::FitROIToVolume(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{

  // resize the ROI to fit the volume
  vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::SafeDownCast(vspNode->GetROINode());
  vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode());


  if (volumeNode && roiNode)
  {
    int disabledModify = roiNode->StartModify();
  
    double xyz[3];
    double center[3];

    vtkMRMLSliceLogic::GetVolumeRASBox(volumeNode, xyz,  center);
    for (int i = 0; i < 3; i++)
    {
      xyz[i] *= 0.5;
    }

    roiNode->SetXYZ(center);
    roiNode->SetRadiusXYZ(xyz);

    roiNode->EndModify(disabledModify);
  }


}

vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::CreateVolumeRenderingDisplayNode()
{
  vtkMRMLVolumeRenderingDisplayNode *node = NULL;

  if (this->GetMRMLScene())
  {
    node = vtkMRMLVolumeRenderingDisplayNode::New();
    node->SetCurrentVolumeMapper(vtkMRMLVolumeRenderingDisplayNode::VTKCPURayCast);
    this->GetMRMLScene()->AddNode(node);
    node->Delete();
  }

  return node;
}

vtkMRMLVolumeRenderingScenarioNode* vtkSlicerVolumeRenderingLogic::CreateScenarioNode()
{
  vtkMRMLVolumeRenderingScenarioNode *node = NULL;

  if (this->GetMRMLScene())
  {
    node = vtkMRMLVolumeRenderingScenarioNode::New();
    this->GetMRMLScene()->AddNode(node);
    node->Delete();
  }

  return node;
}

// Description:
// Remove ViewNode from VolumeRenderingDisplayNode for a VolumeNode,
void vtkSlicerVolumeRenderingLogic::RemoveViewFromVolumeDisplayNodes(
                                          vtkMRMLVolumeNode *volumeNode, 
                                          vtkMRMLViewNode *viewNode)
{
  if (viewNode == NULL || volumeNode == NULL)
    {
    return;
    }

  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode)
      {
      dnode->RemoveViewNodeID(viewNode->GetID());
      }
    }
}

// Description:
// Find volume rendering display node reference in the volume
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetVolumeRenderingDisplayNodeByID(
                                                              vtkMRMLVolumeNode *volumeNode, 
                                                              char *displayNodeID)
{
  if (displayNodeID == NULL || volumeNode == NULL)
    {
    return NULL;
    }

  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode && !strcmp(displayNodeID, dnode->GetID()))
      {
      return dnode;
      }
    }
  return NULL;
}

// Description:
// Find first volume rendering display node
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetFirstVolumeRenderingDisplayNode(
                                                              vtkMRMLVolumeNode *volumeNode)
{
  if (volumeNode == NULL)
    {
    return NULL;
    }
  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode)
      {
      return dnode;
      }
    }
  return NULL;
}

// Description:
// Find volume rendering display node referencing the view node and volume node
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetVolumeRenderingDisplayNodeForViewNode(
                                                              vtkMRMLVolumeNode *volumeNode, 
                                                              vtkMRMLViewNode *viewNode)
{
  if (viewNode == NULL || volumeNode == NULL)
    {
    return NULL;
    }
  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode)
      {
      for (int j=0; j<dnode->GetNumberOfViewNodeIDs(); j++)
        {
        if (!strcmp(viewNode->GetID(), dnode->GetNthViewNodeID(j)))
          {
          return dnode;
          }
        }
      }
    }
  return NULL;
}

// Description:
  // Find volume rendering display node referencing the view node in the scene
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetVolumeRenderingDisplayNodeForViewNode(
                                                              vtkMRMLViewNode *viewNode)
{
  if (viewNode == NULL || viewNode->GetScene() == NULL)
    {
    return NULL;
    }
  std::vector<vtkMRMLNode *> nodes;
  viewNode->GetScene()->GetNodesByClass("vtkMRMLVolumeRenderingDisplayNode", nodes);

  for (unsigned int i=0; i<nodes.size(); i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      nodes[i]);
    if (dnode && dnode->IsViewNodeIDPresent(viewNode->GetID()))
      {
      return dnode;
      }
    }
  return NULL;
}

vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic
::GetFirstVolumeRenderingDisplayNodeByROINode(vtkMRMLAnnotationROINode* roiNode)
{
  if (roiNode == NULL || roiNode->GetScene() == NULL)
    {
    return NULL;
    }
  std::vector<vtkMRMLNode *> nodes;
  roiNode->GetScene()->GetNodesByClass("vtkMRMLVolumeRenderingDisplayNode", nodes);

  for (unsigned int i = 0; i < nodes.size(); ++i)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode =
      vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(nodes[i]);
    if (dnode && dnode->GetROINodeID() &&
        !strcmp(dnode->GetROINodeID(), roiNode->GetID()))
      {
      return dnode;
      }
    }
  return NULL;
}

// Description:
// Update vtkMRMLVolumeRenderingDisplayNode from VolumeNode,
// if needed create vtkMRMLVolumePropertyNode and vtkMRMLAnnotationROINode
// and initioalize them from VolumeNode
void vtkSlicerVolumeRenderingLogic::UpdateDisplayNodeFromVolumeNode(
                                          vtkMRMLVolumeRenderingDisplayNode *displayNode,
                                          vtkMRMLVolumeNode *volumeNode,
                                          vtkMRMLVolumePropertyNode **propNode,
                                          vtkMRMLAnnotationROINode **roiNode)
{

  if (volumeNode == NULL)
    {
    displayNode->SetAndObserveVolumeNodeID(NULL);
    return;
    }

  displayNode->SetAndObserveVolumeNodeID(volumeNode->GetID());

  if (*propNode == NULL)
    {
    *propNode = vtkMRMLVolumePropertyNode::New();
    this->GetMRMLScene()->AddNode(*propNode);
    (*propNode)->Delete();
    }
  displayNode->SetAndObserveVolumePropertyNodeID((*propNode)->GetID());

  if (*roiNode == NULL)
    {
    *roiNode = vtkMRMLAnnotationROINode::New();
    // by default, show the ROI only if cropping is enabled
    (*roiNode)->SetVisibility(displayNode->GetCroppingEnabled());
    this->GetMRMLScene()->AddNode(*roiNode);
    (*roiNode)->Delete();
    }
  displayNode->SetAndObserveROINodeID((*roiNode)->GetID());

  this->UpdateVolumePropertyFromImageData(displayNode);

  this->FitROIToVolume(displayNode);
}

//----------------------------------------------------------------------------

vtkMRMLVolumePropertyNode* vtkSlicerVolumeRenderingLogic::AddVolumePropertyFromFile (const char* filename)
{
  vtkMRMLVolumePropertyNode *vpNode = vtkMRMLVolumePropertyNode::New();
  vtkMRMLVolumePropertyStorageNode *vpStorageNode = vtkMRMLVolumePropertyStorageNode::New();

  // check for local or remote files
  int useURI = 0; // false;
  if (this->GetMRMLScene()->GetCacheManager() != NULL)
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    }
  
  itksys_stl::string name;
  const char *localFile;
  if (useURI)
    {
    vpStorageNode->SetURI(filename);
     // reset filename to the local file name
    localFile = ((this->GetMRMLScene())->GetCacheManager())->GetFilenameFromURI(filename);
    }
  else
    {
    vpStorageNode->SetFileName(filename);
    localFile = filename;
    }
  const itksys_stl::string fname(localFile);
  // the model name is based on the file name (itksys call should work even if
  // file is not on disk yet)
  name = itksys::SystemTools::GetFilenameName(fname);
  
  // check to see which node can read this type of file
  if (!vpStorageNode->SupportedFileType(name.c_str()))
    {
    vpStorageNode->Delete();
    vpStorageNode = NULL;
    }

  /* don't read just yet, need to add to the scene first for remote reading
  if (vpStorageNode->ReadData(vpNode) != 0)
    {
    storageNode = vpStorageNode;
    }
  */
  if (vpStorageNode != NULL)
    {
    std::string uname( this->GetMRMLScene()->GetUniqueNameByString(name.c_str()));

    vpNode->SetName(uname.c_str());

    this->GetMRMLScene()->SaveStateForUndo();

    vpNode->SetScene(this->GetMRMLScene());
    vpStorageNode->SetScene(this->GetMRMLScene());

    this->GetMRMLScene()->AddNodeNoNotify(vpStorageNode);  
    vpNode->SetAndObserveStorageNodeID(vpStorageNode->GetID());

    this->GetMRMLScene()->AddNode(vpNode);  

    //this->Modified();  

    // the scene points to it still
    vpNode->Delete();

    // now set up the reading
    int retval = vpStorageNode->ReadData(vpNode);
    if (retval != 1)
      {
      vtkErrorMacro("AddVolumePropertyFromFile: error reading " << filename);
      this->GetMRMLScene()->RemoveNode(vpNode);
      this->GetMRMLScene()->RemoveNode(vpStorageNode);
      vpNode = NULL;
      }
    }
  else
    {
    vtkDebugMacro("Couldn't read file, returning null model node: " << filename);
    vpNode->Delete();
    vpNode = NULL;
    }
  if (vpStorageNode)
    {
    vpStorageNode->Delete();
    }
  return vpNode;  
}

//---------------------------------------------------------------------------
vtkMRMLScene* vtkSlicerVolumeRenderingLogic::GetPresetsScene()
{
  if (!this->PresetsScene)
    {
    this->PresetsScene = vtkMRMLScene::New();
    this->LoadPresets(this->PresetsScene);
    }
  return this->PresetsScene;
}

//---------------------------------------------------------------------------
bool vtkSlicerVolumeRenderingLogic::LoadPresets(vtkMRMLScene* scene)
{
  vtkMRMLVolumePropertyNode *vrNode = vtkMRMLVolumePropertyNode::New();
  //Register node class
  this->PresetsScene->RegisterNodeClass(vrNode);
  vrNode->Delete();

  vtksys_stl::string presetFileName(
    this->GetModuleShareDirectory());
  presetFileName += "/presets.xml";

  scene->SetURL(presetFileName.c_str());
  return scene->Connect() == 1;
}
