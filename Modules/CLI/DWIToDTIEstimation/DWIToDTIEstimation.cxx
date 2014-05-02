
// DWIToDTIEstimation includes
#include "DWIToDTIEstimationCLP.h"

// MRML includes
#include <vtkMRMLNRRDStorageNode.h>

// vtkITK includes
#include <vtkITKNewOtsuThresholdImageFilter.h>

// vtkTeem includes
#include <vtkNRRDReader.h>
#include <vtkNRRDWriter.h>
#include <vtkTeemEstimateDiffusionTensor.h>
#include <vtkTensorMask.h>

// VTK includes
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkVersion.h>

// ITK includes
#include <itkFloatingPointExceptions.h>

#define MAT_TOL 1e-6
bool transformsNotEqual(const vtkMatrix4x4* mat1, const vtkMatrix4x4* mat2);

int main( int argc, char * argv[] )
{
  itk::FloatingPointExceptions::Disable();
  bool applyMask = false;

  PARSE_ARGS;
    {
    vtkNew<vtkNRRDReader> reader;
    reader->SetFileName(inputVolume.c_str() );
    reader->Update();
    if( reader->GetReadStatus() )
      {
      std::cerr << argv[0] << ": Error reading Diffusion file" << std::endl;
      return EXIT_FAILURE;
      }

    vtkNew<vtkDoubleArray> bValues;
    vtkNew<vtkDoubleArray> grads;
    vtkNew<vtkMRMLNRRDStorageNode> helper;

    if( !helper->ParseDiffusionInformation(reader.GetPointer(), grads.GetPointer(), bValues.GetPointer()) )
      {
      std::cerr << argv[0] << ": Error parsing Diffusion information" << std::endl;
      return EXIT_FAILURE;
      }
    vtkNew<vtkTeemEstimateDiffusionTensor> estim;

    estim->SetInputConnection(reader->GetOutputPort() );
    estim->SetNumberOfGradients(grads->GetNumberOfTuples() );
    estim->SetDiffusionGradients(grads.GetPointer());
    estim->SetBValues(bValues.GetPointer());
    estim->SetShiftNegativeEigenvalues(ShiftNegativeEigenvalues);

    // Compute Transformation that brings the gradients to ijk
    // double *sp = reader->GetOutput()->GetSpacing();
    vtkNew<vtkMatrix4x4> mf;
    mf->DeepCopy(reader->GetMeasurementFrameMatrix() );
    vtkNew<vtkMatrix4x4> rasToIjkRotation;
    rasToIjkRotation->DeepCopy(reader->GetRasToIjkMatrix() );
    // Set Translation to zero
    for( int i = 0; i < 3; i++ )
      {
      rasToIjkRotation->SetElement(i, 3, 0);
      }
    // Remove scaling in rasToIjk to make a real rotation matrix
    double col[3];
    for( int jjj = 0; jjj < 3; jjj++ )
      {
      for( int iii = 0; iii < 3; iii++ )
        {
        col[iii] = rasToIjkRotation->GetElement(iii, jjj);
        }
      vtkMath::Normalize(col);
      for( int iii = 0; iii < 3; iii++ )
        {
        rasToIjkRotation->SetElement(iii, jjj, col[iii]);
        }
      }

    vtkNew<vtkTransform> trans;
    trans->PostMultiply();
    trans->SetMatrix(mf.GetPointer());
    trans->Concatenate(rasToIjkRotation.GetPointer());
    trans->Update();

    estim->SetTransform(trans.GetPointer());
    if( estimationMethod == std::string("LS") )
      {
      estim->SetEstimationMethodToLLS();
      }
    else if( estimationMethod == std::string("WLS") )
      {
      estim->SetEstimationMethodToWLS();
      }
    estim->Update();
    vtkImageData *tensorImage = estim->GetOutput();
    tensorImage->GetPointData()->SetScalars(NULL);

    // Read the tensor mask
    vtkNew<vtkImageData> mask;
    if( strlen(inputMaskVolume.c_str() ) > 0 )
      {
      vtkNew<vtkNRRDReader> maskReader;
      maskReader->SetFileName(inputMaskVolume.c_str() );
      maskReader->Update();
      if( maskReader->GetReadStatus() )
        {
        std::cerr << argv[0] << ": Error reading Mask file" << std::endl;
        return EXIT_FAILURE;
        }

      // Check if the transforms are equal
      if( transformsNotEqual(maskReader->GetRasToIjkMatrix(), reader->GetRasToIjkMatrix() ) )
        {
        std::cerr << argv[0] << ": Error reading Mask file, wrong coordinate space" << std::endl;
        return EXIT_FAILURE;
        }

      vtkNew<vtkImageCast> cast;
      cast->SetInputConnection(maskReader->GetOutputPort() );
      cast->SetOutputScalarTypeToUnsignedChar();
      cast->Update();

      mask->DeepCopy(cast->GetOutput() );
      applyMask = true;
      }
    else
      {
      applyMask = false;
      }

    // Mask tensor
    vtkNew<vtkTensorMask> tensorMask;
    tensorMask->SetNumberOfThreads(1);
    if( applyMask )
      {
      tensorMask->SetMaskAlpha(0.0);
#if (VTK_MAJOR_VERSION <= 5)
      tensorMask->SetInput(tensorImage);
      tensorMask->SetMaskInput(mask.GetPointer());
#else
      tensorMask->SetInputData(tensorImage);
      tensorMask->SetMaskInputData(mask.GetPointer());
#endif
      tensorMask->Update();
      tensorImage = tensorMask->GetOutput();
      }
    /**/
    // Compute IjkToRas (used by Writer)
    vtkMatrix4x4* ijkToRasMatrix = reader->GetRasToIjkMatrix();
    ijkToRasMatrix->Invert();

    // Save tensor
    vtkNew<vtkNRRDWriter> writer;
    tensorImage->GetPointData()->SetScalars(NULL);
#if (VTK_MAJOR_VERSION <= 5)
    writer->SetInput(tensorImage);
#else
    writer->SetInputData(tensorImage);
#endif
    writer->SetFileName( outputTensor.c_str() );
    writer->UseCompressionOn();
    writer->SetIJKToRASMatrix( ijkToRasMatrix );
    // Compute measurement frame: Take into account that we have transformed
    // the gradients so tensor components are defined in ijk.
    rasToIjkRotation->Invert();
    writer->SetMeasurementFrameMatrix(rasToIjkRotation.GetPointer());
    writer->Write();

    // Save baseline
    vtkNew<vtkNRRDWriter> writer2;
#if (VTK_MAJOR_VERSION <= 5)
    writer2->SetInput(estim->GetBaseline() );
#else
    writer2->SetInputData(estim->GetBaseline() );
#endif
    writer2->SetFileName( outputBaseline.c_str() );
    writer2->UseCompressionOn();
    writer2->SetIJKToRASMatrix( ijkToRasMatrix );
    writer2->Write();

    return EXIT_SUCCESS;
    }
}

bool transformsNotEqual(const vtkMatrix4x4* mat1, const vtkMatrix4x4* mat2)
{
  for( int i = 0; i < 4; i++ )
    {
    for( int j = 0; j < 4; j++ )
      {
      if( fabs(mat1->GetElement(i, j) - mat2->GetElement(i, j) ) > MAT_TOL )
        {
        return true;
        }
      }
    }
  return false;
}
