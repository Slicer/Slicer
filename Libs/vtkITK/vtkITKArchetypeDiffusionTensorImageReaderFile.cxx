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


#include "vtkCommand.h"
#include "vtkDataArray.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"


#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkDiffusionTensor3D.h"
#include "itkOrientImageFilter.h"

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
template <class T>
void vtkITKExecuteDataFromFileDiffusionTensor3D(
  vtkITKArchetypeDiffusionTensorImageReaderFile* self,
  vtkFloatArray* tensors,
  vtkImageData *data)
{
  typedef itk::DiffusionTensor3D<T> DiffusionTensor3DPixelType;
  typedef itk::Image<DiffusionTensor3DPixelType,3> ImageType;
  typedef itk::ImageSource<ImageType> FilterType;
  typename FilterType::Pointer filter;
  typedef itk::ImageFileReader<ImageType> ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(self->GetFileName(0));
  if (self->GetUseNativeCoordinateOrientation())
    {
    filter = reader;
    }
  else
    {
    typename itk::OrientImageFilter<ImageType,ImageType>::Pointer orient2 =
      itk::OrientImageFilter<ImageType,ImageType>::New();
    orient2->SetDebug(self->GetDebug());
    orient2->SetInput(reader->GetOutput());
    orient2->UseImageDirectionOn();
    orient2->SetDesiredCoordinateOrientation(self->GetDesiredCoordinateOrientation());
    filter = orient2;
    }
  // Check pixel type. It must be vector for DTI
  reader->UpdateOutputInformation();

  // This is a conservative test for dti. It filters out most non-dti
  // data without having to read the entire file
  unsigned int numberOfComponents = reader->GetImageIO()->GetNumberOfComponents();
  if (numberOfComponents != 9 && numberOfComponents != 6)
    {
    itkGenericExceptionMacro(<< "number of components is: "
                             << numberOfComponents
                             << " but expected 6 or 9");

    return;
    }

  // pixel type and number of components are correct. OK to read image data
  filter->UpdateLargestPossibleRegion();
  itk::ImageRegionConstIteratorWithIndex< ImageType >
    it( reader->GetOutput(), reader->GetOutput()->GetLargestPossibleRegion() );
  tensors->SetNumberOfComponents(9);
  tensors->SetNumberOfTuples(data->GetNumberOfPoints());
  tensors->Modified();
  for ( it.GoToBegin(); !it.IsAtEnd() ; ++it )
    {
    const itk::Index<3u> index = it.GetIndex();
    vtkIdType position = data->FindPoint(index[0], index[1], index[2]);
    float value[9];
    const DiffusionTensor3DPixelType tensor = it.Get();
    for (int i=0; i<3; i++)
      {
      for(int j=0; j<3; j++)
        {
        value[i + 3 * j] = float(tensor(i, j));
        }
      }
    tensors->SetTupleValue(position, value);
    }
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

    // If there is only one file in the series, just use an image file reader
  if (this->FileNames.size() == 1)
    {
    vtkDebugMacro("DiffusionTensorImageReaderFile: only one file: " << this->FileNames[0].c_str());
    switch (this->OutputScalarType)
      {
      vtkTemplateMacro(vtkITKExecuteDataFromFileDiffusionTensor3D<VTK_TT>(
        this, tensors, data));
      default:
        vtkErrorMacro(<< "UpdateFromFile: Unknown data type " << this->OutputScalarType);
      }
    data->GetPointData()->SetTensors(tensors);
    }
  else
    {
    // ERROR - should have used the series reader
    vtkErrorMacro("There is more than one file, use the vtkITKArchetypeImageSeriesReader instead");
    }
}

void vtkITKArchetypeDiffusionTensorImageReaderFile::ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&,void* data)
{
  vtkITKArchetypeDiffusionTensorImageReaderFile* me=reinterpret_cast<vtkITKArchetypeDiffusionTensorImageReaderFile*>(data);
  me->Progress=obj->GetProgress();
  me->InvokeEvent(vtkCommand::ProgressEvent,&me->Progress);
}
