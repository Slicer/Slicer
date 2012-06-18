// Slicer includes
#include "TractographyLabelMapSeedingCLP.h"
#include "vtkDiffusionTensorMathematics.h"
#include "vtkSeedTracts.h"

// ITK includes
#include <itkConfigure.h>
#if ITK_VERSION_MAJOR >= 4
#include <itkFloatingPointExceptions.h>
#endif

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkITKArchetypeDiffusionTensorImageReaderFile.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkPolyDataWriter.h>
#include <vtkMath.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageThreshold.h>
#include <vtkPointData.h>
#include <vtksys/SystemTools.hxx>

#include <string>

int main( int argc, char * argv[] )
{

#if ITK_VERSION_MAJOR >= 4
  itk::FloatingPointExceptions::Disable();
#endif

  PARSE_ARGS;
  try
    {
    vtkSmartPointer<vtkITKArchetypeDiffusionTensorImageReaderFile> reader = vtkSmartPointer<vtkITKArchetypeDiffusionTensorImageReaderFile>::New();
    reader->SetArchetype(InputVolume.c_str() );
    reader->SetSingleFile(1);
    reader->Update();

    if( reader->GetOutput()->GetPointData()->GetTensors() == NULL )
      {
      std::cerr << argv[0] << ": No tensor data" << std::endl;
      return EXIT_FAILURE;
      }


    vtkSmartPointer<vtkImageData> ROI;
    vtkSmartPointer<vtkITKArchetypeImageSeriesReader> reader2 = vtkSmartPointer<vtkITKArchetypeImageSeriesReader>::New();
    vtkSmartPointer<vtkImageCast> imageCast = vtkSmartPointer<vtkImageCast>::New();
    vtkSmartPointer<vtkDiffusionTensorMathematics> math = vtkSmartPointer<vtkDiffusionTensorMathematics>::New();
    vtkSmartPointer<vtkImageThreshold> th = vtkSmartPointer<vtkImageThreshold>::New();
    vtkSmartPointer<vtkMatrix4x4> ROIRASToIJK = vtkSmartPointer<vtkMatrix4x4>::New();

    if (InputROI.length() > 0)
    {
      reader2->AddFileName(InputROI.c_str() );
      reader2->Update();

      if( reader2->GetOutput()->GetPointData()->GetScalars() == NULL )
        {
        std::cerr << argv[0] << ": No roi data" << std::endl;
        return EXIT_FAILURE;
        }

      // cast roi to short data type
      imageCast->SetOutputScalarTypeToShort();
      imageCast->SetInput(reader2->GetOutput() );
      imageCast->Update();

      ROI = imageCast->GetOutput();

      // Set up the matrix that will take points in ROI
      // to RAS space.  Code assumes this is world space
      // since  we have no access to external transforms.
      // This will only work if no transform is applied to
      // ROI and tensor volumes.
      //
      ROIRASToIJK->DeepCopy(reader2->GetRasToIjkMatrix() );
    } else { // If the mask does not exist, create one
      math->SetInput(0, reader->GetOutput());

      if( StoppingMode == std::string("LinearMeasurement") || StoppingMode == std::string("LinearMeasure") )
        {
        math->SetOperationToLinearMeasure();
        }
      else if( StoppingMode == std::string("PlanarMeasurement") || StoppingMode == std::string("PlanarMeasure") )
        {
        math->SetOperationToPlanarMeasure();
        }
      else if( StoppingMode == std::string("FractionalAnisotropy") )
        {
        math->SetOperationToFractionalAnisotropy();
        }
      else
        {
        std::cerr << "Mode " << StoppingMode << " is not supported" << endl;
        return EXIT_FAILURE;
        }
      math->Update();

      th->SetInput(math->GetOutput());
      th->ThresholdBetween(ClTh,1);
      th->SetInValue(ROIlabel);
      th->SetOutValue(0);
      th->ReplaceInOn();
      th->ReplaceOutOn();
      th->SetOutputScalarTypeToShort();
      th->Update();
      ROI = th->GetOutput();

      // Set up the matrix that will take points in ROI
      // to RAS space.  Code assumes this is world space
      // since  we have no access to external transforms.
      // This will only work if no transform is applied to
      // ROI and tensor volumes.
      ROIRASToIJK->DeepCopy(reader->GetRasToIjkMatrix() );
    }


    vtkSmartPointer<vtkSeedTracts> seed = vtkSmartPointer<vtkSeedTracts>::New();

    // 1. Set Input
    seed->SetInputTensorField(reader->GetOutput() );

    // 2. Set Up matrices
    vtkSmartPointer<vtkMatrix4x4> TensorRASToIJK = vtkSmartPointer<vtkMatrix4x4>::New();
    TensorRASToIJK->DeepCopy(reader->GetRasToIjkMatrix() );

    // VTK seeding is in ijk space with voxel scale included.
    // Calculate the matrix that goes from tensor "scaled IJK",
    // the array with voxels that know their size (what vtk sees for tract seeding)
    // to our RAS.
    double sp[3];
    reader->GetOutput()->GetSpacing(sp);
    vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
    trans->Identity();
    trans->PreMultiply();
    trans->SetMatrix(TensorRASToIJK);
    // Trans from IJK to RAS
    trans->Inverse();
    // Take into account spacing (remove from matrix) to compute Scaled IJK to RAS matrix
    trans->Scale(1 / sp[0], 1 / sp[1], 1 / sp[2]);
    trans->Inverse();

    // Set Transformation to seeding class
    seed->SetWorldToTensorScaledIJK(trans);

    // Rotation part of matrix is only thing tensor is transformed by.
    // This is to transform output tensors into RAS space.
    // Tensors are output along the fibers.
    // This matrix is not used for calculating tractography.
    // The following should be replaced with finite strain method
    // rather than assuming rotation part of the matrix according to
    // slicer convention.
    vtkSmartPointer<vtkMatrix4x4> TensorRASToIJKRotation = vtkSmartPointer<vtkMatrix4x4>::New();
    TensorRASToIJKRotation->DeepCopy(TensorRASToIJK);
    // Set Translation to zero
    for( int i = 0; i < 3; i++ )
      {
      TensorRASToIJKRotation->SetElement(i, 3, 0);
      }
    // Remove scaling in rasToIjk to make a real rotation matrix
    double col[3];
    for( int jjj = 0; jjj < 3; jjj++ )
      {
      for( int iii = 0; iii < 3; iii++ )
        {
        col[iii] = TensorRASToIJKRotation->GetElement(iii, jjj);
        }
      vtkMath::Normalize(col);
      for( int iii = 0; iii < 3; iii++ )
        {
        TensorRASToIJKRotation->SetElement(iii, jjj, col[iii]);
        }
      }
    TensorRASToIJKRotation->Invert();
    seed->SetTensorRotationMatrix(TensorRASToIJKRotation);

    // vtkSmartPointer<vtkNRRDWriter> iwriter = vtkSmartPointer<vtkNRRDWriter>::New();

    // 3. Set up ROI (not based on Cl mask), from input now

    vtkSmartPointer<vtkTransform> trans2 = vtkSmartPointer<vtkTransform>::New();
    trans2->Identity();
    trans2->PreMultiply();

    // no longer assume this ROI is in tensor space
    // trans2->SetMatrix(TensorRASToIJK);
    trans2->SetMatrix(ROIRASToIJK);
    trans2->Inverse();
    seed->SetROIToWorld(trans2);


    // PENDING: Do merging with input ROI

    seed->SetInputROI(ROI);
    seed->SetInputROIValue(ROIlabel);
    seed->UseStartingThresholdOn();
    seed->SetStartingThreshold(ClTh);


    // 4. Set Tractography specific parameters

    if( WriteToFile )
      {
      seed->SetFileDirectoryName(OutputDirectory.c_str() );
      if( FilePrefix.length() > 0 )
        {
        seed->SetFilePrefix(FilePrefix.c_str() );
        }
      }

    if( UseIndexSpace )
      {
      seed->SetIsotropicSeeding(0);
      }
    else
      {
      seed->SetIsotropicSeeding(1);
      }

    seed->SetIsotropicSeedingResolution(SeedSpacing);
    seed->SetMinimumPathLength(MinimumLength);
    seed->UseVtkHyperStreamlinePoints();
    vtkSmartPointer<vtkHyperStreamlineDTMRI> streamer = vtkSmartPointer<vtkHyperStreamlineDTMRI>::New();
    seed->SetVtkHyperStreamlinePointsSettings(streamer);

    if( StoppingMode == std::string("LinearMeasurement") || StoppingMode == std::string("LinearMeasure") )
      {
      streamer->SetStoppingModeToLinearMeasure();
      }
    else if( StoppingMode == std::string("PlanarMeasurement") || StoppingMode == std::string("PlanarMeasure") )
      {
      streamer->SetStoppingModeToPlanarMeasure();
      }
    else if( StoppingMode == std::string("FractionalAnisotropy") )
      {
      streamer->SetStoppingModeToFractionalAnisotropy();
      }
    else
      {
      std::cerr << "Mode " << StoppingMode << " is not supported" << endl;
      return EXIT_FAILURE;
      }

    streamer->SetStoppingThreshold(StoppingValue);
    streamer->SetMaximumPropagationDistance(MaximumLength);
    streamer->SetRadiusOfCurvature(StoppingCurvature);
    streamer->SetIntegrationStepLength(IntegrationStepLength);

    // Temp fix to provide a scalar
    // seed->GetInputTensorField()->GetPointData()->SetScalars(math->GetOutput()->GetPointData()->GetScalars());

    // 5. Run the thing
    seed->SeedStreamlinesInROI();

    // 6. Extra5ct PolyData in RAS
    vtkSmartPointer<vtkPolyData> outFibers = vtkSmartPointer<vtkPolyData>::New();

    // Save result
    if ( !WriteToFile )
      {
      std::string fileExtension = vtksys::SystemTools::GetFilenameLastExtension(OutputFibers.c_str());
      if (fileExtension == ".vtk")
        {
          vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
          seed->TransformStreamlinesToRASAndAppendToPolyData(outFibers);
          writer->SetFileName(OutputFibers.c_str());
          writer->SetFileTypeToBinary();
          writer->SetInput(outFibers);
          writer->Write();
        }
      else 
        {
        if (fileExtension != ".vtp")
          {
          cerr << "Extension not recognize, saving the information in VTP format" << endl;
          }
        vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
        seed->TransformStreamlinesToRASAndAppendToPolyData(outFibers);
        writer->SetFileName(OutputFibers.c_str() );
        writer->SetInput(outFibers);
        writer->Write();
        }
      }
  }
  catch( ... )
    {
    cout << "default exception";
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
