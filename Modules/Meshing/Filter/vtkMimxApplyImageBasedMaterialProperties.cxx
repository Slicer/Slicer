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

#include "vtkMath.h"

vtkCxxRevisionMacro(vtkMimxApplyImageBasedMaterialProperties, "$Revision: 1.8 $");
vtkStandardNewMacro(vtkMimxApplyImageBasedMaterialProperties);

vtkMimxApplyImageBasedMaterialProperties::vtkMimxApplyImageBasedMaterialProperties()
{
        //this->ITKImage = NULL;
        this->VTKImage = NULL;
        this->ImageTransform = NULL;
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

  if ((!this->VTKImage)  )
  {
        vtkErrorMacro("Set VTK image");
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
//  typedef itk::Mesh<double, 3> MeshType;
//  ImageType::PointType p0;
//  ImageType::PointType p1;

//  IntegerType::IndexType StartIndex;
//  IntegerType::IndexType EndIndex;

  double p0[4], p1[4];
  double p0ijk[4], p1ijk[4];
 // vtkIDType StartPointIndex, EndPointIndex;

  input->BuildLinks();
  for (i=0; i < numCells; i++)
  {
          if (elset->GetValue(i))
          {
                        input->GetCellBounds(i, bounds);
                        p0[0] = bounds[0];      p0[1] = bounds[2];      p0[2] = bounds[4];
                        p1[0] = bounds[1];      p1[1] = bounds[3];      p1[2] = bounds[5];
                        p0[3] = p1[3] = 1.0;
                        std::cerr << "VTK Image " << this->VTKImage << std::endl;
                        if ( this->VTKImage && this->ImageTransform )
                        {
//                        this->ITKImage->TransformPhysicalPointToIndex(p0, StartIndex);
//                        this->ITKImage->TransformPhysicalPointToIndex(p1, EndIndex);

                          // we map from physical (RAS) coords to Index coords (ijk) using the
                          // matrix to xform to RAS coords since the mesh is in RAS coords in slicer
                          this->ImageTransform->MultiplyPoint(p0,p0ijk);
                          this->ImageTransform->MultiplyPoint(p1,p1ijk);
                          // now find the indices into the image corresponding to this ijk location
                          //StartPointIndex = this->VTKImage->FindPoint(p0ijk);

                        }
                        else
                        {
                          vtkErrorMacro("MimxApplyImageBasedMaterialProperties not initialized correctly!");
                        }
std::cerr << "Point 0 " << p0 << std::endl;
std::cerr << "Point 1 " << p1 << std::endl;
std::cerr << "Start Index: " << p0ijk << std::endl;
std::cerr << "End Index: " << p1ijk << std::endl;

                        // switch labels around so all coords are in increasing order
                        for (int j=0;j<3;j++)
                        {
                        if (p0ijk[j] > p1ijk[j])
                        {
                          int tmpValue = p0ijk[j];
                          p0ijk[j] = p1ijk[j];
                          p1ijk[j] = tmpValue;
                        }
                        }
                        std::cout << "Start Index: " << p0ijk << std::endl;
                        std::cout << "End Index: " << p1ijk << std::endl;
                        
                        // now calculate the intensity values to apply to the mesh point.  To do this, the
                        // boundary box points are passed to the appropriate calculation algorithm.  Parameters
                        // are passed as IJK space indices
                        double intensityvalue;

                        int p0ijk_floor[3], p1ijk_floor[3];
                        p0ijk_floor[0] = vtkMath::Floor(p0ijk[0]); p0ijk_floor[1] = vtkMath::Floor(p0ijk[1]); p0ijk_floor[2] = vtkMath::Floor(p0ijk[2]);
                        p1ijk_floor[0] = vtkMath::Floor(p1ijk[0]); p1ijk_floor[1] = vtkMath::Floor(p1ijk[1]); p1ijk_floor[2] = vtkMath::Floor(p1ijk[2]);

                        switch (this->IntensityCalculationMode)
                        {
                          case AVERAGE:
//                            intensityvalue = this->GetAverageSubRegionIntensityValue(StartIndex, EndIndex);
                              intensityvalue = this->GetAverageSubRegionIntensityValue(p0ijk_floor, p1ijk_floor);
                            break;
                          case MEDIAN:
//                            intensityvalue = this->GetMedianSubRegionIntensityValue(StartIndex, EndIndex);
                              intensityvalue = this->GetMedianSubRegionIntensityValue(p0ijk_floor, p1ijk_floor);
                            break;
                          case MAXIMUM:
//                            intensityvalue = this->GetMaximumSubRegionIntensityValue(StartIndex, EndIndex);
                              intensityvalue = this->GetMaximumSubRegionIntensityValue(p0ijk_floor, p1ijk_floor);
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
        GetAverageSubRegionIntensityValue(int StartIndex[3], int EndIndex[3])
{
        int i, j, k;
        double *PixelIntensity;
        double AccumulatedIntensity = 0.0;
        double CurrentIndex[3];

        // calculate how many pixels we will accumulate
        int size = (EndIndex[2] - StartIndex[2] + 1);
        size *= (EndIndex[1] - StartIndex[1] + 1);
        size *= (EndIndex[0] - StartIndex[0] + 1);
        std::cout << "accumulate over: " << size << "pixels" << endl;
        
        mimxFunction::MeanAccumulator<double,double> accumulator(size);
        accumulator.Initialize();

        for(k=StartIndex[2]; k<= EndIndex[2]; k++)
        {
                for(j=StartIndex[1]; j<= EndIndex[1]; j++)
                {
                        for(i=StartIndex[0]; i<= EndIndex[0]; i++)
                        {
                                CurrentIndex[0] = (double)i; CurrentIndex[1] = (double)j; CurrentIndex[2] = (double)k;
                                if ( this->VTKImage )
                                {
                                  // find the index into the pixel array for i,j,k
                                  vtkIdType PixelIndex = this->VTKImage->FindPoint(CurrentIndex);
                                  PixelIntensity = this->VTKImage->GetPointData()->GetScalars()->GetTuple(PixelIndex);
                                  // lookup the scalar value
                                  AccumulatedIntensity += PixelIntensity[0] ;
                                  accumulator(PixelIntensity[0]);
                                }
                                else
                                {
                                    AccumulatedIntensity = 0.0;
                                }
                        }
                }
        }
        
        //PixelIntensity[0] = AccumulatedIntensity/(double)size;
        //return PixelIntensity[0];
        return accumulator.GetValue();
}

double vtkMimxApplyImageBasedMaterialProperties::
        GetMedianSubRegionIntensityValue(int StartIndex[3], int EndIndex[3])
{
    int i, j, k;
    double *PixelIntensity;
    //double AccumulatedIntensity = 0.0;
    double CurrentIndex[3];

    // calculate how many pixels we will accumulate
    int size = (EndIndex[2] - StartIndex[2] + 1);
    size *= (EndIndex[1] - StartIndex[1] + 1);
    size *= (EndIndex[0] - StartIndex[0] + 1);
    std::cout << "accumulate over: " << size << "pixels" << endl;

    mimxFunction::MedianAccumulator<double> accumulator(size);
    accumulator.Initialize();

    for(k=StartIndex[2]; k<= EndIndex[2]; k++)
    {
            for(j=StartIndex[1]; j<= EndIndex[1]; j++)
            {
                    for(i=StartIndex[0]; i<= EndIndex[0]; i++)
                    {
                        CurrentIndex[0] = (double)i; CurrentIndex[1] = (double)j; CurrentIndex[2] = (double)k;
                            if ( this->VTKImage )
                            {
                              // find the index into the pixel array for i,j,k
                                vtkIdType PixelIndex = this->VTKImage->FindPoint(CurrentIndex);
                                // find the contribution of this voxel to the resulting intensity.  If the
                                // voxel maps outside the range then its contribution is zero and a warning is
                                // generated
                                if (PixelIndex >= 0)
                                  PixelIntensity = this->VTKImage->GetPointData()->GetScalars()->GetTuple(PixelIndex);
                                else {
                                    PixelIntensity[0] = 0.0;
                                    std::cerr <<"vtkMimxApplyImageBasedMaterialProperties: off image lookup for ijk= " << i << "," << j << "," << k << endl;;
                                }
                              // incorporate the scalar intensity in the median calculation
                             accumulator(PixelIntensity[0]);
                            }
                    }
            }
    }
    return accumulator.GetValue();
}

double vtkMimxApplyImageBasedMaterialProperties::
        GetMaximumSubRegionIntensityValue(int StartIndex[3], int EndIndex[3])
{
    int i, j, k;
    double *PixelIntensity;
    //double MaximumIntensity = -9999999.0;
    double CurrentIndex[3];

    // calculate how many pixels we will accumulate
    int size = (EndIndex[2] - StartIndex[2] + 1);
    size *= (EndIndex[1] - StartIndex[1] + 1);
    size *= (EndIndex[0] - StartIndex[0] + 1);
    std::cout << "accumulate over: " << size << "pixels" << endl;

        mimxFunction::MaximumAccumulator<double> accumulator(size);
        accumulator.Initialize();

    for(k=StartIndex[2]; k<= EndIndex[2]; k++)
    {
            for(j=StartIndex[1]; j<= EndIndex[1]; j++)
            {
                    for(i=StartIndex[0]; i<= EndIndex[0]; i++)
                    {
                         CurrentIndex[0] = (double)i; CurrentIndex[1] = (double)j; CurrentIndex[2] = (double)k;
                            if ( this->VTKImage )
                            {
                              // find the index into the pixel array for i,j,k
                              vtkIdType PixelIndex = this->VTKImage->FindPoint(CurrentIndex);
                              PixelIntensity = this->VTKImage->GetPointData()->GetScalars()->GetTuple(PixelIndex);
                             // save the largest voxel value we have come across so far as the result
//                              if (MaximumIntensity < PixelIntensity[0])
//                                      MaximumIntensity =  PixelIntensity[0];
                              accumulator(PixelIntensity[0]);
                            }
                    }
            }
    }
    // return the maximum voxel value found in the ROI
    //return MaximumIntensity;
    return accumulator.GetValue();
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

