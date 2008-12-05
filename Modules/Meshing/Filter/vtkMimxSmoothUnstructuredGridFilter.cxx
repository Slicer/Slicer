/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSmoothUnstructuredGridFilter.cxx,v $
Language:  C++
Date:      $Date: 2007/10/09 16:38:15 $
Version:   $Revision: 1.11 $

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

#include "vtkMimxSmoothUnstructuredGridFilter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkGeometryFilter.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxConstrainedSmoothPolyDataFilter.h"

vtkCxxRevisionMacro(vtkMimxSmoothUnstructuredGridFilter, "$Revision: 1.11 $");
vtkStandardNewMacro(vtkMimxSmoothUnstructuredGridFilter);


vtkMimxSmoothUnstructuredGridFilter::vtkMimxSmoothUnstructuredGridFilter()
{
        this->SetNumberOfInputPorts(2);
        this->ConstrainedSmoothing = 0;
        this->Source = NULL;
        this->NumberOfIterations = 1;
}

vtkMimxSmoothUnstructuredGridFilter::~vtkMimxSmoothUnstructuredGridFilter()
{
}

int vtkMimxSmoothUnstructuredGridFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
 
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkPolyData *source = vtkPolyData::SafeDownCast(
          sourceInfo->Get(vtkDataObject::DATA_OBJECT()));
  // get the input and output
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
          inInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
          outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType numPts,numCells;
  // Check input
  //
  numPts=input->GetNumberOfPoints();
  numCells=input->GetNumberOfCells();
  if (numPts < 1 || numCells < 1)
  {
          vtkErrorMacro(<<"No data to smooth!");
          return 1;
  }

  output->CopyStructure(input);
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());

  if ( this->NumberOfIterations <= 0 ) 
  { //don't do anything! pass data through
          return 1;
  }

  // if smoothing variables are valid, begin smoothing
  // construct a connectivity array which stores the connectivity
  // neighbors of any given vertex
  // start with surface vertices

  vtkGeometryFilter* geofil = vtkGeometryFilter::New();
  geofil->SetInput(input);
  geofil->Update();
  // integer array stores the value of zero corresponding to
  // an interior node
  //vtkIntArray* intarray = vtkIntArray::New();
  //intarray->SetNumberOfValues(input->GetNumberOfPoints());
  //// connected neighbors for interior nodes
  //vtkCellArray* cellarray = vtkCellArray::New();

  //geofil->GetOutput()->BuildLinks();
  //input->BuildLinks();
  //vtkIdList* connectedneighborlist = vtkIdList::New();

  //vtkIdList *cellidlist, *ptids;
  //cellidlist = vtkIdList::New();
  //ptids = vtkIdList::New();

  //// check if the vertex is on the surface or not
  //// non-zero if on the surface and zero if interior
  //for(int i=0; i < geofil->GetOutput()->GetPoints()->GetNumberOfPoints(); i++)
  //{
         // vtkIdType* cellids;
         // unsigned short ncells;
         // geofil->GetOutput()->GetPointCells(i,ncells,cellids);
         // intarray->SetValue(i,ncells);
         // // if the vertex is interior
         // if(!ncells)
         // {
                //  input->GetPointCells(i,cellidlist);
                //  if(cellidlist->GetNumberOfIds() > 0)
                //  {
                //      
                //  for(int j=0; j< cellidlist->GetNumberOfIds(); j++)
                //  {
                //        vtkIdType cellid = cellidlist->GetId(j);
                //        
                //        input->GetCellPoints(cellid,ptids);
                //        // compute the connected vertices
                //      bool status = false;
                //      int k=0;
                //    while (!status) 
                //       {
                //               if(i==ptids->GetId(k)) status = true;
                //               k++;
                //     }
                //      this->InsertConnectedNeighbors(k-1,ptids,connectedneighborlist);
                //      }
                //  cellarray->InsertNextCell(connectedneighborlist->GetNumberOfIds());
                //  for(int k=0; k < connectedneighborlist->GetNumberOfIds(); k++)
                //  {
                //        cellarray->InsertCellPoint(connectedneighborlist->GetId(k));
                //  }
                //  ////// cout<<i<<"  ";
                //  //for(int k=0; k < connectedneighborlist->GetNumberOfIds(); k++)
                //  //{
                //       // // cout<<connectedneighborlist->GetId(k)<<"  ";
                //  //}
                //  //// cout <<endl;
                //  connectedneighborlist->Initialize();
         // }
                //  else
                //  {
                //        intarray->SetValue(i,1);
                //  }
         // }
  //}
  //connectedneighborlist->Delete();
  /*if(this->ConstrainedSmoothing)
  {*/
        vtkSmoothPolyDataFilter* smooth = vtkSmoothPolyDataFilter::New();
  /*}
  else
  {*/
          //vtkSmoothPolyDataFilter* smooth = vtkSmoothPolyDataFilter::New();
  //}
  smooth->SetInput(geofil->GetOutput());

 // smooth->BoundarySmoothingOn();
  smooth->SetNumberOfIterations(this->NumberOfIterations);
  smooth->SetRelaxationFactor(1.0);
 // smooth->FeatureEdgeSmoothingOn();
 // smooth->SetSource(this->Source);
  smooth->SetSource(source);
  smooth->Update();

  //vtkIdType npts;
  //vtkIdType* pts;
  //double x[3];
  // reset interior nodes
  smooth->GetOutput()->BuildLinks();
  for (int i=0; i < smooth->GetOutput()->GetNumberOfPoints(); i++)
  {
                vtkIdType *cells;
                unsigned short numcells;
          smooth->GetOutput()->GetPointCells(i,numcells,cells);
          if(!numcells)
          {
                  smooth->GetOutput()->GetPoints()->SetPoint(i, input->GetPoint(i));
          }
  }
  output->SetPoints(smooth->GetOutput()->GetPoints());

//  for(int i=0; i < this->NumberOfIterations; i++)
//  {
//        //vtkPolyData *smoothinput = vtkPolyData::SafeDownCast(smooth->GetOutput());
//        //ofstream FileOutput;
//        //FileOutput.open("points.txt", std::ios::out);
//        //for (int i=0; i<smoothinput->GetNumberOfPoints(); i++)
//        //{
//               // double x[3];
//               // smoothinput->GetPoint(i,x);
//               // FileOutput << i<<"  "<< x[0]<<"  "<< x[1]<<"  "<< x[2]<<endl;
//        //}
//        //FileOutput.close();
//        cellarray->InitTraversal();
//        for(int j=0; j < output->GetNumberOfPoints(); j++)
//        {
//                if(!intarray->GetValue(j))
//                {
//                cellarray->GetNextCell(npts,pts);
//                double newloc[3];
//                for (int m=0; m<3; m++)
//                 {
//                   newloc[m] = 0.0;
//                 }
//                for(int k=0; k <npts; k++)
//                {
//                        output->GetPoint(pts[k],x);
//                        for (int m=0; m<3; m++)
//                          {
//                           newloc[m] = newloc[m] + x[m];
//                          }
//                }
//                for (int m=0; m<3; m++)
//                  {
//                   newloc[m] = newloc[m]/npts;
//                  }
////              output->GetPoints()->SetPoint(j,newloc);
//                }
//        }
//        //vtkPolyData::SafeDownCast(smooth->GetInput())->SetPoints(output->GetPoints());
//  }
 
  geofil->Delete();
  smooth->Delete();
return 1;
}



void vtkMimxSmoothUnstructuredGridFilter::InsertConnectedNeighbors
        (vtkIdType Pos, vtkIdList* CellIdList, vtkIdList* ConnectedNeighbors)
{
        // right now the values have been hard coded for a hexahedron 
        // unstructured grid
        switch (Pos) {
                case 0:
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(1));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(3));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(4));
                        break;
                case 1:
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(0));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(2));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(5));
                        break;
                case 2:
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(1));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(3));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(6));
                        break;
                case 3:
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(0));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(2));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(7));
                        break;
                case 4:
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(5));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(7));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(0));
                        break;
                case 5:
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(4));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(6));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(1));
                        break;
                case 6:
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(5));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(7));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(2));
                        break;
                case 7:
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(4));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(6));
                        ConnectedNeighbors->InsertUniqueId(CellIdList->GetId(3));
                        break;
        }
}

int vtkMimxSmoothUnstructuredGridFilter::FillInputPortInformation(int port, vtkInformation *info)
{
        if(port == 0)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
                return 1;
        }
        else if(port == 1)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
        }
        return 0;
}

void vtkMimxSmoothUnstructuredGridFilter::SetSource(vtkPolyData *source)
{
        this->SetInput(1, source);
}

void vtkMimxSmoothUnstructuredGridFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
