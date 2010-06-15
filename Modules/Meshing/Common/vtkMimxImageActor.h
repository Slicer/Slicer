/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxImageActor.h,v $
Language:  C++
Date:      $Date: 2008/07/11 03:12:06 $
Version:   $Revision: 1.3 $

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

// .NAME vtkMimxImageActor - a 3D non-orthogonal axes representation
// .SECTION Description
// vtkMimxImageActor is the abstract base class for all the pipeline setup for
// different types of datatypes. Data types concidered are vtkPolyData,
// vtkStructuredGrid (both plane and solid) and vtkUnstructuredGrid.

#ifndef __vtkMimxImageActor_h
#define __vtkMimxImageActor_h

#include "vtkMimxActorBase.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageToVTKImageFilter.h"
#include "vtkImageData.h"
#include "vtkMimxCommonWin32Header.h"
#include "vtkMatrix4x4.h"


class vtkImageFlip;
class vtkImagePlaneWidget;
class vtkRenderWindowInteractor;
class vtkMatrix4x4;

typedef itk::Image<signed short, 3>  ImageType;

class VTK_MIMXCOMMON_EXPORT vtkMimxImageActor : public vtkMimxActorBase
{
public:
  static vtkMimxImageActor *New();
  vtkTypeRevisionMacro(vtkMimxImageActor,vtkMimxActorBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkImageData* GetDataSet();
  //void SetDataType(int){};
  ImageType* GetITKImage();
  void SetITKImageFilePath(const char* FPath);

  // added so we can initialize from Slicer volume
  void SetImageDataSet(vtkImageData* image);
  void SetImageDataSet(vtkImageData* image, vtkMatrix4x4* matrix, double origin[3], double spacing[3]);
  void InitializePlaneWidgets();
  void InitializePlaneWidgets(vtkMatrix4x4* matrix, double origin[3], double spacing[3]);

  vtkSetObjectMacro(RASToIJKMatrix,vtkMatrix4x4);
  vtkGetObjectMacro(RASToIJKMatrix,vtkMatrix4x4);

  void SetInteractor(vtkRenderWindowInteractor *Int)
  {
         this->Interactor = Int;
  }
  void DisplayActor(int Display);
  int GetActorVisibility();

  vtkMimxImageActor();
  ~vtkMimxImageActor();

   double Origin[3];  // info copied from originating volume
   double Spacing[3]; // info copied from originating volume

protected:
  typedef itk::ImageFileReader<ImageType> ReaderType;
  typedef itk::ImageToVTKImageFilter<ImageType> FilterType;

  // this is the matrix showing orientation in Slicer3 world space. It is initialized when the actor
  // is created during a slicer volume import
  vtkMatrix4x4 *RASToIJKMatrix;
  ReaderType::Pointer Reader;
  FilterType::Pointer   Filter;
  vtkImagePlaneWidget *PlaneX;
  vtkImagePlaneWidget *PlaneY;
  vtkImagePlaneWidget *PlaneZ;
  vtkRenderWindowInteractor *Interactor;
  vtkImageFlip* FlipFilter;

  // this is a pointer to the current VTK form of the dataset.  All methods are
  // required to update this so it is always current.

  vtkImageData* SavedImage;


private:
  vtkMimxImageActor(const vtkMimxImageActor&);  // Not implemented.
  void operator=(const vtkMimxImageActor&);  // Not implemented.
};

#endif

