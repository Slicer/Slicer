/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkVolumeMathLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkVolumeMathLogic.h"
#include "vtkMRMLScalarVolumeNode.h"

#include "vtkImageAccumulate.h"
#include "vtkImageThreshold.h"
#include "vtkImageMathematics.h"
#include "vtkImageToImageStencil.h"

vtkVolumeMathLogic* vtkVolumeMathLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVolumeMathLogic");
  if(ret)
    {
      return (vtkVolumeMathLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkVolumeMathLogic;
}


//----------------------------------------------------------------------------
vtkVolumeMathLogic::vtkVolumeMathLogic()
{
  this->VolumeMathNode = NULL;
  this->SetProgress(0);
}

//----------------------------------------------------------------------------
vtkVolumeMathLogic::~vtkVolumeMathLogic()
{
  vtkSetMRMLNodeMacro(this->VolumeMathNode, NULL);
}

//----------------------------------------------------------------------------
void vtkVolumeMathLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//----------------------------------------------------------------------------
void vtkVolumeMathLogic::Apply()
{
  int lo, hi;
  
  // check if MRML node is present 
  if (this->VolumeMathNode == NULL)
    {
    vtkErrorMacro("No input VolumeMathNode found");
    return;
    }


  // find input grayscale volume
  vtkMRMLScalarVolumeNode *inGrayscaleVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->VolumeMathNode->GetInputGrayscaleRef()));
  if (inGrayscaleVolume == NULL)
    {
    vtkErrorMacro("No input volume found with id= " << this->VolumeMathNode->GetInputGrayscaleRef());
    return;
    }
  
  // find input labelmap volume
  vtkMRMLScalarVolumeNode *inLabelmapVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->VolumeMathNode->GetInputLabelmapRef()));
  if (inLabelmapVolume == NULL)
    {
    vtkErrorMacro("No input volume found with id= " << this->VolumeMathNode->GetInputLabelmapRef());
    return;
    }
  
  this->InvokeEvent(vtkVolumeMathLogic::StartLabelStats, (void*)"start label stats");
  

  //clear the LabelStats result list
  this->VolumeMathNode->LabelStats.clear();
 
  vtkImageAccumulate *stataccum = vtkImageAccumulate::New();
  stataccum->SetInput(inLabelmapVolume->GetImageData());
  stataccum->Update();
  lo = (int)(stataccum->GetMin())[0];
  hi = (int)(stataccum->GetMax())[0];
  stataccum->Delete();

  std::string tmpString("Label\tCount\tMin\tMax\tMean\tStdDev\n");
  this->VolumeMathNode->SetResultText(tmpString.c_str());

   for(int i = lo; i <= hi; i++ ) 
   {
     this->SetProgress((float)i/hi);
     std::string event_message = "Label ";
     std::stringstream s;
     s << i;
     event_message.append(s.str());
     this->InvokeEvent(vtkVolumeMathLogic::LabelStatsOuterLoop, (void*)event_message.c_str());
     //logic copied from slicer2 VolumeMath MaskStat
     // create the binary volume of the label
     vtkImageThreshold *thresholder = vtkImageThreshold::New();
     thresholder->SetInput(inLabelmapVolume->GetImageData());
     thresholder->SetInValue(1);
     thresholder->SetOutValue(0);
     thresholder->ReplaceOutOn();
     thresholder->ThresholdBetween(i,i);
     thresholder->SetOutputScalarType(inGrayscaleVolume->GetImageData()->GetScalarType());
     thresholder->Update();
     
     this->InvokeEvent(vtkVolumeMathLogic::LabelStatsInnerLoop, (void*)"0.25");
     
     // use vtk's statistics class with the binary labelmap as a stencil
     vtkImageToImageStencil *stencil = vtkImageToImageStencil::New();
     stencil->SetInput(thresholder->GetOutput());
     stencil->ThresholdBetween(1, 1);
     
     this->InvokeEvent(vtkVolumeMathLogic::LabelStatsInnerLoop, (void*)"0.5");
     
    vtkImageAccumulate *stat1 = vtkImageAccumulate::New();
    stat1->SetInput(inGrayscaleVolume->GetImageData());
    stat1->SetStencil(stencil->GetOutput());
    stat1->Update();
    
    stencil->Delete();

 this->InvokeEvent(vtkVolumeMathLogic::LabelStatsInnerLoop, (void*)"0.75");

    if ( (stat1->GetVoxelCount()) > 0 ) 
      {
      std::cout << i << "\t";
      std::cout << stat1->GetVoxelCount() << "\t";
      std::cout << (stat1->GetMin())[0] <<  "\t";
      std::cout << (stat1->GetMax())[0] << "\t";
      std::cout << (stat1->GetMean())[0] << "\t";
      std::cout << (stat1->GetStandardDeviation())[0] << std::endl;
      std::cout << std::endl;
      
      //add an entry to the LabelStats list
      vtkMRMLVolumeMathNode::LabelStatsEntry entry;
      entry.Label = i;
      entry.Count = stat1->GetVoxelCount();
      entry.Min = (int)(stat1->GetMin())[0];
      entry.Max = (int)(stat1->GetMax())[0];
      entry.Mean = (stat1->GetMean())[0];
      entry.StdDev = (stat1->GetStandardDeviation())[0];
      
   this->InvokeEvent(vtkVolumeMathLogic::LabelStatsInnerLoop, (void*)"1");

      this->VolumeMathNode->LabelStats.push_back(entry);

      {
      std::stringstream ss;
      std::string str;
      ss << i;
      ss >> str;
      tmpString.append(str);
      tmpString.append("\t");
      }  
      {
      std::stringstream ss;
      std::string str;
      ss << stat1->GetVoxelCount();
      ss >> str;
      tmpString.append(str);
      tmpString.append("\t");
      }
      {
      std::stringstream ss;
      std::string str;
      ss << (stat1->GetMin())[0];
      ss >> str;
      tmpString.append(str);
      tmpString.append("\t");
      }
      {
      std::stringstream ss;
      std::string str;
      ss << (stat1->GetMax())[0];
      ss >> str;
      tmpString.append(str);
      tmpString.append("\t");
      }
      {
      std::stringstream ss;
      std::string str;
      ss << (stat1->GetMean())[0];
      ss >> str;
      tmpString.append(str);
      tmpString.append("\t");
      }
      {
      std::stringstream ss;
      std::string str;
      ss << (stat1->GetStandardDeviation())[0];
      ss >> str;
      tmpString.append(str);
      tmpString.append("\t");
      }

      tmpString.append("\n");
      this->VolumeMathNode->SetResultText(tmpString.c_str());
      }
    //  multMath->Delete();
    thresholder->Delete();
    stat1->Delete();
    }
   this->InvokeEvent(vtkVolumeMathLogic::EndLabelStats, (void*)"end label stats");
}

float  vtkVolumeMathLogic::GetProgress(){
  return this->Progress;
}

void  vtkVolumeMathLogic::SetProgress(float p){
  this->Progress = p;
}
