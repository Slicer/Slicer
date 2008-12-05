/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSelectSurface.cxx,v $
Language:  C++
Date:      $Date: 2007/08/21 19:49:14 $
Version:   $Revision: 1.6 $

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

#include "vtkMimxSelectSurface.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCleanPolyData.h"
#include "vtkDataSet.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkGarbageCollector.h"
#include "vtkGenericCell.h"
#include "vtkGeometryFilter.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLine.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include "vtkSelectPolyData.h"
#include "vtkMimxPolyDataSingleSourceShortestPath.h"
#include "vtkUnstructuredGrid.h"

#include "vtkPolyDataWriter.h"

vtkCxxRevisionMacro(vtkMimxSelectSurface, "$Revision: 1.6 $");
vtkStandardNewMacro(vtkMimxSelectSurface);

vtkCxxSetObjectMacro(vtkMimxSelectSurface,Loop,vtkIdList);

// Description:
// Instantiate object with InsideOut turned off.
vtkMimxSelectSurface::vtkMimxSelectSurface()
{
  this->SetNumberOfOutputPorts(2);
  this->Loop = NULL;
  vtkPolyData *output2 = vtkPolyData::New();
  this->GetExecutive()->SetOutputData(1, output2);
  output2->Delete();
  this->PointIdListIn = vtkIdList::New();
  this->PointIdListOut = vtkIdList::New();
  this->CellIdListIn = vtkIdList::New();
  this->CellIdListOut = vtkIdList::New();
}

//----------------------------------------------------------------------------
vtkMimxSelectSurface::~vtkMimxSelectSurface()
{
  if (this->Loop)
    {
    this->Loop->Delete();
    }
  this->PointIdListIn->Delete();
  this->PointIdListOut->Delete();
  this->CellIdListIn->Delete();
  this->CellIdListOut->Delete();
}

//----------------------------------------------------------------------------
int vtkMimxSelectSurface::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPolyData* surfpolydata = NULL;
  vtkGeometryFilter *geofil = NULL;
  if(this->Loop->GetNumberOfIds() < 3)
  {
          vtkErrorMacro("Mimimum of 3 points needed to define a contour");
          return 0;
  }

  if(input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
  {
          geofil = vtkGeometryFilter::New();
          geofil->SetInput(vtkUnstructuredGrid::SafeDownCast(input));
          geofil->Update();
          surfpolydata = geofil->GetOutput();
  }
  else
  {
          surfpolydata = vtkPolyData::SafeDownCast(input);
  }

  int i, j, k;
  //this->Loop->Initialize();
  //this->Loop->SetNumberOfIds(4);
  //this->Loop->SetId(0, 394);
  //this->Loop->SetId(1, 388);
  //this->Loop->SetId(2, 424);
  //this->Loop->SetId(3, 430);
  // find the shortest path between the consecutive points
  vtkPoints *PointsInBetween = vtkPoints::New();
 for (i =0; i < this->Loop->GetNumberOfIds()-1; i++) {
         vtkMimxPolyDataSingleSourceShortestPath *path = 
                 vtkMimxPolyDataSingleSourceShortestPath::New();
         path->SetInput(surfpolydata);
         path->SetStartVertex(this->Loop->GetId(i));
         path->SetEndVertex(this->Loop->GetId(i+1));
         path->Update();
         for(j=0; j < path->GetIdList()->GetNumberOfIds()-1; j++)
         {
                 int num = path->GetIdList()->GetId(j);
                PointsInBetween->InsertNextPoint(surfpolydata->GetPoint(
                        path->GetIdList()->GetId(j)));   
         }
         path->Delete();
 }
 // for the line connecting last and first point
 if(this->Loop->GetId(0) != this->Loop->GetId(
         this->Loop->GetNumberOfIds()-1))
 {
         vtkMimxPolyDataSingleSourceShortestPath *path = 
                 vtkMimxPolyDataSingleSourceShortestPath::New();
         path->SetInput(surfpolydata);
         path->SetStartVertex(this->Loop->GetId(this->Loop->GetNumberOfIds()-1));
         path->SetEndVertex(this->Loop->GetId(0));
         path->Update();
         for(j=0; j < path->GetIdList()->GetNumberOfIds(); j++)
         {
                 int num = path->GetIdList()->GetId(j);
                 PointsInBetween->InsertNextPoint(surfpolydata->GetPoint(
                         path->GetIdList()->GetId(j)));  
         }
         path->Delete();
 }
  vtkSelectPolyData *selectpolydata = vtkSelectPolyData::New();
  selectpolydata->SetLoop(PointsInBetween);
  selectpolydata->SetInput(surfpolydata);
  selectpolydata->Update();

  // store the point id lists in the element set inside the loop

  unsigned short ncells;
  vtkIdType *cells;

  selectpolydata->GetOutput()->BuildLinks();
  for(i = 0; i < selectpolydata->GetOutput()->GetNumberOfPoints(); i++)
  {
        selectpolydata->GetOutput()->GetPointCells(i, ncells, cells);
        if(ncells)      
        {
                this->PointIdListIn->InsertNextId(i);
        }
  }
  
  // point id list for the elements outside the loop
  // also store a point list of all the surface points
  vtkIdList *surfpoilist = vtkIdList::New();
  surfpolydata->BuildLinks();
  for(i = 0; i < surfpolydata->GetNumberOfPoints(); i++)
  {
          surfpolydata->GetPointCells(i, ncells, cells);
          if(ncells)    {
                  surfpoilist->InsertNextId(i);
                  if(this->PointIdListIn->IsId(i) == -1)
                  {
                        this->PointIdListOut->InsertNextId(i);                  
                  }
         }
  }
  // boundary nodes are common to both point lists
  selectpolydata->GetSelectionEdges()->BuildLinks();
  for (i=0; i<selectpolydata->GetSelectionEdges()->GetNumberOfPoints(); i++)
  {
        selectpolydata->GetSelectionEdges()->GetPointCells(i, ncells, cells);
        if(ncells)      
        {
                this->PointIdListOut->InsertNextId(i);  
        }
  }
        
  /*vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
  writer->SetInput(selectpolydata->GetSelectionEdges());
  writer->SetFileName("polydata3.vtk");
  writer->Write();*/
//  output->SetPoints(surfpolydata->GetPoints());
  vtkPolyData *outputout =  vtkPolyData::SafeDownCast(this->GetExecutive()->GetOutputData(1));
//  outputout->SetPoints(surfpolydata->GetPoints());

  // for unstructured grid input
  // cell id list inside the loop. all the points of the polygon
  // should lie inside the loop
  vtkCellArray *incellarray = vtkCellArray::New();

  vtkIdType npts, *pts;

if(input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
{
        vtkGenericCell *gencell = vtkGenericCell::New();
        vtkIdList *faceids;
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(input);
         for (i=0; i <ugrid->GetNumberOfCells(); i++)
         {
                 gencell->Initialize();
                ugrid->GetCell(i, gencell);
                for (j=0; j < gencell->GetNumberOfFaces(); j++)
                {
                        faceids = gencell->GetFace(j)->GetPointIds();
                        int status = 0;
                        for( k=0; k < faceids->GetNumberOfIds(); k++)
                        {
                                if(this->PointIdListIn->IsId(faceids->GetId(k)) == -1)
                                {
                                        status = 1;
                                        break;
                                }
                        }
                        if (!status)
                        {
                                this->CellIdListIn->InsertNextId(i);
                                break;
                        }
                }
         }
         gencell->Delete();
}

// for polydata input
// in this loop both cell ids and cells belonging to the loop are
// stored
// to store which cell in the surface has already been input
vtkIdList *surfidlist = vtkIdList::New();

for (i =0; i < surfpolydata->GetNumberOfCells(); i++)
{
        surfpolydata->GetCellPoints(i, npts, pts);
        int status = 0;
        // check whether all the points lie inside the loop
        for(j=0; j < npts; j++)
        {
                if(this->PointIdListIn->IsId(pts[j]) == -1)     status = 1;
        }
        if(!status)
        {
                surfidlist->InsertNextId(i);
                if(geofil)      this->CellIdListIn->InsertNextId(i);
                incellarray->InsertNextCell(npts);
                for(j=0; j < npts; j++)
                {
                        incellarray->InsertCellPoint(pts[j]);
                }
        }
}
  // cell id list outside the loop
 // for the unstructured grid the cells can be categorized into 3
// groups. group 1 , surface cells belonging to the loop. group 2,
// surface cells not belonging to the loop. group 3, all the 
// interior cells.
  vtkCellArray *outcellarray = vtkCellArray::New();
// for the unstructured grid
// list all the surface cells not belonging to the loop
  if(input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
  {
          // create a list of all the surface elements

          vtkGenericCell *gencell = vtkGenericCell::New();
          vtkIdList *faceids;
          vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(input);
          for (i=0; i <ugrid->GetNumberOfCells(); i++)
          {
                  gencell->Initialize();
                  ugrid->GetCell(i, gencell);
                  for (j=0; j < gencell->GetNumberOfFaces(); j++)
                  {
                          faceids = gencell->GetFace(j)->GetPointIds();
                          int status = 0;
                          for( k=0; k < faceids->GetNumberOfIds(); k++)
                          {
                                  if(surfpoilist->IsId(faceids->GetId(k)) == -1)
                                  {
                                          status = 1;
                                          break;
                                  }
                          }
                          if (!status)
                          {
                                  if(this->CellIdListIn->IsId(i) == -1) this->CellIdListOut->InsertNextId(i);
                                  break;
                          }
                  }
          }
          gencell->Delete();

  }

  // for polydata input
  // in this loop both cell ids and cells not belonging to the loop are
  // stored
  for (i =0; i < surfpolydata->GetNumberOfCells(); i++)
  {
          if(surfidlist->IsId(i) == -1)
          {
          surfpolydata->GetCellPoints(i, npts, pts);
          //int status = 0;
          //// check whether all the points lie inside the loop
          //for(j=0; j < npts; j++)
          //{
                 // if(this->PointIdListOut->IsId(pts[j]) == -1)        status = 1;
          //}
          //if(!status)
          //{
                  if(geofil)    this->CellIdListIn->InsertNextId(i);
                  outcellarray->InsertNextCell(npts);
                  for(j=0; j < npts; j++)
                  {
                          outcellarray->InsertCellPoint(pts[j]);
                  }
          //}
          }
  }
 // for(i=0; i < surfpoilist->GetNumberOfIds(); i++)
 // {
        //  if(this->PointIdListIn->IsId(surfpoilist->GetId(i)) == -1 && this->PointIdListOut->IsId(surfpoilist->GetId(i)) == -1)
        //  {
        //        cout << surfpoilist->GetId(i)<<endl;
        //  }
 // }
 // ofstream OFile;
 // OFile.open("ids.txt", std::ios::out);
 // OFile << "In points"<<endl;
 // for(i =0; i < this->PointIdListIn->GetNumberOfIds(); i++)
 // {
        //OFile << this->PointIdListIn->GetId(i)<<endl;
 // }

 // OFile << "out points"<<endl;
 // for(i =0; i < this->PointIdListOut->GetNumberOfIds(); i++)
 // {
        //  OFile << this->PointIdListOut->GetId(i)<<endl;
 // }
 // OFile << "surf points"<<endl;
 // for(i =0; i < surfpoilist->GetNumberOfIds(); i++)
 // {
        //  OFile << surfpoilist->GetId(i)<<endl;
 // }

 // OFile << "In cells"<<endl;
 // for(i =0; i < this->CellIdListIn->GetNumberOfIds(); i++)
 // {
        //  OFile << this->CellIdListIn->GetId(i)<<endl;
 // }

 // OFile << "Out cells"<<endl;
 // for(i =0; i < this->CellIdListOut->GetNumberOfIds(); i++)
 // {
        //  OFile << this->CellIdListOut->GetId(i)<<endl;
 // }

  vtkPolyData *polydatain, *polydataout;
  polydatain = vtkPolyData::New();
  polydatain->SetPoints(surfpolydata->GetPoints());
  polydatain->SetPolys(incellarray);

  polydataout = vtkPolyData::New();
  polydataout->SetPoints(surfpolydata->GetPoints());
  polydataout->SetPolys(outcellarray);

  vtkCleanPolyData *cleanin = vtkCleanPolyData::New();
  cleanin->SetInput(polydatain);
  cleanin->Update();
  output->DeepCopy(cleanin->GetOutput());
  cleanin->Delete();

  vtkCleanPolyData *cleanout = vtkCleanPolyData::New();
  cleanout->SetInput(polydataout);
  cleanout->Update();
  outputout->DeepCopy(cleanout->GetOutput());
  cleanout->Delete();

  if(geofil)    geofil->Delete();
  surfpoilist->Delete();
  selectpolydata->Delete();
  incellarray->Delete();
  outcellarray->Delete();
  surfidlist->Delete();
  polydatain->Delete();
  polydataout->Delete();
  return 1;
}
//---------------------------------------------------------------------------
vtkPolyData* vtkMimxSelectSurface::GetSurfaceIn()
{
        return this->GetOutput();
}
//---------------------------------------------------------------------------
vtkPolyData* vtkMimxSelectSurface::GetSurfaceOut()
{
        return vtkPolyData::SafeDownCast(this->GetExecutive()->GetOutputData(1));
}
//----------------------------------------------------------------------------
unsigned long int vtkMimxSelectSurface::GetMTime()
{
  unsigned long mTime=this->Superclass::GetMTime();
  unsigned long time;

  if ( this->Loop != NULL )
    {
    time = this->Loop->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}
//---------------------------------------------------------------------------
int vtkMimxSelectSurface::FillInputPortInformation(int, vtkInformation *info)
{
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
        return 1;
}
//----------------------------------------------------------------------------
void vtkMimxSelectSurface::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

