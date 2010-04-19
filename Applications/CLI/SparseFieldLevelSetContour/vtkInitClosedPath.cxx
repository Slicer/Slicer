#include "vtkInitClosedPath.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTriangleFilter.h"
#include "vtkSmartPointer.h"

vtkCxxRevisionMacro(vtkInitClosedPath, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkInitClosedPath);

vtkInitClosedPath::vtkInitClosedPath()
{
  
  //this->SetNumberOfInputPorts(2);
  // port 0: polyData
  
  
  // NO! use a SET method before calling
      // (deprecated) port 1: array of vertex points (sparse, initialization)

 // this->SetNumberOfOutputPorts(1);
  // port 0: polyData
  
  // NO! need it ???
    // (deprecated) port 1: vertex points (dense active contour)

  this->activeContourVertIdx = vtkSmartPointer<vtkIntArray>::New();
}

void vtkInitClosedPath::SetSource(vtkPolyData *source)
{
  this->SetInput(1, source);
}

vtkPolyData *vtkInitClosedPath::GetSource()
{
  if (this->GetNumberOfInputConnections(1) < 1)
    {
    return NULL;
    }
  return vtkPolyData::SafeDownCast(
    this->GetExecutive()->GetInputData(1, 0));
}

// Where the bulk of work is done!
int vtkInitClosedPath::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
   // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
 // vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  
  // update progress bar at some intervals
  this->UpdateProgress(0.15);

    // get the input and ouptut
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *source = 0;
  //if (sourceInfo) // This does nothing!
  //  {
  //  source = vtkPolyData::SafeDownCast(
  //    sourceInfo->Get(vtkDataObject::DATA_OBJECT()));
  //  }
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  
  // Check input
  int numVerts=input->GetNumberOfPoints();
  int numCells=input->GetNumberOfCells();
  if (numVerts < 1 || numCells < 1)
    {
    vtkErrorMacro(<<"No data to smooth!");
    return 1;
    }
  
  { // copy everything through first, before updating...
    output->CopyStructure(input);
    output->GetPointData()->PassData(input->GetPointData());
    output->GetCellData()->PassData(input->GetCellData());
    vtkDebugMacro(<<"Analyzing ____ ...");
  }
 
 
  vtkDataArray* contourIdxArrayIn = input->GetPointData()->GetArray("ActiveContourVertexIndices");
  if( NULL == contourIdxArrayIn )  { 
    activeContourVertIdx->SetName("ActiveContourVertexIndices");
    activeContourVertIdx->SetNumberOfComponents(1);
    activeContourVertIdx->SetNumberOfTuples( numVerts ); 
      // Strange: vtk assumes this has same size as mesh? 
      // this should be arbitrary value in principle. 
      // workaround: 'terminate' the list with "-1" index
    output->GetPointData()->AddArray( activeContourVertIdx );
  }
  else { // if it already exists, verify that we point to it
     activeContourVertIdx = vtkIntArray::SafeDownCast( contourIdxArrayIn );
     std::string name( activeContourVertIdx->GetName( ) );
     cout<<"re-using existing array named: "<<name<<"\n";
  }



  // update progress bar at some intervals
  this->UpdateProgress(1.0);
 
  return 1;
 }
  
int vtkInitClosedPath::FillInputPortInformation(int port,
                                                      vtkInformation *info)
{
  if (!this->Superclass::FillInputPortInformation(port, info))
    {
    return 0;
    }
  
  if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}


void vtkInitClosedPath::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Truth: " << 42 << "\n";
  if ( this->GetSource() )
    {
      os << indent << "Source: " << static_cast<void *>(this->GetSource()) << "\n";
    }
  else
    {
    os << indent << "Source (none)\n";
    }
}
 
