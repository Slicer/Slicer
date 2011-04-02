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
#include "vtkMRMLSliceLogic.h"

// MRML includes
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
#include <vtkPointData.h>

#include "vtkMRMLVolumeRenderingScenarioNode.h"
#include "vtkMRMLVolumeRenderingParametersNode.h"

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

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

void vtkSlicerVolumeRenderingLogic::UpdateTranferFunctionRangeFromImage(vtkMRMLVolumeRenderingParametersNode* vspNode)
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

void vtkSlicerVolumeRenderingLogic::UpdateFgTranferFunctionRangeFromImage(vtkMRMLVolumeRenderingParametersNode* vspNode)
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

void vtkSlicerVolumeRenderingLogic::UpdateVolumePropertyFromDisplayNode(vtkMRMLVolumeRenderingParametersNode* vspNode)
{
  if (vspNode == NULL || (!vspNode->GetFollowVolumeDisplayNode()) )
  {
    return;
  }
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
}

void vtkSlicerVolumeRenderingLogic::UpdateVolumePropertyFromImageData(vtkMRMLVolumeRenderingParametersNode* vspNode)
{
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
    
    if (vpNode)
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
}

void vtkSlicerVolumeRenderingLogic::SetupFgVolumePropertyFromImageData(vtkMRMLVolumeRenderingParametersNode* vspNode)
{
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
}
void vtkSlicerVolumeRenderingLogic::FitROIToVolume(vtkMRMLVolumeRenderingParametersNode* vspNode)
{
  // resize the ROI to fit the volume
  vtkMRMLROINode *roiNode = vtkMRMLROINode::SafeDownCast(vspNode->GetROINode());
  vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode());

  if (volumeNode && roiNode)
  {
    double xyz[3];
    double center[3];

    vtkMRMLSliceLogic::GetVolumeRASBox(volumeNode, xyz,  center);
    for (int i = 0; i < 3; i++)
    {
      xyz[i] *= 0.5;
    }

    roiNode->SetXYZ(center);
    roiNode->SetRadiusXYZ(xyz);
  }
}

vtkMRMLVolumeRenderingParametersNode* vtkSlicerVolumeRenderingLogic::CreateParametersNode()
{
  vtkMRMLVolumeRenderingParametersNode *node = NULL;

  if (this->GetMRMLScene())
  {
    node = vtkMRMLVolumeRenderingParametersNode::New();
    node->SetCurrentVolumeMapper(0);
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
// Update VolumeRenderingParametersNode from VolumeNode,
// if needed create vtkMRMLVolumePropertyNode and vtkMRMLROINode
// and initioalize them from VolumeNode
void vtkSlicerVolumeRenderingLogic::UpdateParametersNodeFromVolumeNode(
                                          vtkMRMLVolumeRenderingParametersNode *volumeRenderingParametersNode, 
                                          vtkMRMLVolumeNode *volumeNode, 
                                          vtkMRMLVolumePropertyNode **propNode,
                                          vtkMRMLROINode **roiNode)
{

  if (volumeNode == NULL) 
  {
    volumeRenderingParametersNode->SetAndObserveVolumeNodeID(NULL);
    return;
  }

  volumeRenderingParametersNode->SetAndObserveVolumeNodeID(volumeNode->GetID());

  if (*propNode == NULL)
  {
    *propNode = vtkMRMLVolumePropertyNode::New();
    this->GetMRMLScene()->AddNode(*propNode);
    (*propNode)->Delete();
  }
  volumeRenderingParametersNode->SetAndObserveVolumePropertyNodeID((*propNode)->GetID());

  if (*roiNode == NULL)
  {
    *roiNode = vtkMRMLROINode::New();
    this->GetMRMLScene()->AddNode(*roiNode);
    (*roiNode)->Delete();
  }
  volumeRenderingParametersNode->SetAndObserveROINodeID((*roiNode)->GetID());

  this->UpdateVolumePropertyFromImageData(volumeRenderingParametersNode);

  this->FitROIToVolume(volumeRenderingParametersNode);
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
