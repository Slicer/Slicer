
// vtkTeem includes
#include <vtkDiffusionTensorMathematics.h>
#include <vtkNRRDReader.h>
#include <vtkNRRDWriter.h>

// VTK includes
#include <vtkImageData.h>

// ITK includes
#include <itkFloatingPointExceptions.h>

#include "DiffusionTensorScalarMeasurementsCLP.h"

int main( int argc, char * argv[] )
{
  itk::FloatingPointExceptions::Disable();

  PARSE_ARGS;

  vtkNRRDReader *reader = vtkNRRDReader::New();
  reader->SetFileName(inputVolume.c_str() );
  reader->Update();

  if( reader->GetOutput()->GetPointData()->GetTensors() == NULL )
    {
    std::cerr << argv[0] << ": No tensor data" << std::endl;
    return EXIT_FAILURE;
    }
  vtkDiffusionTensorMathematics *math = vtkDiffusionTensorMathematics::New();

  math->SetInputConnection(0, reader->GetOutputPort() );

  if( operation == std::string("Trace") )
    {
    math->SetOperationToTrace();
    }
  else if( operation == std::string("Determinant") )
    {
    math->SetOperationToDeterminant();
    }
  else if( operation == std::string("RelativeAnisotropy") )
    {
    math->SetOperationToRelativeAnisotropy();
    }
  else if( operation == std::string("FractionalAnisotropy") )
    {
    math->SetOperationToFractionalAnisotropy();
    }
  else if( operation == std::string("Mode") )
    {
    math->SetOperationToMode();
    }
  else if( operation == std::string("LinearMeasurement") || operation == std::string("LinearMeasure") )
    {
    math->SetOperationToLinearMeasure();
    }
  else if( operation == std::string("PlanarMeasurement") || operation == std::string("PlanarMeasure") )
    {
    math->SetOperationToPlanarMeasure();
    }
  else if( operation == std::string("SphericalMeasurement") || operation == std::string("SphericalMeasure") )
    {
    math->SetOperationToSphericalMeasure();
    }
  else if( operation == std::string("MinEigenvalue") )
    {
    math->SetOperationToMinEigenvalue();
    }
  else if( operation == std::string("MidEigenvalue") )
    {
    math->SetOperationToMiddleEigenvalue();
    }
  else if( operation == std::string("MaxEigenvalue") )
    {
    math->SetOperationToMaxEigenvalue();
    }
  else if( operation == std::string("ColorByOrientation") )
    {
    math->SetOperationToColorByOrientation();
    }
  else if( operation == std::string("ColorByMode") )
    {
    math->SetOperationToColorByMode();
    }
  else if( operation == std::string("MaxEigenvalueProjectionX") )
    {
    math->SetOperationToMaxEigenvalueProjectionX();
    }
  else if( operation == std::string("MaxEigenvalueProjectionY") )
    {
    math->SetOperationToMaxEigenvalueProjectionY();
    }
  else if( operation == std::string("MaxEigenvalueProjectionZ") )
    {
    math->SetOperationToMaxEigenvalueProjectionZ();
    }
  else if( operation == std::string("RAIMaxEigenvecX") )
    {
    math->SetOperationToRAIMaxEigenvecX();
    }
  else if( operation == std::string("RAIMaxEigenvecY") )
    {
    math->SetOperationToRAIMaxEigenvecY();
    }
  else if( operation == std::string("RAIMaxEigenvecZ") )
    {
    math->SetOperationToRAIMaxEigenvecZ();
    }
  else if( operation == std::string("MaxEigenvecX") )
    {
    math->SetOperationToMaxEigenvecX();
    }
  else if( operation == std::string("MaxEigenvecY") )
    {
    math->SetOperationToMaxEigenvecY();
    }
  else if( operation == std::string("MaxEigenvecZ") )
    {
    math->SetOperationToMaxEigenvecZ();
    }

  else if( operation == std::string("D11") )
    {
    math->SetOperationToD11();
    }
  else if( operation == std::string("D22") )
    {
    math->SetOperationToD22();
    }
  else if( operation == std::string("D33") )
    {
    math->SetOperationToD33();
    }
  else if( operation == std::string("ParallelDiffusivity") )
    {
    math->SetOperationToParallelDiffusivity();
    }
  else if( operation == std::string("PerpendicularDiffusivity") )
    {
    math->SetOperationToPerpendicularDiffusivity();
    }
  else
    {
    std::cerr << argv[0] << ": Operation " << operation << "not supported" << std::endl;
    return EXIT_FAILURE;
    }

  math->Update();

  // Save result
  vtkNRRDWriter *writer = vtkNRRDWriter::New();
  writer->SetInputConnection(math->GetOutputPort() );
  writer->SetFileName( outputScalar.c_str() );
  writer->UseCompressionOn();
  // Compute IjkToRas (used by Writer)
  reader->GetRasToIjkMatrix()->Invert();
  writer->SetIJKToRASMatrix( reader->GetRasToIjkMatrix() );
  writer->Write();

  reader->Delete();
  writer->Delete();
  math->Delete();

  return EXIT_SUCCESS;
}
