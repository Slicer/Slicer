/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxApplyImageBasedMaterialProperties.h,v $
Language:  C++
Date:      $Date: 2008/08/03 20:20:24 $
Version:   $Revision: 1.5 $

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

// .NAME vtkMimxApplyImageBasedMaterialProperties - Apply material properties based on 
// image intensity
// .SECTION Description
// vtkMimxApplyImageBasedMaterialProperties is a filter that takes an unstructured grid 
// and itkImage as inputs. The filter calculates material property value based on image
// intensity values.
// Cell Data scalar named Material_Properties is attached to the output.

// .SECTION See Also

#ifndef __vtkMimxApplyImageBasedMaterialProperties_h
#define __vtkMimxApplyImageBasedMaterialProperties_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkMimxFilterWin32Header.h"

//#include "itkImage.h"
//#include "itkImage.h"
//#include "itkIndex.h"

#include "vtkImageData.h"
#include "vtkMatrix4x4.h"

class vtkUnstructuredGrid;
//typedef itk::Image<signed short, 3>  ImageType;
//typedef itk::Image<signed short, 3> OrientImageType;
//typedef itk::Index<3> IntegerType;


class VTK_MIMXFILTER_EXPORT vtkMimxApplyImageBasedMaterialProperties : public vtkUnstructuredGridAlgorithm
{
public:
  enum { AVERAGE=1, MEDIAN=2, MAXIMUM=3 };
  static vtkMimxApplyImageBasedMaterialProperties *New();
  vtkTypeRevisionMacro(vtkMimxApplyImageBasedMaterialProperties,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkSetMacro(ElementSetName, const char*);
  vtkSetMacro(ConstantA, double);
  vtkGetMacro(ConstantA, double);
  vtkSetMacro(ConstantB, double);
  vtkGetMacro(ConstantB, double);
  vtkSetMacro(ConstantC, double);
  vtkGetMacro(ConstantC, double);
  vtkSetMacro(IntensityCalculationMode, int);
  vtkGetMacro(IntensityCalculationMode, int);

  vtkSetObjectMacro(VTKImage,vtkImageData);
  vtkSetObjectMacro(ImageTransform,vtkMatrix4x4);

protected:
  vtkMimxApplyImageBasedMaterialProperties();
  ~vtkMimxApplyImageBasedMaterialProperties();
  
  //slicer uses VTK image data volumes instead of ITK
//  ImageType::Pointer ITKImage;
//  OrientImageType::Pointer ITKOrientImage;
  vtkImageData *VTKImage;
  vtkMatrix4x4 *ImageTransform;

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int , vtkInformation *);

  // these accumulation routines are passed 3D bboxes in IJK coordinates to iterate over. The
  // arguments are (int[3], int[3]) because points have already been transfored from RAS 2 IJK
  // when these routines are called
  double GetAverageSubRegionIntensityValue(int StartIndex[3], int EndIndex[3]);
  double GetMedianSubRegionIntensityValue(int StartIndex[3], int EndIndex[3]);
  double GetMaximumSubRegionIntensityValue(int StartIndex[3], int EndIndex[3]);

  double CalculateMaterialProperties(double Value);
  const char *ElementSetName;
  double ConstantA;
  double ConstantB;
  double ConstantC;
  int IntensityCalculationMode;
private:
  vtkMimxApplyImageBasedMaterialProperties(const vtkMimxApplyImageBasedMaterialProperties&);  // Not implemented.
  void operator=(const vtkMimxApplyImageBasedMaterialProperties&);  // Not implemented.
};

#endif
