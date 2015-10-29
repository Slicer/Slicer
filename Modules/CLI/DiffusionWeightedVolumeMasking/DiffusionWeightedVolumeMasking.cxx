
// SlicerBaseLogic includes
#include "vtkImageConnectivity.h"

// MRMLCore includes
#include <vtkMRMLNRRDStorageNode.h>

// vtkITK includes
#include <vtkITKNewOtsuThresholdImageFilter.h>

// vtkTeem includes
#include <vtkNRRDReader.h>
#include <vtkNRRDWriter.h>

// VTK includes
#include <vtkImageExtractComponents.h>
#include <vtkImageSeedConnectivity.h>
#include <vtkImageWeightedSum.h>
#include <vtkNew.h>
#include <vtkVersion.h>

// ITK includes
#include "itkFloatingPointExceptions.h"

// XXX # Workaround bug in packaging of DCMTK 3.6.0 on Debian.
//     # See http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=637687
#ifdef HAVE_CONFIG_H
#undef HAVE_CONFIG_H
#endif

#include "DiffusionWeightedVolumeMaskingCLP.h"

#define GRAD_0_TOL 1e-6

int main( int argc, char * argv[] )
{

  itk::FloatingPointExceptions::Disable();

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

    // Compute the mean baseline image
    vtkNew<vtkImageWeightedSum> imageWeightedSum;
    imageWeightedSum->NormalizeByWeightOn();

    int b0_count = 0;
    for( int gradient_n = 0; gradient_n < grads->GetNumberOfTuples(); gradient_n++ )
      {
      double* gradient = grads->GetTuple3(gradient_n);
      if( fabs(gradient[0]) + fabs(gradient[1]) + fabs(gradient[2]) < GRAD_0_TOL )
        {
        vtkNew<vtkImageExtractComponents> extractComponents;
#if (VTK_MAJOR_VERSION <= 5)
        extractComponents->SetInput(reader->GetOutput() );
#else
        extractComponents->SetInputConnection(reader->GetOutputPort() );
#endif
        extractComponents->SetComponents(gradient_n);
        extractComponents->Update();

        imageWeightedSum->AddInputConnection(extractComponents->GetOutputPort() );
        imageWeightedSum->SetWeight(b0_count++, 1.);
        }
      }
    imageWeightedSum->Update();

    if( b0_count == 0 )
      {
      std::cerr << argv[0] << ": Error parsing Diffusion information, no B0 images" << std::endl;
      return EXIT_FAILURE;
      }

    // compute DWI mask
    vtkNew<vtkITKNewOtsuThresholdImageFilter> otsu;
#if (VTK_MAJOR_VERSION <= 5)
    otsu->SetInput(imageWeightedSum->GetOutput() );
#else
    otsu->SetInputConnection(imageWeightedSum->GetOutputPort() );
#endif
    otsu->SetOmega(1 + otsuOmegaThreshold);
    otsu->SetOutsideValue(1);
    otsu->SetInsideValue(0);
    otsu->Update();

    vtkNew<vtkImageData> mask;
    mask->DeepCopy(otsu->GetOutput());

    int *dims = mask->GetDimensions();
    int  px = dims[0] / 2;
    int  py = dims[1] / 2;
    int  pz = dims[2] / 2;

    vtkNew<vtkImageCast> cast;
#if (VTK_MAJOR_VERSION <= 5)
    cast->SetInput(mask.GetPointer());
#else
    cast->SetInputData(mask.GetPointer());
#endif
    cast->SetOutputScalarTypeToUnsignedChar();
    cast->Update();

    vtkNew<vtkImageSeedConnectivity> con;
#if (VTK_MAJOR_VERSION <= 5)
    con->SetInput(cast->GetOutput() );
#else
    con->SetInputConnection(cast->GetOutputPort() );
#endif
    con->SetInputConnectValue(1);
    con->SetOutputConnectedValue(1);
    con->SetOutputUnconnectedValue(0);
    con->AddSeed(px, py, pz);
    con->Update();

    vtkNew<vtkImageCast> cast1;
#if (VTK_MAJOR_VERSION <= 5)
    cast1->SetInput(con->GetOutput() );
#else
    cast1->SetInputConnection(con->GetOutputPort() );
#endif
    cast1->SetOutputScalarTypeToShort();
    cast1->Update();

    vtkNew<vtkImageConnectivity> conn;

    if( removeIslands )
      {
      conn->SetBackground(1);
      conn->SetMinForeground( -32768);
      conn->SetMaxForeground( 32767);
      conn->SetFunctionToRemoveIslands();
      conn->SetMinSize(10000);
      conn->SliceBySliceOn();
#if (VTK_MAJOR_VERSION <= 5)
      conn->SetInput(cast1->GetOutput() );
#else
      conn->SetInputConnection(cast1->GetOutputPort() );
#endif
      conn->Update();
      }

    vtkNew<vtkImageCast> cast2;
    cast2->SetOutputScalarTypeToUnsignedChar();

    if( removeIslands )
      {
#if (VTK_MAJOR_VERSION <= 5)
      cast2->SetInput(conn->GetOutput() );
#else
      cast2->SetInputConnection(conn->GetOutputPort() );
#endif
      }
    else
      {
#if (VTK_MAJOR_VERSION <= 5)
      cast2->SetInput(cast1->GetOutput() );
#else
      cast2->SetInputConnection(cast1->GetOutputPort() );
#endif
      }

    vtkMatrix4x4* ijkToRasMatrix = reader->GetRasToIjkMatrix();
    ijkToRasMatrix->Invert();

    // Save baseline
    vtkNew<vtkNRRDWriter> writer;
    writer->SetInputConnection(imageWeightedSum->GetOutputPort() );
    writer->SetFileName( outputBaseline.c_str() );
    writer->UseCompressionOn();
    writer->SetIJKToRASMatrix( ijkToRasMatrix );
    writer->Write();

    // Save mask
    vtkNew<vtkNRRDWriter> writer2;
    if( removeIslands )
      {
#if (VTK_MAJOR_VERSION <= 5)
      writer2->SetInput(conn->GetOutput() );
#else
      writer2->SetInputConnection(conn->GetOutputPort() );
#endif
      }
    else
      {
#if (VTK_MAJOR_VERSION <= 5)
      writer2->SetInput(cast1->GetOutput() );
#else
      writer2->SetInputConnection(cast1->GetOutputPort() );
#endif
      }
    writer2->SetFileName( thresholdMask.c_str() );
    writer2->UseCompressionOn();
    writer2->SetIJKToRASMatrix( ijkToRasMatrix );
    writer2->Write();

    }

  return EXIT_SUCCESS;
}
