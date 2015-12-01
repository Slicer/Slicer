#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "FiberBundleLabelSelectCLP.h"

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
  PARSE_ARGS;

  try
  {
  // Label operations
  int includeOperation = 0; // 0-OR; 1-AND
  if (PassOperation == std::string("OR"))
    {
    includeOperation = 0;
    }
  else if (PassOperation == std::string("AND"))
    {
    includeOperation = 1;
    }
  else
    {
    std::cerr << "unknown include operation";
    return EXIT_FAILURE;
    }

  int excludeOperation = 0; // 0-AND; 1-OR
  if (NoPassOperation == std::string("OR"))
    {
    excludeOperation = 0;
    }
  else if (NoPassOperation == std::string("AND"))
    {
    excludeOperation = 1;
    }
  else
    {
    std::cerr << "unknown exclude operation";
    return EXIT_FAILURE;
    }

  // Read in Label volume inputs
  vtkNew<vtkImageCast> imageCastLabel_A;
  vtkNew<vtkITKArchetypeImageSeriesScalarReader> readerLabel_A;
  readerLabel_A->SetArchetype(InputLabel_A.c_str());
  readerLabel_A->SetUseOrientationFromFile(1);
  readerLabel_A->SetUseNativeOriginOn();
  readerLabel_A->SetOutputScalarTypeToNative();
  readerLabel_A->SetDesiredCoordinateOrientationToNative();
  readerLabel_A->SetSingleFile(1);
  readerLabel_A->Update();

  imageCastLabel_A->SetOutputScalarTypeToShort();
  imageCastLabel_A->SetInputConnection(readerLabel_A->GetOutputPort() );
  imageCastLabel_A->Update();

  // Read in fiber bundle input to be selected from VTK or VTP
  std::string extension1 = vtksys::SystemTools::GetFilenameLastExtension(InputFibers.c_str());
  std::string extension = vtksys::SystemTools::LowerCase(extension1);

  vtkSmartPointer<vtkPolyData> input;
  if (extension == std::string(".vtk"))
    {
    vtkNew<vtkPolyDataReader> readerPD;
    readerPD->SetFileName(InputFibers.c_str());
    readerPD->Update();
    input = vtkPolyData::SafeDownCast(readerPD->GetOutput());
    }
  else if (extension == std::string(".vtp"))
    {
      vtkNew<vtkXMLPolyDataReader> readerPD;
      readerPD->SetFileName(InputFibers.c_str());
      readerPD->Update();
      input = vtkPolyData::SafeDownCast(readerPD->GetOutput());
    }


  //1. Set up matrices to put fibers into ijk space of volume
  // This assumes fibers are in RAS space of volume (i.e. RAS==world)
  vtkNew<vtkMatrix4x4> Label_A_RASToIJK;
  Label_A_RASToIJK->DeepCopy(readerLabel_A->GetRasToIjkMatrix());

  //the volume we're probing knows its spacing so take this out of the matrix
  double sp[3];
  imageCastLabel_A->GetOutput()->GetSpacing(sp);
  vtkNew<vtkTransform> trans;
  trans->Identity();
  trans->PreMultiply();
  trans->SetMatrix(Label_A_RASToIJK.GetPointer());

  // 2. Find polylines
  int inExt[6];
  imageCastLabel_A->GetOutputInformation(0)->Get(
    vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inExt);

  vtkPoints *inPts =input->GetPoints();
  vtkIdType numPts = inPts->GetNumberOfPoints();
  vtkCellArray *inLines = input->GetLines();
  vtkIdType numLines = inLines->GetNumberOfCells();
  vtkIdType npts=0, *pts=NULL;

  if ( !inPts || numPts  < 1 || !inLines || numLines < 1 )
    {
    return EXIT_SUCCESS;
    }

  std::vector<bool> addLines;
  vtkIdType numNewPts = 0;
  vtkIdType numNewCells = 0;
  vtkIdType j;
  double p[3];

  unsigned int label;

  int *labelDims = imageCastLabel_A->GetOutput()->GetDimensions();
  // Check lines
  vtkIdType inCellId;
  for (inCellId=0, inLines->InitTraversal();
       inLines->GetNextCell(npts,pts); inCellId++)
    {
    if (npts < 2)
      {
      addLines.push_back(false);
      std::cerr << "Less than two points in line " << inCellId << std::endl;
      continue; //skip this polyline
      }

    std::vector<bool> passAll;
    for (label=0; label<PassLabel.size(); label++)
      {
      passAll.push_back(false);
      }

    double pIJK[3];
    int pt[3];
    short *inPtr;
    bool addLine = false;
    bool pass = false;
    bool nopass = false;
    for (j=0; j < npts; j++)
      {
      inPts->GetPoint(pts[j],p);
      trans->TransformPoint(p,pIJK);
      pt[0]= (int) floor(pIJK[0]);
      pt[1]= (int) floor(pIJK[1]);
      pt[2]= (int) floor(pIJK[2]);
      if (pt[0] < 0 || pt[1] < 0 || pt[2] < 0 ||
          pt[0] >= labelDims[0] || pt[1] >= labelDims[1] || pt[2] >= labelDims[2])
        {
        continue;
        }

      inPtr = (short *) imageCastLabel_A->GetOutput()->GetScalarPointer(pt);

      if (excludeOperation == 0) // OR
        {
        for(label=0; label<NotPassLabel.size() && !nopass; label++)
          {
          nopass = (*inPtr == NotPassLabel[label]);
          }
        }
      else if (excludeOperation == 1) // AND
        {
        bool nopassAll = (NotPassLabel.size() > 0);
        for(label=0; label<NotPassLabel.size(); label++)
          {
          nopassAll = nopassAll & (*inPtr == NotPassLabel[label]);
          }
        nopass = nopassAll;
        }
      if (nopass)
        {
        break; //Skip this one, dont check points
        }

      if (PassLabel.size() == 0)
        {
        pass = true;
        }
      else
        {
        if (includeOperation == 0) // OR
          {
          for(label=0; label<PassLabel.size() &&  !pass; label++)
            {
              pass = (*inPtr == PassLabel[label]);
            }
          }
        else if (includeOperation == 1) // AND
          {
          for(label=0; label<PassLabel.size(); label++)
            {
            if (*inPtr == PassLabel[label])
              {
              passAll[label] = true;
              break;
              }
            } // for(label=0;
          } // else if (includeOperation == 1)
        } // if !(PassLabel.size() == 0)
      } //for (j=0; j < npts; j++)

    if (includeOperation == 1 && PassLabel.size() > 0) // AND
      {
      pass = true;
      for (label=0; label<passAll.size(); label++)
        {
        pass = pass & passAll[label];
        }
      }
    addLine = pass && !nopass;

    addLines.push_back(addLine);
    if (addLine)
      {
      // Add line to the output
      numNewPts += npts;
      numNewCells++;
      }
    } //for (inCellId=0, inLines->InitTraversal();

  //Preallocate PolyData elements
  vtkNew<vtkPolyData> outFibers;

  vtkNew<vtkPoints> points;
  points->Allocate(numNewPts);
  outFibers->SetPoints(points.GetPointer());

  vtkNew<vtkCellArray> outFibersCellArray;
  outFibersCellArray->Allocate(numNewPts+numNewCells);
  outFibers->SetLines(outFibersCellArray.GetPointer());

  // If the input has point data, including tensors or scalar arrays, copy them to the output.
  // Currently this ignores cell data, which may be added in the future if needed.
  // Check for point data arrays to keep and allocate them.
  int numberArrays = input->GetPointData()->GetNumberOfArrays();

  for (int arrayIdx = 0; arrayIdx < numberArrays; arrayIdx++)
    {
      vtkDataArray *oldArray = input->GetPointData()->GetArray(arrayIdx);
      vtkSmartPointer<vtkFloatArray> newArray = vtkSmartPointer<vtkFloatArray>::New();
      newArray->SetNumberOfComponents(oldArray->GetNumberOfComponents());
      newArray->SetName(oldArray->GetName());
      newArray->Allocate(newArray->GetNumberOfComponents()*numNewPts);
      outFibers->GetPointData()->AddArray(newArray);
    }

  vtkIdType ptId = 0;

  for (inCellId=0, inLines->InitTraversal();
       inLines->GetNextCell(npts,pts); inCellId++)
    {
    if (addLines[inCellId])
      {
      outFibersCellArray->InsertNextCell(npts);
      for (j=0; j < npts; j++)
        {
        inPts->GetPoint(pts[j],p);
        points->InsertPoint(ptId,p);
        outFibersCellArray->InsertCellPoint(ptId);

        // Copy point data from input fiber
        for (int arrayIdx = 0; arrayIdx < numberArrays; arrayIdx++)
          {
            vtkDataArray *newArray = outFibers->GetPointData()->GetArray(arrayIdx);
            vtkDataArray *oldArray = input->GetPointData()->GetArray(newArray->GetName());
            newArray->InsertNextTuple(oldArray->GetTuple(pts[j]));
          }

        ptId++;
        }
      }
    }

  // Copy array attributes from input (TENSORS, scalars, etc)
  for (int arrayIdx = 0; arrayIdx < numberArrays; arrayIdx++)
    {
    int attr = input->GetPointData()->IsArrayAnAttribute(arrayIdx);
    if (attr >= 0)
      {
      outFibers->GetPointData()->SetActiveAttribute(input->GetPointData()->GetArray(arrayIdx)->GetName(), attr);
      }
    }

  //3. Save the output in VTK or VTP
  std::string extension2 = vtksys::SystemTools::GetFilenameLastExtension(OutputFibers.c_str());
  std::string extension_output = vtksys::SystemTools::LowerCase(extension2);
  if (extension_output == std::string(".vtk"))
    {
      vtkNew<vtkPolyDataWriter> writer;
      writer->SetFileName(OutputFibers.c_str());
           writer->SetInputData(outFibers.GetPointer());
           writer->SetFileTypeToBinary();
           writer->Write();
    }
  else if (extension_output == std::string(".vtp"))
    {
           vtkNew<vtkXMLPolyDataWriter> writer;
           writer->SetFileName(OutputFibers.c_str());
           writer->SetInputData(outFibers.GetPointer());
           writer->SetDataModeToBinary();
           writer->Write();
    }
  }
  catch ( ... )
      {
        std::cerr << "default exception";
        return EXIT_FAILURE;
      }

  return EXIT_SUCCESS;

}
