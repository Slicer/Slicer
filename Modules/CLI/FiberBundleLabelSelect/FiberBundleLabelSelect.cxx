#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "FiberBundleLabelSelectCLP.h"

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

#include <vtkITKArchetypeImageSeriesScalarReader.h>
#include <vtkImageCast.h>

#include "vtkImageData.h"

#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkTransform.h"
#include "vtkXMLPolyDataWriter.h"


int main( int argc, char * argv[] )
{
  PARSE_ARGS;
  
  try
  {
  // Read in Label volume inputs
  vtkSmartPointer<vtkImageCast> imageCastLabel_A = vtkSmartPointer<vtkImageCast>::New();
  vtkSmartPointer<vtkITKArchetypeImageSeriesScalarReader> readerLabel_A = vtkSmartPointer<vtkITKArchetypeImageSeriesScalarReader>::New();
  readerLabel_A->SetArchetype(InputLabel_A.c_str());
  readerLabel_A->SetUseOrientationFromFile(1);
  readerLabel_A->SetUseNativeOriginOn();
  readerLabel_A->SetOutputScalarTypeToNative();
  readerLabel_A->SetDesiredCoordinateOrientationToNative();
  readerLabel_A->SetSingleFile(1);
  readerLabel_A->Update();

  imageCastLabel_A->SetOutputScalarTypeToShort();
  imageCastLabel_A->SetInput(readerLabel_A->GetOutput() );
  imageCastLabel_A->Update();

  
  // Read in fiber bundle input to be selected.
  vtkSmartPointer<vtkXMLPolyDataReader> readerPD = vtkSmartPointer<vtkXMLPolyDataReader>::New();
  readerPD->SetFileName(InputFibers.c_str());
  readerPD->Update();


  //1. Set up matrices to put fibers into ijk space of volume
  // This assumes fibers are in RAS space of volume (i.e. RAS==world)
  vtkSmartPointer<vtkMatrix4x4> Label_A_RASToIJK = vtkSmartPointer<vtkMatrix4x4>::New();
  Label_A_RASToIJK->DeepCopy(readerLabel_A->GetRasToIjkMatrix());
  
  //the volume we're probing knows its spacing so take this out of the matrix
  double sp[3];
  imageCastLabel_A->GetOutput()->GetSpacing(sp);
  vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
  trans->Identity();
  trans->PreMultiply();
  trans->SetMatrix(Label_A_RASToIJK);

  /**
  // Trans from IJK to RAS
  trans->Inverse();
  // Take into account spacing to compute Scaled IJK
  trans->Scale(1/sp[0],1/sp[1],1/sp[2]);
  // now it's RAS to scaled IJK
  trans->Inverse();
 ***/

  // 2. Find polylines
  int inExt[6];
  imageCastLabel_A->GetOutput()->GetWholeExtent(inExt);

  vtkPolyData *input = vtkPolyData::SafeDownCast(readerPD->GetOutput());

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
    double pIJK[3];
    int pt[3];
    unsigned int label;
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
      inPtr = (short *) imageCastLabel_A->GetOutput()->GetScalarPointer(pt);

      for(label=0; label<NotPassLabel.size() && !nopass; label++)
        {
        nopass = (*inPtr == NotPassLabel[label]);
        }
      if (nopass)
        break; //Skip this one

      for(label=0; label<PassLabel.size() &&  !pass; label++)
        {
          pass = (*inPtr == PassLabel[label]);
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
    }
  
  // Add lines

  //Preallocate PolyData elements
  vtkSmartPointer<vtkPolyData> outFibers = vtkSmartPointer<vtkPolyData>::New();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->Allocate(numNewPts);
  outFibers->SetPoints(points);

  vtkSmartPointer<vtkCellArray> outFibersCellArray = vtkSmartPointer<vtkCellArray>::New();
  outFibersCellArray->Allocate(numNewPts+numNewCells);
  outFibers->SetLines(outFibersCellArray);

  //outFibersCellArray->SetNumberOfCells(numNewCells);
  //outFibersCellArray = outFibers->GetLines();

  //vtkIdTypeArray *cellArray=outFibersCellArray->GetData();
  //cellArray->SetNumberOfTuples(numNewPts+numNewCells);

  vtkSmartPointer<vtkFloatArray> newTensors = vtkSmartPointer<vtkFloatArray>::New();
  newTensors->SetNumberOfComponents(9);
  newTensors->Allocate(9*numNewPts);
  outFibers->GetPointData()->SetTensors(newTensors);
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
  vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetFileName(OutputFibers.c_str());
  writer->SetInput( outFibers );
  writer->Write();
  }
  catch ( ... )
      {
        std::cerr << "default exception";
        EXIT_FAILURE;
      }

  return EXIT_SUCCESS;

}
