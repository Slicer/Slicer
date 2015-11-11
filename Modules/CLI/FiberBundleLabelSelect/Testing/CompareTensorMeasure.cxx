#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkPluginFilterWatcher.h"
#include "itkPluginUtilities.h"

// vtkITK includes
#include <vtkITKArchetypeImageSeriesScalarReader.h>

// VTK includes
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTimerLog.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkVersion.h>

// added for taking VTK and VTP as input and output
#include <vtkPolyDataReader.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <iostream>
#include <algorithm>
#include <string>

int main( int argc, char * argv[] )
{

  if( argc < 3 )
    {
    std::cerr << "Both output and baseline are required!" << std::endl;
    return EXIT_FAILURE;
    }

  std::string outputPath = argv[1];
  std::string baselinePath =argv[2];

  // Read output and baseline
  std::string extension = vtksys::SystemTools::LowerCase(vtksys::SystemTools::GetFilenameLastExtension(outputPath));
  vtkSmartPointer<vtkPolyData> output;
  if (extension == std::string(".vtk"))
    {
    vtkNew<vtkPolyDataReader> readerPD;
    readerPD->SetFileName(outputPath.c_str());
    readerPD->Update();
    output = vtkPolyData::SafeDownCast(readerPD->GetOutput());
    }
  else if (extension == std::string(".vtp"))
    {
    vtkNew<vtkXMLPolyDataReader> readerPD;
    readerPD->SetFileName(outputPath.c_str());
    readerPD->Update();
    output = vtkPolyData::SafeDownCast(readerPD->GetOutput());
    }

  vtkSmartPointer<vtkPolyData> baseline;
  vtkNew<vtkPolyDataReader> readerPD;
  readerPD->SetFileName(baselinePath.c_str());
  readerPD->Update();
  baseline = vtkPolyData::SafeDownCast(readerPD->GetOutput());

  // Point and fiber numbers
  vtkPoints *inPts = output->GetPoints();
  vtkIdType numPtsOutput = inPts->GetNumberOfPoints();
  vtkCellArray *inLines = output->GetLines();
  vtkIdType numLinesOutput = inLines->GetNumberOfCells();

  inPts = baseline->GetPoints();
  vtkIdType numPtsBaseline = inPts->GetNumberOfPoints();
  inLines = baseline->GetLines();
  vtkIdType numLinesBaseline = inLines->GetNumberOfCells();


  std::cout << "   Output : nunber of points " << numPtsOutput << ", number of fibers  " << numLinesOutput << std::endl;
  std::cout << " Baseline : nunber of points " << numPtsBaseline << ", number of fibers  " << numLinesBaseline << std::endl;

  if (numPtsOutput != numPtsBaseline || numLinesOutput != numLinesBaseline)
    {
    std::cerr << "Not matched! " << std::endl;
    return EXIT_FAILURE;
    }

  for (int i=0; i<output->GetPointData()->GetNumberOfArrays(); i++)
    {
    vtkDataArray *arr = output->GetPointData()->GetArray(i);
    std::string tensorNameOutput = arr->GetName();
    int numberOfComponentsOutput = arr->GetNumberOfComponents();

    arr = baseline->GetPointData()->GetArray(i);
    std::string tensorNameBaseline = arr->GetName();
    int numberOfComponentsBaseline = arr->GetNumberOfComponents();

    std::cout << "   Output : tensor name " << tensorNameOutput << ", number of components  " << numberOfComponentsOutput << std::endl;
    std::cout << " Baseline : tensor name " << tensorNameBaseline << ", number of components  " << numberOfComponentsBaseline << std::endl;

    if (tensorNameOutput != tensorNameBaseline || numberOfComponentsOutput != numberOfComponentsBaseline)
      {
      std::cerr << "Not matched! " << std::endl;
      return EXIT_FAILURE;
      }
    }

  return EXIT_SUCCESS;
}





