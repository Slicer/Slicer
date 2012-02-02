
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
#include <vtkSmartPointer.h>

// ITK includes
#if ITK_VERSION_MAJOR >= 4
#include <itkFloatingPointExceptions.h>
#endif

#define MAT_TOL 1e-6
bool transformsNotEqual(const vtkMatrix4x4* mat1, const vtkMatrix4x4* mat2);

int main( int argc, char * argv[] )
{
#if ITK_VERSION_MAJOR >= 4
  itk::FloatingPointExceptions::Disable();
#endif
  bool applyMask = false;

  PARSE_ARGS;
    {
    vtkSmartPointer<vtkNRRDReader> reader =
      vtkSmartPointer<vtkNRRDReader>::New();
    reader->SetFileName(inputVolume.c_str() );
    reader->Update();
    if( reader->GetReadStatus() )
      {
      std::cerr << argv[0] << ": Error reading Diffusion file" << std::endl;
      return EXIT_FAILURE;
      }

    vtkSmartPointer<vtkDoubleArray> bValues =
      vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkDoubleArray> grads =
      vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkMRMLNRRDStorageNode> helper =
      vtkSmartPointer<vtkMRMLNRRDStorageNode>::New();

    if( !helper->ParseDiffusionInformation(reader, grads, bValues) )
      {
      std::cerr << argv[0] << ": Error parsing Diffusion information" << std::endl;
      return EXIT_FAILURE;
      }
    vtkSmartPointer<vtkTeemEstimateDiffusionTensor> estim =
      vtkSmartPointer<vtkTeemEstimateDiffusionTensor>::New();

    estim->SetInput(reader->GetOutput() );
    estim->SetNumberOfGradients(grads->GetNumberOfTuples() );
    estim->SetDiffusionGradients(grads);
    estim->SetBValues(bValues);
    estim->SetShiftNegativeEigenvalues(ShiftNegativeEigenvalues);

    // Compute Transformation that brings the gradients to ijk
    // double *sp = reader->GetOutput()->GetSpacing();
    vtkSmartPointer<vtkMatrix4x4> mf =
      vtkSmartPointer<vtkMatrix4x4>::New();
    mf->DeepCopy(reader->GetMeasurementFrameMatrix() );
    vtkSmartPointer<vtkMatrix4x4> rasToIjkRotation =
      vtkSmartPointer<vtkMatrix4x4>::New();
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

    vtkSmartPointer<vtkTransform> trans =
      vtkSmartPointer<vtkTransform>::New();
    trans->PostMultiply();
    trans->SetMatrix(mf);
    trans->Concatenate(rasToIjkRotation);
    trans->Update();

    estim->SetTransform(trans);
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
    vtkSmartPointer<vtkImageData> mask = vtkSmartPointer<vtkImageData>::New();
    if( strlen(inputMaskVolume.c_str() ) > 0 )
      {
      vtkSmartPointer<vtkNRRDReader> maskReader =
        vtkSmartPointer<vtkNRRDReader>::New();
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

      vtkSmartPointer<vtkImageCast> cast =
        vtkSmartPointer<vtkImageCast>::New();
      cast->SetInput(maskReader->GetOutput() );
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
    vtkSmartPointer<vtkTensorMask> tensorMask =
      vtkSmartPointer<vtkTensorMask>::New();
    tensorMask->SetNumberOfThreads(1);
    if( applyMask )
      {
      tensorMask->SetMaskAlpha(0.0);
      tensorMask->SetInput(tensorImage);
      tensorMask->SetMaskInput(mask);
      tensorMask->Update();
      tensorImage = tensorMask->GetOutput();
      }
    /**/
    // Compute IjkToRas (used by Writer)
    vtkSmartPointer<vtkMatrix4x4> ijkToRasMatrix = reader->GetRasToIjkMatrix();
    ijkToRasMatrix->Invert();

    // Save tensor
    vtkSmartPointer<vtkNRRDWriter> writer =
      vtkSmartPointer<vtkNRRDWriter>::New();
    tensorImage->GetPointData()->SetScalars(NULL);
    writer->SetInput(tensorImage);
    writer->SetFileName( outputTensor.c_str() );
    writer->UseCompressionOn();
    writer->SetIJKToRASMatrix( ijkToRasMatrix );
    // Compute measurement frame: Take into account that we have transformed
    // the gradients so tensor components are defined in ijk.
    rasToIjkRotation->Invert();
    writer->SetMeasurementFrameMatrix( rasToIjkRotation );
    writer->Write();

    // Save baseline
    vtkSmartPointer<vtkNRRDWriter> writer2 =
      vtkSmartPointer<vtkNRRDWriter>::New();
    writer2->SetInput(estim->GetBaseline() );
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
