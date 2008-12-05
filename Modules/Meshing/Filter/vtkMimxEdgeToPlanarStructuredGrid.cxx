/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxEdgeToPlanarStructuredGrid.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.5 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkMimxEdgeToPlanarStructuredGrid.h"
#include "vtkMimxSubdivideCurve.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkUnsignedCharArray.h"
#include "vtkMimxPlanarTransfiniteInterpolation.h"
#include "vtkMimxSolidTransfiniteInterpolation.h"

vtkCxxRevisionMacro(vtkMimxEdgeToPlanarStructuredGrid, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkMimxEdgeToPlanarStructuredGrid);

// Construct object to extract all of the input data.
vtkMimxEdgeToPlanarStructuredGrid::vtkMimxEdgeToPlanarStructuredGrid()
{
        this->Dimensions[0] = 0; this->Dimensions[1] = 0;
        this->SetNumberOfInputPorts(4);
}

vtkMimxEdgeToPlanarStructuredGrid::~vtkMimxEdgeToPlanarStructuredGrid()
{
}

int vtkMimxEdgeToPlanarStructuredGrid::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkPoints *points = vtkPoints::New();
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
          outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if(this->Dimensions[0] == 0 || this->Dimensions[1] == 0)
  {     vtkErrorMacro(<<
  "Structured grid dimensions not set"<< endl);
  points->Delete();
  return 0;
  }
  int dims[3];
  dims[0] = this->Dimensions[0]; dims[1] = this->Dimensions[1]; dims[2] = 1;
  output->SetDimensions(dims);
  points->SetNumberOfPoints(dims[0]*dims[1]);
  output->SetPoints(points);
  // set scalars
  vtkIntArray *intarray = vtkIntArray::New();
  intarray->SetNumberOfValues(dims[0]*dims[1]);
  for (int i=0; i<dims[0]*dims[1]; i++) {
          intarray->SetValue(i, 1);
  }
  output->GetPointData()->SetScalars(intarray);
  intarray->Delete();
 // 7 different scenarios could arise based on combinations of the number of inputs
        vtkInformation *inInfo[4];
        vtkPolyData *polydata[4];
        for(int i=0; i<4; i++)
        {
                inInfo[i] = NULL;
                inInfo[i] = inputVector[i]->GetInformationObject(0);
                polydata[i] = NULL;
                polydata[i] = vtkPolyData::SafeDownCast(inInfo[i]->Get(vtkDataObject::DATA_OBJECT()));
                if(polydata[i] == NULL)
                {
                        vtkErrorMacro(<<" use vtkPolyData::New() for all four inputs  "<<endl);
                        return 0;
                }
                else
                {
                        this->SetEdge(i);
                }
        }
        
        if(polydata[0]->GetPoints()  && !polydata[1]->GetPoints() &&
                polydata[2]->GetPoints() && !polydata[3]->GetPoints()){
                        vtkPoints *polydatapoints = vtkPoints::New();
                        polydatapoints->SetNumberOfPoints(2);
                        polydatapoints->SetPoint(0,polydata[0]->GetPoint
                                (polydata[0]->GetNumberOfPoints()-1));
                        polydatapoints->SetPoint(1,polydata[2]->GetPoint
                                (polydata[2]->GetNumberOfPoints()-1));
                        polydata[1]->SetPoints(polydatapoints);
                        polydatapoints->Initialize();

                        polydatapoints->SetNumberOfPoints(2);
                        polydatapoints->SetPoint(0,polydata[0]->GetPoint(0));
                        polydatapoints->SetPoint(1,polydata[2]->GetPoint(0));
                        polydata[3]->SetPoints(polydatapoints);
                        polydatapoints->Delete();               
                }
                if(!polydata[0]->GetPoints() && polydata[1]->GetPoints() &&
                        !polydata[2]->GetPoints() && polydata[3]->GetPoints())
        {
                vtkPoints *polydatapoints = vtkPoints::New();
                polydatapoints->SetNumberOfPoints(2);
                polydatapoints->SetPoint(0,polydata[3]->GetPoint
                        (polydata[0]->GetNumberOfPoints()-1));
                polydatapoints->SetPoint(1,polydata[2]->GetPoint
                        (polydata[2]->GetNumberOfPoints()-1));
                polydata[1]->SetPoints(polydatapoints);
                polydatapoints->Initialize();

                polydatapoints->SetNumberOfPoints(2);
                polydatapoints->SetPoint(0,polydata[0]->GetPoint(0));
                polydatapoints->SetPoint(1,polydata[2]->GetPoint(0));
                polydata[3]->SetPoints(polydatapoints);
                polydatapoints->Delete();               

        }

  // from edge data calculate the boundary nodes

          vtkPointSet* pointset = vtkStructuredGrid::New();
          vtkMimxPlanarTransfiniteInterpolation *planeinterp = vtkMimxPlanarTransfiniteInterpolation::New();
          pointset->SetPoints(output->GetPoints());
          planeinterp->SetInput(pointset);
        
         planeinterp->SetIDiv(this->Dimensions[0]);
         planeinterp->SetJDiv(this->Dimensions[1]);
         planeinterp->Update();
        output->SetPoints(planeinterp->GetOutput()->GetPoints());
        points->Delete();
         pointset->Delete();
         planeinterp->Delete();
 
  return 1;
}
int vtkMimxEdgeToPlanarStructuredGrid::FillInputPortInformation(int port,
                                                                                                                          vtkInformation *info)
{
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
        return 1;
}

void vtkMimxEdgeToPlanarStructuredGrid::SetEdge(int EdgeNum, vtkPolyData* Polydata)
{
        this->SetInput(EdgeNum, Polydata);
}

void vtkMimxEdgeToPlanarStructuredGrid::SetDimensions(int dim[2])
{
        this->Dimensions[0] = dim[0];
        this->Dimensions[1] = dim[1];
}

int vtkMimxEdgeToPlanarStructuredGrid::SetEdge(int EdgeNum)
{
        vtkPolyData *polydata  = NULL;
        polydata = vtkPolyData::SafeDownCast(this->GetInput(EdgeNum));

        vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(this->GetOutputDataObject(0));

        if(EdgeNum == 0){
                for(int i = 0; i < polydata->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(i,polydata->GetPoint(i));
                        //cout <<i<<"  ";
                        if(polydata->GetPointData()->GetScalars())
                        {
                                double status[1];
                                polydata->GetPointData()->GetScalars()->GetTuple(i,status);
                                if(!status[0])  
                                        output->GetPointData()->GetScalars()->SetTuple(i,status);
                        }
                }
                //cout<<endl;
                return 1;
        }

        if(EdgeNum == 1){
                for(int i = 0; i < polydata->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(this->Dimensions[0]*(i+1)-1,polydata->GetPoint(i));
                        //cout <<this->Dimensions[0]*(i+1)-1<<"  ";
                        if(polydata->GetPointData()->GetScalars())
                        {
                                double status[1];
                                polydata->GetPointData()->GetScalars()->GetTuple(i,status);
                                if(!status[0])  output->GetPointData()->GetScalars()->SetTuple(this->Dimensions[0]*(i+1)-1,status);
                        }
                }
                //cout<<endl;
                return 1;
        }

        if(EdgeNum == 2){
                for(int i = 0; i < polydata->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint((this->Dimensions[1]-1)*
                                this->Dimensions[0]+i,polydata->GetPoint(i));
                        //cout<<(this->Dimensions[1]-1)*this->Dimensions[0]+i<<"  ";
                        if(polydata->GetPointData()->GetScalars())
                        {
                                double status[1];
                                polydata->GetPointData()->GetScalars()->GetTuple(i,status);
                                if(!status[0])  output->GetPointData()->GetScalars()->SetTuple
                                        ((this->Dimensions[1]-1)*this->Dimensions[0]+i,status);
                        }

                }
                //cout<<endl;
                return 1;
        }

        if(EdgeNum == 3){
                for(int i = 0; i < polydata->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(this->Dimensions[0]*(i),
                                polydata->GetPoint(i));
                        //cout<<this->Dimensions[0]*(i)<<"  ";
                        if(polydata->GetPointData()->GetScalars())
                        {
                                double status[1];
                                polydata->GetPointData()->GetScalars()->GetTuple(i,status);
                                if(!status[0])  output->GetPointData()->GetScalars()->
                                        SetTuple(this->Dimensions[0]*(i),status);
                        }

                }
                //cout<<endl;
                return 1;
        }
}

void vtkMimxEdgeToPlanarStructuredGrid::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
