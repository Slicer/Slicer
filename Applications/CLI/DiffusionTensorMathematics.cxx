#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include <iostream>
#include <algorithm>
#include <string>

#include "itkPluginFilterWatcher.h"
#include "itkPluginUtilities.h"

#include "vtkDiffusionTensorMathematics.h"

#include "vtkNRRDReader.h"
#include "vtkNRRDWriter.h"
#include "vtkImageData.h"
#include "DiffusionTensorMathematicsCLP.h"


int main( int argc, const char * argv[] )
{

  PARSE_ARGS;

  vtkNRRDReader *reader = vtkNRRDReader::New();
  reader->SetFileName(inputVolume.c_str());
  reader->Update();
  
  if ( reader->GetOutput()->GetPointData()->GetTensors() == NULL )
    {
    std::cerr << argv[0] << ": No tensor data" << std::endl;
    return EXIT_FAILURE;
    }
  vtkDiffusionTensorMathematics *math = vtkDiffusionTensorMathematics::New();

  math->SetInput(0, reader->GetOutput());
  math->SetInput(1, reader->GetOutput());


  if (operation == std::string("Trace"))
    {
    math->SetOperationToTrace();
    }
  else if(operation == std::string("Determinant"))
    {
    math->SetOperationToDeterminant();
    }
  else if (operation == std::string("RelativeAnisotropy"))
    {
    math->SetOperationToRelativeAnisotropy();
    }
  else if (operation == std::string("FractionalAnisotropy"))
    {
    math->SetOperationToFractionalAnisotropy();
    }
  else if (operation == std::string("Mode"))
    {
    math->SetOperationToMode();
    }
  else if (operation == std::string("LinearMeasurement"))
    {
    math->SetOperationToLinearMeasure();
    }
  else if (operation == std::string("PlanarMeasurement"))
    {
    math->SetOperationToPlanarMeasure();
    }
  else if (operation == std::string("SphericalMeasurement"))
    {
    math->SetOperationToSphericalMeasure();
    }
  else
    {
    std::cerr << argv[0] <<": Operation "<< operation<< "not supported"<< std::endl;
    return EXIT_FAILURE;
    }            

  math->Update();

  //Save result
  vtkNRRDWriter *writer = vtkNRRDWriter::New();
  writer->SetInput(math->GetOutput());
  writer->SetFileName( outputScalar.c_str() );
  writer->UseCompressionOn();
  //Compute IjkToRas (used by Writer)
  reader->GetRasToIjkMatrix()->Invert();
  writer->SetIJKToRASMatrix( reader->GetRasToIjkMatrix() );
  writer->Write();

  return EXIT_SUCCESS;
}
