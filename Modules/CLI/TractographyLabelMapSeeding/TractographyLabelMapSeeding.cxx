// Slicer includes
#include "TractographyLabelMapSeedingCLP.h"
#include "vtkDiffusionTensorMathematics.h"
#include "vtkSeedTracts.h"

// ITK includes
#include <itkConfigure.h>

// VTK includes
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageThreshold.h>
#include <vtkITKArchetypeDiffusionTensorImageReaderFile.h>
#include <vtkITKArchetypeImageSeriesScalarReader.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>
#include <vtkXMLPolyDataWriter.h>
#include <vtkVersion.h>

// STD includes
#include <string>

int main( int argc, char * argv[] )
{
  PARSE_ARGS;
  try
    {
    vtkNew<vtkITKArchetypeDiffusionTensorImageReaderFile> reader;
    reader->SetArchetype(InputVolume.c_str() );
    reader->SetSingleFile(1);
    reader->SetUseOrientationFromFile(1);
    reader->SetUseNativeOriginOn();
    reader->SetOutputScalarTypeToNative();
    reader->SetDesiredCoordinateOrientationToNative();
    reader->Update();

    if( reader->GetOutput()->GetPointData()->GetTensors() == NULL )
      {
      std::cerr << argv[0] << ": No tensor data" << std::endl;
      return EXIT_FAILURE;
      }


    vtkSmartPointer<vtkImageData> ROI;
#if (VTK_MAJOR_VERSION > 5)
    vtkSmartPointer<vtkInformation> ROIPipelineInfo;
#endif
    vtkNew<vtkITKArchetypeImageSeriesScalarReader> reader2;
    vtkNew<vtkImageCast> imageCast;
    vtkNew<vtkDiffusionTensorMathematics> math;
    vtkNew<vtkImageThreshold> th;
    vtkNew<vtkMatrix4x4> ROIRASToIJK;

    if (InputROI.length() > 0)
    {
      reader2->SetArchetype(InputROI.c_str());
      reader2->SetUseOrientationFromFile(1);
      reader2->SetUseNativeOriginOn();
      reader2->SetOutputScalarTypeToNative();
      reader2->SetDesiredCoordinateOrientationToNative();
      reader2->SetSingleFile(1);
      reader2->Update();

      if( reader2->GetOutput()->GetPointData()->GetScalars() == NULL )
        {
        std::cerr << argv[0] << ": No roi data" << std::endl;
        return EXIT_FAILURE;
        }

      // cast roi to short data type
      imageCast->SetOutputScalarTypeToShort();
#if (VTK_MAJOR_VERSION <=5)
      imageCast->SetInput(reader2->GetOutput() );
#else
      imageCast->SetInputConnection(reader2->GetOutputPort() );
#endif
      imageCast->Update();

      ROI = imageCast->GetOutput();
#if (VTK_MAJOR_VERSION > 5)
      ROIPipelineInfo = imageCast->GetOutputInformation(0);
#endif

      // Set up the matrix that will take points in ROI
      // to RAS space.  Code assumes this is world space
      // since  we have no access to external transforms.
      // This will only work if no transform is applied to
      // ROI and tensor volumes.
      //
      ROIRASToIJK->DeepCopy(reader2->GetRasToIjkMatrix() );
    } else { // If the mask does not exist, create one
#if (VTK_MAJOR_VERSION <=5)
      math->SetInput(0, reader->GetOutput());
#else
      math->SetInputConnection(0, reader->GetOutputPort());
#endif

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

#if (VTK_MAJOR_VERSION <=5)
      th->SetInput(math->GetOutput());
#else
      th->SetInputConnection(math->GetOutputPort());
#endif
      th->ThresholdBetween(ClTh,1);
      th->SetInValue(ROIlabel);
      th->SetOutValue(0);
      th->ReplaceInOn();
      th->ReplaceOutOn();
      th->SetOutputScalarTypeToShort();
      th->Update();
      ROI = th->GetOutput();
#if (VTK_MAJOR_VERSION > 5)
      ROIPipelineInfo = th->GetOutputInformation(0);
#endif

      // Set up the matrix that will take points in ROI
      // to RAS space.  Code assumes this is world space
      // since  we have no access to external transforms.
      // This will only work if no transform is applied to
      // ROI and tensor volumes.
      ROIRASToIJK->DeepCopy(reader->GetRasToIjkMatrix() );
    }


    vtkNew<vtkSeedTracts> seed;

    // 1. Set Input
    seed->SetInputTensorField(reader->GetOutput() );

    // 2. Set Up matrices
    vtkNew<vtkMatrix4x4> TensorRASToIJK;
    TensorRASToIJK->DeepCopy(reader->GetRasToIjkMatrix() );

    // VTK seeding is in ijk space with voxel scale included.
    // Calculate the matrix that goes from tensor "scaled IJK",
    // the array with voxels that know their size (what vtk sees for tract seeding)
    // to our RAS.
    double sp[3];
    reader->GetOutput()->GetSpacing(sp);
    vtkNew<vtkTransform> trans;
    trans->Identity();
    trans->PreMultiply();
    trans->SetMatrix(TensorRASToIJK.GetPointer());
    // Trans from IJK to RAS
    trans->Inverse();
    // Take into account spacing (remove from matrix) to compute Scaled IJK to RAS matrix
    trans->Scale(1 / sp[0], 1 / sp[1], 1 / sp[2]);
    trans->Inverse();

    // Set Transformation to seeding class
    seed->SetWorldToTensorScaledIJK(trans.GetPointer());

    // Rotation part of matrix is only thing tensor is transformed by.
    // This is to transform output tensors into RAS space.
    // Tensors are output along the fibers.
    // This matrix is not used for calculating tractography.
    // The following should be replaced with finite strain method
    // rather than assuming rotation part of the matrix according to
    // slicer convention.
    vtkNew<vtkMatrix4x4> TensorRASToIJKRotation;
    TensorRASToIJKRotation->DeepCopy(TensorRASToIJK.GetPointer());
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
    seed->SetTensorRotationMatrix(TensorRASToIJKRotation.GetPointer());

    // 3. Set up ROI (not based on Cl mask), from input now

    vtkNew<vtkTransform> trans2;
    trans2->Identity();
    trans2->PreMultiply();

    // no longer assume this ROI is in tensor space
    // trans2->SetMatrix(TensorRASToIJK.GetPointer());
    trans2->SetMatrix(ROIRASToIJK.GetPointer());
    trans2->Inverse();
    seed->SetROIToWorld(trans2.GetPointer());


    // PENDING: Do merging with input ROI

    seed->SetInputROI(ROI);
#if (VTK_MAJOR_VERSION > 5)
    seed->SetInputROIPipelineInfo(ROIPipelineInfo);
#endif
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
    vtkNew<vtkHyperStreamlineDTMRI> streamer;
    seed->SetVtkHyperStreamlinePointsSettings(streamer.GetPointer());

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

    // Save result
    if ( !WriteToFile )
      {
      // 6. Extra5ct PolyData in RAS
      vtkNew<vtkPolyData> outFibers;
      seed->TransformStreamlinesToRASAndAppendToPolyData(outFibers.GetPointer());

      std::string fileExtension = vtksys::SystemTools::LowerCase( vtksys::SystemTools::GetFilenameLastExtension(OutputFibers.c_str()) );
      if (fileExtension == ".vtk")
        {
          vtkNew<vtkPolyDataWriter> writer;
          writer->SetFileName(OutputFibers.c_str());
          writer->SetFileTypeToBinary();
#if (VTK_MAJOR_VERSION <= 5)
          writer->SetInput(outFibers.GetPointer());
#else
          writer->SetInputData(outFibers.GetPointer());
#endif
          writer->Write();
        }
      else
        {
        if (fileExtension != ".vtp")
          {
          cerr << "Extension not recognize, saving the information in VTP format" << endl;
          }
        vtkNew<vtkXMLPolyDataWriter> writer;
        writer->SetFileName(OutputFibers.c_str() );
#if (VTK_MAJOR_VERSION <= 5)
        writer->SetInput(outFibers.GetPointer());
#else
        writer->SetInputData(outFibers.GetPointer());
#endif
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
