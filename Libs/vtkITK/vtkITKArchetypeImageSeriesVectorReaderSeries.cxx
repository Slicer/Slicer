/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://www.na-mic.org/svn/Slicer3/trunk/Libs/vtkITK/vtkITKArchetypeImageSeriesVectorReaderSeries.cxx $
  Date:      $Date: 2007-08-15 17:04:27 -0400 (Wed, 15 Aug 2007) $
  Version:   $Revision: 4068 $

==========================================================================*/

#include "vtkITKArchetypeImageSeriesVectorReaderSeries.h"

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

vtkStandardNewMacro(vtkITKArchetypeImageSeriesVectorReaderSeries);
vtkCxxRevisionMacro(vtkITKArchetypeImageSeriesVectorReaderSeries, "$Revision: 4068 $");

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderSeries::vtkITKArchetypeImageSeriesVectorReaderSeries()
{
}

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderSeries::~vtkITKArchetypeImageSeriesVectorReaderSeries() 
{
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesVectorReaderSeries::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "vtk ITK Archetype Image Series Vector Reader\n";
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkITKArchetypeImageSeriesVectorReaderSeries::ExecuteData(vtkDataObject *output)
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
      typedef type    VectorPixelType;\
      typedef itk::VectorImage<VectorPixelType,3> image##typeN;\
      typedef itk::ImageSource<image##typeN> FilterType; \
      FilterType::Pointer filter; \
      itk::ImageSeriesReader<image##typeN>::Pointer reader##typeN = \
      itk::ImageSeriesReader<image##typeN>::New(); \
      itk::CStyleCommand::Pointer pcl=itk::CStyleCommand::New(); \
      pcl->SetCallback((itk::CStyleCommand::FunctionPointer)&ReadProgressCallback); \
      pcl->SetClientData(this); \
      reader##typeN->SetFileNames(this->FileNames); \
      reader##typeN->ReleaseDataFlagOn(); \
      reader##typeN->GetOutput()->SetVectorLength(3); \
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

  // END VECTOR MACRO


    // If there is only one file in the series, just use an image file reader
  if (this->FileNames.size() == 1)
    {
    vtkErrorMacro("ImageSeriesVectorReaderSeries: only one file: " << this->FileNames[0].c_str() << " use the VectorReaderFile instead!");
    }    
  else
    {
    // use the series reader
    try 
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
          vtkErrorMacro(<< "UpdateFromFile Series: Unknown data type " << this->OutputScalarType);
        }
      }
    catch (itk::ExceptionObject & e)
      {
      vtkErrorMacro(<< "Exception from vtkITK MegaMacro: " << e << "\n");
      }
    }
}


void vtkITKArchetypeImageSeriesVectorReaderSeries::ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&,void* data)
{
  vtkITKArchetypeImageSeriesVectorReaderSeries* me=reinterpret_cast<vtkITKArchetypeImageSeriesVectorReaderSeries*>(data);
  me->Progress=obj->GetProgress();
  me->InvokeEvent(vtkCommand::ProgressEvent,&me->Progress);
}
