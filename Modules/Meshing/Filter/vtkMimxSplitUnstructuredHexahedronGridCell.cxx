/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSplitUnstructuredHexahedronGridCell.cxx,v $
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

#include "vtkMimxSplitUnstructuredHexahedronGridCell.h"

#include "vtkCell.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMergeCells.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkGenericCell.h"

vtkCxxRevisionMacro(vtkMimxSplitUnstructuredHexahedronGridCell, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkMimxSplitUnstructuredHexahedronGridCell);

// Description:

struct Node
{
        int CellNum;        
        int EdgeNum;
        Node* Parent;
        Node* PeerPrev;
        Node* PeerNext;
        Node* Child;
};

vtkMimxSplitUnstructuredHexahedronGridCell::vtkMimxSplitUnstructuredHexahedronGridCell()
{
        this->IdList = NULL;
}

vtkMimxSplitUnstructuredHexahedronGridCell::~vtkMimxSplitUnstructuredHexahedronGridCell()
{
}

int vtkMimxSplitUnstructuredHexahedronGridCell::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
 
  if(!this->IdList)
  {
          vtkErrorMacro("Enter the point numbers of the edge to be divided,  edge should contain only two points");
          return 0;
  }
  if(IdList->GetNumberOfIds() !=2)
  {
          vtkErrorMacro("Edge should contain only two points");
          return 0;
  }

  vtkDebugMacro(<< "Check if the edge points input are valid ?");
  vtkIdType numCells = input->GetNumberOfCells();
  vtkIdType numPts = input->GetNumberOfPoints();

// Initialize and check input
//
  int i, edge_num;
  bool status = false;
        int count_ele = 0;
  do {
        for(i=0; i < 12; i++)
        {
                vtkCell *cell = input->GetCell(count_ele)->GetEdge(i);
                //vtkIdType pt0 = cell->GetPointId(0);
                //vtkIdType pt1 = cell->GetPointId(1);
                if(cell->GetPointId(0) == IdList->GetId(0) || cell->GetPointId(1) == IdList->GetId(0))
                {
                        if(cell->GetPointId(0) == IdList->GetId(1) || cell->GetPointId(1) == IdList->GetId(1))
                        {
                                status = true;
                                break;
                        }
                }
        }
        count_ele++;
  } while(count_ele < input->GetNumberOfCells() && !status);

  if(!status)
  {
          vtkErrorMacro("Invalid edge points entered, check for correct edge points input");
          return 0;
  }
  // if the input is valid
   edge_num = i;
  double x1[3], x2[3];
  //        get points based on the plane being divided
  //        lines accessed are in CW
  // generate the connectivity to replace the original cell with
  //        the cell closest to the origin
//  vtkIdType* pts_replace;
  if(edge_num ==1 || edge_num == 3 || edge_num == 5 || edge_num == 7)
  {
          i = 1;
  }
  if(edge_num == 0 || edge_num == 2 || edge_num == 4 || edge_num == 6)
  {
          i = 3;
  }
  if(edge_num > 7 && edge_num < 12)
  {
          i = 2;
  }
  this->BuildMeshSeedLinks(count_ele-1, i);


  // storage of output
  // initially store the intermediate grid which has duplicate points
  vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
  ugrid->Allocate(input->GetNumberOfCells()*2,input->GetNumberOfCells()*2);
  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(0);
  for (int i=0; i < input->GetNumberOfPoints(); i++)
  {
          points->InsertNextPoint(input->GetPoint(i));
  }
  ugrid->SetPoints(points);
  points->Delete();
//        ugrid->CopyStructure(input);
  for(i=0; i <input->GetNumberOfCells(); i++)
  {
          vtkIdList *locallist = vtkIdList::New();
          locallist->DeepCopy(input->GetCell(i)->GetPointIds());
                ugrid->InsertNextCell(input->GetCellType(i), locallist);
        locallist->Delete();
  }
  for (int i=0; i < input->GetNumberOfCells(); i++)
  {
          if(this->MeshSeedLinks(1,i+1))
          {
                  vtkIdList *pts_replace = vtkIdList::New();
                  input->GetCellPoints(i,pts_replace);
                  //        list for the new cell being inserted into the grid
                  vtkIdList* idlistnew = vtkIdList::New();
                  idlistnew->SetNumberOfIds(8);
                  int j;
                  for(j=0; j <8; j++)        {idlistnew->SetId(j,pts_replace->GetId(j));}

//                  if(edge_num ==1 || edge_num == 3 || edge_num == 5 || edge_num == 7)
                  if(this->MeshSeedLinks(1,i+1) == 1)
                  {
                          // calculate the coordinates of the points
                          // point is located at the center of the line chosen
                          input->GetPoint(pts_replace->GetId(0),x1);        input->GetPoint(pts_replace->GetId(3),x2);
                          ugrid->GetPoints()->InsertNextPoint(
                                  (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                          input->GetPoint(pts_replace->GetId(4),x1);        input->GetPoint(pts_replace->GetId(7),x2);
                          ugrid->GetPoints()->InsertNextPoint(
                                  (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                          input->GetPoint(pts_replace->GetId(5),x1);        input->GetPoint(pts_replace->GetId(6),x2);
                          ugrid->GetPoints()->InsertNextPoint(
                                  (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                          input->GetPoint(pts_replace->GetId(1),x1);        input->GetPoint(pts_replace->GetId(2),x2);
                          ugrid->GetPoints()->InsertNextPoint(
                                  (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                          pts_replace->SetId(2, ugrid->GetNumberOfPoints()-1);
                          pts_replace->SetId(3, ugrid->GetNumberOfPoints()-4);
                          pts_replace->SetId(6, ugrid->GetNumberOfPoints()-2);
                          pts_replace->SetId(7, ugrid->GetNumberOfPoints()-3);
                          //for (int j=0; j < 8; j++)
                          //{
                                 // ugrid->GetCell(i)->GetPointIds()->SetId(j,pts_replace->GetId(j]);
                          //}
                          vtkIdType pts_idtype[8];
                          for(int k=0; k<8; k++)        pts_idtype[k] = pts_replace->GetId(k);
                          ugrid->ReplaceCell(i,8,pts_idtype);
                          // generate the connectivity to insert cell farther from the origin
                          //        into grid 
                          idlistnew->SetId(0,ugrid->GetNumberOfPoints()-4);
                          idlistnew->SetId(1,ugrid->GetNumberOfPoints()-1);
                          idlistnew->SetId(4,ugrid->GetNumberOfPoints()-3);
                          idlistnew->SetId(5,ugrid->GetNumberOfPoints()-2);
                          ugrid->InsertNextCell(12, idlistnew);

                          idlistnew->Delete();

                  }

//                  if(edge_num > 7 && edge_num < 12)
                  if(this->MeshSeedLinks(1,i+1) == 2)
                  {
                          // calculate the coordinates of the points
                          // point is located at the center of the line chosen
                          input->GetPoint(pts_replace->GetId(0),x1);        input->GetPoint(pts_replace->GetId(4),x2);
                          ugrid->GetPoints()->InsertNextPoint(
                                  (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                          input->GetPoint(pts_replace->GetId(1),x1);        input->GetPoint(pts_replace->GetId(5),x2);
                          ugrid->GetPoints()->InsertNextPoint(
                                  (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                          input->GetPoint(pts_replace->GetId(2),x1);        input->GetPoint(pts_replace->GetId(6),x2);
                          ugrid->GetPoints()->InsertNextPoint(
                                  (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                          input->GetPoint(pts_replace->GetId(3),x1);        input->GetPoint(pts_replace->GetId(7),x2);
                          ugrid->GetPoints()->InsertNextPoint(
                                  (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                          pts_replace->SetId(4, ugrid->GetNumberOfPoints()-4);
                          pts_replace->SetId(5, ugrid->GetNumberOfPoints()-3);
                          pts_replace->SetId(6, ugrid->GetNumberOfPoints()-2);
                          pts_replace->SetId(7, ugrid->GetNumberOfPoints()-1);
                          vtkIdType pts_idtype[8];
                          for(int k=0; k<8; k++)        pts_idtype[k] = pts_replace->GetId(k);
                          ugrid->ReplaceCell(i,8,pts_idtype);
                          // generate the connectivity to insert cell farther from the origin
                          //        into grid 
                          idlistnew->SetId(0,ugrid->GetNumberOfPoints()-4);
                          idlistnew->SetId(1,ugrid->GetNumberOfPoints()-3);
                          idlistnew->SetId(2,ugrid->GetNumberOfPoints()-2);
                          idlistnew->SetId(3,ugrid->GetNumberOfPoints()-1);
                          ugrid->InsertNextCell(12, idlistnew);
                          idlistnew->Delete();

                  }

//                  if(edge_num == 0 || edge_num == 2 || edge_num == 4 || edge_num == 6)
                  if( this->MeshSeedLinks(1,i+1) == 3)
                  {
                          // calculate the coordinates of the points
                          // point is located at the center of the line chosen
                          input->GetPoint(pts_replace->GetId(0),x1);        input->GetPoint(pts_replace->GetId(1),x2);
                          ugrid->GetPoints()->InsertNextPoint(
                                  (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                          input->GetPoint(pts_replace->GetId(3),x1);        input->GetPoint(pts_replace->GetId(2),x2);
                          ugrid->GetPoints()->InsertNextPoint(
                                  (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                          input->GetPoint(pts_replace->GetId(7),x1);        input->GetPoint(pts_replace->GetId(6),x2);
                          ugrid->GetPoints()->InsertNextPoint(
                                  (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                          input->GetPoint(pts_replace->GetId(4),x1);        input->GetPoint(pts_replace->GetId(5),x2);
                          ugrid->GetPoints()->InsertNextPoint(
                                  (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                          pts_replace->SetId(1, ugrid->GetNumberOfPoints()-4);
                          pts_replace->SetId(2, ugrid->GetNumberOfPoints()-3);
                          pts_replace->SetId(5, ugrid->GetNumberOfPoints()-1);
                          pts_replace->SetId(6, ugrid->GetNumberOfPoints()-2);
                          vtkIdType pts_idtype[8];
                          for(int k=0; k<8; k++)        pts_idtype[k] = pts_replace->GetId(k);
                          ugrid->ReplaceCell(i,8,pts_idtype);
                          // generate the connectivity to insert cell farther from the origin
                          //        into grid 
                          idlistnew->SetId(0,ugrid->GetNumberOfPoints()-4);
                          idlistnew->SetId(3,ugrid->GetNumberOfPoints()-3);
                          idlistnew->SetId(4,ugrid->GetNumberOfPoints()-1);
                          idlistnew->SetId(7,ugrid->GetNumberOfPoints()-2);
                          ugrid->InsertNextCell(12, idlistnew);

                          idlistnew->Delete();
                  }

          }
          int temp = input->GetNumberOfCells();
  }
  //ugrid->SetPoints(points);
          ugrid->Squeeze();

        vtkMergeCells* mergecells = vtkMergeCells::New();
        mergecells->SetUnstructuredGrid(output);
        mergecells->MergeDuplicatePointsOn();
        mergecells->SetTotalNumberOfDataSets(1);
        mergecells->SetTotalNumberOfCells(ugrid->GetNumberOfCells());
        mergecells->SetTotalNumberOfPoints(ugrid->GetNumberOfPoints());

        mergecells->MergeDataSet(ugrid);
        mergecells->Finish();
        ugrid->Delete();
        // merge cells now
//        ugrid->Delete();

   return 1;
}


// mesh seed copy

void vtkMimxSplitUnstructuredHexahedronGridCell::BuildMeshSeedLinks(int CellNum, int EdgeNum)
{
//        this->Links = 1;
        vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(this->GetInput());
        this->MeshSeedLinks.SetSize(1,input->GetNumberOfCells());
        this->MeshSeedLinks.Set(0);
        this->MeshSeedCheck.SetSize(input->GetNumberOfCells(), 1);
        vtkIdList *neigh_cell_list = vtkIdList::New();

        CLinkedList<Node*> *meshseedtree =  new CLinkedList<Node*>;

        // starting point for the tree to be constructed
//        for (int i=0; i<input->GetNumberOfCells(); i++)
//        {
//                for (int j=1; j<=3; j++)
//                {
                        int i = CellNum;
                        int j = EdgeNum;
                        this->MeshSeedCheck.Set(0);
                        // starting point for the tree to be constructed
                        this->MeshSeedLinks(1,i+1) = j;
                        meshseedtree->Add(new Node);
                        meshseedtree->GetLastItem()->Parent = NULL;
                        meshseedtree->GetLastItem()->CellNum = i;
                        meshseedtree->GetLastItem()->EdgeNum = j;
                        // get only the neighboring cells that have not already been traversed
                        // first generate the tree structure checking for the edges being shared
                        vtkIdType cellnum = i;
                        this->MeshSeedCheck(i+1,1) = j;
                        neigh_cell_list->Initialize();
                        this->GetCellNeighbors(cellnum, neigh_cell_list);
                        Node *currpar = meshseedtree->GetLastItem();
                        currpar->CellNum = cellnum;
                        currpar->EdgeNum = j;
                        currpar->PeerPrev = NULL;
                        currpar->PeerNext = NULL;
                        currpar->Parent = NULL;
                        currpar->Child = NULL;
                        Node *currchild;
                        Node *currparpar = currpar;
                        Node *currpeerprev;
                        if(neigh_cell_list->GetNumberOfIds() !=0)
                        {
                                while (currparpar != NULL)
                                {
                                        int k;
                                        k=-1;
                                        int count = 0;
                                        do 
                                        {
                                                k++;
                                                if(currpar->EdgeNum == 1 )
                                                {this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1) = this->CheckIfCellsShareEdgeX(currpar->CellNum,neigh_cell_list->GetId(k),i);}
                                                else
                                                {if(currpar->EdgeNum == 2){this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1) = this->CheckIfCellsShareEdgeY(currpar->CellNum,neigh_cell_list->GetId(k),i);}
                                                else{this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1) = this->CheckIfCellsShareEdgeZ(currpar->CellNum,neigh_cell_list->GetId(k),i);}}
                                                if (this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1))
                                                {
                                                        if (!count)
                                                        {
                                                                currpar->Child = new Node;
                                                                meshseedtree->Add(currpar->Child);
                                                                currchild = currpar->Child;
                                                                currchild->CellNum = neigh_cell_list->GetId(k);
                                                                currchild->EdgeNum = this->MeshSeedLinks(1,neigh_cell_list->GetId(k)+1);
                                                                currchild->PeerPrev = NULL;
                                                                currchild->Parent = currpar;
                                                                count ++;
                                                        }
                                                        else{
                                                                currchild->PeerNext = new Node;        
                                                                meshseedtree->Add(currchild->PeerNext);
                                                                currpeerprev = currchild;
                                                                currchild = currchild->PeerNext;
                                                                currchild->PeerPrev = currpeerprev;
                                                                currchild->Parent = currpar;
                                                                currchild->PeerNext = NULL;
                                                                currchild->CellNum = neigh_cell_list->GetId(k);
                                                                currchild->EdgeNum = this->MeshSeedLinks(1,neigh_cell_list->GetId(k)+1);
                                                                count++;
                                                        }
                                                }
                                        } while(k < neigh_cell_list->GetNumberOfIds()-1);                                                

                                        //                                currparpar = currchild->Parent;
                                        if (!count)
                                        {
                                                if(currpar->PeerPrev !=NULL)        currpar = currpar->PeerPrev;
                                                else        
                                                {
                                                        if(currpar->Parent != NULL)currpar = currpar->Parent;
                                                }
                                        }
                                        else
                                        {
                                                currpar = currchild;
                                        }
                                        if (currpar->Parent != NULL)
                                        {
                                                this->GetCellNeighbors(currpar->CellNum, neigh_cell_list);
                                                if(neigh_cell_list->GetNumberOfIds() == 0)
                                                {
                                                        do 
                                                        {
                                                                if(currpar->PeerPrev == NULL)
                                                                {
                                                                        currpar = currpar->Parent;
                                                                }
                                                                else
                                                                {
                                                                        currpar = currpar->PeerPrev;
                                                                        this->GetCellNeighbors(currpar->CellNum, neigh_cell_list);                                                
                                                                }
                                                        } while(neigh_cell_list->GetNumberOfIds() == 0 && currpar->Parent != NULL);
                                                }
                                        }
                                        currparpar = currpar->Parent;
                                }
                        }
                        meshseedtree->Delete();
                        //                for (int k=0; k < input->GetNumberOfCells(); k++)
                        //{
                        //cout << this->MeshSeedLinks(1, k+1)<<"  ";
                        //}
                        //cout<<endl;
        //        }
        //}
        neigh_cell_list->Delete();
        delete meshseedtree;
}

void vtkMimxSplitUnstructuredHexahedronGridCell::GetCellNeighbors(vtkIdType cellnum, vtkIdList *cellids)
{
        cellids->Initialize();
        vtkIdList *ptids = vtkIdList::New();
        vtkIdList *ptids_temp = vtkIdList::New();
        vtkIdList *cellids_temp = vtkIdList::New();
        vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(this->GetInput());
        input->GetCellPoints(cellnum,ptids);
        for(int i=0; i < ptids->GetNumberOfIds(); i++)
        {
                //                ptids_temp->Initialize();
                ptids_temp->SetNumberOfIds(1);
                ptids_temp->SetId(0,ptids->GetId(i));
                cellids_temp->Initialize();
                input->GetCellNeighbors(cellnum,ptids_temp, cellids_temp);
                for(int j=0; j<cellids_temp->GetNumberOfIds(); j++)
                {
                        if(!this->MeshSeedCheck(cellids_temp->GetId(j)+1, 1))
                        {
                                cellids->InsertUniqueId(cellids_temp->GetId(j));
                        }
                }
        }

        vtkIdType pt;
        for (int i=0; i<cellids->GetNumberOfIds(); i++)
        {
                pt = cellids->GetId(i);
        }
        ptids->Delete();
        ptids_temp->Delete();
        cellids_temp->Delete();
}

int vtkMimxSplitUnstructuredHexahedronGridCell::CheckIfCellsShareEdgeX(int cellnum1, int cellnum2, int parcell)
{
        bool status1 = false;
        bool status2 = false;
        vtkGenericCell *cell = vtkGenericCell::New();
        vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(this->GetInput());
        input->GetCell(cellnum1, cell);
        vtkGenericCell *cell_comp = vtkGenericCell::New();
        input->GetCell(cellnum2, cell_comp);
        vtkIdType pt1, pt2;
        // start with x axis for 0 and 3
        int k;
        for(k=0; k<8; k++)
        {
                if(cell_comp->GetPointId(k) == cell->GetPointId(0))
                {
                        status1 = true;
                        break;
                }
        }
        pt1 = k;

        for(k=0; k<8; k++)
        {
                if(cell_comp->GetPointId(k) == cell->GetPointId(3))
                {
                        status2 = true;
                        break;
                }
        }        
        pt2 = k;

        if(status1 && status2)
        {                
                this->MeshSeedLinks(1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                cell->Delete();
                cell_comp->Delete();

                return 1;
        }
        else
        {
                // 1 and 2
                status1 = false; status2 = false;
                for(k=0; k<8; k++)
                {
                        if(cell_comp->GetPointId(k) == cell->GetPointId(1))
                        {
                                status1 = true;
                                break;
                        }
                }
                pt1 = k;

                for(k=0; k<8; k++)
                {
                        if(cell_comp->GetPointId(k) == cell->GetPointId(2))
                        {
                                status2 = true;
                                break;
                        }
                }
                pt2 = k;

                if(status1 && status2)
                {

                        this->MeshSeedLinks(1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                        cell->Delete();
                        cell_comp->Delete();
                        return 1;
                }
                else
                {
                        // 4 and 7
                        status1 = false; status2 = false;
                        for(k=0; k<8; k++)
                        {
                                if(cell_comp->GetPointId(k) == cell->GetPointId(4))
                                {
                                        status1 = true;
                                        break;
                                }
                        }
                        pt1 = k;

                        for(k=0; k<8; k++)
                        {
                                if(cell_comp->GetPointId(k) == cell->GetPointId(7))
                                {
                                        status2 = true;
                                        break;
                                }
                        }
                        pt2 = k;

                        if(status1 && status2)
                        {

                                this->MeshSeedLinks(1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                                cell->Delete();
                                cell_comp->Delete();
                                return 1;
                        }
                        else
                        {
                                // 5 and 6
                                status1 = false; status2 = false;
                                for(k=0; k<8; k++)
                                {
                                        if(cell_comp->GetPointId(k) == cell->GetPointId(6))
                                        {
                                                status1 = true;
                                                break;
                                        }
                                }
                                pt1 = k;

                                for(k=0; k<8; k++)
                                {
                                        if(cell_comp->GetPointId(k) == cell->GetPointId(5))
                                        {
                                                status2 = true;
                                                break;
                                        }
                                }
                                pt2 = k;

                                if(status1 && status2)
                                {

                                        this->MeshSeedLinks(1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                                        cell->Delete();
                                        cell_comp->Delete();
                                        return 1;
                                }
                        }
                }
        }
        cell->Delete();
        cell_comp->Delete();
        return 0;
}

int vtkMimxSplitUnstructuredHexahedronGridCell::CheckIfCellsShareEdgeY(int cellnum1, int cellnum2, int parcell)
{
        bool status1 = false;
        bool status2 = false;
        vtkGenericCell *cell = vtkGenericCell::New();
        vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(this->GetInput());
        input->GetCell(cellnum1, cell);
        vtkGenericCell *cell_comp = vtkGenericCell::New();
        input->GetCell(cellnum2, cell_comp);
        vtkIdType pt1, pt2;
        // start with x axis for 0 and 4
        int k;
        for(k=0; k<8; k++)
        {
                if(cell_comp->GetPointId(k) == cell->GetPointId(0))
                {
                        status1 = true;
                        break;
                }
        }
        pt1 = k;

        for(k=0; k<8; k++)
        {
                if(cell_comp->GetPointId(k) == cell->GetPointId(4))
                {
                        status2 = true;
                        break;
                }
        }        
        pt2 = k;

        if(status1 && status2)
        {                

                this->MeshSeedLinks(1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                cell->Delete();
                cell_comp->Delete();
                return 1;
        }
        else
        {
                // 1 and 5
                status1 = false; status2 = false;
                for(k=0; k<8; k++)
                {
                        if(cell_comp->GetPointId(k) == cell->GetPointId(1))
                        {
                                status1 = true;
                                break;
                        }
                }
                pt1 = k;

                for(k=0; k<8; k++)
                {
                        if(cell_comp->GetPointId(k) == cell->GetPointId(5))
                        {
                                status2 = true;
                                break;
                        }
                }
                pt2 = k;

                if(status1 && status2)
                {

                        this->MeshSeedLinks(1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                        cell->Delete();
                        cell_comp->Delete();
                        return 1;
                }
                else
                {
                        // 2 and 6
                        status1 = false; status2 = false;
                        for(k=0; k<8; k++)
                        {
                                if(cell_comp->GetPointId(k) == cell->GetPointId(2))
                                {
                                        status1 = true;
                                        break;
                                }
                        }
                        pt1 = k;

                        for(k=0; k<8; k++)
                        {
                                if(cell_comp->GetPointId(k) == cell->GetPointId(6))
                                {
                                        status2 = true;
                                        break;
                                }
                        }
                        pt2 = k;

                        if(status1 && status2)
                        {

                                this->MeshSeedLinks(1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                                cell->Delete();
                                cell_comp->Delete();
                                return 1;
                        }
                        else
                        {
                                // 3 and 7
                                status1 = false; status2 = false;
                                for(k=0; k<8; k++)
                                {
                                        if(cell_comp->GetPointId(k) == cell->GetPointId(3))
                                        {
                                                status1 = true;
                                                break;
                                        }
                                }
                                pt1 = k;

                                for(k=0; k<8; k++)
                                {
                                        if(cell_comp->GetPointId(k) == cell->GetPointId(7))
                                        {
                                                status2 = true;
                                                break;
                                        }
                                }
                                pt2 = k;

                                if(status1 && status2)
                                {

                                        this->MeshSeedLinks(1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                                        cell->Delete();
                                        cell_comp->Delete();
                                        return 1;
                                }
                        }
                }
        }
        cell->Delete();
        cell_comp->Delete();

        return 0;
}

int vtkMimxSplitUnstructuredHexahedronGridCell::CheckIfCellsShareEdgeZ(int cellnum1, int cellnum2, int parcell)
{
        bool status1 = false;
        bool status2 = false;
        vtkGenericCell *cell = vtkGenericCell::New();
        vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(this->GetInput());
        input->GetCell(cellnum1, cell);
        vtkGenericCell *cell_comp = vtkGenericCell::New();
        input->GetCell(cellnum2, cell_comp);
        vtkIdType pt1, pt2;
        // start with z axis for 0 and 1
        int k;
        for(k=0; k<8; k++)
        {
                if(cell_comp->GetPointId(k) == cell->GetPointId(0))
                {
                        status1 = true;
                        break;
                }
        }
        pt1 = k;

        for(k=0; k<8; k++)
        {
                if(cell_comp->GetPointId(k) == cell->GetPointId(1))
                {
                        status2 = true;
                        break;
                }
        }        
        pt2 = k;

        if(status1 && status2)
        {                

                this->MeshSeedLinks(1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                cell->Delete();
                cell_comp->Delete();
                return 1;
        }
        else
        {
                // 2 and 3
                status1 = false; status2 = false;
                for(k=0; k<8; k++)
                {
                        if(cell_comp->GetPointId(k) == cell->GetPointId(3))
                        {
                                status1 = true;
                                break;
                        }
                }
                pt1 = k;

                for(k=0; k<8; k++)
                {
                        if(cell_comp->GetPointId(k) == cell->GetPointId(2))
                        {
                                status2 = true;
                                break;
                        }
                }
                pt2 = k;

                if(status1 && status2)
                {

                        this->MeshSeedLinks(1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                        cell->Delete();
                        cell_comp->Delete();
                        return 1;
                }
                else
                {
                        // 4 and 5
                        status1 = false; status2 = false;
                        for(k=0; k<8; k++)
                        {
                                if(cell_comp->GetPointId(k) == cell->GetPointId(4))
                                {
                                        status1 = true;
                                        break;
                                }
                        }
                        pt1 = k;

                        for(k=0; k<8; k++)
                        {
                                if(cell_comp->GetPointId(k) == cell->GetPointId(5))
                                {
                                        status2 = true;
                                        break;
                                }
                        }
                        pt2 = k;

                        if(status1 && status2)
                        {

                                this->MeshSeedLinks(1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                                cell->Delete();
                                cell_comp->Delete();
                                return 1;
                        }
                        else
                        {
                                // 6 and 7
                                status1 = false; status2 = false;
                                for(k=0; k<8; k++)
                                {
                                        if(cell_comp->GetPointId(k) == cell->GetPointId(6))
                                        {
                                                status1 = true;
                                                break;
                                        }
                                }
                                pt1 = k;

                                for(k=0; k<8; k++)
                                {
                                        if(cell_comp->GetPointId(k) == cell->GetPointId(7))
                                        {
                                                status2 = true;
                                                break;
                                        }
                                }
                                pt2 = k;

                                if(status1 && status2)
                                {

                                        this->MeshSeedLinks(1, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
                                        cell->Delete();
                                        cell_comp->Delete();
                                        return 1;
                                }
                        }
                }
        }
        cell->Delete();
        cell_comp->Delete();
        return 0;
}

int vtkMimxSplitUnstructuredHexahedronGridCell::WhichEdgeOfCellBeingCompared(vtkIdType vertex1, vtkIdType vertex2)
{
        // check if edge is X (0-3)
        if(vertex1 == 0 || vertex1 == 3)
        {
                if(vertex2 == 0 || vertex2 == 3)        return 1;
        }
        // check if edge is X (1-2)
        if(vertex1 == 1 || vertex1 == 2)
        {
                if(vertex2 == 1 || vertex2 == 2)        return 1;
        }
        // check if edge is X (4-7)
        if(vertex1 == 4 || vertex1 == 7)
        {
                if(vertex2 == 4 || vertex2 == 7)        return 1;
        }
        // check if edge is X (5-6)
        if(vertex1 == 5 || vertex1 == 6)
        {
                if(vertex2 == 5 || vertex2 == 6)        return 1;
        }
        // check if edge is Z (0-1)
        if(vertex1 == 0 || vertex1 == 1)
        {
                if(vertex2 == 0 || vertex2 == 1)        return 3;
        }
        // check if edge is Z (2-3)
        if(vertex1 == 2 || vertex1 == 3)
        {
                if(vertex2 == 2 || vertex2 == 3)        return 3;
        }
        // check if edge is Z (4-5)
        if(vertex1 == 4 || vertex1 == 5)
        {
                if(vertex2 == 4 || vertex2 == 5)        return 3;
        }
        // check if edge is Z (6-7)
        if(vertex1 == 6 || vertex1 == 7)
        {
                if(vertex2 == 6 || vertex2 == 7)        return 3;
        }
        // check if edge is Y (0-4)
        if(vertex1 == 0 || vertex1 == 4)
        {
                if(vertex2 == 0 || vertex2 == 4)        return 2;
        }
        // check if edge is Y (1-5)
        if(vertex1 == 1 || vertex1 == 5)
        {
                if(vertex2 == 1 || vertex2 == 5)        return 2;
        }
        // check if edge is Y (2-6)
        if(vertex1 == 2 || vertex1 == 6)
        {
                if(vertex2 == 2 || vertex2 == 6)        return 2;
        }
        // check if edge is Y (3-7)
        if(vertex1 == 3 || vertex1 == 7)
        {
                if(vertex2 == 3 || vertex2 == 7)        return 2;
        }
        return 0;
}


// end mesh seed
void vtkMimxSplitUnstructuredHexahedronGridCell::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
