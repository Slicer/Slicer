/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#include "vtkITKArchetypeImageSeriesVectorReader.h"

#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include <vtkCommand.h>

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
vtkCxxRevisionMacro(vtkITKArchetypeImageSeriesVectorReader, "$Revision$");

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
  //data->UpdateInformation();
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
        itk::CStyleCommand::Pointer pcl=itk::CStyleCommand::New(); \
        pcl->SetCallback((itk::CStyleCommand::FunctionPointer)&ReadProgressCallback); \
      pcl->SetClientData(this); \
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

// DIMENSION in this case is 3, but each VectorImage has a dynamic number of components at each xyz voxel
// the FromFile macro is needed to access the MGH stats files with a measurement for each subject at each voxel
// TODO: implement the FromSeries macro
#define DIMENSION 3
#define vtkITKExecuteDataFromFileVectorComponents(typeN, type) \
    case typeN: \
    {\
      typedef itk::Vector<type, DIMENSION>    VectorPixelType;\
      typedef itk::VectorImage<VectorPixelType,DIMENSION> image3##typeN;\
      typedef itk::ImageSource<image3##typeN> FilterType; \
      FilterType::Pointer filter; \
      itk::ImageFileReader<image3##typeN>::Pointer reader3##typeN = \
            itk::ImageFileReader<image3##typeN>::New(); \
      reader3##typeN->SetFileName(this->FileNames[0].c_str()); \
      if (this->UseNativeCoordinateOrientation) \
        { \
        filter = reader3##typeN; \
        } \
      else \
        { \
        itk::OrientImageFilter<image3##typeN,image3##typeN>::Pointer orient3##typeN = \
              itk::OrientImageFilter<image3##typeN,image3##typeN>::New(); \
        if (this->Debug) {orient3##typeN->DebugOn();} \
        orient3##typeN->SetInput(reader3##typeN->GetOutput()); \
        orient3##typeN->UseImageDirectionOn(); \
        orient3##typeN->SetDesiredCoordinateOrientation(this->DesiredCoordinateOrientation); \
        filter = orient3##typeN; \
        } \
       filter->UpdateLargestPossibleRegion();\
      itk::ImportImageContainer<unsigned long, VectorPixelType>::Pointer PixelContainer3##typeN;\
      PixelContainer3##typeN = filter->GetOutput()->GetPixelContainer();\
      void *ptr = static_cast<void *> (PixelContainer3##typeN->GetBufferPointer());\
      (dynamic_cast<vtkImageData *>( output))->GetPointData()->GetScalars()->SetVoidArray(ptr, PixelContainer3##typeN->Size(), 0);\
      PixelContainer3##typeN->ContainerManageMemoryOff();\
    }\
    break
  // END VECTOR COMPONENTS MACRO

    // If there is only one file in the series, just use an image file reader
  if (this->FileNames.size() == 1)
    {
    vtkDebugMacro("ImageSeriesVectorReader: only one file: " << this->FileNames[0].c_str());
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
          vtkErrorMacro(<< "UpdateFromFile: Unknown data type " << this->OutputScalarType);
        }
      }
    else 
      {
      // have a potentially variable number of components, use the VectorImage macro
      vtkDebugMacro("UpdateFromFile: number of components " << this->GetNumberOfComponents() << ", using the vtkITKExecuteDataFromFileVectorComponents macro to read into a VectorImage");
      switch (this->OutputScalarType)
        {
        vtkITKExecuteDataFromFileVectorComponents(VTK_DOUBLE, double);
        vtkITKExecuteDataFromFileVectorComponents(VTK_FLOAT, float);
        vtkITKExecuteDataFromFileVectorComponents(VTK_LONG, long);
        vtkITKExecuteDataFromFileVectorComponents(VTK_UNSIGNED_LONG, unsigned long);
        vtkITKExecuteDataFromFileVectorComponents(VTK_INT, int);
        vtkITKExecuteDataFromFileVectorComponents(VTK_UNSIGNED_INT, unsigned int);
        vtkITKExecuteDataFromFileVectorComponents(VTK_SHORT, short);
        vtkITKExecuteDataFromFileVectorComponents(VTK_UNSIGNED_SHORT, unsigned short);
        vtkITKExecuteDataFromFileVectorComponents(VTK_CHAR, char);
        vtkITKExecuteDataFromFileVectorComponents(VTK_UNSIGNED_CHAR, unsigned char);
        default:
          vtkErrorMacro(<< "UpdateFromFile: Unknown data type " << this->OutputScalarType);
        }
      }
    }
  else
    {
    // use the series reader
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
          vtkErrorMacro(<< "UpdateFromFile: Unknown data type " << this->OutputScalarType);
        }
      }
    else 
      {
      vtkErrorMacro(<< "UpdateFromFile: Unsupported Number Of Components for series reader: 3 != " << this->GetNumberOfComponents());
      }
    }
}


void vtkITKArchetypeImageSeriesVectorReader::ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&,void* data)
{
  vtkITKArchetypeImageSeriesVectorReader* me=reinterpret_cast<vtkITKArchetypeImageSeriesVectorReader*>(data);
  me->Progress=obj->GetProgress();
  me->InvokeEvent(vtkCommand::ProgressEvent,&me->Progress);
}
