/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkITKArchetypeImageSeriesVectorReader.cxx,v $
  Date:      $Date: 2006/02/08 17:40:23 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
#include "vtkITKArchetypeImageSeriesVectorReader.h"

#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"

#ifndef vtkFloatingPointType
#define vtkFloatingPointType float
#endif

#include "itkArchetypeSeriesFileNames.h"
#include "itkImage.h"
#include "itkVector.h"
#include "itkOrientImageFilter.h"
#include "itkImageSeriesReader.h"
#include "itkImageFileReader.h"
#include "itkImportImageContainer.h"
#include "itkImageRegion.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkGDCMImageIO.h"
#include <itksys/SystemTools.hxx>

vtkStandardNewMacro(vtkITKArchetypeImageSeriesVectorReader);
vtkCxxRevisionMacro(vtkITKArchetypeImageSeriesVectorReader, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReader::vtkITKArchetypeImageSeriesVectorReader()
{
}

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReader::~vtkITKArchetypeImageSeriesVectorReader() 
{
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesVectorReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "vtk ITK Archetype Image Series Vector Reader\n";
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkITKArchetypeImageSeriesVectorReader::ExecuteData(vtkDataObject *output)
{
  if (!this->Superclass::Archetype)
    {
      vtkErrorMacro("An Archetype must be specified.");
      return;
    }

  vtkImageData *data = vtkImageData::SafeDownCast(output);
  data->UpdateInformation();
  data->SetExtent(0,0,0,0,0,0);
  data->AllocateScalars();
  data->SetExtent(data->GetWholeExtent());

  /// VECTOR MACRO
#define vtkITKExecuteDataFromSeriesVector(typeN, type) \
    case typeN: \
    {\
      typedef itk::Vector<type, 3>    VectorPixelType;\
      typedef itk::Image<VectorPixelType,3> image##typeN;\
      typedef itk::ImageSource<image##typeN> FilterType; \
      FilterType::Pointer filter; \
      itk::ImageSeriesReader<image##typeN>::Pointer reader##typeN = \
            itk::ImageSeriesReader<image##typeN>::New(); \
      reader##typeN->SetFileNames(this->FileNames); \
      reader##typeN->ReleaseDataFlagOn(); \
      if (this->UseNativeCoordinateOrientation) \
        { \
        filter = reader##typeN; \
        } \
      else \
        { \
        itk::OrientImageFilter<image##typeN,image##typeN>::Pointer orient##typeN = \
            itk::OrientImageFilter<image##typeN,image##typeN>::New(); \
        if (this->Debug) {orient##typeN->DebugOn();} \
        orient##typeN->SetInput(reader##typeN->GetOutput()); \
        orient##typeN->UseImageDirectionOn(); \
        orient##typeN->SetDesiredCoordinateOrientation(this->DesiredCoordinateOrientation); \
        filter = orient##typeN; \
        }\
      filter->UpdateLargestPossibleRegion(); \
      itk::ImportImageContainer<unsigned long, VectorPixelType>::Pointer PixelContainer##typeN;\
      PixelContainer##typeN = filter->GetOutput()->GetPixelContainer();\
      void *ptr = static_cast<void *> (PixelContainer##typeN->GetBufferPointer());\
      (dynamic_cast<vtkImageData *>( output))->GetPointData()->GetScalars()->SetVoidArray(ptr, PixelContainer##typeN->Size(), 0);\
      PixelContainer##typeN->ContainerManageMemoryOff();\
    }\
    break

#define vtkITKExecuteDataFromFileVector(typeN, type) \
    case typeN: \
    {\
      typedef itk::Vector<type, 3>    VectorPixelType;\
      typedef itk::Image<VectorPixelType,3> image2##typeN;\
      typedef itk::ImageSource<image2##typeN> FilterType; \
      FilterType::Pointer filter; \
      itk::ImageFileReader<image2##typeN>::Pointer reader2##typeN = \
            itk::ImageFileReader<image2##typeN>::New(); \
      reader2##typeN->SetFileName(this->FileNames[0].c_str()); \
      if (this->UseNativeCoordinateOrientation) \
        { \
        filter = reader2##typeN; \
        } \
      else \
        { \
        itk::OrientImageFilter<image2##typeN,image2##typeN>::Pointer orient2##typeN = \
              itk::OrientImageFilter<image2##typeN,image2##typeN>::New(); \
        if (this->Debug) {orient2##typeN->DebugOn();} \
        orient2##typeN->SetInput(reader2##typeN->GetOutput()); \
        orient2##typeN->UseImageDirectionOn(); \
        orient2##typeN->SetDesiredCoordinateOrientation(this->DesiredCoordinateOrientation); \
        filter = orient2##typeN; \
        } \
       filter->UpdateLargestPossibleRegion();\
      itk::ImportImageContainer<unsigned long, VectorPixelType>::Pointer PixelContainer2##typeN;\
      PixelContainer2##typeN = filter->GetOutput()->GetPixelContainer();\
      void *ptr = static_cast<void *> (PixelContainer2##typeN->GetBufferPointer());\
      (dynamic_cast<vtkImageData *>( output))->GetPointData()->GetScalars()->SetVoidArray(ptr, PixelContainer2##typeN->Size(), 0);\
      PixelContainer2##typeN->ContainerManageMemoryOff();\
    }\
    break
  // END VECTOR MACRO

    // If there is only one file in the series, just use an image file reader
  if (this->FileNames.size() == 1)
    {
      if (this->GetNumberOfComponents() == 3) 
      {
       switch (this->OutputScalarType)
        {
          vtkITKExecuteDataFromFileVector(VTK_DOUBLE, double);
          vtkITKExecuteDataFromFileVector(VTK_FLOAT, float);
          vtkITKExecuteDataFromFileVector(VTK_LONG, long);
          vtkITKExecuteDataFromFileVector(VTK_UNSIGNED_LONG, unsigned long);
          vtkITKExecuteDataFromFileVector(VTK_INT, int);
          vtkITKExecuteDataFromFileVector(VTK_UNSIGNED_INT, unsigned int);
          vtkITKExecuteDataFromFileVector(VTK_SHORT, short);
          vtkITKExecuteDataFromFileVector(VTK_UNSIGNED_SHORT, unsigned short);
          vtkITKExecuteDataFromFileVector(VTK_CHAR, char);
          vtkITKExecuteDataFromFileVector(VTK_UNSIGNED_CHAR, unsigned char);
        default:
          vtkErrorMacro(<< "UpdateFromFile: Unknown data type");
        }
    }
      else 
    {
      vtkErrorMacro(<< "UpdateFromFile: Unsupported Number Of Components: 3 != " << this->GetNumberOfComponents());
    }
    }
  else
    {
      if (this->GetNumberOfComponents() == 3)
      {
    switch (this->OutputScalarType)
      {
        vtkITKExecuteDataFromSeriesVector(VTK_DOUBLE, double);
        vtkITKExecuteDataFromSeriesVector(VTK_FLOAT, float);
        vtkITKExecuteDataFromSeriesVector(VTK_LONG, long);
        vtkITKExecuteDataFromSeriesVector(VTK_UNSIGNED_LONG, unsigned long);
        vtkITKExecuteDataFromSeriesVector(VTK_INT, int);
        vtkITKExecuteDataFromSeriesVector(VTK_UNSIGNED_INT, unsigned int);
        vtkITKExecuteDataFromSeriesVector(VTK_SHORT, short);
        vtkITKExecuteDataFromSeriesVector(VTK_UNSIGNED_SHORT, unsigned short);
        vtkITKExecuteDataFromSeriesVector(VTK_CHAR, char);
        vtkITKExecuteDataFromSeriesVector(VTK_UNSIGNED_CHAR, unsigned char);
      default:
        vtkErrorMacro(<< "UpdateFromFile: Unknown data type");
      }
      }
      else 
    {
          vtkErrorMacro(<< "UpdateFromFile: Unsupported Number Of Components: 3 != " << this->GetNumberOfComponents());
    }
    }
}
