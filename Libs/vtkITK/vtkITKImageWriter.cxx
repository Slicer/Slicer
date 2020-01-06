/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

// vtkITK includes
#include "vtkITKImageWriter.h"

// VTK includes
#include <vtkFloatArray.h>
#include <vtkImageExport.h>
#include <vtkImageFlip.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkITKUtility.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVersion.h>

// VTKsys includes
#include <vtksys/SystemTools.hxx>

// ITK includes
#include <itkDiffusionTensor3D.h>
#include <itkImageFileWriter.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <itkMetaDataObjectBase.h>
#include <itkVTKImageImport.h>


vtkStandardNewMacro(vtkITKImageWriter);

// helper function
template <class  TPixelType, int Dimension>
void ITKWriteVTKImage(vtkITKImageWriter *self, vtkImageData *inputImage, char *fileName,
                      vtkMatrix4x4* rasToIjkMatrix, vtkMatrix4x4* MeasurementFrameMatrix=nullptr) {

  typedef  itk::Image<TPixelType, Dimension> ImageType;

  vtkMatrix4x4 *ijkToRasMatrix = vtkMatrix4x4::New();

  if (rasToIjkMatrix == nullptr)
    {
    std::cerr << "ITKWriteVTKImage: rasToIjkMatrix is null" << std::endl;
    }
  else
    {
    vtkMatrix4x4::Invert(rasToIjkMatrix, ijkToRasMatrix);
    }
  ijkToRasMatrix->Transpose();

  typename ImageType::DirectionType direction;
  typename ImageType::PointType origin;
  direction.SetIdentity();

  double mag[3];
  int i;
  for (i=0; i<3; i++)
    {
    // normalize vectors
    mag[i] = 0;
    for (int j=0; j<3; j++)
      {
      mag[i] += ijkToRasMatrix->GetElement(i,j)* ijkToRasMatrix->GetElement(i,j);
      }
    if (mag[i] == 0.0)
      {
      mag[i] = 1;
      }
    mag[i] = sqrt(mag[i]);
    }

  for ( i=0; i<3; i++)
    {
    int j;
    for (j=0; j<3; j++)
      {
      ijkToRasMatrix->SetElement(i, j, ijkToRasMatrix->GetElement(i,j)/mag[i]);
      }
    }


  // ITK image direction are in LPS space
  // convert from ijkToRas to ijkToLps
  vtkMatrix4x4* rasToLpsMatrix = vtkMatrix4x4::New();
  rasToLpsMatrix->Identity();
  rasToLpsMatrix->SetElement(0,0,-1);
  rasToLpsMatrix->SetElement(1,1,-1);

  vtkMatrix4x4* ijkToLpsMatrix = vtkMatrix4x4::New();
  vtkMatrix4x4::Multiply4x4(ijkToRasMatrix, rasToLpsMatrix, ijkToLpsMatrix);

  for ( i=0; i<Dimension; i++)
    {
    origin[i] =  ijkToRasMatrix->GetElement(3,i);
    int j;
    for (j=0; j<Dimension; j++)
      {
      if (Dimension == 2)
        {
        direction[j][i] = (i == j) ? 1. : 0;
        }
      else
        {
        direction[j][i] =  ijkToLpsMatrix->GetElement(i,j);
        }
      }
    }

  rasToLpsMatrix->Delete();
  ijkToRasMatrix->Delete();
  ijkToLpsMatrix->Delete();

  origin[0] *= -1;
  origin[1] *= -1;

  // itk import for input itk images
  typedef typename itk::VTKImageImport<ImageType> ImageImportType;
  typename ImageImportType::Pointer itkImporter = ImageImportType::New();

  // vtk export for  vtk image
  vtkNew<vtkImageExport> vtkExporter;
  vtkNew<vtkImageFlip> vtkFlip;

  // writer
  typedef typename itk::ImageFileWriter<ImageType> ImageWriterType;
  typename ImageWriterType::Pointer   itkImageWriter =  ImageWriterType::New();

  if ( self->GetUseCompression() )
    {
    itkImageWriter->UseCompressionOn();
    }
    else
    {
    itkImageWriter->UseCompressionOff();
    }


  // set pipeline for the image
  vtkFlip->SetInputData( inputImage );
  vtkExporter->SetInputData ( inputImage );
  vtkFlip->SetFilteredAxis(1);
  vtkFlip->FlipAboutOriginOn();

  ConnectPipelines(vtkExporter.GetPointer(), itkImporter);

  // write image
  if(self->GetImageIOClassName())
    {
    itk::LightObject::Pointer objectType =
      itk::ObjectFactoryBase::CreateInstance(self->GetImageIOClassName());
    itk::ImageIOBase* imageIOType = dynamic_cast< itk::ImageIOBase * >(
      objectType.GetPointer());
    if(imageIOType){
      itkImageWriter->SetImageIO(imageIOType);
      }
    }
  itkImageWriter->SetInput(itkImporter->GetOutput());

  if (MeasurementFrameMatrix != nullptr)
    {
    typedef std::vector<std::vector<double> >    DoubleVectorType;
    typedef itk::MetaDataObject<DoubleVectorType>     MetaDataDoubleVectorType;
    const itk::MetaDataDictionary &        dictionary = itkImageWriter->GetMetaDataDictionary();

    itk::MetaDataDictionary::ConstIterator itr = dictionary.Begin();
    itk::MetaDataDictionary::ConstIterator end = dictionary.End();

    while( itr != end )
      {
      // Get Measurement Frame
      itk::MetaDataObjectBase::Pointer  entry = itr->second;
      MetaDataDoubleVectorType::Pointer entryvalue
        = dynamic_cast<MetaDataDoubleVectorType *>( entry.GetPointer() );
      if( entryvalue )
        {
        int pos = itr->first.find( "NRRD_measurement frame" );
        if( pos != -1 )
          {
          DoubleVectorType tagvalue;
          tagvalue.resize( 3 );
          for( int i = 0; i < 3; i++ )
            {
            tagvalue[i].resize( 3 );
            for( int j = 0; j < 3; j++ )
              {
              tagvalue[i][j] = MeasurementFrameMatrix->GetElement(i, j);
              }
            }
          entryvalue->SetMetaDataObjectValue( tagvalue );
          }
        }
        ++itr;
        }
      }

  try
    {
    itkImporter->GetOutput()->SetDirection(direction);
    itkImporter->GetOutput()->Update();
    itkImporter->GetOutput()->SetOrigin(origin);
    itkImporter->GetOutput()->SetSpacing(mag);
    itkImageWriter->SetFileName( fileName );
    itkImageWriter->Update();
    }
  catch (itk::ExceptionObject& exception)
    {
    exception.Print(std::cerr);
    throw exception;
    }
}

//----------------------------------------------------------------------------
template <class  TPixelType>
void ITKWriteVTKImage(vtkITKImageWriter *self, vtkImageData *inputImage, char *fileName,
                      vtkMatrix4x4* rasToIjkMatrix, vtkMatrix4x4* measurementFrameMatrix=nullptr)
{
  std::string fileExtension = vtksys::SystemTools::LowerCase( vtksys::SystemTools::GetFilenameLastExtension(fileName) );
  bool saveAsJPEG = (fileExtension == ".jpg") || (fileExtension == ".jpeg");
  if (saveAsJPEG)
    {
    ITKWriteVTKImage<TPixelType, 2>(self, inputImage, fileName, rasToIjkMatrix);
    }
  else // 3D
    {
    ITKWriteVTKImage<TPixelType, 3>(self, inputImage, fileName, rasToIjkMatrix, measurementFrameMatrix);
    }
}

//----------------------------------------------------------------------------
vtkITKImageWriter::vtkITKImageWriter()
{
  this->FileName = nullptr;
  this->RasToIJKMatrix = nullptr;
  this->MeasurementFrameMatrix = nullptr;
  this->UseCompression = 0;
  this->ImageIOClassName = nullptr;
}


//----------------------------------------------------------------------------
vtkITKImageWriter::~vtkITKImageWriter()
{
  // get rid of memory allocated for file names
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = nullptr;
    }

  if (this->ImageIOClassName)
    {
    delete [] this->ImageIOClassName;
    this->ImageIOClassName = nullptr;
    }
}


//----------------------------------------------------------------------------
void vtkITKImageWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "FileName: " <<
    (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "ImageIOClassName: " <<
    (this->ImageIOClassName ? this->ImageIOClassName : "(none)") << "\n";
}


//----------------------------------------------------------------------------
// This function sets the name of the file.
void vtkITKImageWriter::SetFileName(const char *name)
{
  if ( this->FileName && name && (!strcmp(this->FileName,name)))
    {
    return;
    }
  if (!name && !this->FileName)
    {
    return;
    }
  if (this->FileName)
    {
    delete [] this->FileName;
    }

  this->FileName = new char[strlen(name) + 1];
  strcpy(this->FileName, name);
  this->Modified();
}

//----------------------------------------------------------------------------
// Writes all the data from the input.
void vtkITKImageWriter::Write()
{
  vtkImageData* inputImage = this->GetImageDataInput(0);
  vtkPointData* pointData = nullptr;
  if (inputImage)
    {
    pointData = inputImage->GetPointData();
    }
  if (pointData == nullptr)
    {
    vtkErrorMacro(<<"vtkITKImageWriter: No image to write");
    return;
    }
  if ( ! this->FileName )
    {
    vtkErrorMacro(<<"vtkITKImageWriter: Please specify a FileName");
    return;
    }

  this->UpdateInformation();
  if (this->GetOutputInformation(0))
    {
    this->GetOutputInformation(0)->Set(
      vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
      this->GetOutputInformation(0)->Get(
        vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()), 6);
    }
  int inputDataType =
    pointData->GetScalars() ? pointData->GetScalars()->GetDataType() :
    pointData->GetTensors() ? pointData->GetTensors()->GetDataType() :
    pointData->GetVectors() ? pointData->GetVectors()->GetDataType() :
    pointData->GetNormals() ? pointData->GetNormals()->GetDataType() :
    0;
  int inputNumberOfScalarComponents =
    pointData->GetScalars() ? pointData->GetScalars()->GetNumberOfComponents() :
    pointData->GetTensors() ? pointData->GetTensors()->GetNumberOfComponents() :
    pointData->GetVectors() ? pointData->GetVectors()->GetNumberOfComponents() :
    pointData->GetNormals() ? pointData->GetNormals()->GetNumberOfComponents() :
    0;

  if (inputNumberOfScalarComponents == 1)
    {
    // take into consideration the scalar type
    switch (inputDataType)
      {
      case VTK_DOUBLE:
        {
        ITKWriteVTKImage<double>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_FLOAT:
        {
        ITKWriteVTKImage<float>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_LONG:
        {
        ITKWriteVTKImage<long>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_LONG:
        {
        ITKWriteVTKImage<unsigned long>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_INT:
        {
        ITKWriteVTKImage<int>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_INT:
        {
        ITKWriteVTKImage<unsigned int>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_SHORT:
        {
        ITKWriteVTKImage<short>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_SHORT:
        {
        ITKWriteVTKImage<unsigned short>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_CHAR:
        {
        ITKWriteVTKImage<char>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_CHAR:
        {
        ITKWriteVTKImage<unsigned char>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      default:
        vtkErrorMacro(<< "Execute: Unknown output ScalarType");
        return;
      }
    } // scalar
  else if (inputNumberOfScalarComponents == 3)
    {
    // take into consideration the scalar type
    switch (inputDataType)
      {
      case VTK_DOUBLE:
        {
        typedef itk::Vector<double, 3> VectorPixelType;
        ITKWriteVTKImage<VectorPixelType>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_FLOAT:
        {
        typedef itk::Vector<float, 3> VectorPixelType;
        ITKWriteVTKImage<VectorPixelType>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_SHORT:
        {
        typedef itk::Vector<unsigned short, 3> VectorPixelType;
        ITKWriteVTKImage<VectorPixelType>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_CHAR:
        {
        typedef itk::Vector<unsigned char, 3> VectorPixelType;
        ITKWriteVTKImage<VectorPixelType>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      default:
        vtkErrorMacro(<< "Execute: Unknown output ScalarType");
        return;
      }
    } // vector
  else if (inputNumberOfScalarComponents == 4)
    {
    // take into consideration the scalar type
    switch (inputDataType)
      {
      case VTK_DOUBLE:
        {
        typedef itk::Vector<double, 4> VectorPixelType;
        ITKWriteVTKImage<VectorPixelType>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_FLOAT:
        {
        typedef itk::Vector<float, 4> VectorPixelType;
        ITKWriteVTKImage<VectorPixelType>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_SHORT:
        {
        typedef itk::Vector<unsigned short, 4> VectorPixelType;
        ITKWriteVTKImage<VectorPixelType>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_CHAR:
        {
        typedef itk::Vector<unsigned char, 4> VectorPixelType;
        ITKWriteVTKImage<VectorPixelType>(this, inputImage, this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      default:
        vtkErrorMacro(<< "Execute: Unknown output ScalarType");
        return;
      }
    } // 4-vector
  else if (inputNumberOfScalarComponents == 9)
    {
    // take into consideration the scalar type
    switch (inputDataType)
      {
      case VTK_FLOAT:
        {
        typedef itk::DiffusionTensor3D<float> TensorPixelType;
        vtkNew<vtkImageData> outImage;
        outImage->SetDimensions(inputImage->GetDimensions());
        outImage->SetOrigin(0, 0, 0);
        outImage->SetSpacing(1, 1, 1);
        outImage->AllocateScalars(VTK_FLOAT, 6);
        vtkFloatArray* out = vtkFloatArray::SafeDownCast(outImage->GetPointData()->GetScalars());
        vtkFloatArray* in = vtkFloatArray::SafeDownCast(inputImage->GetPointData()->GetTensors());
        float inValue[9];
        float outValue[6];
        for(int i=0; i<out->GetNumberOfTuples(); i++)
          {
          in->GetTypedTuple(i, inValue);
          //ITK expect tensors saved in upper-triangular format
          outValue[0] = inValue[0];
          outValue[1] = inValue[1];
          outValue[2] = inValue[2];
          outValue[3] = inValue[4];
          outValue[4] = inValue[7];
          outValue[5] = inValue[8];
          out->SetTuple(i, outValue);
          }

        ITKWriteVTKImage<TensorPixelType>(this, outImage.GetPointer(),
          this->GetFileName(), this->RasToIJKMatrix, this->MeasurementFrameMatrix);
        }
        inputImage->GetPointData()->SetScalars(nullptr);
        break;
      default:
        vtkErrorMacro(<< "Execute: Unknown output ScalarType");
        return;
      }
    }
  else
    {
    vtkErrorMacro(<< "Can only export 1 or 3 component images, current image has " << inputNumberOfScalarComponents << " components");
    return;
    }
}
