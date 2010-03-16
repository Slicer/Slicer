/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxApplyImageBasedMaterialProperties.cxx,v $
Language:  C++
Date:      $Date: 2008/08/10 00:48:34 $
Version:   $Revision: 1.8 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkMimxApplyImageBasedMaterialProperties.h"

#include "vtkCellData.h"
#include "vtkExecutive.h"
#include "vtkHexahedron.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkDoubleArray.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"
#include "itkMesh.h"
#include "itkMimxAccumulator.h"

vtkCxxRevisionMacro(vtkMimxApplyImageBasedMaterialProperties, "$Revision: 1.8 $");
vtkStandardNewMacro(vtkMimxApplyImageBasedMaterialProperties);

vtkMimxApplyImageBasedMaterialProperties::vtkMimxApplyImageBasedMaterialProperties()
{
        this->ITKImage = NULL;
        this->ITKOrientImage = NULL;
        this->ElementSetName = NULL;
        this->ConstantA = 0.0;
        this->ConstantB = 3790.0;
        this->ConstantC = 3.0;
        this->IntensityCalculationMode = AVERAGE;
}

vtkMimxApplyImageBasedMaterialProperties::~vtkMimxApplyImageBasedMaterialProperties()
{
}


int vtkMimxApplyImageBasedMaterialProperties::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
std::cerr << "Got Input vtkInformation" << std::endl;

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
std::cerr << "Got Output vtkInformation" << std::endl;

  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
          outInfo->Get(vtkDataObject::DATA_OBJECT()));

  int numPts = input->GetNumberOfPoints();
  int numCells = input->GetNumberOfCells();
        
  if (numPts <=0 || numCells <= 0)
  {
          vtkErrorMacro("Invalid Data");
                return 0;
        }

  if ((!this->ITKImage) && (! this->ITKOrientImage) )
  {
        vtkErrorMacro("Set ITK image");
        return 0;
  }
        
  if(!this->ElementSetName)
  {
          vtkErrorMacro("Set Element Set Name");
          return 0;
  }
  
  vtkIntArray *elset = vtkIntArray::SafeDownCast(
          input->GetCellData()->GetArray(this->ElementSetName));
  if(!elset)
  {
          vtkErrorMacro("Invalid Element Set Name Entered");
          return 0;
  }
  output->DeepCopy(input);
  // loop through all the cells (hexahedron), find the bounds
  // compute the region in the image associated with the bounds
  // from the unstructured grid cells.

  char str[256];
  strcpy(str, this->ElementSetName);
  strcat(str, "_Image_Based_Material_Property");
  vtkDoubleArray *matarray = vtkDoubleArray::New();
  matarray->SetNumberOfValues(numCells);
  matarray->SetName(str);
  int i;
  double bounds[6];
  typedef itk::Mesh<double, 3> MeshType;
  ImageType::PointType p0;
  ImageType::PointType p1;

  IntegerType::IndexType StartIndex;
  IntegerType::IndexType EndIndex;
  input->BuildLinks();
  for (i=0; i < numCells; i++)
  {
          if (elset->GetValue(i))
          {
                        input->GetCellBounds(i, bounds);
                        p0[0] = bounds[0];      p0[1] = bounds[2];      p0[2] = bounds[4];
                        p1[0] = bounds[1];      p1[1] = bounds[3];      p1[2] = bounds[5];
std::cerr << "ITK Image " << this->ITKImage << std::endl;
                        if ( this->ITKImage )
                        {
                        this->ITKImage->TransformPhysicalPointToIndex(p0, StartIndex);
                        this->ITKImage->TransformPhysicalPointToIndex(p1, EndIndex);
                        }
                        else
                        {
                        this->ITKOrientImage->TransformPhysicalPointToIndex(p0, StartIndex);
                        this->ITKOrientImage->TransformPhysicalPointToIndex(p1, EndIndex);
                        }
std::cerr << "Point 0 " << p0 << std::endl;
std::cerr << "Point 1 " << p1 << std::endl;
std::cerr << "Start Index " << StartIndex << std::endl;
std::cerr << "End Index " << EndIndex << std::endl;

                        for (int j=0;j<3;j++)
                        {
                        if (StartIndex[j] > EndIndex[j])
                        {
                          int tmpValue = EndIndex[j];
                          StartIndex[j] = EndIndex[j];
                          EndIndex[j] = tmpValue;
                        }
                        }
                        //std::cout << "Start Index: " << StartIndex << std::endl;
                        //std::cout << "End Index: " << EndIndex << std::endl;
                        
                        double intensityvalue;
                        switch (this->IntensityCalculationMode)
                        {
                          case AVERAGE:
                            intensityvalue = this->GetAverageSubRegionIntensityValue(StartIndex, EndIndex);
                            break;
                          case MEDIAN:
                            intensityvalue = this->GetMedianSubRegionIntensityValue(StartIndex, EndIndex);
                            break;
                          case MAXIMUM:
                            intensityvalue = this->GetMaximumSubRegionIntensityValue(StartIndex, EndIndex);
                            break;
                          default:
                            vtkErrorMacro("Invalid IntensityCalculationMode. Must be AVERAGE, MEDIAN, or MAXIMUM.");
                        return 0;
                        }
                        double matprop = this->CalculateMaterialProperties(intensityvalue);
                        matarray->SetValue(i, matprop);
          }
          else
          {
                matarray->SetValue(i, -1.0);
          }
  }
  output->GetCellData()->AddArray(matarray);
  matarray->Delete();

  strcat(str, "_ReBin");
  if(output->GetCellData()->GetArray(str))
          output->GetCellData()->RemoveArray(str);
  return 1;
}

int vtkMimxApplyImageBasedMaterialProperties::FillInputPortInformation(int port,
                                                                                                                          vtkInformation *info)
{
        if(port == 0)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
                
                return 1;
        }
        else
        {
                vtkErrorMacro("Only one input is allowed");
        }
        return 0;
}

double vtkMimxApplyImageBasedMaterialProperties::
        GetAverageSubRegionIntensityValue(IntegerType StartIndex, IntegerType EndIndex)
{
        int i, j, k;
        double PixelIntensity = 0.0;
        int size = (EndIndex[2] - StartIndex[2] + 1);
        size *= (EndIndex[1] - StartIndex[1] + 1);
        size *= (EndIndex[0] - StartIndex[0] + 1);
        mimxFunction::MeanAccumulator<double,double> accumulator(size);
        accumulator.Initialize();
        
        for(k=StartIndex[2]; k<= EndIndex[2]; k++)
        {
                for(j=StartIndex[1]; j<= EndIndex[1]; j++)
                {
                        for(i=StartIndex[0]; i<= EndIndex[0]; i++)
                        {
                                IntegerType::IndexType CurrentIndex;
                                CurrentIndex[0] = i; CurrentIndex[1] = j; CurrentIndex[2] = k;
                                if ( this->ITKImage )
                                {
                                PixelIntensity = this->ITKImage->GetPixel(CurrentIndex);
                                accumulator( PixelIntensity );
                                }
                                else
                                {
                         double tmpValue = static_cast<double> ( this->ITKOrientImage->GetPixel(CurrentIndex) );
                         //std::cout << "Index : " << CurrentIndex << " " << tmpValue << std::endl;
                         //PixelIntensity += tmpValue;
                         accumulator( tmpValue );
                                }
                        }
                }
        }
        
        PixelIntensity = accumulator.GetValue();
        
        return PixelIntensity;
}

double vtkMimxApplyImageBasedMaterialProperties::
        GetMedianSubRegionIntensityValue(IntegerType StartIndex, IntegerType EndIndex)
{
        int i, j, k;
        double PixelIntensity = 0.0;
        int size = (EndIndex[2] - StartIndex[2] + 1);
        size *= (EndIndex[1] - StartIndex[1] + 1);
        size *= (EndIndex[0] - StartIndex[0] + 1);
        mimxFunction::MedianAccumulator<double> accumulator(size);
        accumulator.Initialize();
        
        for(k=StartIndex[2]; k<= EndIndex[2]; k++)
        {
                for(j=StartIndex[1]; j<= EndIndex[1]; j++)
                {
                        for(i=StartIndex[0]; i<= EndIndex[0]; i++)
                        {
                                IntegerType::IndexType CurrentIndex;
                                CurrentIndex[0] = i; CurrentIndex[1] = j; CurrentIndex[2] = k;
                                if ( this->ITKImage )
                                {
                                PixelIntensity = this->ITKImage->GetPixel(CurrentIndex);
                                accumulator( PixelIntensity );
                                }
                                else
                                {
                         double tmpValue = static_cast<double> ( this->ITKOrientImage->GetPixel(CurrentIndex) );
                         //std::cout << "Index : " << CurrentIndex << " " << tmpValue << std::endl;
                         //PixelIntensity += tmpValue;
                         accumulator( tmpValue );
                                }
                        }
                }
        }
        
        PixelIntensity = accumulator.GetValue();
        
        return PixelIntensity;
}

double vtkMimxApplyImageBasedMaterialProperties::
        GetMaximumSubRegionIntensityValue(IntegerType StartIndex, IntegerType EndIndex)
{
        int i, j, k;
        double PixelIntensity = 0.0;
        int size = (EndIndex[2] - StartIndex[2] + 1);
        size *= (EndIndex[1] - StartIndex[1] + 1);
        size *= (EndIndex[0] - StartIndex[0] + 1);
        mimxFunction::MaximumAccumulator<double> accumulator(size);
        accumulator.Initialize();
        
        for(k=StartIndex[2]; k<= EndIndex[2]; k++)
        {
                for(j=StartIndex[1]; j<= EndIndex[1]; j++)
                {
                        for(i=StartIndex[0]; i<= EndIndex[0]; i++)
                        {
                                IntegerType::IndexType CurrentIndex;
                                CurrentIndex[0] = i; CurrentIndex[1] = j; CurrentIndex[2] = k;
                                if ( this->ITKImage )
                                {
                                PixelIntensity = this->ITKImage->GetPixel(CurrentIndex);
                                accumulator( PixelIntensity );
                                }
                                else
                                {
                         double tmpValue = static_cast<double> ( this->ITKOrientImage->GetPixel(CurrentIndex) );
                         //std::cout << "Index : " << CurrentIndex << " " << tmpValue << std::endl;
                         //PixelIntensity += tmpValue;
                         accumulator( tmpValue );
                                }
                        }
                }
        }
        
        PixelIntensity = accumulator.GetValue();
        
        return PixelIntensity;
}

void vtkMimxApplyImageBasedMaterialProperties::SetITKImage(ImageType::Pointer InputImage)
{
        this->ITKImage = InputImage;
}

void vtkMimxApplyImageBasedMaterialProperties::SetITKOrientedImage(OrientImageType::Pointer InputImage)
{
        this->ITKOrientImage = InputImage;
}

double vtkMimxApplyImageBasedMaterialProperties::CalculateMaterialProperties( double ctValue )
{ 
        double voxelDensity;
        double waterDensity = 1.0;
        /*
        double strainRate = 0.01;
        double constantB = this->ConstantB;
        double constantC = this->ConstantC;
        double constantD = this->ConstantA;
        */
        double modulus;

        voxelDensity = 
                ( waterDensity*ctValue )/1000.0 + waterDensity;
        /* modulus = constantB * pow( strainRate, constantD ) * 
                pow( voxelDensity, constantC ); */
        modulus = this->ConstantA +  this->ConstantB * pow( voxelDensity, this->ConstantC );
        return modulus;
}


void vtkMimxApplyImageBasedMaterialProperties::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

