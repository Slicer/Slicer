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

class vtkImagePlaneWidget;
class vtkRenderWindowInteractor;

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
  void SetInteractor(vtkRenderWindowInteractor *Int)
  {
         this->Interactor = Int;
  }
  void DisplayActor(int Display);
  int GetActorVisibility();
protected:
  vtkMimxImageActor();
  ~vtkMimxImageActor();
  typedef itk::ImageFileReader<ImageType> ReaderType;
  typedef itk::ImageToVTKImageFilter<ImageType> FilterType;
  ReaderType::Pointer Reader;
  FilterType::Pointer   Filter;
  vtkImagePlaneWidget *PlaneX;
  vtkImagePlaneWidget *PlaneY;
  vtkImagePlaneWidget *PlaneZ;
  vtkRenderWindowInteractor *Interactor;

private:
  vtkMimxImageActor(const vtkMimxImageActor&);  // Not implemented.
  void operator=(const vtkMimxImageActor&);  // Not implemented.
};

#endif

