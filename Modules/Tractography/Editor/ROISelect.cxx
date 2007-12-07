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

#include "vtkFloatArray.h"
#include "vtkCellArray.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkSmartPointer.h"
#include "vtkPolyDataWriter.h"
#include "vtkTimerLog.h"
#include "vtkMath.h"

#include "vtkNRRDReader.h"
#include "vtkNRRDWriter.h"
#include "vtkImageData.h"
#include "ROISelectCLP.h"

#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkTransform.h"
#include "vtkXMLPolyDataWriter.h"


int main( int argc, const char * argv[] )
{

  PARSE_ARGS;

  // Read in ROI volume inputs
  vtkNRRDReader *readerROI_A = vtkNRRDReader::New();
  readerROI_A->SetFileName(InputROI_A.c_str());
  readerROI_A->Update();
  
  // Read in fiber bundle input to be selected.
  vtkXMLPolyDataReader *readerPD = vtkXMLPolyDataReader::New();
  readerPD->SetFileName(InputFibers.c_str());
  readerPD->Update();

  //if ( readerPD->GetOutput()->GetPointData()->GetTensors() == NULL )
  //  {
  //  std::cerr << argv[0] << ": No tensor data" << std::endl;
  //  return EXIT_FAILURE;
  //  }

  //1. Set up matrices to put fibers into ijk space of volume
  // This assumes fibers are in RAS space of volume (i.e. RAS==world)
  vtkMatrix4x4 *ROI_A_RASToIJK = vtkMatrix4x4::New();
  ROI_A_RASToIJK->DeepCopy(readerROI_A->GetRasToIjkMatrix());
  
  //the volume we're probing knows its spacing so take this out of the matrix
  double sp[3];
  readerROI_A->GetOutput()->GetSpacing(sp);
  vtkTransform *trans = vtkTransform::New();
  trans->Identity();
  trans->PreMultiply();
  trans->SetMatrix(ROI_A_RASToIJK);

  /***
  // Trans from IJK to RAS
  trans->Inverse();
  // Take into account spacing to compute Scaled IJK
  trans->Scale(1/sp[0],1/sp[1],1/sp[2]);
  // now it's RAS to scaled IJK
  trans->Inverse();
 ***/

  // 2. Find polylines
  int inExt[6];
  readerROI_A->GetOutput()->GetWholeExtent(inExt);

  vtkPolyData *input = vtkPolyData::SafeDownCast(readerPD->GetOutput());

  vtkPoints *inPts =input->GetPoints();
  vtkIdType numPts = inPts->GetNumberOfPoints();
  vtkCellArray *inLines = input->GetLines();
  vtkIdType numLines = inLines->GetNumberOfCells();
  vtkIdType npts=0, *pts=NULL;

  if ( !inPts || numPts  < 1 || !inLines || numLines < 1 )
    {
    EXIT_SUCCESS;
    }

  std::vector<bool> addLines;
  vtkIdType numNewPts = 0;
  vtkIdType numNewCells = 0;
  vtkIdType j;
  double p[3];

  // Check lines
  vtkIdType inCellId;
  for (inCellId=0, inLines->InitTraversal(); 
       inLines->GetNextCell(npts,pts); inCellId++)
    {
    if (npts < 2)
      {
      addLines.push_back(false);
      std::cout << "Less than two points in line" << std::endl;
      continue; //skip this polyline
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
      inPtr = (short *) readerROI_A->GetOutput()->GetScalarPointer(pt);
      if (!inPtr || *inPtr == NotPassLabel)
        {
        nopass = true;
        break; //skip this polyline
        }
      if( *inPtr == PassLabel)
        {
        pass = true;
        }
      }
    if (pass && !nopass) 
      {
      addLine = true;
      }

    addLines.push_back(addLine);
    if (addLine)
      {
      // Add line to the output
      numNewPts += npts;
      numNewCells++;
      }
    }
  
  // Add lines

  //Preallocate PolyData elements
  vtkPolyData *outFibers = vtkPolyData::New();

  vtkPoints *points = vtkPoints::New();
  points->Allocate(numNewPts);
  outFibers->SetPoints(points);
  points->Delete();

  vtkCellArray *outFibersCellArray = vtkCellArray::New();
  outFibersCellArray->Allocate(numNewPts+numNewCells);
  outFibers->SetLines(outFibersCellArray);
  outFibersCellArray->Delete();

  //outFibersCellArray->SetNumberOfCells(numNewCells);
  //outFibersCellArray = outFibers->GetLines();

  //vtkIdTypeArray *cellArray=outFibersCellArray->GetData();
  //cellArray->SetNumberOfTuples(numNewPts+numNewCells);

  vtkFloatArray *newTensors = vtkFloatArray::New();
  newTensors->SetNumberOfComponents(9);
  newTensors->Allocate(9*numNewPts);
  outFibers->GetPointData()->SetTensors(newTensors);
  newTensors->Delete();
  newTensors = static_cast<vtkFloatArray *> (outFibers->GetPointData()->GetTensors());


  vtkIdType ptId = 0;
  vtkDataArray *oldTensors = input->GetPointData()->GetTensors();
  double tensor[9];

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
        if (oldTensors)
          {
          oldTensors->GetTuple(pts[j],tensor);
          newTensors->InsertNextTuple(tensor);
          }
        ptId++;
        }
      }
    }

  //3. Save the output
  vtkXMLPolyDataWriter *writer = vtkXMLPolyDataWriter::New();
  writer->SetFileName(OutputFibers.c_str());
  writer->SetInput( outFibers );
  writer->Write();

  //8. Delete everything
  outFibers->Delete();
  writer->Delete();
  trans->Delete();
  ROI_A_RASToIJK->Delete();
  readerPD->Delete();
  readerROI_A->Delete();

  return EXIT_SUCCESS;
}
