/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Libs/vtkITK/vtkITKArchetypeDiffusionTensorImageReaderFile.cxx $
  Date:      $Date: 2007-08-15 17:04:27 -0400 (Wed, 15 Aug 2007) $
  Version:   $Revision: 4068 $

==========================================================================*/

#include "vtkITKArchetypeDiffusionTensorImageReaderFile.h"

#include "vtkDataArray.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSmartPointer.h"
#include "vtkFloatArray.h"
#include <vtkCommand.h>

#include "itkOrientImageFilter.h"
#include "itkDiffusionTensor3D.h"
#include "itkImageRegionConstIteratorWithIndex.h"

vtkStandardNewMacro(vtkITKArchetypeDiffusionTensorImageReaderFile);
vtkCxxRevisionMacro(vtkITKArchetypeDiffusionTensorImageReaderFile, "$Revision: 4068 $");

//----------------------------------------------------------------------------
vtkITKArchetypeDiffusionTensorImageReaderFile::vtkITKArchetypeDiffusionTensorImageReaderFile()
{
}

//----------------------------------------------------------------------------
vtkITKArchetypeDiffusionTensorImageReaderFile::~vtkITKArchetypeDiffusionTensorImageReaderFile() 
{
}

//----------------------------------------------------------------------------
void vtkITKArchetypeDiffusionTensorImageReaderFile::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "vtk ITK Archetype Image Series DiffusionTensor3D Reader File\n";
}




//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkITKArchetypeDiffusionTensorImageReaderFile::ExecuteData(vtkDataObject *output)
{
  if (!this->Superclass::Archetype)
    {
      vtkErrorMacro("An Archetype must be specified.");
      return;
    }

  vtkImageData *data = vtkImageData::SafeDownCast(output);
  //data->UpdateInformation();
  data->SetExtent(0,0,0,0,0,0);
  data->SetExtent(data->GetWholeExtent());
  data->SetOrigin(0, 0, 0);
  data->SetSpacing(1, 1, 1);
  vtkSmartPointer<vtkFloatArray> tensors = vtkSmartPointer<vtkFloatArray>::New();
  tensors->SetName("ArchetypeReader");

  /// DIFFUSION TENSOR MACRO

#define vtkITKExecuteDataFromFileDiffusionTensor3D(typeN, type) \
    case typeN: \
    {\
      typedef itk::DiffusionTensor3D<type>    DiffusionTensor3DPixelType##typeN;\
      typedef itk::Image<DiffusionTensor3DPixelType##typeN,3> ImageType##typeN;\
      typedef itk::ImageSource<ImageType##typeN> FilterType; \
      FilterType::Pointer filter; \
      typedef itk::ImageFileReader<\
        ImageType##typeN > ReaderType##typeN; \
      ReaderType##typeN::Pointer reader = ReaderType##typeN::New();\
      reader->SetFileName(this->FileNames[0].c_str()); \
      if (this->UseNativeCoordinateOrientation) \
        { \
        filter = reader; \
        } \
      else \
        { \
        itk::OrientImageFilter<ImageType##typeN,ImageType##typeN>::Pointer orient2##typeN = \
              itk::OrientImageFilter<ImageType##typeN,ImageType##typeN>::New(); \
        if (this->Debug) {orient2##typeN->DebugOn();} \
        orient2##typeN->SetInput(reader->GetOutput()); \
        orient2##typeN->UseImageDirectionOn(); \
        orient2##typeN->SetDesiredCoordinateOrientation(this->DesiredCoordinateOrientation); \
        filter = orient2##typeN; \
        } \
      filter->UpdateLargestPossibleRegion();\
      itk::ImageRegionConstIteratorWithIndex< ImageType##typeN >\
        it( reader->GetOutput(), reader->GetOutput()->GetLargestPossibleRegion() ); \
      tensors->SetNumberOfComponents(9);\
      tensors->SetNumberOfTuples(data->GetNumberOfPoints());\
      tensors->Modified();\
      it.GoToBegin();\
      while( !it.IsAtEnd() )\
      {\
        const itk::Index<3u> index = it.GetIndex();\
        vtkIdType position = data->FindPoint(index[0], index[1], index[2]);\
        float value[9];\
        const DiffusionTensor3DPixelType##typeN tensor = it.Get();\
        for (int i=0; i<3; i++)\
          {\
          for(int j=0; j<3; j++)\
            {\
            value[i + 3 * j] = float(tensor(i, j));\
            }\
          }\
        tensors->SetTupleValue(position, value);\
        ++it;\
      }\
    }\
    break
  // END DIFFUSION TENSOR MACRO


    // If there is only one file in the series, just use an image file reader
  if (this->FileNames.size() == 1)
    {
    vtkDebugMacro("DiffusionTensorImageReaderFile: only one file: " << this->FileNames[0].c_str());
    switch (this->OutputScalarType)
      {
      vtkITKExecuteDataFromFileDiffusionTensor3D(VTK_DOUBLE, double);
      vtkITKExecuteDataFromFileDiffusionTensor3D(VTK_FLOAT, float);
      vtkITKExecuteDataFromFileDiffusionTensor3D(VTK_LONG, long);
      vtkITKExecuteDataFromFileDiffusionTensor3D(VTK_UNSIGNED_LONG, unsigned long);
      vtkITKExecuteDataFromFileDiffusionTensor3D(VTK_INT, int);
      vtkITKExecuteDataFromFileDiffusionTensor3D(VTK_UNSIGNED_INT, unsigned int);
      vtkITKExecuteDataFromFileDiffusionTensor3D(VTK_SHORT, short);
      vtkITKExecuteDataFromFileDiffusionTensor3D(VTK_UNSIGNED_SHORT, unsigned short);
      vtkITKExecuteDataFromFileDiffusionTensor3D(VTK_CHAR, char);
      vtkITKExecuteDataFromFileDiffusionTensor3D(VTK_UNSIGNED_CHAR, unsigned char);
      default:
        vtkErrorMacro(<< "UpdateFromFile: Unknown data type " << this->OutputScalarType);
      }
      data->GetPointData()->SetTensors(tensors);
    }    
  else
    {
    // ERROR - should have used the series reader
    vtkErrorMacro("There is more than one file, use the DiffusionTensor3DReaderSeries instead");
    }
}


void vtkITKArchetypeDiffusionTensorImageReaderFile::ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&,void* data)
{
  vtkITKArchetypeDiffusionTensorImageReaderFile* me=reinterpret_cast<vtkITKArchetypeDiffusionTensorImageReaderFile*>(data);
  me->Progress=obj->GetProgress();
  me->InvokeEvent(vtkCommand::ProgressEvent,&me->Progress);
}
