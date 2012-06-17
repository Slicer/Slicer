/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#include "vtkITKImageWriter.h"

#include "vtkSmartPointer.h"
#include "vtkImageExport.h"
#include "vtkImageFlip.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"

#include "itkMetaDataDictionary.h"
#include "itkMetaDataObjectBase.h"
#include "itkMetaDataObject.h"
#include "itkVTKImageImport.h"
#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3D.h"
#include "vtkITKUtility.h"

// VTKsys includes
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkITKImageWriter);
vtkCxxRevisionMacro(vtkITKImageWriter, "$Revision$")

// helper function
template <class  TPixelType, int Dimension>
void ITKWriteVTKImage(vtkITKImageWriter *self, vtkImageData *inputImage, char *fileName,
                      vtkMatrix4x4* rasToIjkMatrix, vtkMatrix4x4* MeasurementFrameMatrix=NULL) {

  typedef  itk::Image<TPixelType, Dimension> ImageType;

  vtkMatrix4x4 *ijkToRasMatrix = vtkMatrix4x4::New();

  if (rasToIjkMatrix == NULL)
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
  for (i=0; i<3; i++) {
    // normalize vectors
    mag[i] = 0;
    for (int j=0; j<3; j++) {
      mag[i] += ijkToRasMatrix->GetElement(i,j)* ijkToRasMatrix->GetElement(i,j);
    }
    if (mag[i] == 0.0) {
      mag[i] = 1;
    }
    mag[i] = sqrt(mag[i]);
    //if (i == 1) { // Y flip
      //mag[i] = -mag[i];
    //}
  }

  for ( i=0; i<3; i++) {
    int j;
    for (j=0; j<3; j++) {
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

  for ( i=0; i<Dimension; i++) {
    origin[i] =  ijkToRasMatrix->GetElement(3,i);
    int j;
    for (j=0; j<Dimension; j++) {
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
  vtkImageExport* vtkExporter = vtkImageExport::New();  
  vtkImageFlip* vtkFlip = vtkImageFlip::New();

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
  vtkFlip->SetInput( inputImage );
  vtkFlip->SetFilteredAxis(1);
  vtkFlip->FlipAboutOriginOn();

  vtkExporter->SetInput ( inputImage );

  ConnectPipelines(vtkExporter, itkImporter);

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

  if (MeasurementFrameMatrix != NULL)
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
  // clean up
  vtkExporter->Delete();
  vtkFlip->Delete();
}

//----------------------------------------------------------------------------
template <class  TPixelType>
void ITKWriteVTKImage(vtkITKImageWriter *self, vtkImageData *inputImage, char *fileName,
                      vtkMatrix4x4* rasToIjkMatrix, vtkMatrix4x4* measurementFrameMatrix=NULL)
{
  std::string fileExtension = vtksys::SystemTools::GetFilenameLastExtension(fileName);
  bool saveAsJPEG =
    (fileExtension == ".jpg") || (fileExtension == ".JPG") ||
    (fileExtension == ".jpeg") || (fileExtension == ".JPEG");
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
  this->FileName = NULL;
  this->RasToIJKMatrix = NULL;
  this->MeasurementFrameMatrix = NULL;
  this->UseCompression = 0;
  this->ImageIOClassName = NULL;
}


//----------------------------------------------------------------------------
vtkITKImageWriter::~vtkITKImageWriter()
{
  // get rid of memory allocated for file names
  if (this->FileName) {
    delete [] this->FileName;
    this->FileName = NULL;
  }

  if (this->ImageIOClassName) {
    delete [] this->ImageIOClassName;
    this->ImageIOClassName = NULL;
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
void vtkITKImageWriter::SetInput(vtkImageData *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

//----------------------------------------------------------------------------
vtkImageData *vtkITKImageWriter::GetInput()
{
  if (this->NumberOfInputs < 1) {
    return NULL;
  }
  
  return (vtkImageData *)(this->Inputs[0]);
}



//----------------------------------------------------------------------------
// This function sets the name of the file. 
void vtkITKImageWriter::SetFileName(const char *name)
{
  if ( this->FileName && name && (!strcmp(this->FileName,name))) {
    return;
  }
  if (!name && !this->FileName) {
    return;
  }
  if (this->FileName) {
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
  if ( this->GetInput() == NULL ) {
    return;
  }
  if ( ! this->FileName ) {
    vtkErrorMacro(<<"vtkITKImageWriter: Please specify a FileName");
    return;
  }

  this->GetInput()->UpdateInformation();
  this->GetInput()->SetUpdateExtent(this->GetInput()->GetWholeExtent());


  if (this->GetInput()->GetNumberOfScalarComponents() == 1) {

    // take into consideration the scalar type
    switch (this->GetInput()->GetScalarType())
      {
      case VTK_DOUBLE:
        {
        ITKWriteVTKImage<double>(this, this->GetInput(),
          this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_FLOAT:
        {
        ITKWriteVTKImage<float>(this, this->GetInput(),
          this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_LONG:
        {
        ITKWriteVTKImage<long>(this, this->GetInput(),
          this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_LONG:
        {
        ITKWriteVTKImage<unsigned long>(this, this->GetInput(),
          this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_INT:
        {
        ITKWriteVTKImage<int>(this, this->GetInput(),
          this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_INT:
        {
        ITKWriteVTKImage<unsigned int>(this, this->GetInput(),
          this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_SHORT:
        {
        ITKWriteVTKImage<short>(this, this->GetInput(),
          this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_SHORT:
        {
        ITKWriteVTKImage<unsigned short>(this, this->GetInput(),
          this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_CHAR:
        {
        ITKWriteVTKImage<char>(this, this->GetInput(),
          this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_CHAR:
        {
        ITKWriteVTKImage<unsigned char>(this, this->GetInput(),
          this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      default:
        vtkErrorMacro(<< "Execute: Unknown output ScalarType");
        return; 
      }
  } // scalar

  else if (this->GetInput()->GetNumberOfScalarComponents() == 3) {

    // take into consideration the scalar type
    switch (this->GetInput()->GetScalarType())
      {
      case VTK_DOUBLE:
        {
        typedef itk::Vector<double, 3> VectorPixelType;
        ITKWriteVTKImage<VectorPixelType>(this, this->GetInput(),
          this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_FLOAT:
        {
        typedef itk::Vector<float, 3> VectorPixelType;
        ITKWriteVTKImage<VectorPixelType>(this, this->GetInput(),
          this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_SHORT:
        {
        typedef itk::Vector<unsigned short, 3> VectorPixelType;
        ITKWriteVTKImage<VectorPixelType>(this, this->GetInput(), this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      case VTK_UNSIGNED_CHAR:
        {
        typedef itk::Vector<unsigned char, 3> VectorPixelType;
        ITKWriteVTKImage<VectorPixelType>(this, this->GetInput(), this->GetFileName(), this->RasToIJKMatrix);
        }
        break;
      default:
        vtkErrorMacro(<< "Execute: Unknown output ScalarType");
        return; 
      }
  } // vector
  else if (this->GetInput()->GetNumberOfScalarComponents() == 9) {

    // take into consideration the scalar type
    switch (this->GetInput()->GetScalarType())
      {
      case VTK_FLOAT:
        {
        typedef itk::DiffusionTensor3D<float> TensorPixelType;
        vtkSmartPointer<vtkImageData> outImage = vtkSmartPointer<vtkImageData>::New();
        outImage->SetDimensions(this->GetInput()->GetDimensions());
        outImage->SetWholeExtent(this->GetInput()->GetWholeExtent());
        outImage->SetOrigin(0, 0, 0);
        outImage->SetSpacing(1, 1, 1);
        outImage->SetNumberOfScalarComponents(6);
        outImage->SetScalarTypeToFloat();
        outImage->AllocateScalars();
        vtkFloatArray* out = vtkFloatArray::SafeDownCast(outImage->GetPointData()->GetScalars());
        vtkFloatArray* in = vtkFloatArray::SafeDownCast(this->GetInput()->GetPointData()->GetTensors());
        float inValue[9];
        float outValue[6];
        for(int i=0; i<out->GetNumberOfTuples(); i++)
          {
          in->GetTupleValue(i, inValue);
          //ITK expect tensors saved in upper-triangular format
          outValue[0] = inValue[0];
          outValue[1] = inValue[1];
          outValue[2] = inValue[2];
          outValue[3] = inValue[4];
          outValue[4] = inValue[7];
          outValue[5] = inValue[8];
          out->SetTuple(i, outValue);
          }

        ITKWriteVTKImage<TensorPixelType>(this, outImage,
          this->GetFileName(), this->RasToIJKMatrix, this->MeasurementFrameMatrix);
        }
        this->GetInput()->GetPointData()->SetScalars(NULL);
        break;
      default:
        vtkErrorMacro(<< "Execute: Unknown output ScalarType");
        return; 
      }
  }
  else {
    vtkErrorMacro(<< "Can only export 1 or 3 component images");
    return; 
  }
}




