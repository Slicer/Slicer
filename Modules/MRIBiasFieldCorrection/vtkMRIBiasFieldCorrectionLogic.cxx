/*=auto====================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRIBiasFieldCorrectionLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $
Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

====================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRIBiasFieldCorrectionLogic.h"
//#include "vtkMRIBiasFieldCorrectionGUI.h"
#include "vtkMRIBiasFieldCorrection.h"
//#include "vtkImageMeanIntensityNormalization.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"

#include "vtkExtractVOI.h"
#include "vtkImageThreshold.h"
#include "vtkImageClip.h"
#include "vtkImageReslice.h"


#include "vtkDataArray.h"
#include "vtkObjectFactory.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkGenericAttribute.h"

#define vtkMRIBiasFieldCorrectionLogic_DebugMacro(msg) \
  std::cout << __FILE__ << "\nLine " << __LINE__ << " " << msg << std::endl;

////////////////////////////////////////////////////////

#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkKWScale.h"

////////////////////////////////////////////////////////
#include "itkImage.h"
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"

#include "itkN3MRIBiasFieldCorrectionImageFilter.h"
#include "itkBSplineControlPointImageFilter.h"

#include "itkBinaryThresholdImageFilter.h"

#include "itkBSplineControlPointImageFilter.h"
#include "itkExpImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkShrinkImageFilter.h"

#include "itkImageToVTKImageFilter.h"
#include "itkVTKImageToImageFilter.h"

#include "vtkImageCast.h"
////////////////////////////////////////////////////////

#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkKWScale.h"
#include "vtkMRIBiasFieldCorrectionGUI.h"

///////////////////////////////////////////////////////


vtkMRIBiasFieldCorrectionLogic* vtkMRIBiasFieldCorrectionLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRIBiasFieldCorrectionLogic");
  if(ret)
    {
      return (vtkMRIBiasFieldCorrectionLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRIBiasFieldCorrectionLogic;
}


//----------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionLogic::vtkMRIBiasFieldCorrectionLogic()
{
  this->MRIBiasFieldCorrectionNode = NULL;
}

//----------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionLogic::~vtkMRIBiasFieldCorrectionLogic()
{
  vtkSetMRMLNodeMacro(this->MRIBiasFieldCorrectionNode, NULL);
}

//----------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

void vtkMRIBiasFieldCorrectionLogic::Apply()
{
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");

  // check if MRML node is present 
  if (this->MRIBiasFieldCorrectionNode == NULL)
    {
    vtkErrorMacro("No input GradientAnisotropicDiffusionFilterNode found");
    return;
    }

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));
  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }
    
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
     // find mask volume
  vtkMRMLScalarVolumeNode *maskVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetMaskVolumeRef()));
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  if (maskVolume == NULL)
    {
    vtkErrorMacro("No mask volume found with id= " << this->MRIBiasFieldCorrectionNode->GetMaskVolumeRef());
    return;
    }
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  // find output volume
  vtkMRMLScalarVolumeNode *outVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetOutputVolumeRef()));
  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found with id= " << this->MRIBiasFieldCorrectionNode->GetOutputVolumeRef());
    return;
    }

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (outVolume->GetName());
  std::string id (outVolume->GetID());

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  outVolume->SetName(name.c_str());
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  typedef itk::Image< float,  3 >   InputImageType;
  typedef itk::Image< unsigned char,   3>   MaskImageType;
  typedef itk::Image< float,  3 >   OutputImageType;
  typedef itk::Image< float, 3 >  InternalImageType; 
 
  typedef itk::ImageFileReader< InputImageType >  ReaderType;

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  ReaderType::Pointer  reader = ReaderType::New();
// input volume conversion
  vtkImageCast* VtkCaster = vtkImageCast::New();
  VtkCaster->SetInput(inVolume->GetImageData());
  VtkCaster->SetOutputScalarTypeToFloat(); 
  VtkCaster->Modified();
  VtkCaster->Update();
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
    typedef itk::VTKImageToImageFilter< InternalImageType >VTK2ITKConnectorFilterType;
    VTK2ITKConnectorFilterType::Pointer VTK2ITKconnector =VTK2ITKConnectorFilterType::New();
    VTK2ITKconnector->SetInput( VtkCaster->GetOutput() );
    VTK2ITKconnector->GetImporter()->Update();
    VTK2ITKconnector->Update();
    
// mask volume conversion
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
    vtkImageCast* VtkCasterM = vtkImageCast::New();
  VtkCasterM->SetInput(maskVolume->GetImageData());
  VtkCasterM->SetOutputScalarTypeToFloat(); 
  VtkCasterM->Modified();
  VtkCasterM->Update();
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
    typedef itk::VTKImageToImageFilter< InternalImageType >VTK2ITKConnectorFilterTypeM;
    VTK2ITKConnectorFilterTypeM::Pointer VTK2ITKconnectorM =VTK2ITKConnectorFilterTypeM::New();
    VTK2ITKconnectorM->SetInput( VtkCasterM->GetOutput() );
    VTK2ITKconnectorM->GetImporter()->Update();
    VTK2ITKconnectorM->Update();

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
// processing                               
  typedef itk::ShrinkImageFilter<InputImageType, InputImageType> ShrinkerType;
  ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( VTK2ITKconnector->GetOutput() );
  shrinker->SetShrinkFactors( 1 );                                     
                                       
    
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  typedef itk::BinaryThresholdImageFilter<InputImageType, MaskImageType>  mFilterType;
               
  mFilterType::Pointer mfilter = mFilterType::New();  
  mfilter->SetInput( VTK2ITKconnectorM->GetOutput() );
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  mfilter->SetLowerThreshold(1);
  mfilter->SetOutsideValue(0);
  mfilter->SetInsideValue(1);  
  mfilter->UpdateLargestPossibleRegion();
   
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  MaskImageType::Pointer maskImage = NULL;
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  maskImage = mfilter->GetOutput();
  typedef itk::ShrinkImageFilter<MaskImageType, MaskImageType> MaskShrinkerType;
  MaskShrinkerType::Pointer maskshrinker = MaskShrinkerType::New();
  maskshrinker->SetInput( maskImage );
  maskshrinker->SetShrinkFactors( 1 );
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  std::cout<<"shrink factor: "<<this->MRIBiasFieldCorrectionNode->GetShrink()<< std::endl;
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  shrinker->SetShrinkFactors( this->MRIBiasFieldCorrectionNode->GetShrink());
  maskshrinker->SetShrinkFactors(this->MRIBiasFieldCorrectionNode->GetShrink());
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  shrinker->Update();
  shrinker->UpdateLargestPossibleRegion();
  maskshrinker->Update();
  maskshrinker->UpdateLargestPossibleRegion();
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  typedef itk::N3MRIBiasFieldCorrectionImageFilter<InputImageType, MaskImageType,
    InputImageType> CorrecterType;
  CorrecterType::Pointer correcter = CorrecterType::New();
  correcter->SetInput( shrinker->GetOutput() );
  correcter->SetMaskImage( maskshrinker->GetOutput() );
  
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  std::cout<<"number of iteration: "<<this->MRIBiasFieldCorrectionNode->GetMax()<< std::endl;
    correcter->SetMaximumNumberOfIterations( this->MRIBiasFieldCorrectionNode->GetMax() );

  std::cout<<"number of fitting level: "<<this->MRIBiasFieldCorrectionNode->GetNum()<< std::endl;
    correcter->SetNumberOfFittingLevels(this->MRIBiasFieldCorrectionNode->GetNum());
    
  std::cout<<"Wiener Filter Noise: "<<this->MRIBiasFieldCorrectionNode->GetWien()<< std::endl;
    correcter->SetWeinerFilterNoise(this->MRIBiasFieldCorrectionNode->GetWien());
    
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  std::cout<<"Bias Field Full..: "<<this->MRIBiasFieldCorrectionNode->GetField()<< std::endl;
    correcter->SetBiasFieldFullWidthAtHalfMaximum(this->MRIBiasFieldCorrectionNode->GetField());
    
   std::cout<<"Convergence Threshold: "<<this->MRIBiasFieldCorrectionNode->GetCon()<< std::endl;   
    correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode->GetCon());
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  correcter->Update();
 
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
    typedef itk::BSplineControlPointImageFilter<
    CorrecterType::BiasFieldControlPointLatticeType, 
    CorrecterType::ScalarImageType> BSplinerType;
  BSplinerType::Pointer bspliner = BSplinerType::New();
  bspliner->SetInput( correcter->GetBiasFieldControlPointLattice() );
  bspliner->SetSplineOrder( correcter->GetSplineOrder() );
  bspliner->SetSize(
    VTK2ITKconnector->GetOutput()->GetLargestPossibleRegion().GetSize() );
  bspliner->SetOrigin( VTK2ITKconnector->GetOutput()->GetOrigin() );
  bspliner->SetDirection( VTK2ITKconnector->GetOutput()->GetDirection() );
  bspliner->SetSpacing( VTK2ITKconnector->GetOutput()->GetSpacing() );
  bspliner->Update();
 
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
    correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode->GetCon());

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  InputImageType::Pointer logField = InputImageType::New();
  logField->SetOrigin( bspliner->GetOutput()->GetOrigin() );
  logField->SetSpacing( bspliner->GetOutput()->GetSpacing() );
  logField->SetRegions(
    bspliner->GetOutput()->GetLargestPossibleRegion().GetSize() );
  logField->SetDirection( bspliner->GetOutput()->GetDirection() );
  logField->Allocate();

    correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode->GetCon());

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  itk::ImageRegionIterator<CorrecterType::ScalarImageType> ItB(
    bspliner->GetOutput(),
    bspliner->GetOutput()->GetLargestPossibleRegion() );
  itk::ImageRegionIterator<InputImageType> ItF( logField,
    logField->GetLargestPossibleRegion() );
  for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
    {
    ItF.Set( ItB.Get()[0] );
    }
   
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
    correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode->GetCon());

  typedef itk::ExpImageFilter<InputImageType, InputImageType> ExpFilterType;
  ExpFilterType::Pointer expFilter = ExpFilterType::New();
  expFilter->SetInput( logField );
  expFilter->Update();
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
    correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode->GetCon());

  typedef itk::DivideImageFilter<InputImageType, InputImageType, InputImageType> DividerType;
  DividerType::Pointer divider = DividerType::New();
  divider->SetInput1( VTK2ITKconnector->GetOutput() );
  divider->SetInput2( expFilter->GetOutput() );
  divider->Update();  
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");

  
 typedef itk::ImageToVTKImageFilter< InternalImageType >
ITK2VTKConnectorFilterType;
        ITK2VTKConnectorFilterType::Pointer ITK2VTKconnector =
ITK2VTKConnectorFilterType::New();
        ITK2VTKconnector->GetExporter()->SetInput(divider->GetOutput()); 
        ITK2VTKconnector->GetImporter()->Update(); 
  

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  vtkImageData* image = vtkImageData::New(); 
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  image->DeepCopy( ITK2VTKconnector->GetImporter()->GetOutput());
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  outVolume->SetAndObserveImageData(image);
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  image->Delete();
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  outVolume->SetModifiedSinceRead(1);
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
}

void vtkMRIBiasFieldCorrectionLogic::SliceProcess(vtkTransform* xyToijk,double dim0,double dim1)
{
  return;
}
#if 0
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
 // check if MRML node is present
  if (this->MRIBiasFieldCorrectionNode == NULL)
    {
    vtkErrorMacro("No input MRIBiasFieldCorrectionNode found");
    return;
    }

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));
  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  // create storage volume for preview
  vtkMRMLScalarVolumeNode *stoVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetStorageVolumeRef()));
  if (stoVolume == NULL)
    {
    vtkErrorMacro("No storage volume found with id= " << this->MRIBiasFieldCorrectionNode->GetStorageVolumeRef());
    return;
    }
    
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
 // create Mask volume for processing
  vtkMRMLScalarVolumeNode *maskVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetMaskVolumeRef()));
  if (maskVolume == NULL)
    {
    vtkErrorMacro("No mask volume found with id= " << this->MRIBiasFieldCorrectionNode->GetMaskVolumeRef());
    return;
    }

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
 // create output volume for preview
  vtkMRMLScalarVolumeNode *outVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetOutputVolumeRef()));
  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found with id= " << this->MRIBiasFieldCorrectionNode->GetOutputVolumeRef());
    return;
    }


  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (stoVolume->GetName());
  std::string id (stoVolume->GetID());

  stoVolume->CopyOrientation(inVolume);
  stoVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  stoVolume->SetName(name.c_str());
  
  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  outVolume->SetName(name.c_str());
  
  this->STORAGE = vtkImageData::New(); 
  this->PREVIEW = vtkImageData::New(); 
  
  this->STORAGE->DeepCopy(inVolume->GetImageData());
  this->PREVIEW->DeepCopy(inVolume->GetImageData());
  
  //stoVolume->SetAndObserveImageData(this->PREVIEW);
  //GET SIZE OF THE ARRAY TO BE PROCESSED AND THE FIRST PIXEL IN FRAME

  double size1 = 0;
  double size2 = 0;
  double xyPt[4];
  double ijkPt[3];
  int begin[2];

  begin[0] = 0;
  begin[1] = 0;
  
  xyPt[1] = round(dim1/2);
  xyPt[2] = 0;
  xyPt[3] = 1;
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  int* extent  = inVolume->GetImageData()->GetWholeExtent();
  
  for(int i = 0; i < dim0; i++) {       
  xyPt[0] = round(i);
  xyToijk->MultiplyPoint(xyPt,ijkPt);
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  if(ijkPt[0]<0 || ijkPt[0]>=extent[1] ||ijkPt[1]<0 || ijkPt[1]>=extent[3] ||ijkPt[2]<0 || ijkPt[2]>=extent[5] ){
  //std::cout<<"OUT OF VOI"<<std::endl;
  }
  else{
  if(size1 == 0){
  begin[0] = i;
  }
  size1++;
  }
  }
  
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  
  xyPt[0] = round(dim0/2);
  
  for(int i = 0; i < dim1; i++) {       
        xyPt[1] = round(i);
        xyToijk->MultiplyPoint(xyPt,ijkPt);
  //STORAGE->SetScalarComponentFromDouble(ijkPt[0],ijkPt[1],ijkPt[2],0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt[0],ijkPt[1],ijkPt[2],0));
  if(ijkPt[0]<0 || ijkPt[0]>=extent[1] ||ijkPt[1]<0 || ijkPt[1]>=extent[3] ||ijkPt[2]<0 || ijkPt[2]>=extent[5] ){
  //std::cout<<"OUT OF VOI"<<std::endl;
  }
  else{
  if(size2 == 0){
  begin[1] = i;
  }
  size2++;
  }
  }
  //std::cout<<"DIM : "<< dim0 <<" SIZE : "<< size1 <<std::endl;
  //std::cout<<"DIM : "<< dim1 <<" SIZE : "<< size2 <<std::endl;
 //stoVolume->SetModifiedSinceRead(1);
 
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
 // GET BOUNDS OF THE ARRAY IN IJK
 int size[6];
 
 xyPt[0] = size1;
 xyPt[1] = 0;
 xyPt[2] = 0;
 xyPt[3] = 1;

 double ijkPt1[3];
 double ijkPt2[3];
 double ijkPt3[3];
 
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
 xyToijk->MultiplyPoint(xyPt,ijkPt1);
  
 xyPt[0] = 0;
 xyPt[1] = size2;
 xyPt[2] = 0;
 xyPt[3] = 1;
 
 xyToijk->MultiplyPoint(xyPt,ijkPt2);
 
 xyPt[0] = 0;
 xyPt[1] = 0;
 xyPt[2] = 0;
 xyPt[3] = 1;
 
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
 xyToijk->MultiplyPoint(xyPt,ijkPt3);
 
 
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
 size[0] = sqrt((ijkPt1[0]-ijkPt3[0])*(ijkPt1[0]-ijkPt3[0]));
 size[1] = sqrt((ijkPt2[0]-ijkPt3[0])*(ijkPt2[0]-ijkPt3[0]));
 size[2] = sqrt((ijkPt1[1]-ijkPt3[1])*(ijkPt1[1]-ijkPt3[1]));
 size[3] = sqrt((ijkPt2[1]-ijkPt3[1])*(ijkPt2[1]-ijkPt3[1]));
 size[4] = sqrt((ijkPt1[2]-ijkPt3[2])*(ijkPt1[2]-ijkPt3[2]));
 size[5] = sqrt((ijkPt2[2]-ijkPt3[2])*(ijkPt2[2]-ijkPt3[2]));
 

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
 /*std::cout<<"SIZE 2: "<< size[2] <<std::endl;
 std::cout<<"SIZE 2: "<< size[3] <<std::endl;
 std::cout<<"SIZE 3: "<< size[4] <<std::endl;
 std::cout<<"SIZE 3: "<< size[5] <<std::endl;*/
 
 int compt = 0;
 int pos[2];
 for(int i = 0; i < 6; i++){
 if (size[i] > 0)
 {
 size[compt] = size[i];
 if(i<2)
 pos[compt] = 0;
 if(i>1 && i<4)
 pos[compt] = 1;
 if(i>3)
 pos[compt] = 2;
 compt ++;
 }
 }

double populateXY[4];
double populateIJK[4];

populateXY[2] = 0;
populateXY[3] = 1;

//GET EVOLUTION TO POPULATE (POSITION INCREMENT OR DECREMENT)
double direction[4][2];
int start = begin[0]+begin[1];
for(int i = begin[0]; i < begin[0]+2; i++) {
for(int j = begin[1]; j < begin[1]+2; j++) {
populateXY[0] = i;
populateXY[1] = j;
xyToijk->MultiplyPoint(populateXY,populateIJK);
/*std::cout<<"TEST X: "<<populateIJK[pos[0]]<<std::endl;
std::cout<<"TEST Y: "<<populateIJK[pos[1]]<<std::endl;*/
direction[i+j-start][0] = populateIJK[pos[0]];
direction[i+j-start][1] = populateIJK[pos[1]];
}
}

/*
std::cout<<"DIRECTIONS"<<std::endl;
std::cout<< direction[0][0] << " " << direction[0][1] <<std::endl;
std::cout<< direction[1][0] <<" " <<  direction[1][1] <<std::endl;
std::cout<< direction[2][0] << " " << direction[2][1] <<std::endl;
std::cout<< direction[3][0] << " " << direction[3][1] <<std::endl;*/

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
double evolution[2];

if(direction[0][0] == direction [1][0]){
if(direction[0][0]-direction[2][0] < 0){
evolution[0] = 1;
}
else{
evolution[0] = 0;
}
}
else{
if(direction[0][0]-direction[1][0] < 0){
evolution[0] = 1;
}
else{
evolution[0] = 0;
}
}

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
if(direction[0][1] == direction [1][1]){
if(direction[0][1]-direction[2][1] < 0){
evolution[1] = 1;
}
else{
evolution[1] = 0;
}
}
else{
if(direction[0][1]-direction[1][1] < 0){
evolution[1] = 1;
}
else{
evolution[1] = 0;
}
}

 xyPt[0] = begin[0];
 xyPt[1] = begin[1];
 xyPt[2] = 0;
 xyPt[3] = 1;
 
 double originIJK[3];
 
 xyToijk->MultiplyPoint(xyPt,originIJK);
 
 /*int originIJK[3];
 originIJK[0] = originIJK1[0];
 originIJK[1] = originIJK1[1];
 originIJK[2] = originIJK1[2];*/
 
 
std::cout<<"ORIGIN IJK"<<std::endl;
std::cout<<"FIRST: "<< originIJK[0] <<std::endl;
std::cout<<"SECOND: "<< originIJK[1] <<std::endl;
std::cout<<"THIRD: "<< originIJK[2] <<std::endl;

std::cout<<"ORIGIN XY"<<std::endl;
std::cout<<"FIRST: "<< begin[0] <<std::endl;
std::cout<<"SECOND: "<< begin[1] <<std::endl;

std::cout<<"SIZE OF THE ARRAY TO BE PROCESSED XY"<<std::endl;
 std::cout<<"SIZE : "<< size1  <<std::endl;
 std::cout<<"SIZE : "<< size2  <<std::endl;

std::cout<<"SIZE OF THE ARRAY TO BE PROCESSED IJK"<<std::endl;
 std::cout<<"SIZE : "<< size[0]  <<std::endl;
 std::cout<<"SIZE : "<< size[1]  <<std::endl;

std::cout<<"POINTS MOVING"<<std::endl;
std::cout<<"FIRST: "<< pos[0] <<std::endl;
std::cout<<"SECOND: "<< pos[1] <<std::endl;

std::cout<<"DIRECTIONS"<<std::endl;
std::cout<<"FIRST: "<< evolution[0] <<std::endl;
std::cout<<"SECOND: "<< evolution[1] <<std::endl;

// POPULATE THE CORRESPONDING ARRAY AND MASK

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
    int wholeExtent[6];

    wholeExtent[0]=0;    wholeExtent[2]=0;  wholeExtent[4]=0;  wholeExtent[5]=0;  
  
    wholeExtent[1] = size[0]-1;  wholeExtent[3] = size[1]-1;

    this->STORAGE = vtkImageData::New();
    this->STORAGE->SetWholeExtent( wholeExtent );  
    this->STORAGE->SetNumberOfScalarComponents(1);  
    this->STORAGE->SetOrigin(0.0,0.0,0.0); 
    this->STORAGE->SetDimensions(size[0],size[1],1);    
    this->STORAGE->AllocateScalars();
  
    vtkDataArray* outStorage=this->STORAGE->GetPointData()->GetScalars();  
    
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
    this->MASK = vtkImageData::New();
    this->MASK->SetWholeExtent( wholeExtent );  
    this->MASK->SetNumberOfScalarComponents(1);  
    this->MASK->SetOrigin(0.0,0.0,0.0); 
    this->MASK->SetDimensions(size[0],size[1],1);    
    this->MASK->AllocateScalars();
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
    vtkDataArray* outMask=this->MASK->GetPointData()->GetScalars();

// ARRAY EXTRACTION

if(pos[0]==0 && pos[1] == 1){
if(evolution[0] == 0 && evolution[1] == 0){
for (int j=0;j<size[0];j++) {
for (int i=0;i<size[1];i++)     {
outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]-j,originIJK[1]-i,originIJK[2],0));
outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]-j,originIJK[1]-i,originIJK[2],0));
}}}
if(evolution[0] == 0 && evolution[1] == 1){
for (int j=0;j<size[0];j++) {
for (int i=0;i<size[1];i++)    { 
outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]-j,originIJK[1]+i,originIJK[2],0));
outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]-j,originIJK[1]+i,originIJK[2],0));
}}}
if(evolution[0] == 1 && evolution[1] == 0){
for (int j=0;j<size[0];j++) {
for (int i=0;i<size[1];i++)    { 
outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]+j,originIJK[1]-i,originIJK[2],0));
outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]+j,originIJK[1]-i,originIJK[2],0));
}}}
if(evolution[0] == 1 && evolution[1] == 1){
for (int j=0;j<size[0];j++) {
for (int i=0;i<size[1];i++) {    
outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]+j,originIJK[1]+i,originIJK[2],0));
outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]+j,originIJK[1]+i,originIJK[2],0));
}}}

}
if(pos[0]==1 && pos[1] == 2){
if(evolution[0] == 0 && evolution[1] == 0){
for (int j=0;j<size[0];j++) {
for (int i=0;i<size[1];i++)  {   
outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0],originIJK[1]-j,originIJK[2]-i,0));
outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0],originIJK[1]-j,originIJK[2]-i,0));
}}}
if(evolution[0] == 0 && evolution[1] == 1){
for (int j=0;j<size[0];j++) {
for (int i=0;i<size[1];i++) {    
outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0],originIJK[1]-j,originIJK[2]+i,0));
outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0],originIJK[1]-j,originIJK[2]+i,0));
}}}
if(evolution[0] == 1 && evolution[1] == 0){
for (int j=0;j<size[0];j++) {
for (int i=0;i<size[1];i++)    { 
outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0],originIJK[1]+j,originIJK[2]-i,0));
outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0],originIJK[1]+j,originIJK[2]-i,0));
}}}
if(evolution[0] == 1 && evolution[1] == 1){
for (int j=0;j<size[0];j++) {
for (int i=0;i<size[1];i++)    { 
outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0],originIJK[1]+j,originIJK[2]+i,0));
outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0],originIJK[1]+j,originIJK[2]+i,0));
}}}
}

if(pos[0]==0 && pos[1] == 2){
if(evolution[0] == 0 && evolution[1] == 0){
for (int j=0;j<size[0];j++) {
for (int i=0;i<size[1];i++)   {  
outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]-j,originIJK[1],originIJK[2]-i,0));
outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]-j,originIJK[1],originIJK[2]-i,0));
}}}
if(evolution[0] == 0 && evolution[1] == 1){
for (int j=0;j<size[0];j++) {
for (int i=0;i<size[1];i++)   {  
outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]-j,originIJK[1],originIJK[2]+i,0));
outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]-j,originIJK[1],originIJK[2]+i,0));
}}}
if(evolution[0] == 1 && evolution[1] == 0){
for (int j=0;j<size[0];j++) {
for (int i=0;i<size[1];i++)   {  
outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]+j,originIJK[1],originIJK[2]-i,0));
outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]+j,originIJK[1],originIJK[2]-i,0));
}}}
if(evolution[0] == 1 && evolution[1] == 1){
for (int j=0;j<size[0];j++) {
for (int i=0;i<size[1];i++)   {  
outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]+j,originIJK[1],originIJK[2]+i,0));
outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble(originIJK[0]+j,originIJK[1],originIJK[2]+i,0));
}}}
}

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
outVolume->SetAndObserveImageData(this->MASK);
outVolume->SetModifiedSinceRead(1);
// PROCESS THE SLICE


  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  typedef itk::Image< float,  2 >   InputImageType;
  typedef itk::Image< unsigned char,   2 >   MaskImageType;
  typedef itk::Image< float,  2 >   OutputImageType;
  typedef itk::Image< float, 2 >  InternalImageType; 
  
  typedef itk::ImageFileReader< InputImageType >  ReaderType;

  ReaderType::Pointer  reader = ReaderType::New();
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
// CREATION OF THE ITK IMAGES

// FOR THE SLICE TO BE PROCESSED
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  vtkImageCast* VtkCaster = vtkImageCast::New();
  VtkCaster->SetInput(this->STORAGE);

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  VtkCaster->SetOutputScalarTypeToFloat(); 
  VtkCaster->Modified();
  VtkCaster->UpdateWholeExtent();
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
    typedef itk::VTKImageToImageFilter< InternalImageType >
VTK2ITKConnectorFilterType;
        VTK2ITKConnectorFilterType::Pointer VTK2ITKconnector =
VTK2ITKConnectorFilterType::New();
        VTK2ITKconnector->SetInput( VtkCaster->GetOutput() );
        VTK2ITKconnector->Update();
        
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
// FOR THE MASK TO BE USED

  vtkImageCast* VtkCasterM = vtkImageCast::New();
  VtkCasterM->SetInput(this->MASK);

  VtkCasterM->SetOutputScalarTypeToFloat(); 
  VtkCasterM->Modified();
  VtkCasterM->UpdateWholeExtent();
  
    typedef itk::VTKImageToImageFilter< InternalImageType > VTK2ITKConnectorFilterTypeM;
        VTK2ITKConnectorFilterTypeM::Pointer VTK2ITKconnectorM = VTK2ITKConnectorFilterTypeM::New();
        VTK2ITKconnectorM->SetInput( VtkCasterM->GetOutput() );
        VTK2ITKconnectorM->Update();

//PROCESSING
     
  typedef itk::ShrinkImageFilter<InputImageType, InputImageType> ShrinkerType;
  ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( VTK2ITKconnector->GetOutput() );
  shrinker->SetShrinkFactors( 1 );                                     
                                       
    
  typedef itk::BinaryThresholdImageFilter<
               InputImageType, MaskImageType>  mFilterType;
               
  mFilterType::Pointer mfilter = mFilterType::New();  

  mfilter->SetInput( VTK2ITKconnectorM->GetOutput() );
    
  mfilter->SetLowerThreshold(1);
  mfilter->SetOutsideValue(0);
  mfilter->SetInsideValue(1);  

   MaskImageType::Pointer maskImage = NULL;
  
  maskImage = mfilter->GetOutput();

  typedef itk::ShrinkImageFilter<MaskImageType, MaskImageType> MaskShrinkerType;
  MaskShrinkerType::Pointer maskshrinker = MaskShrinkerType::New();

  maskshrinker->SetInput( maskImage );
  maskshrinker->SetShrinkFactors( 1 );
  
  shrinker->SetShrinkFactors( this->MRIBiasFieldCorrectionNode->GetShrink());
  maskshrinker->SetShrinkFactors(this->MRIBiasFieldCorrectionNode->GetShrink());

  
  shrinker->Update();

  maskshrinker->Update();
  

  typedef itk::N3MRIBiasFieldCorrectionImageFilter<InputImageType, MaskImageType,
    InputImageType> CorrecterType;
  CorrecterType::Pointer correcter = CorrecterType::New();
  correcter->SetInput( shrinker->GetOutput() );
  correcter->SetMaskImage( maskshrinker->GetOutput() );
  
    correcter->SetMaximumNumberOfIterations( this->MRIBiasFieldCorrectionNode->GetMax() );

    correcter->SetNumberOfFittingLevels(this->MRIBiasFieldCorrectionNode->GetNum());

    correcter->SetWeinerFilterNoise(this->MRIBiasFieldCorrectionNode->GetWien());
    
    correcter->SetBiasFieldFullWidthAtHalfMaximum(this->MRIBiasFieldCorrectionNode->GetField());
     
    correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode->GetCon());

    correcter->Update();

    correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode->GetCon());

    typedef itk::BSplineControlPointImageFilter<
    CorrecterType::BiasFieldControlPointLatticeType, 
    CorrecterType::ScalarImageType> BSplinerType;
  BSplinerType::Pointer bspliner = BSplinerType::New();
  bspliner->SetInput( correcter->GetBiasFieldControlPointLattice() );
  bspliner->SetSplineOrder( correcter->GetSplineOrder() );
  bspliner->SetSize(
    VTK2ITKconnector->GetOutput()->GetLargestPossibleRegion().GetSize() );
  bspliner->SetOrigin( VTK2ITKconnector->GetOutput()->GetOrigin() );
  bspliner->SetDirection( VTK2ITKconnector->GetOutput()->GetDirection() );
  bspliner->SetSpacing( VTK2ITKconnector->GetOutput()->GetSpacing() );
  bspliner->Update();

  InputImageType::Pointer logField = InputImageType::New();
  logField->SetOrigin( bspliner->GetOutput()->GetOrigin() );
  logField->SetSpacing( bspliner->GetOutput()->GetSpacing() );
  logField->SetRegions(
    bspliner->GetOutput()->GetLargestPossibleRegion().GetSize() );
  logField->SetDirection( bspliner->GetOutput()->GetDirection() );
  logField->Allocate();

  itk::ImageRegionIterator<CorrecterType::ScalarImageType> ItB(
    bspliner->GetOutput(),
    bspliner->GetOutput()->GetLargestPossibleRegion() );
  itk::ImageRegionIterator<InputImageType> ItF( logField,
    logField->GetLargestPossibleRegion() );
  for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
    {
    ItF.Set( ItB.Get()[0] );
    }

  typedef itk::ExpImageFilter<InputImageType, InputImageType> ExpFilterType;
  ExpFilterType::Pointer expFilter = ExpFilterType::New();
  expFilter->SetInput( logField );
  expFilter->Update();

  typedef itk::DivideImageFilter<InputImageType, InputImageType, InputImageType> DividerType;
  DividerType::Pointer divider = DividerType::New();
  divider->SetInput1( VTK2ITKconnector->GetOutput() );
  divider->SetInput2( expFilter->GetOutput() );
  divider->Update();  
  
 typedef itk::ImageToVTKImageFilter< InternalImageType >
ITK2VTKConnectorFilterType;
        ITK2VTKConnectorFilterType::Pointer ITK2VTKconnector =
ITK2VTKConnectorFilterType::New();
        ITK2VTKconnector->GetExporter()->SetInput(divider->GetOutput()); 
        ITK2VTKconnector->GetImporter()->Update(); 
        
  std::cout<<"CORRECTION DONE"<<std::endl;
  
  vtkImageData* image = vtkImageData::New(); 
  image->SetWholeExtent(wholeExtent);
  image->SetNumberOfScalarComponents(1);
  image->SetOrigin(0.0,0.0,0.0); 
  image->SetDimensions(size[0],size[1],1);  
  image->AllocateScalars();
  image->DeepCopy( ITK2VTKconnector->GetImporter()->GetOutput());
  
  vtkDataArray* outStorage2=image->GetPointData()->GetScalars();
// PUT ARRAY BACK IN THE VOLUME

if(pos[0]==0 && pos[1] == 1){
if(evolution[0] == 0 && evolution[1] == 0){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
//outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt3[0]-j,ijkPt3[1]-i,ijkPt3[2],0));
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]-j-1,originIJK[1]-i,originIJK[2],0,outStorage2->GetComponent(i*(size[0])+j,1));
}
if(evolution[0] == 0 && evolution[1] == 1){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
//outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt3[0]-j,ijkPt3[1]+i,ijkPt3[2],0));
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]-j-1,originIJK[1]+i,originIJK[2],0,outStorage2->GetComponent(i*(size[0])+j,1));
}
if(evolution[0] == 1 && evolution[1] == 0){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
//outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt3[0]+j,ijkPt3[1]-i,ijkPt3[2],0));
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]+j+1,originIJK[1]-i,originIJK[2],0,outStorage2->GetComponent(i*(size[0])+j,1));
}
if(evolution[0] == 1 && evolution[1] == 1){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
//outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt3[0]+j,ijkPt3[1]+i,ijkPt3[2],0));
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]+j+1,originIJK[1]+i,originIJK[2],0,outStorage2->GetComponent(i*(size[0])+j,1));
}

}

if(pos[0]==1 && pos[1] == 2){
if(evolution[0] == 0 && evolution[1] == 0){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
//outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt3[0]-j,ijkPt3[1]-i,ijkPt3[2],0));
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0],originIJK[1]-j-1,originIJK[2]-i,0,outStorage2->GetComponent(i*(size[0])+j,1));
}
if(evolution[0] == 0 && evolution[1] == 1){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
//outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt3[0]-j,ijkPt3[1]+i,ijkPt3[2],0));
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0],originIJK[1]-j-1,originIJK[2]+i,0,outStorage2->GetComponent(i*(size[0])+j,1));
}
if(evolution[0] == 1 && evolution[1] == 0){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
//outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt3[0]+j,ijkPt3[1]-i,ijkPt3[2],0));
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0],originIJK[1]+j,originIJK[2]-i,0,outStorage2->GetComponent(i*(size[0])+j,1));
}
if(evolution[0] == 1 && evolution[1] == 1){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
//outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt3[0]+j,ijkPt3[1]+i,ijkPt3[2],0));
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0],originIJK[1]+j+1,originIJK[2]+i,0,outStorage2->GetComponent(i*(size[0])+j,1));
}

}

if(pos[0]==0 && pos[1] == 2){
if(evolution[0] == 0 && evolution[1] == 0){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
//outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt3[0]-j,ijkPt3[1]-i,ijkPt3[2],0));
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]-j,originIJK[1],originIJK[2]-i,0,outStorage2->GetComponent(i*(size[0])+j,1));
}
if(evolution[0] == 0 && evolution[1] == 1){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
//outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt3[0]-j,ijkPt3[1]+i,ijkPt3[2],0));
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]-j,originIJK[1],originIJK[2]+i,0,outStorage2->GetComponent(i*(size[0])+j,1));
}
if(evolution[0] == 1 && evolution[1] == 0){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
//outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt3[0]+j,ijkPt3[1]-i,ijkPt3[2],0));
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]+j,originIJK[1],originIJK[2]-i,0,outStorage2->GetComponent(i*(size[0])+j,1));
}
if(evolution[0] == 1 && evolution[1] == 1){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
//outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt3[0]+j,ijkPt3[1]+i,ijkPt3[2],0));
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]+j+1,originIJK[1],originIJK[2]+i,0,outStorage2->GetComponent(i*(size[0])+j,1));
}

}


  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");

stoVolume->SetAndObserveImageData(this->PREVIEW);
stoVolume->SetModifiedSinceRead(1);

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
}
#endif

void vtkMRIBiasFieldCorrectionLogic::ApplyPreview(double red, double yellow, double green,vtkImageData* image)
{
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
 // check if MRML node is present 
  if (this->MRIBiasFieldCorrectionNode == NULL)
    {
    vtkErrorMacro("No input MRIBiasFieldCorrectionNode found");
    return;
    }
  
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));
  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }
  
  // find output volume
  vtkMRMLScalarVolumeNode *stoVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetStorageVolumeRef()));
  if (stoVolume == NULL)
    {
    vtkErrorMacro("No storage volume found with id= " << this->MRIBiasFieldCorrectionNode->GetStorageVolumeRef());
    return;
    }



  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (stoVolume->GetName());
  std::string id (stoVolume->GetID());

  stoVolume->CopyOrientation(inVolume);
  stoVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  stoVolume->SetName(name.c_str());
  
  this->STORAGE = vtkImageData::New(); 
  STORAGE->DeepCopy(inVolume->GetImageData());

  stoVolume->SetAndObserveImageData(this->STORAGE);

/*
  vtkSlicerApplicationLogic *applicationLogic = this->Logic->GetApplicationLogic();
  applicationLogic->GetSelectionNode()->SetReferenceActiveVolumeID(this->MRIBiasFieldCorrectionNode->GetOutputVolumeRef());
  applicationLogic->PropagateVolumeSelection();
    */

  // COMPUTE RAS COORDINATES
  
  double rasPt[4] = {red, yellow, green, 1};
  
  std::cout<<"IN LOGIC RAS"<<std::endl;
  std::cout<<"RED: "<< rasPt[0] <<std::endl;
  std::cout<<"YEL: "<< rasPt[1] <<std::endl;
  std::cout<<"GRE: "<< rasPt[2] <<std::endl;

  // COMPUTE IJK COORDINATES
  
  double ijkPt[4];
  vtkMatrix4x4* rasToijk = vtkMatrix4x4::New();
  inVolume->GetRASToIJKMatrix(rasToijk);
  rasToijk->MultiplyPoint(rasPt,ijkPt);

  std::cout<<"IN LOGIC IJK"<<std::endl;
  std::cout<<"RED: "<< ijkPt[0] <<std::endl;
  std::cout<<"YEL: "<< ijkPt[1] <<std::endl;
  std::cout<<"GRE: "<< ijkPt[2] <<std::endl;

  // SLICES PROCESSING

    int wholeExtent[6];
    int* dims = inVolume->GetImageData()->GetDimensions();
    int* dim  = inVolume->GetImageData()->GetDimensions();
    double sp[3];   inVolume->GetImageData()->GetSpacing(sp);

    vtkDataArray* inp=this->STORAGE->GetPointData()->GetScalars();

    int dty=inp->GetDataType();

  vtkMatrix4x4 *mat = vtkMatrix4x4::New();
    inVolume->GetIJKToRASMatrix(mat);

  //std::cout<<"SCAN ORDER"<<std::endl;
  //std::cout<<vtkMRMLVolumeNode::ComputeScanOrderFromIJKToRAS(mat)
  //<<std::endl;
  //this->ScanOrderEntry->GetWidget()->SetValue(
  //vtkMRMLVolumeNode::ComputeScanOrderFromIJKToRAS(mat));
  //inVolume->GetRASToIJKMatrix(rasToijk);

  vtkDataArray* sto= stoVolume->GetImageData()->GetPointData()->
    GetScalars();

  //// CORONAL EXTRACTION 2D
   /*
    wholeExtent[0]=0;    wholeExtent[2]=0;  wholeExtent[4]=0;  wholeExtent[5]=0;  
  
    wholeExtent[1] = dims[0]-1;  wholeExtent[3] = dims[1]-1;

    this->CORONAL = vtkImageData::New();
    this->CORONAL->SetWholeExtent( wholeExtent );  
    this->CORONAL->SetScalarType(inVolume->GetImageData()->GetScalarType());  
    this->CORONAL->SetNumberOfScalarComponents(1); 
  
    //this->CORONAL->SetSpacing(sp);  
    this->CORONAL->SetScalarType(dty);  
    this->CORONAL->SetNumberOfScalarComponents(1);  
    this->CORONAL->SetOrigin(0.0,0.0,0.0); 


    this->CORONAL->SetDimensions(dim[0],dim[1],1);  
    this->CORONAL->SetWholeExtent(0,dim[0]-1,0,dim[1]-1,0,0);  
    this->CORONAL->AllocateScalars();
  
    vtkDataArray* outCoronal=this->CORONAL->GetPointData()->GetScalars();  


  
    offset=dim[0]*dim[1]*round(ijkPt[2]);
  
    for (int j=0;j<dim[1];j++)    
for (int i=0;i<dim[0];i++)      
outCoronal->SetComponent(i+dim[0]*j,0,inp->GetComponent(i+dim[0]*j+offset,1));//-1


    // VOLUME RECONSTRUCTION
 
    for (int j=0;j<dim[1];j++)    
for (int i=0;i<dim[0];i++)      
sto->SetComponent(i+dim[0]*j+offset,0,outCoronal->GetComponent(i+dim[0]*j,1));//+1
*/

    // VOLUME RECONSTRUCTION


     //// SAGITTAL EXTRACTION 2D
       
    wholeExtent[0]=0;    wholeExtent[2]=0;  wholeExtent[4]=0;  wholeExtent[5]=0;  
  
    wholeExtent[1] = dims[2]-1;  wholeExtent[3] = dims[1]-1;

    this->SAGITTAL = vtkImageData::New();
    this->SAGITTAL->SetWholeExtent( wholeExtent );  
    this->SAGITTAL->SetSpacing(sp);  
    this->SAGITTAL->SetScalarType(dty);  
    this->SAGITTAL->SetNumberOfScalarComponents(1);  
    this->SAGITTAL->SetOrigin(0.0,0.0,0.0); 


    this->SAGITTAL->SetDimensions(dim[2],dim[1],1);    
    this->SAGITTAL->AllocateScalars();
  
    vtkDataArray* outSagittal=this->SAGITTAL->GetPointData()->GetScalars();  


    for (int j=0;j<dim[2];j++) 
for (int i=0;i<dim[1];i++)     
outSagittal->SetComponent(i*(dim[2])+j,0,-50);//inp->GetComponent(dim[0]-ijkPt[1] -2 +dim[0]*dim[1]*j + i*dim[0],1));


    // VOLUME RECONSTRUCTION
 
    for (int j=0;j<dim[2];j++)    
for (int i=0;i<dim[1];i++)      
sto->SetComponent(ijkPt[1] +dim[0]*dim[1]*j + i*dim[0],0,outSagittal->GetComponent(i*dim[2]+j,1));//dim[0]-ijkPt[1]


//// TRANSVERSAL EXTRACTION
    /* 
    wholeExtent[0]=0;    wholeExtent[2]=0;  wholeExtent[4]=0;  wholeExtent[5]=dims[2]-1;  
  
    wholeExtent[1] = dims[0]-1;  wholeExtent[3] = 0;

    this->TRANSVERSAL = vtkImageData::New();
    this->TRANSVERSAL->SetWholeExtent( wholeExtent );  
    this->TRANSVERSAL->SetSpacing(sp);  
    this->TRANSVERSAL->SetScalarType(dty);  
    this->TRANSVERSAL->SetNumberOfScalarComponents(1);  
    this->TRANSVERSAL->SetOrigin(0.0,0.0,0.0); 


    this->TRANSVERSAL->SetDimensions(dim[0],1,dim[2]);    
    this->TRANSVERSAL->AllocateScalars();
  
    vtkDataArray* outTransversal=this->TRANSVERSAL->GetPointData()->GetScalars();  

  
    for (int j=0;j<dim[2];j++) 
for (int i=0;i<dim[0];i++)     
outTransversal->SetComponent(i+j*dim[0],0,inp->GetComponent(dim[0]*(dim[1] - ijkPt[0]-1) + i + dim[0]*dim[1]*j,1));


    // VOLUME RECONSTRUCTION
 
    for (int j=0;j<dim[2];j++)    
for (int i=0;i<dim[1];i++)      
sto->SetComponent(dim[0]*(dim[1] - ijkPt[0]) + i + dim[0]*dim[1]*j,0,outTransversal->GetComponent(i+j*dim[1],1));

*/
//// TRANSVERSAL EXTRACTION 2D
  /*  
    wholeExtent[0]=0;    wholeExtent[2]=0;  wholeExtent[4]=0;  wholeExtent[5]=0;  
  
    wholeExtent[1] = dims[0]-1;  wholeExtent[3] = dims[2]-1;

    this->TRANSVERSAL = vtkImageData::New();
    this->TRANSVERSAL->SetWholeExtent( wholeExtent );  
    this->TRANSVERSAL->SetSpacing(sp);  
    this->TRANSVERSAL->SetScalarType(dty);  
    this->TRANSVERSAL->SetNumberOfScalarComponents(1);  
    this->TRANSVERSAL->SetOrigin(0.0,0.0,0.0); 


    this->TRANSVERSAL->SetDimensions(dim[0],dim[2],1);    
    this->TRANSVERSAL->AllocateScalars();
  
    vtkDataArray* outTransversal=this->TRANSVERSAL->GetPointData()->GetScalars();  

  
    for (int j=0;j<dim[2];j++) 
for (int i=0;i<dim[0];i++)     
outTransversal->SetComponent(i+j*dim[0],0,0);//inp->GetComponent(dim[0]*(dim[1] - ijkPt[0]-1) + i + dim[0]*dim[1]*j,1));


    // VOLUME RECONSTRUCTION
 
    for (int j=0;j<dim[2];j++)    
for (int i=0;i<dim[0];i++)      
sto->SetComponent(dim[0]*(dim[1] - ijkPt[0]-1) + i + dim[0]*dim[1]*j+2,0,outTransversal->GetComponent(i+j*dim[0],1));

*/
//stoVolume->SetAndObserveImageData(image);
//stoVolume->SetModifiedSinceRead(1); 

  
  typedef itk::Image< float,  2 >   InputImageType;
  typedef itk::Image< unsigned char,   2 >   MaskImageType;
  typedef itk::Image< float,  2 >   OutputImageType;
  typedef itk::Image< float, 2 >  InternalImageType; 
  
  std::cout<<"2d"<< std::endl;
  
  typedef itk::ImageFileReader< InputImageType >  ReaderType;

  ReaderType::Pointer  reader = ReaderType::New();

  vtkImageCast* VtkCaster = vtkImageCast::New();
  VtkCaster->SetInput(image);

  VtkCaster->SetOutputScalarTypeToFloat(); 
  VtkCaster->Modified();
  VtkCaster->UpdateWholeExtent();
  
    typedef itk::VTKImageToImageFilter< InternalImageType >
VTK2ITKConnectorFilterType;
        VTK2ITKConnectorFilterType::Pointer VTK2ITKconnector =
VTK2ITKConnectorFilterType::New();
        VTK2ITKconnector->SetInput( VtkCaster->GetOutput() );
        VTK2ITKconnector->Update();
                              
  typedef itk::ShrinkImageFilter<InputImageType, InputImageType> ShrinkerType;
  ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( VTK2ITKconnector->GetOutput() );
  shrinker->SetShrinkFactors( 1 );                                     
                                       
    
  typedef itk::BinaryThresholdImageFilter<
               InputImageType, MaskImageType>  mFilterType;
               
  mFilterType::Pointer mfilter = mFilterType::New();  

  mfilter->SetInput( VTK2ITKconnector->GetOutput() );
  
  mfilter->SetLowerThreshold(1);
  mfilter->SetOutsideValue(0);
  mfilter->SetInsideValue(1);  

  MaskImageType::Pointer maskImage = NULL;
  
  maskImage = mfilter->GetOutput();

  typedef itk::ShrinkImageFilter<MaskImageType, MaskImageType> MaskShrinkerType;
  MaskShrinkerType::Pointer maskshrinker = MaskShrinkerType::New();

  maskshrinker->SetInput( maskImage );
  maskshrinker->SetShrinkFactors( 1 );

   std::cout<<"shrink factor: "<<this->MRIBiasFieldCorrectionNode->GetShrink()<< std::endl;
  
  shrinker->SetShrinkFactors( this->MRIBiasFieldCorrectionNode->GetShrink());
  maskshrinker->SetShrinkFactors(this->MRIBiasFieldCorrectionNode->GetShrink());

  
  shrinker->Update();

  maskshrinker->Update();
  

  typedef itk::N3MRIBiasFieldCorrectionImageFilter<InputImageType, MaskImageType,
    InputImageType> CorrecterType;
  CorrecterType::Pointer correcter = CorrecterType::New();
  correcter->SetInput( shrinker->GetOutput() );
  correcter->SetMaskImage( maskshrinker->GetOutput() );
  
  std::cout<<"number of iteration: "<<this->MRIBiasFieldCorrectionNode->GetMax()<< std::endl;
    correcter->SetMaximumNumberOfIterations( this->MRIBiasFieldCorrectionNode->GetMax() );

  std::cout<<"number of fitting level: "<<this->MRIBiasFieldCorrectionNode->GetNum()<< std::endl;
    correcter->SetNumberOfFittingLevels(this->MRIBiasFieldCorrectionNode->GetNum());
    
  std::cout<<"Wiener Filter Noise: "<<this->MRIBiasFieldCorrectionNode->GetWien()<< std::endl;
    correcter->SetWeinerFilterNoise(this->MRIBiasFieldCorrectionNode->GetWien());
    
  std::cout<<"Bias Field Full..: "<<this->MRIBiasFieldCorrectionNode->GetField()<< std::endl;
    correcter->SetBiasFieldFullWidthAtHalfMaximum(this->MRIBiasFieldCorrectionNode->GetField());
    
   std::cout<<"Convergence Threshold: "<<this->MRIBiasFieldCorrectionNode->GetCon()<< std::endl;   
    correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode->GetCon());

    correcter->Update();

 std::cout<<"UPDATE: "<<this->MRIBiasFieldCorrectionNode->GetCon()<< std::endl;   
    correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode->GetCon());

    typedef itk::BSplineControlPointImageFilter<
    CorrecterType::BiasFieldControlPointLatticeType, 
    CorrecterType::ScalarImageType> BSplinerType;
  BSplinerType::Pointer bspliner = BSplinerType::New();
  bspliner->SetInput( correcter->GetBiasFieldControlPointLattice() );
  bspliner->SetSplineOrder( correcter->GetSplineOrder() );
  bspliner->SetSize(
    VTK2ITKconnector->GetOutput()->GetLargestPossibleRegion().GetSize() );
  bspliner->SetOrigin( VTK2ITKconnector->GetOutput()->GetOrigin() );
  bspliner->SetDirection( VTK2ITKconnector->GetOutput()->GetDirection() );
  bspliner->SetSpacing( VTK2ITKconnector->GetOutput()->GetSpacing() );
  bspliner->Update();

  InputImageType::Pointer logField = InputImageType::New();
  logField->SetOrigin( bspliner->GetOutput()->GetOrigin() );
  logField->SetSpacing( bspliner->GetOutput()->GetSpacing() );
  logField->SetRegions(
    bspliner->GetOutput()->GetLargestPossibleRegion().GetSize() );
  logField->SetDirection( bspliner->GetOutput()->GetDirection() );
  logField->Allocate();

  itk::ImageRegionIterator<CorrecterType::ScalarImageType> ItB(
    bspliner->GetOutput(),
    bspliner->GetOutput()->GetLargestPossibleRegion() );
  itk::ImageRegionIterator<InputImageType> ItF( logField,
    logField->GetLargestPossibleRegion() );
  for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
    {
    ItF.Set( ItB.Get()[0] );
    }

  typedef itk::ExpImageFilter<InputImageType, InputImageType> ExpFilterType;
  ExpFilterType::Pointer expFilter = ExpFilterType::New();
  expFilter->SetInput( logField );
  expFilter->Update();

  typedef itk::DivideImageFilter<InputImageType, InputImageType, InputImageType> DividerType;
  DividerType::Pointer divider = DividerType::New();
  divider->SetInput1( VTK2ITKconnector->GetOutput() );
  divider->SetInput2( expFilter->GetOutput() );
  divider->Update();  
  
 typedef itk::ImageToVTKImageFilter< InternalImageType >
ITK2VTKConnectorFilterType;
        ITK2VTKConnectorFilterType::Pointer ITK2VTKconnector =
ITK2VTKConnectorFilterType::New();
        ITK2VTKconnector->GetExporter()->SetInput(divider->GetOutput()); 
        ITK2VTKconnector->GetImporter()->Update(); 
  
  vtkImageData* image2 = vtkImageData::New(); 
  image2->DeepCopy( ITK2VTKconnector->GetImporter()->GetOutput());
  /*this->CORONAL->UpdateInformation();
  
  for (int j=0;j<dim[1];j++)    
for (int i=0;i<dim[0];i++)      
sto->SetComponent(i+dim[0]*j+offset+1,0,outCoronal->GetComponent(i+dim[0]*j,1));*/
  
  stoVolume->SetAndObserveImageData(image2);
  //outVolume->SetOrigin(-originOutvolume[0],-originOutvolume[1],0);
  
  stoVolume->SetModifiedSinceRead(1); 

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
}

void vtkMRIBiasFieldCorrectionLogic::Preview()
{
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
 // check if MRML node is present 
  if (this->MRIBiasFieldCorrectionNode == NULL)
    {
    vtkErrorMacro("No input MRIBiasFieldCorrectionNode found");
    return;
    }
  
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));
  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }
  
  // find output volume
  vtkMRMLScalarVolumeNode *stoVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetStorageVolumeRef()));
  if (stoVolume == NULL)
    {
    vtkErrorMacro("No storage volume found with id= " << this->MRIBiasFieldCorrectionNode->GetStorageVolumeRef());
    return;
    }



  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (stoVolume->GetName());
  std::string id (stoVolume->GetID());

  stoVolume->CopyOrientation(inVolume);
  stoVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  stoVolume->SetName(name.c_str());

  // copy RASToIJK matrix, and other attributes from input to output
  /*
  vtkMatrix4x4 *rasToIjk = vtkMatrix4x4::New();
  vtkMatrix4x4 *ijkToRas = vtkMatrix4x4::New();
  inVolume->GetRASToIJKMatrix(rasToIjk);
  vtkMatrix4x4::Invert(rasToIjk, ijkToRas);
  ijkToRas->Transpose();
  */
  //stoVolume->SetName(name.c_str());
  
  stoVolume->SetAndObserveImageData(inVolume->GetImageData());
 /*
 double maxmin[2];
 inVolume->GetImageData()->GetScalarRange(maxmin);

 this->ImageThreshold=vtkImageThreshold::New(); 
 this->ImageThreshold->SetInput(this->CurrentSlide);
 this->ImageThreshold->ThresholdByUpper(this->MRIBiasFieldCorrectionNode->GetConductance()); 
 this->ImageThreshold->SetInValue(maxmin[1]); 
 this->ImageThreshold->SetOutValue(maxmin[0]); 
 this->ImageThreshold->Update();
 */
   // set ouput of the filter to VolumeNode's ImageData
  // TODO FIX the bug of the image is deallocated unless we do DeepCopy
  //vtkImageData* image = vtkImageData::New(); 
  //image->DeepCopy( this->ImageThreshold->GetOutput());
  //image->UpdateInformation();
  
  
  //image->Delete();*/
  
   // outVolume->SetAndObserveImageData(image);
      /*  rasToIJK.Invert()
        ijkToRAS = rasToIJK
        outVolume.SetIJKToRASMatrix(rasToIJK)
        origin = ijkToRAS.MultiplyPoint(lowerIJK[0],lowerIJK[1],lowerIJK[2],1.0)*/
        //outVolume->SetOrigin(-originOutvolume[0],-originOutvolume[1],0);
  
  stoVolume->SetModifiedSinceRead(1); 
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
}

void vtkMRIBiasFieldCorrectionLogic::Cut()
{
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  // check if MRML node is present 
  if (this->MRIBiasFieldCorrectionNode == NULL)
    {
    vtkErrorMacro("No input GradientAnisotropicDiffusionFilterNode found");
    return;
    }
  
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));
  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  // Calculate the center of the volume
  int extent[6];
  double spacing[3];
  double origin[3];
  inVolume->GetImageData()->GetWholeExtent(extent);
  inVolume->GetImageData()->GetSpacing(spacing);
  inVolume->GetImageData()->GetOrigin(origin);

  double center[4];
  center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
  center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
  center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);
  center[4] = 1;

  // Matrices for axial, coronal, sagittal, oblique view orientations

  // Extract a slice in the desired orientation

  int wholeExtent[6], dims[3];

  inVolume->GetImageData()->GetDimensions(dims);

  wholeExtent[0]=0;
  wholeExtent[2]=0;
  wholeExtent[4]=0;
  wholeExtent[5]=0;

  wholeExtent[1] = dims[0]-1;
  wholeExtent[3] = dims[1]-1;

  this->CurrentSlide = vtkImageData::New();
  this->CurrentSlide->SetWholeExtent( wholeExtent );
  this->CurrentSlide->SetScalarType(inVolume->GetImageData()->
      GetScalarType());
  this->CurrentSlide->SetNumberOfScalarComponents(1);

  int dim[3];
  inVolume->GetImageData()->GetDimensions(dim);
  double ori[3];  inVolume->GetImageData()->GetOrigin(ori);
  double sp[3];   inVolume->GetImageData()->GetSpacing(sp);

  vtkDataArray* inp=inVolume->GetImageData()->GetPointData()->
    GetScalars();

  int dty=inp->GetDataType();

  this->CurrentSlide->SetSpacing(sp);
  this->CurrentSlide->SetScalarType(dty);
  this->CurrentSlide->SetNumberOfScalarComponents(1);
  this->CurrentSlide->SetOrigin(0.0,0.0,0.0);

  this->CurrentSlide->SetDimensions(dim[0],dim[1],1);
  this->CurrentSlide->SetWholeExtent(0,dim[0]-1,0,dim[1]-1,0,0);
  this->CurrentSlide->AllocateScalars();

  vtkDataArray* out=this->CurrentSlide->GetPointData()->GetScalars();

  originOutvolume[0] = dim[0]/2;
  originOutvolume[1] = dim[1]/2;
  originOutvolume[2] = dim[2]/2;

  //image 1

  this->Image1 = vtkImageData::New();
  this->Image1->DeepCopy( this->CurrentSlide);


  //image 2

  int offset2=dim[0]*dim[1]*round(dim[2]/4);//this->SliceNo;

  for (int j=0;j<dim[1];j++)
    {
    for (int i=0;i<dim[0];i++)
      {
      out->SetComponent(i+((dim[1]-1)*dim[0])-j*dim[0],0,inp->GetComponent(i+dim[0]*j+offset2,1));
      }
    }

  this->Image2 = vtkImageData::New();
  this->Image2->DeepCopy( this->CurrentSlide);

//image 3

int offset3=dim[0]*dim[1]*round(dim[2]/2);//this->SliceNo;
 std::cout<<"offset3: "<<offset3<<std::endl;
for (int j=0;j<dim[1];j++)
for (int i=0;i<dim[0];i++)
out->SetComponent(i+((dim[1]-1)*dim[0])-j*dim[0],0,inp->GetComponent(i+dim[0]*j+offset3,1));


this->Image3 = vtkImageData::New();
this->Image3->DeepCopy( this->CurrentSlide);

//image 4

int offset4=dim[0]*dim[1]*round(dim[2]*3/4);//this->SliceNo; 
 std::cout<<"offset4: "<<offset4<<std::endl;
for (int j=0;j<dim[1];j++)    
for (int i=0;i<dim[0];i++)      
out->SetComponent(i+((dim[1]-1)*dim[0])-j*dim[0],0,inp->GetComponent(i+dim[0]*j+offset4,1));


this->Image4 = vtkImageData::New();
this->Image4->DeepCopy( this->CurrentSlide);

//image 5

int offset5=dim[0]*dim[1]*(dim[2]-1);//this->SliceNo; 
 
for (int j=0;j<dim[1];j++)    
for (int i=0;i<dim[0];i++)      
out->SetComponent(i+((dim[1]-1)*dim[0])-j*dim[0],0,inp->GetComponent(i+dim[0]*j+offset5,1));


this->Image5 = vtkImageData::New();
this->Image5->DeepCopy( this->CurrentSlide);

  //vtkMRMLMRIBiasFieldCorrectionNode* n = this->GetMRIBiasFieldCorrectionNode();
  /*
  std::cout << "Storage node ID: " << this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetStorageVolumeRef()) << std::endl;
  
  vtkMRMLVolumeNode* modelNode = vtkMRMLVolumeNode::New();
  
  modelNode->SetScene(this->GetMRMLScene());
  
  this->GetMRMLScene()->AddNode(modelNode);
   */
  //n->SetStorageVolumeRef(modelNode->GetID());
  
  //std::cout << "Storage node ID: " << this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetStorageVolumeRef()) << std::endl;
  
  //subArray = inArray()
  //inVolume->GetImageData()->GetPointData()->CopyAllOn();
  /*
  vtkDataArray * inputArray = vtkDataArray::New();
  inVolume->GetImageData()->GetPointData()->AddArray(inputArray);//.ToArray();
  
  
  vtkDataArray * subArray = vtkDataArray::New();
  
  # get an array of the input volume, extract the sub
        a = inVolume.inputArray.ToArray()
        sub = a[lowerIJK[2]:upperIJK[2], lowerIJK[1]:upperIJK[1], lowerIJK[0]:upperIJK[0]]
        
        # set up output node
        outImage = slicer.vtkImageData()
        outImage.SetDimensions(sub.shape[2],sub.shape[1],sub.shape[0])
        outImage.AllocateScalars()
        outImage.ToArray()[:] = sub[:]
        outVolume.SetAndObserveImageData(outImage)
        rasToIJK.Invert()
        ijkToRAS = rasToIJK
        outVolume.SetIJKToRASMatrix(rasToIJK)
        origin = ijkToRAS.MultiplyPoint(lowerIJK[0],lowerIJK[1],lowerIJK[2],1.0)
        outVolume.SetOrigin(origin[0], origin[1], origin[2])
        outVolume.ModifiedSinceReadOn()

  */
  /*
  //Slice 1
  
  this->ImageReslice1 = vtkImageReslice::New();
 // vtkImageReslice *reslice = vtkImageReslice::New();
  this->ImageReslice1->SetInput(inVolume->GetImageData());
  this->ImageReslice1->SetInterpolationModeToLinear();
  //this->ImageReslice1->SetOutputExtent(bounds[0]+1,bounds[1]-1,bounds[2]+1, bounds[3]-1,round(bounds[5]/2), round(bounds[5]/2));
  
  this->ImageReslice1->SetResliceAxesDirectionCosines(axialElements); 
  
  this->ImageReslice1->SetOutputDimensionality(2);
  
  this->ImageReslice1->SetResliceAxesOrigin(center[0],center[1],center[2]-20);
 
  this->ImageReslice1->GetOutput()->UpdateInformation();
  
  this->ImageReslice1->Update();
    
    
  //Slice 2
  
    this->ImageReslice2 = vtkImageReslice::New();
 // vtkImageReslice *reslice = vtkImageReslice::New();
  this->ImageReslice2->SetInput(inVolume->GetImageData());
  this->ImageReslice2->SetInterpolationModeToLinear();
  //this->ImageReslice1->SetOutputExtent(bounds[0]+1,bounds[1]-1,bounds[2]+1, bounds[3]-1,round(bounds[5]/2), round(bounds[5]/2));
  
  this->ImageReslice2->SetResliceAxesDirectionCosines(axialElements); 
  
  this->ImageReslice2->SetOutputDimensionality(2);
  
  this->ImageReslice2->SetResliceAxesOrigin(center[0],center[1],center[2]-10);
 
  this->ImageReslice2->GetOutput()->UpdateInformation();
  
  this->ImageReslice2->Update();  
  
    //Slice 2
  
    this->ImageReslice3 = vtkImageReslice::New();
 // vtkImageReslice *reslice = vtkImageReslice::New();
  this->ImageReslice3->SetInput(inVolume->GetImageData());
  this->ImageReslice3->SetInterpolationModeToLinear();
  //this->ImageReslice1->SetOutputExtent(bounds[0]+1,bounds[1]-1,bounds[2]+1, bounds[3]-1,round(bounds[5]/2), round(bounds[5]/2));
  
  this->ImageReslice3->SetResliceAxesDirectionCosines(axialElements); 
  
  this->ImageReslice3->SetOutputDimensionality(2);
  
  this->ImageReslice3->SetResliceAxesOrigin(center[0],center[1],center[2]);
 
  this->ImageReslice3->GetOutput()->UpdateInformation();
  
  this->ImageReslice3->Update();  
  
    //Slice 2
  
    this->ImageReslice4 = vtkImageReslice::New();
 // vtkImageReslice *reslice = vtkImageReslice::New();
  this->ImageReslice4->SetInput(inVolume->GetImageData());
  this->ImageReslice4->SetInterpolationModeToLinear();
  //this->ImageReslice1->SetOutputExtent(bounds[0]+1,bounds[1]-1,bounds[2]+1, bounds[3]-1,round(bounds[5]/2), round(bounds[5]/2));
  
  this->ImageReslice4->SetResliceAxesDirectionCosines(axialElements); 
  
  this->ImageReslice4->SetOutputDimensionality(2);
  
  this->ImageReslice4->SetResliceAxesOrigin(center[0],center[1],center[2]+10);
 
  this->ImageReslice4->GetOutput()->UpdateInformation();
  
  this->ImageReslice4->Update();  
  
    //Slice 2
  
    this->ImageReslice5 = vtkImageReslice::New();
 // vtkImageReslice *reslice = vtkImageReslice::New();
  this->ImageReslice5->SetInput(inVolume->GetImageData());
  this->ImageReslice5->SetInterpolationModeToLinear();
  //this->ImageReslice1->SetOutputExtent(bounds[0]+1,bounds[1]-1,bounds[2]+1, bounds[3]-1,round(bounds[5]/2), round(bounds[5]/2));
  
  this->ImageReslice5->SetResliceAxesDirectionCosines(axialElements); 
  
  this->ImageReslice5->SetOutputDimensionality(2);
  
  this->ImageReslice5->SetResliceAxesOrigin(center[0],center[1],center[2]+20);
 
  this->ImageReslice5->GetOutput()->UpdateInformation();
  
  this->ImageReslice5->Update();  

  this->CurrentSlide = vtkImageData::New();
  this->CurrentSlide->DeepCopy( this->ImageReslice1->GetOutput());
  this->CurrentSlide->SetOrigin(0,0,0);
*/
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
}


int vtkMRIBiasFieldCorrectionLogic::InitMaxThreshold()
{
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));
  

double maxmin[2];
inVolume->GetImageData()->GetScalarRange(maxmin);

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
return maxmin[1];
  
  
}

int vtkMRIBiasFieldCorrectionLogic::InitMinThreshold()
{
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));
  

double maxmin[2];
inVolume->GetImageData()->GetScalarRange(maxmin);

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
return maxmin[0];
  
  
}

int vtkMRIBiasFieldCorrectionLogic::AxialMin()
{
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));
  

double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
return bounds[0];
  
  
}

int vtkMRIBiasFieldCorrectionLogic::AxialMax()
{
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));
  

double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
return bounds[1];
  
}
int vtkMRIBiasFieldCorrectionLogic::SagittalMax()
{
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));
  

double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
return bounds[3];
  
}
int vtkMRIBiasFieldCorrectionLogic::CoronalMax()
{
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
  
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));
  

double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);
  
  vtkMRIBiasFieldCorrectionLogic_DebugMacro("");
return bounds[5];
  
}

