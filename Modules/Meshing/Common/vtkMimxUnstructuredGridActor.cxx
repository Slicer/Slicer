/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridActor.cxx,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
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

#include "vtkMimxUnstructuredGridActor.h"

#include "mimxLinkedList.h"
#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkCellTypes.h"
#include "vtkDataSetMapper.h"
#include "vtkGenericCell.h"
#include "vtkHexahedron.h"
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkTransform.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxUnstructuredGridActor, "$Revision: 1.6 $");

vtkStandardNewMacro(vtkMimxUnstructuredGridActor);

// node which store the information during grid traversal
struct Node
{
  int CellNum;  
  int EdgeNum;
  Node* Parent;
  Node* PeerPrev;
  Node* PeerNext;
  Node* Child;
};

vtkMimxUnstructuredGridActor::vtkMimxUnstructuredGridActor()
{
  vtkPoints *points = vtkPoints::New();
  this->UnstructuredGrid = vtkUnstructuredGrid::New();
  this->UnstructuredGrid->SetPoints(points);
  points->Delete();
  this->UnstructuredGridMapper = vtkDataSetMapper::New();
  this->Actor = vtkActor::New();
  // set up the pipe line
  this->UnstructuredGridMapper->SetInput(this->UnstructuredGrid);
  this->Actor->SetMapper(this->UnstructuredGridMapper);
  this->DataType = ACTOR_NONE;
  this->Links = 0;
  this->ElementSetName = NULL;
}

vtkMimxUnstructuredGridActor::~vtkMimxUnstructuredGridActor()
{
  this->UnstructuredGrid->Delete();
  this->UnstructuredGridMapper->Delete();
  this->Actor->Delete();
}

//vtkDataSet* vtkMimxUnstructuredGridActor::GetDataSet()
//{
//  return vtkDataSet::SafeDownCast(this->UnstructuredGrid);
//}
vtkUnstructuredGrid* vtkMimxUnstructuredGridActor::GetDataSet()
{
  return this->UnstructuredGrid;
}

int vtkMimxUnstructuredGridActor::MeshSeedFromAverageElementLength(double ElLength)
{
  if (this->UnstructuredGrid->GetNumberOfCells() < 1 || this->UnstructuredGrid->GetNumberOfPoints() < 8) {
    //cout<<"Invalid input grid, check number of points and cells ";
    return 0;
  }
  // check input cell type
  vtkCellTypes *celltypes = vtkCellTypes::New();
  this->UnstructuredGrid->GetCellTypes(celltypes);
  int i;
  for(i =0; i < celltypes->GetNumberOfTypes(); i++)
  {
    if(celltypes->GetCellType(i) != 12)
    {
      //cout << "input polydata must contain only hexahedrons";
      celltypes->Delete();
      return 0;
    }
  }
  celltypes->Delete();
  // initialize all the contents
  this->EstimateMeshSeedsBasedOnAverageElementLength(ElLength);
  return 1;
}

void vtkMimxUnstructuredGridActor::BuildMeshSeedLinks()
{
  this->Links = 1;
  this->MeshSeedLinks.SetSize(this->UnstructuredGrid->GetNumberOfCells()*3,
    this->UnstructuredGrid->GetNumberOfCells());
  this->MeshSeedLinks.Set(0);
  this->MeshSeedCheck.SetSize(this->UnstructuredGrid->GetNumberOfCells(), 1);
  vtkIdList *neigh_cell_list = vtkIdList::New();

  CLinkedList<Node*> *meshseedtree =  new CLinkedList<Node*>;

  // starting point for the tree to be constructed
  for (int i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++)
  {
    for (int j=1; j<=3; j++)
    {
      this->MeshSeedCheck.Set(0);
      // starting point for the tree to be constructed
      this->MeshSeedLinks(3*i+j,i+1) = j;
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
            {this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1) = this->CheckIfCellsShareEdgeX(currpar->CellNum,neigh_cell_list->GetId(k),i,j);}
            else
            {if(currpar->EdgeNum == 2){this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1) = this->CheckIfCellsShareEdgeY(currpar->CellNum,neigh_cell_list->GetId(k),i,j);}
            else{this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1) = this->CheckIfCellsShareEdgeZ(currpar->CellNum,neigh_cell_list->GetId(k),i,j);}}
            if (this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1))
            {
              if (!count)
              {
                currpar->Child = new Node;
                meshseedtree->Add(currpar->Child);
                currchild = currpar->Child;
                currchild->CellNum = neigh_cell_list->GetId(k);
                currchild->EdgeNum = this->MeshSeedLinks(3*i+j,neigh_cell_list->GetId(k)+1);
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
                currchild->EdgeNum = this->MeshSeedLinks(3*i+j,neigh_cell_list->GetId(k)+1);
                count++;
              }
            }
          } while(k < neigh_cell_list->GetNumberOfIds()-1);            

          //        currparpar = currchild->Parent;
          if (!count)
          {
            if(currpar->PeerPrev !=NULL)  currpar = currpar->PeerPrev;
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
      /*    for (int k=0; k < this->UnstructuredGrid->GetNumberOfCells(); k++)
      {
      //cout << this->MeshSeedLinks(3*i +j, k+1)<<"  ";
      }
      //cout<<endl;*/
    }
  }

  ofstream FileOutput;
  FileOutput.open("debug.txt",std::ios::out);
  for (int i=0; i < this->UnstructuredGrid->GetNumberOfCells()*3; i++)
  {
    for (int j=0; j<this->UnstructuredGrid->GetNumberOfCells(); j++) {
      FileOutput << this->MeshSeedLinks(i+1, j+1)<<"  ";
    }
    FileOutput<<endl;
  }

  neigh_cell_list->Delete();
  delete meshseedtree;
}

void vtkMimxUnstructuredGridActor::GetCellNeighbors(vtkIdType cellnum, vtkIdList *cellids)
{
  cellids->Initialize();
  vtkIdList *ptids = vtkIdList::New();
  vtkIdList *ptids_temp = vtkIdList::New();
  vtkIdList *cellids_temp = vtkIdList::New();
  this->UnstructuredGrid->GetCellPoints(cellnum,ptids);
  for(int i=0; i < ptids->GetNumberOfIds(); i++)
  {
    //    ptids_temp->Initialize();
    ptids_temp->SetNumberOfIds(1);
    ptids_temp->SetId(0,ptids->GetId(i));
    cellids_temp->Initialize();
    this->UnstructuredGrid->GetCellNeighbors(cellnum,ptids_temp, cellids_temp);
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

int vtkMimxUnstructuredGridActor::CheckIfCellsShareEdgeX(int cellnum1, int cellnum2, int parcell, int edgenum)
{
  bool status1 = false;
  bool status2 = false;
  vtkGenericCell *cell = vtkGenericCell::New();
  this->UnstructuredGrid->GetCell(cellnum1, cell);
  vtkGenericCell *cell_comp = vtkGenericCell::New();
  this->UnstructuredGrid->GetCell(cellnum2, cell_comp);
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
    this->MeshSeedLinks(3*parcell+edgenum, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
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

      this->MeshSeedLinks(3*parcell+edgenum, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
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

        this->MeshSeedLinks(3*parcell+edgenum, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
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

          this->MeshSeedLinks(3*parcell+edgenum, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
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

int vtkMimxUnstructuredGridActor::CheckIfCellsShareEdgeY(int cellnum1, int cellnum2, int parcell, int edgenum)
{
  bool status1 = false;
  bool status2 = false;
  vtkGenericCell *cell = vtkGenericCell::New();
  this->UnstructuredGrid->GetCell(cellnum1, cell);
  vtkGenericCell *cell_comp = vtkGenericCell::New();
  this->UnstructuredGrid->GetCell(cellnum2, cell_comp);
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

    this->MeshSeedLinks(3*parcell+edgenum, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
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

      this->MeshSeedLinks(3*parcell+edgenum, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
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

        this->MeshSeedLinks(3*parcell+edgenum, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
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

          this->MeshSeedLinks(3*parcell+edgenum, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
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

// lower numbered bounding box cells take precedence over higher numbered
// bounding boxes.
void vtkMimxUnstructuredGridActor::EstimateMeshSeedsBasedOnAverageElementLength(double ElLength)
{
  if(!this->Links)  
  {
    this->BuildMeshSeedLinks();
  }

  if(this->UnstructuredGrid)
  {
    int i;
    int dim[3];  
    if(this->UnstructuredGrid->GetCellData()->GetArray("Mesh_Seed"))
    {
      vtkIntArray *MeshSeedValues = vtkIntArray::SafeDownCast(this->UnstructuredGrid->
        GetCellData()->GetArray("Mesh_Seed"));
      dim[0] = 0; dim[1] = 0; dim[2] = 0;

      for (i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++) {
        MeshSeedValues->SetTupleValue(i, dim);
      }
    }
    else{
      vtkIntArray *meshseedarray = vtkIntArray::New();
      meshseedarray->SetNumberOfComponents(3);
      meshseedarray->SetNumberOfTuples(this->UnstructuredGrid->GetNumberOfCells());
      meshseedarray->SetName("Mesh_Seed");
      dim[0] = 0; dim[1] = 0; dim[2] = 0;
      for (i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++) {
        meshseedarray->SetTupleValue(i,dim);
      }
      this->UnstructuredGrid->GetCellData()->AddArray(meshseedarray);
      meshseedarray->Delete();

    }

    vtkIntArray *MeshSeedValues = vtkIntArray::SafeDownCast(this->UnstructuredGrid->GetCellData()->
      GetArray("Mesh_Seed"));
    vtkGenericCell *cell = vtkGenericCell::New();
    for(int i =0; i < this->UnstructuredGrid->GetNumberOfCells(); i++)
    {
      cell->Initialize();
      this->UnstructuredGrid->GetCell(i, cell);
      MeshSeedValues->GetTupleValue(i, dim);
      // calculate the average length of X edges,
      // 0-3, 1-2, 4-7, 5-6
      if(!dim[0])// if the X meshseed is not precalculated
      {
        double lengthX = 0.0;
        double a[3], b[3];
        this->UnstructuredGrid->GetPoint(cell->GetPointId(0),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(3),b);
        lengthX = lengthX + sqrt(vtkMath::Distance2BetweenPoints(a,b));
        this->UnstructuredGrid->GetPoint(cell->GetPointId(1),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(2),b);
        lengthX = lengthX + sqrt(vtkMath::Distance2BetweenPoints(a,b));
        this->UnstructuredGrid->GetPoint(cell->GetPointId(4),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(7),b);
        lengthX = lengthX + sqrt(vtkMath::Distance2BetweenPoints(a,b));
        this->UnstructuredGrid->GetPoint(cell->GetPointId(5),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(6),b);
        lengthX = lengthX + sqrt(vtkMath::Distance2BetweenPoints(a,b));
        lengthX = lengthX/4.0;
        dim[0] = int(lengthX/ElLength)+1;
        if(dim[0] == 1)  dim[0] = dim[0] +1;
        MeshSeedValues->SetTupleValue(i, dim);
        //        this->MeshSeedX->SetValue(i,int(lengthX/length)+1);
      }
      this->SetHigherNumberedBBoxMeshSeed(i,1);
      // calculate the average length of X edges,
      // 0-4, 1-5, 2-6, 3-7
      if(!dim[1])// if the X meshseed is not precalculated
      {
        double lengthY = 0.0;
        double a[3], b[3];
        this->UnstructuredGrid->GetPoint(cell->GetPointId(0),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(4),b);
        lengthY = lengthY + sqrt(vtkMath::Distance2BetweenPoints(a,b));
        this->UnstructuredGrid->GetPoint(cell->GetPointId(1),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(5),b);
        lengthY = lengthY + sqrt(vtkMath::Distance2BetweenPoints(a,b));
        this->UnstructuredGrid->GetPoint(cell->GetPointId(2),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(6),b);
        lengthY = lengthY + sqrt(vtkMath::Distance2BetweenPoints(a,b));
        this->UnstructuredGrid->GetPoint(cell->GetPointId(3),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(7),b);
        lengthY = lengthY + sqrt(vtkMath::Distance2BetweenPoints(a,b));
        lengthY = lengthY/4.0;
        dim[1] = int(lengthY/ElLength)+1;
        if(dim[1] == 1)  dim[1] = dim[1] +1;
        MeshSeedValues->SetTupleValue(i, dim);
        //        this->MeshSeedY->SetValue(i,int(lengthY/length)+1);
      }
      this->SetHigherNumberedBBoxMeshSeed(i,2);
      // calculate the average length of Z edges,
      // 0-1, 2-3, 4-5, 6-7
      if(!dim[2])// if the X meshseed is not precalculated
      {
        double lengthZ = 0.0;
        double a[3], b[3];
        this->UnstructuredGrid->GetPoint(cell->GetPointId(0),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(1),b);
        lengthZ = lengthZ + sqrt(vtkMath::Distance2BetweenPoints(a,b));
        this->UnstructuredGrid->GetPoint(cell->GetPointId(2),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(3),b);
        lengthZ = lengthZ + sqrt(vtkMath::Distance2BetweenPoints(a,b));
        this->UnstructuredGrid->GetPoint(cell->GetPointId(4),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(5),b);
        lengthZ = lengthZ + sqrt(vtkMath::Distance2BetweenPoints(a,b));
        this->UnstructuredGrid->GetPoint(cell->GetPointId(6),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(7),b);
        lengthZ = lengthZ + sqrt(vtkMath::Distance2BetweenPoints(a,b));
        lengthZ = lengthZ/4.0;
        dim[2] = int(lengthZ/ElLength)+1;
        if(dim[2] == 1)  dim[2] = dim[2] +1;
        MeshSeedValues->SetTupleValue(i, dim);
        //        this->MeshSeedZ->SetValue(i,int(lengthZ/length)+1);
      }
      this->SetHigherNumberedBBoxMeshSeed(i,3);
    }
    cell->Delete();
    //ofstream FileOutput;
    //FileOutput.open("meshseed.txt", std::ios::out);
    //cout <<"Mesh Seed"<<endl;
    for(int i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++)
    {
      MeshSeedValues->GetTupleValue(i, dim);
//      FileOutput <<dim[0]<<"  "<<dim[1]<<"  "<<dim[2]<<endl;
    }
  }
}


int vtkMimxUnstructuredGridActor::CheckIfCellsShareEdgeZ(int cellnum1, int cellnum2, int parcell, int edgenum)
{
  bool status1 = false;
  bool status2 = false;
  vtkGenericCell *cell = vtkGenericCell::New();
  this->UnstructuredGrid->GetCell(cellnum1, cell);
  vtkGenericCell *cell_comp = vtkGenericCell::New();
  this->UnstructuredGrid->GetCell(cellnum2, cell_comp);
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

    this->MeshSeedLinks(3*parcell+edgenum, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
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

      this->MeshSeedLinks(3*parcell+edgenum, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
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

        this->MeshSeedLinks(3*parcell+edgenum, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
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

          this->MeshSeedLinks(3*parcell+edgenum, cellnum2 + 1) = this->WhichEdgeOfCellBeingCompared(pt1,pt2);
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

int vtkMimxUnstructuredGridActor::WhichEdgeOfCellBeingCompared(vtkIdType vertex1, vtkIdType vertex2)
{
  // check if edge is X (0-3)
  if(vertex1 == 0 || vertex1 == 3)
  {
    if(vertex2 == 0 || vertex2 == 3)  return 1;
  }
  // check if edge is X (1-2)
  if(vertex1 == 1 || vertex1 == 2)
  {
    if(vertex2 == 1 || vertex2 == 2)  return 1;
  }
  // check if edge is X (4-7)
  if(vertex1 == 4 || vertex1 == 7)
  {
    if(vertex2 == 4 || vertex2 == 7)  return 1;
  }
  // check if edge is X (5-6)
  if(vertex1 == 5 || vertex1 == 6)
  {
    if(vertex2 == 5 || vertex2 == 6)  return 1;
  }
  // check if edge is Z (0-1)
  if(vertex1 == 0 || vertex1 == 1)
  {
    if(vertex2 == 0 || vertex2 == 1)  return 3;
  }
  // check if edge is Z (2-3)
  if(vertex1 == 2 || vertex1 == 3)
  {
    if(vertex2 == 2 || vertex2 == 3)  return 3;
  }
  // check if edge is Z (4-5)
  if(vertex1 == 4 || vertex1 == 5)
  {
    if(vertex2 == 4 || vertex2 == 5)  return 3;
  }
  // check if edge is Z (6-7)
  if(vertex1 == 6 || vertex1 == 7)
  {
    if(vertex2 == 6 || vertex2 == 7)  return 3;
  }
  // check if edge is Y (0-4)
  if(vertex1 == 0 || vertex1 == 4)
  {
    if(vertex2 == 0 || vertex2 == 4)  return 2;
  }
  // check if edge is Y (1-5)
  if(vertex1 == 1 || vertex1 == 5)
  {
    if(vertex2 == 1 || vertex2 == 5)  return 2;
  }
  // check if edge is Y (2-6)
  if(vertex1 == 2 || vertex1 == 6)
  {
    if(vertex2 == 2 || vertex2 == 6)  return 2;
  }
  // check if edge is Y (3-7)
  if(vertex1 == 3 || vertex1 == 7)
  {
    if(vertex2 == 3 || vertex2 == 7)  return 2;
  }
  cout <<"error"<<endl;
  return 0;
}

void vtkMimxUnstructuredGridActor::SetHigherNumberedBBoxMeshSeed(int boxnum, int axis)
{
  int dim[3], dimmod[3];
  vtkIntArray *MeshSeedValues = vtkIntArray::SafeDownCast(
    this->UnstructuredGrid->GetCellData()->GetArray("Mesh_Seed"));
  for(int i = boxnum+1; i < this->UnstructuredGrid->GetNumberOfCells(); i++)
  {
    int j = this->MeshSeedLinks(boxnum*3+axis,i+1);
    if(j)
    {
      MeshSeedValues->GetTupleValue(boxnum, dim);
      MeshSeedValues->GetTupleValue(i, dimmod);

      dimmod[j-1] = dim[axis-1];
      MeshSeedValues->SetTupleValue(i, dimmod);
    }
  }
}

void vtkMimxUnstructuredGridActor::ChangeMeshSeed(int BoxNum, int Axis, int SeedNum)
{
  if(!this->Links)  
  {
    this->BuildMeshSeedLinks();
  }

  // change the mesh seed of the BoxNum first
  if(this->UnstructuredGrid->GetCellData()->GetArray("Mesh_Seed"))
  {
    vtkIntArray *MeshSeedValues = vtkIntArray::SafeDownCast(
      this->UnstructuredGrid->GetCellData()->GetArray("Mesh_Seed"));
    int dim[3], dimmod[3];
    MeshSeedValues->GetTupleValue(BoxNum, dim);
    dim[Axis] = SeedNum;
    MeshSeedValues->SetTupleValue(BoxNum, dim);

    // change all the other relevant boundingbox mesh seeds
    for(int i = 0; i < this->UnstructuredGrid->GetNumberOfCells(); i++)
    {
      int j = this->MeshSeedLinks(BoxNum*3+Axis+1,i+1);
      if(j)
      {
        MeshSeedValues->GetTupleValue(BoxNum, dim);
        MeshSeedValues->GetTupleValue(i, dimmod);

        dimmod[j-1] = dim[Axis];
        MeshSeedValues->SetTupleValue(i, dimmod);
      }
    }
  }
  //else{
  //  cout <<" No mesh seed values to change "<<endl;
  //}
  //cout <<"Mesh Seed"<<endl;
  //for(int i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++)
  //{
  //  MeshSeedValues->GetTupleValue(i, dim);
  //  cout <<dim[0]<<"  "<<dim[1]<<"  "<<dim[2]<<endl;
  //}
}

void vtkMimxUnstructuredGridActor::SetDataType(int UserTypeData)
{
  this->DataType = UserTypeData;
  if(this->DataType == ACTOR_FE_MESH)
  {
    if (!this->UnstructuredGrid->GetCellData()->GetArray("Fe_Mesh")) {
      vtkIntArray *intarray = vtkIntArray::New();
      intarray->SetNumberOfValues(this->UnstructuredGrid->GetNumberOfCells());
      for (int i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++) {
        intarray->SetValue(i,1);
      }
      intarray->SetName("Fe_Mesh");
      this->UnstructuredGrid->GetCellData()->AddArray(intarray);
      intarray->Delete();
      return;
    }
  }
  else{
    if(!this->UnstructuredGrid->GetCellData()->GetArray("Bounding_Box"))
    {
      if (!this->UnstructuredGrid->GetCellData()->GetArray("Bounding_Box")) {
        vtkIntArray *intarray = vtkIntArray::New();
        intarray->SetNumberOfValues(this->UnstructuredGrid->GetNumberOfCells());
        for (int i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++) {
          intarray->SetValue(i,1);
        }
        intarray->SetName("Bounding_Box");
        this->UnstructuredGrid->GetCellData()->AddArray(intarray);
        intarray->Delete();
        return;
      }
    }
  }
}
void vtkMimxUnstructuredGridActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

