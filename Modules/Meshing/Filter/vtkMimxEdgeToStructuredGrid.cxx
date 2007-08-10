/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxEdgeToStructuredGrid.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.7 $

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

#include "vtkMimxEdgeToStructuredGrid.h"
#include "vtkMimxSubdivideCurve.h"

#include "vtkCellData.h"
#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkUnsignedCharArray.h"
#include "vtkMimxPlanarTransfiniteInterpolation.h"
#include "vtkMimxSolidTransfiniteInterpolation.h"
#include "vtkStructuredGridWriter.h"

vtkCxxRevisionMacro(vtkMimxEdgeToStructuredGrid, "$Revision: 1.7 $");
vtkStandardNewMacro(vtkMimxEdgeToStructuredGrid);

// Construct object to extract all of the input data.
vtkMimxEdgeToStructuredGrid::vtkMimxEdgeToStructuredGrid()
{
        this->SetNumberOfInputPorts(12);
        this->Dimensions[0] = 0; this->Dimensions[1] = 0; 
        this->Dimensions[2] = 0;
        this->SetPlane(VTKIS_PLANE_NONE);
}

vtkMimxEdgeToStructuredGrid::~vtkMimxEdgeToStructuredGrid()
{
}

int vtkMimxEdgeToStructuredGrid::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkPoints *points = vtkPoints::New();
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
          outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if(this->Dimensions[0] == 0 || this->Dimensions[1] == 0 || 
          this->Dimensions[2] == 0)
  {        vtkErrorMacro(<<
  "Structured grid dimensions not set"<< endl);
  points->Delete();
  return 0;
  }
        output->SetDimensions(this->Dimensions);
  points->SetNumberOfPoints(this->Dimensions[0]*
          this->Dimensions[1]*this->Dimensions[2]);

  output->SetPoints(points);

  vtkInformation *inInfo[12];
  vtkPolyData *polydata[12];
  for(int i=0; i<12; i++)
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
  }

  if(this->Plane == VTKIS_PLANE_IJ)
  {
          vtkPoints *polydatapoints0 = vtkPoints::New();
          // to create curve 0
          polydatapoints0->SetNumberOfPoints(2);
          polydatapoints0->SetPoint(0,polydata[3]->GetPoint(0));
          polydatapoints0->SetPoint(1,polydata[1]->GetPoint(0));
          polydata[0]->SetPoints(polydatapoints0);
          polydatapoints0->Delete();
          vtkCellArray *cellarray0 = vtkCellArray::New();
          cellarray0->InsertNextCell(2);
          cellarray0->InsertCellPoint(0); cellarray0->InsertCellPoint(1);
          polydata[0]->SetLines(cellarray0);
          cellarray0->Delete();

          // to create curve 2
          vtkPoints *polydatapoints2 = vtkPoints::New();
          polydatapoints2->SetNumberOfPoints(2);
          polydatapoints2->SetPoint(0,polydata[3]->GetPoint(polydata[3]->GetNumberOfPoints()-1));
          polydatapoints2->SetPoint(1,polydata[1]->GetPoint(polydata[1]->GetNumberOfPoints()-1));
          polydata[2]->SetPoints(polydatapoints2);
          vtkCellArray *cellarray2 = vtkCellArray::New();
          cellarray2->InsertNextCell(2);
          cellarray2->InsertCellPoint(0); cellarray2->InsertCellPoint(1);
          polydata[2]->SetLines(cellarray2);
          cellarray2->Delete();
          polydatapoints2->Delete();
          // to create curve 4
          vtkPoints *polydatapoints4 = vtkPoints::New();
          polydatapoints4->SetNumberOfPoints(2);
          polydatapoints4->SetPoint(0,polydata[7]->GetPoint(0));
          polydatapoints4->SetPoint(1,polydata[5]->GetPoint(0));
          polydata[4]->SetPoints(polydatapoints4);
          polydatapoints4->Delete();
          vtkCellArray *cellarray4 = vtkCellArray::New();
          cellarray4->InsertNextCell(2);
          cellarray4->InsertCellPoint(0); cellarray4->InsertCellPoint(1);
          polydata[4]->SetLines(cellarray4);
          cellarray4->Delete();

          // to create curve 6          
          vtkPoints *polydatapoints6 = vtkPoints::New();
          polydatapoints6->SetNumberOfPoints(2);
          polydatapoints6->SetPoint(0,polydata[7]->GetPoint(polydata[7]->GetNumberOfPoints()-1));
          polydatapoints6->SetPoint(1,polydata[5]->GetPoint(polydata[5]->GetNumberOfPoints()-1));
          polydata[6]->SetPoints(polydatapoints6);
          polydatapoints6->Delete();
          vtkCellArray *cellarray6 = vtkCellArray::New();
          cellarray6->InsertNextCell(2);
          cellarray6->InsertCellPoint(0); cellarray6->InsertCellPoint(1);
          polydata[6]->SetLines(cellarray6);
          cellarray6->Delete();

          // to create curve 8
          vtkPoints *polydatapoints8 = vtkPoints::New();
          polydatapoints8->SetNumberOfPoints(2);
          polydatapoints8->SetPoint(0,polydata[3]->GetPoint(0));
          polydatapoints8->SetPoint(1,polydata[7]->GetPoint(0));
          polydata[8]->SetPoints(polydatapoints8);
          polydatapoints8->Delete();
          vtkCellArray *cellarray8 = vtkCellArray::New();
          cellarray8->InsertNextCell(2);
          cellarray8->InsertCellPoint(0); cellarray8->InsertCellPoint(1);
          polydata[8]->SetLines(cellarray8);
          cellarray8->Delete();

          // to create curve 9
          vtkPoints *polydatapoints9 = vtkPoints::New();
          polydatapoints9->SetNumberOfPoints(2);
          polydatapoints9->SetPoint(0,polydata[1]->GetPoint(0));
          polydatapoints9->SetPoint(1,polydata[5]->GetPoint(0));
          polydata[9]->SetPoints(polydatapoints9);
          polydatapoints9->Delete();
          vtkCellArray *cellarray9 = vtkCellArray::New();
          cellarray9->InsertNextCell(2);
          cellarray9->InsertCellPoint(0); cellarray9->InsertCellPoint(1);
          polydata[9]->SetLines(cellarray9);
          cellarray9->Delete();

          // to create curve 11
          vtkPoints *polydatapoints11 = vtkPoints::New();
          polydatapoints11->SetNumberOfPoints(2);
          polydatapoints11->SetPoint(0,polydata[1]->GetPoint(polydata[1]->GetNumberOfPoints()-1));
          polydatapoints11->SetPoint(1,polydata[5]->GetPoint(polydata[5]->GetNumberOfPoints()-1));
          polydata[11]->SetPoints(polydatapoints11);
          polydatapoints11->Delete();
          vtkCellArray *cellarray11 = vtkCellArray::New();
          cellarray11->InsertNextCell(2);
          cellarray11->InsertCellPoint(0); cellarray11->InsertCellPoint(1);
          polydata[11]->SetLines(cellarray11);
          cellarray11->Delete();

          // to create curve 10
          vtkPoints *polydatapoints10 = vtkPoints::New();
          polydatapoints10->SetNumberOfPoints(2);
          polydatapoints10->SetPoint(0,polydata[3]->GetPoint(polydata[3]->GetNumberOfPoints()-1));
          polydatapoints10->SetPoint(1,polydata[7]->GetPoint(polydata[7]->GetNumberOfPoints()-1));
          polydata[10]->SetPoints(polydatapoints10);
          polydatapoints10->Delete();
          vtkCellArray *cellarray10 = vtkCellArray::New();
          cellarray10->InsertNextCell(2);
          cellarray10->InsertCellPoint(0); cellarray10->InsertCellPoint(1);
          polydata[10]->SetLines(cellarray10);
          cellarray10->Delete();

  }
  for(int i=0; i <12; i++)
  {
          vtkInformation *inInfo;
          inInfo = NULL;
          inInfo = inputVector[i]->GetInformationObject(0);
          if(inInfo){
                  vtkPolyData* polydata  = NULL;
                  polydata = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
                  if(polydata->GetNumberOfPoints() < 2)
                  {
                          vtkErrorMacro(<<"Number of points defining  Edge  "<<i<<
                                  "  should be 2 or more"<<endl);
                          return 0;
                          points->Delete();
                  }
                  else{
                          this->SetEdge(i);
                  }
          }
          else
          {
                  vtkErrorMacro(<<"Data not set for Edge  "<<i << endl);
                  points->Delete();
                  return 0;
          }
  }

  // from edge data calculate the boundary nodes

  for(int i = 0; i < 6; i++)
  {
          vtkPoints* points = NULL;
          vtkPointSet* pointset = vtkStructuredGrid::New();
          vtkMimxPlanarTransfiniteInterpolation *planeinterp = vtkMimxPlanarTransfiniteInterpolation::New();
          pointset->SetPoints(this->GetFace(i));
          planeinterp->SetInput(pointset);
        
          if(i==0 || i==1)
          {
                  planeinterp->SetIDiv(this->Dimensions[1]);
                  planeinterp->SetJDiv(this->Dimensions[2]);
          }

          if(i==2 || i==3)
          {
                  planeinterp->SetIDiv(this->Dimensions[0]);
                  planeinterp->SetJDiv(this->Dimensions[2]);
          }
        
          if(i==4 || i==5)
          {
                  planeinterp->SetIDiv(this->Dimensions[0]);
                  planeinterp->SetJDiv(this->Dimensions[1]);
          }
         planeinterp->Update();
         this->SetFace(i,planeinterp->GetOutput()->GetPoints());
         this->GetFace(i)->Delete();
         pointset->Delete();
         planeinterp->Delete();
  }
 // double x[3];
 // for(int i=0; i < output->GetPoints()->GetNumberOfPoints(); i++)
 // {
        //output->GetPoint(i,x);
        ////// cout <<i<<"  "<<x[0]<<"  "<<x[1]<<"  "<<x[2]<<endl;
 // }
  // generate the interior nodes of the solid mesh
  vtkMimxSolidTransfiniteInterpolation *solidinterp = vtkMimxSolidTransfiniteInterpolation::New();
  solidinterp->SetIDiv(this->Dimensions[0]);
  solidinterp->SetJDiv(this->Dimensions[1]);
  solidinterp->SetKDiv(this->Dimensions[2]);
  vtkPointSet *pointset = vtkStructuredGrid::New();
  pointset->SetPoints(output->GetPoints());
  solidinterp->SetInput(pointset);
  solidinterp->Update();
  output->SetPoints(solidinterp->GetOutput()->GetPoints());
  output->Modified();
  solidinterp->Delete();
 /* vtkStructuredGridWriter *writer = vtkStructuredGridWriter::New();
 writer->SetInput(output);
 writer->SetFileName("sgrid.vtk");
 writer->Write();*/
  return 1;
}
int vtkMimxEdgeToStructuredGrid::FillInputPortInformation(int port, vtkInformation *info)
{
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
        return 1;
}

void vtkMimxEdgeToStructuredGrid::SetEdge(int EdgeNum, vtkPolyData* Polydata)
{
        this->SetInput(EdgeNum, Polydata);
}

int vtkMimxEdgeToStructuredGrid::SetEdge(int EdgeNum)
{
        
        vtkPolyData *polydata  = NULL;
        polydata = vtkPolyData::SafeDownCast(this->GetInput(EdgeNum));
        
        vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(this->GetOutputDataObject(0));
//        vtkInformation *outInfo = outputVector->GetInformationObject(0);
//        vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
//                outInfo->Get(vtkDataObject::DATA_OBJECT()));

        if(EdgeNum == 0){
                vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                subdivide->SetInput(polydata);
                subdivide->SetNumberOfDivisions(this->Dimensions[0]-1);
                subdivide->Update();
                for(int i = 0; i < subdivide->GetOutput()->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(i,subdivide->GetOutput()->GetPoint(i));
        //                // cout <<i<<"  ";
                }
        //        // cout<<endl;
                subdivide->Delete();
                return 1;
        }

        if(EdgeNum == 1){
                vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                subdivide->SetInput(polydata);
                subdivide->SetNumberOfDivisions(this->Dimensions[1]-1);
                subdivide->Update();
                for(int i = 0; i < subdivide->GetOutput()->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(this->Dimensions[0]*(i+1)-1,
                                subdivide->GetOutput()->GetPoint(i));
        //                // cout<<this->Dimensions[0]*(i+1)-1<<"  ";
                }
        //        // cout<<endl;
                subdivide->Delete();
                return 1;
        }

        if(EdgeNum == 2){
                vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                subdivide->SetInput(polydata);
                subdivide->SetNumberOfDivisions(this->Dimensions[0]-1);
                subdivide->Update();
                for(int i = 0; i < subdivide->GetOutput()->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint((this->Dimensions[1]-1)*
                                this->Dimensions[0]+i,subdivide->GetOutput()->GetPoint(i));
        //                // cout<<(this->Dimensions[1]-1)*this->Dimensions[0]+i<<"  ";
                }
        //        // cout<<endl;
                subdivide->Delete();
                return 1;
        }

        if(EdgeNum == 3){
                vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                subdivide->SetInput(polydata);
                subdivide->SetNumberOfDivisions(this->Dimensions[1]-1);
                subdivide->Update();
                for(int i = 0; i < subdivide->GetOutput()->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(this->Dimensions[0]*(i),
                                subdivide->GetOutput()->GetPoint(i));
        //                // cout<<this->Dimensions[0]*(i)<<"  ";
                }
        //        // cout<<endl;
                subdivide->Delete();
                return 1;
        }

        if(EdgeNum == 4){
                vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                subdivide->SetInput(polydata);
                subdivide->SetNumberOfDivisions(this->Dimensions[0]-1);
                subdivide->Update();
                for(int i = 0; i < subdivide->GetOutput()->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(this->Dimensions[1]*(this->Dimensions[2]-1)
                                *this->Dimensions[0]+i,subdivide->GetOutput()->GetPoint(i));
//                        // cout<<this->Dimensions[1]*(this->Dimensions[2]-1)*this->Dimensions[0]+i<<"  ";
                }
        //        // cout<<endl;
                subdivide->Delete();
                return 1;
        }

        if(EdgeNum == 5){
                vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                subdivide->SetInput(polydata);
                subdivide->SetNumberOfDivisions(this->Dimensions[1]-1);
                subdivide->Update();
                for(int i = 0; i < subdivide->GetOutput()->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(this->Dimensions[1]*(this->Dimensions[2]-1)
                                *this->Dimensions[0] + this->Dimensions[0]*(i+1)-1,
                                subdivide->GetOutput()->GetPoint(i));
//                        // cout<<this->Dimensions[1]*(this->Dimensions[2]-1)
//                                *this->Dimensions[0] + this->Dimensions[0]*(i+1)-1<<"  ";
                }
//                // cout<<endl;
                subdivide->Delete();
                return 1;
        }

        if(EdgeNum == 6){
                vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                subdivide->SetInput(polydata);
                subdivide->SetNumberOfDivisions(this->Dimensions[0]-1);
                subdivide->Update();
                for(int i = 0; i < subdivide->GetOutput()->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(this->Dimensions[1]*(this->Dimensions[2]-1)
                                *this->Dimensions[0]+ (this->Dimensions[1]-1)*
                                this->Dimensions[0]+i,subdivide->GetOutput()->GetPoint(i));
                        //// cout<<this->Dimensions[1]*(this->Dimensions[2]-1)
                        //        *this->Dimensions[0]+ (this->Dimensions[1]-1)*
                        //        this->Dimensions[0]+i<<"  ";
                }
//                // cout<<endl;
                subdivide->Delete();
                return 1;
        }

        if(EdgeNum == 7){
                vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                subdivide->SetInput(polydata);
                subdivide->SetNumberOfDivisions(this->Dimensions[1]-1);
                subdivide->Update();
                for(int i = 0; i < subdivide->GetOutput()->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(this->Dimensions[1]*(this->Dimensions[2]-1)
                                *this->Dimensions[0]+this->Dimensions[0]*(i),
                                subdivide->GetOutput()->GetPoint(i));
                        //// cout<<this->Dimensions[1]*(this->Dimensions[2]-1)
                        //        *this->Dimensions[0]+this->Dimensions[0]*(i)<<"  ";
                }
        //        // cout<<endl;
                subdivide->Delete();
                return 1;
        }

        if(EdgeNum == 8){
                vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                subdivide->SetInput(polydata);
                subdivide->SetNumberOfDivisions(this->Dimensions[2]-1);
                subdivide->Update();
                for(int i = 0; i < subdivide->GetOutput()->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(i*this->Dimensions[0]
                                *this->Dimensions[1],subdivide->GetOutput()->GetPoint(i));
//                        // cout<<i*this->Dimensions[0]*this->Dimensions[1]<<"  ";
                }
//                // cout<<endl;
                subdivide->Delete();
                return 1;
        }

        if(EdgeNum == 9){
                vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                subdivide->SetInput(polydata);
                subdivide->SetNumberOfDivisions(this->Dimensions[2]-1);
                subdivide->Update();
                for(int i = 0; i < subdivide->GetOutput()->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(i*this->Dimensions[0]
                        *this->Dimensions[1]+this->Dimensions[0]-1,
                                subdivide->GetOutput()->GetPoint(i));
//                        // cout<<i*this->Dimensions[0]*this->Dimensions[1]+this->Dimensions[0]-1<<"  ";
                }
//                // cout<<endl;
                subdivide->Delete();
                return 1;
        }

        if(EdgeNum == 11){
                vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                subdivide->SetInput(polydata);
                subdivide->SetNumberOfDivisions(this->Dimensions[2]-1);
                subdivide->Update();
                for(int i = 0; i < subdivide->GetOutput()->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(i*this->Dimensions[0]
                        *this->Dimensions[1]+this->Dimensions[0]*this->Dimensions[1]-1,
                                subdivide->GetOutput()->GetPoint(i));
                        /*// cout<<i*this->Dimensions[0]
                        *this->Dimensions[1]+this->Dimensions[0]*this->Dimensions[1]-1<<"  ";*/
                }
        //        // cout<<endl;
                subdivide->Delete();
                return 1;
        }

        if(EdgeNum == 10){
                vtkMimxSubdivideCurve *subdivide = vtkMimxSubdivideCurve::New();
                subdivide->SetInput(polydata);
                subdivide->SetNumberOfDivisions(this->Dimensions[2]-1);
                subdivide->Update();
                for(int i = 0; i < subdivide->GetOutput()->GetNumberOfPoints(); i++)
                {
                        output->GetPoints()->SetPoint(i*this->Dimensions[0]
                        *this->Dimensions[1]+this->Dimensions[0]*(this->Dimensions[1]-1),
                                subdivide->GetOutput()->GetPoint(i));
        /*                // cout<<i*this->Dimensions[0]
                        *this->Dimensions[1]+this->Dimensions[0]*(this->Dimensions[1]-1)<<"  ";*/
                }
        //        // cout<<endl;
                subdivide->Delete();
                return 1;
        }
        return 0;
}

vtkPoints* vtkMimxEdgeToStructuredGrid::GetFace(int FaceNum)
{
        vtkPoints *PointList = vtkPoints::New();

        int dim[3];
        this->GetOutput()->GetDimensions(dim);

        if(FaceNum == 0)
        {
                PointList->SetNumberOfPoints(dim[1]*dim[2]);
                for(int y=0; y < dim[2]; y++)
                {
                        for(int x=0; x < dim[1]; x++)
                        {
                                PointList->InsertPoint(y*dim[1] + x, this->GetOutput()
                                        ->GetPoint(y*dim[0]*dim[1]+x*dim[0]));
                                //// cout << y*dim[1]+x<<"  "<<y*dim[0]*dim[1]+x*dim[0]<<endl;
                        }
                }
                return PointList;
        }
        if(FaceNum == 1)
        {
                PointList->SetNumberOfPoints(dim[1]*dim[2]);
                for(int y=0; y < dim[2]; y++)
                {
                        for(int x=0; x < dim[1]; x++)
                        {
                                PointList->InsertPoint(y*dim[1] + x, this->GetOutput()
                                        ->GetPoint(y*dim[0]*dim[1]+x*dim[0]+dim[0]-1));
                                //// cout <<y*dim[1] + x<<"  "<<y*dim[0]*dim[1]+x*dim[0]+dim[0]-1<<endl;
                        }
                }
                return PointList;
        }
        if(FaceNum == 2)
        {
                PointList->SetNumberOfPoints(dim[0]*dim[2]);
                for(int y=0; y < dim[2]; y++)
                {
                        for(int z=0; z < dim[0]; z++)
                        {
                                PointList->InsertPoint(y*dim[0] + z, 
                                        this->GetOutput()->GetPoint(y*dim[0]*dim[1]+z));
                                //// cout << y*dim[0]*dim[1]+z<<"  "<<endl;
                        }
                }
                return PointList;
        }

        if(FaceNum == 3)
        {
                PointList->SetNumberOfPoints(dim[0]*dim[2]);
                for(int y=0; y < dim[2]; y++)
                {
                        for(int z=0; z < dim[0]; z++)
                        {
                                PointList->InsertPoint(y*dim[0] + z,this->GetOutput()->
                                        GetPoint(y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z));
                                //// cout << y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z<<"  "<<endl;
                        }
                }
                return PointList;
        }

        if(FaceNum == 4)
        {
                PointList->SetNumberOfPoints(dim[0]*dim[1]);
                for(int x=0; x < dim[1]; x++)
                {
                        for(int z=0; z < dim[0]; z++)
                        {
                                PointList->InsertPoint(x*dim[0] + z, 
                                        this->GetOutput()->GetPoint(x*dim[0]+z));
                                //// cout <<x*dim[0] + z <<endl;
                        }
                }
                return PointList;
        }

        if(FaceNum == 5)
        {
                PointList->SetNumberOfPoints(dim[0]*dim[1]);
                for(int x=0; x < dim[1]; x++)
                {
                        for(int z=0; z < dim[0]; z++)
                        {
                                PointList->InsertPoint(x*dim[0] + z, this->GetOutput()
                                        ->GetPoint((dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z));
                                //// cout<<(dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z<<endl;
                        }
                }
        return PointList;
        }
        return NULL;
}

int vtkMimxEdgeToStructuredGrid::SetFace(int FaceNum, vtkPoints *PointsList)
{
        int dim[3];
        this->GetOutput()->GetDimensions(dim);

        if(FaceNum == 0)
        {
                for(int y=1; y < dim[2]-1; y++)
                {
                        for(int x=1; x < dim[1]-1; x++)
                        {
                                this->GetOutput()->GetPoints()
                                        ->SetPoint(y*dim[0]*dim[1]+x*dim[0],
                                        PointsList->GetPoint(y*dim[1] + x));
                                //// cout<<y*dim[0]*dim[1]+x*dim[0]<<"  "<<y*dim[1] + x<<endl;
                        }
                }
                return 1;
        }
        
        if(FaceNum == 1)
        {
                for(int y=1; y < dim[2]-1; y++)
                {
                        for(int x=1; x < dim[1]-1; x++)
                        {
                                this->GetOutput()->GetPoints()->SetPoint(y*dim[0]*dim[1]+x*dim[0]+dim[0]-1,
                                        PointsList->GetPoint(y*dim[1] + x));
                                //// cout <<y*dim[0]*dim[1]+x*dim[0]+dim[0]-1<<"  "<<y*dim[1] + x<<endl;
                        }
                }
                return 1;
        }

        if(FaceNum == 2)
        {
                for(int y=1; y < dim[2]-1; y++)
                {
                        for(int z=1; z < dim[0]-1; z++)
                        {
                                this->GetOutput()->GetPoints()->SetPoint(
                                        y*dim[0]*dim[1]+z,PointsList->GetPoint(y*dim[0] + z));
                                //// cout << y*dim[0]*dim[1]+z<<endl;
                        }
                }
                return 1;
        }
        
        if(FaceNum == 3)
        {
                for(int y=1; y < dim[2]-1; y++)
                {
                        for(int z=1; z < dim[0]-1; z++)
                        {
                                this->GetOutput()->GetPoints()->SetPoint
                                        (y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z,
                                        PointsList->GetPoint(y*dim[0] + z));
                                //// cout<<y*dim[0]*dim[1]+(dim[1]-1)*dim[0]+z<<endl;
                        }
                }
                return 1;
        }
        if(FaceNum == 4)
        {
                for(int x=1; x < dim[1]-1; x++)
                {
                        for(int z=1; z < dim[0]-1; z++)
                        {
                                this->GetOutput()->GetPoints()->SetPoint(x*dim[0]+z,
                                        PointsList->GetPoint(x*dim[0] + z));
                                //// cout<<x*dim[0]+z<<endl;
                        }
                }
                return 1;
        }

        if(FaceNum == 5)
        {
                for(int x=1; x < dim[1]-1; x++)
                {
                        for(int z=1; z < dim[0]-1; z++)
                        {
                                this->GetOutput()->GetPoints()->SetPoint
                                        ((dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z,
                                        PointsList->GetPoint(x*dim[0] + z));
                                //// cout<<(dim[2]-1)*dim[1]*dim[0]+x*dim[0]+z<<endl;
                        }
                }
                return 1;
        }
        return 0;
}
void vtkMimxEdgeToStructuredGrid::SetDimensions(int dim[3])
{
        this->Dimensions[0] = dim[2];
        this->Dimensions[1] = dim[0];
        this->Dimensions[2] = dim[1];
}

void vtkMimxEdgeToStructuredGrid::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
