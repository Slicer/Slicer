/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://www.na-mic.org/svn/Slicer3/trunk/Libs/vtkITK/vtkITKArchetypeImageSeriesVectorReaderFile.cxx $
  Date:      $Date: 2007-08-15 17:04:27 -0400 (Wed, 15 Aug 2007) $
  Version:   $Revision: 4068 $

==========================================================================*/

#include "vtkITKArchetypeImageSeriesVectorReaderFile.h"

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

vtkStandardNewMacro(vtkITKArchetypeImageSeriesVectorReaderFile);
vtkCxxRevisionMacro(vtkITKArchetypeImageSeriesVectorReaderFile, "$Revision: 4068 $");

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderFile::vtkITKArchetypeImageSeriesVectorReaderFile()
{
}

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderFile::~vtkITKArchetypeImageSeriesVectorReaderFile() 
{
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesVectorReaderFile::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "vtk ITK Archetype Image Series Vector Reader File\n";
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkITKArchetypeImageSeriesVectorReaderFile::ExecuteData(vtkDataObject *output)
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

#define vtkITKExecuteDataFromFileVector(typeN, type) \
    case typeN: \
    {\
      typedef itk::Vector<type, 3>    VectorPixelType;\
      typedef itk::VectorImage<type,3> image2##typeN;\
      typedef itk::ImageSource<image2##typeN> FilterType; \
      FilterType::Pointer filter; \
      typedef itk::ImageFileReader<\
        image2##typeN > ReaderType##typeN; \
      ReaderType##typeN::Pointer reader2##typeN = ReaderType##typeN::New();\
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
      itk::ImportImageContainer<unsigned long, type >::Pointer PixelContainer2##typeN;\
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
    vtkDebugMacro("ImageSeriesVectorReaderFile: only one file: " << this->FileNames[0].c_str());
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
    // ERROR - should have used the series reader
    vtkErrorMacro("There is more than one file, use the VectorReaderSeries instead");
    }
}


void vtkITKArchetypeImageSeriesVectorReaderFile::ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&,void* data)
{
  vtkITKArchetypeImageSeriesVectorReaderFile* me=reinterpret_cast<vtkITKArchetypeImageSeriesVectorReaderFile*>(data);
  me->Progress=obj->GetProgress();
  me->InvokeEvent(vtkCommand::ProgressEvent,&me->Progress);
}
