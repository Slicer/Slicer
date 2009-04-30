/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridActor.cxx,v $
Language:  C++
Date:      $Date: 2008/08/10 00:48:34 $
Version:   $Revision: 1.19 $

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
#include "vtkStringArray.h"
#include "vtkDataArray.h"

vtkCxxRevisionMacro(vtkMimxUnstructuredGridActor, "$Revision: 1.19 $");

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
  this->Actor->GetProperty()->SetRepresentationToWireframe();
  this->DataType = ACTOR_NONE;
  this->Links = 0;
  this->ElementSetName = NULL;
  this->FillColor[0] = this->FillColor[1] = this->FillColor[2] = 1.0;
  this->OutlineColor[0] = this->OutlineColor[1] = this->OutlineColor[2] = 1.0;
  this->SetDisplayType( DisplayOutline );
}

vtkMimxUnstructuredGridActor::~vtkMimxUnstructuredGridActor()
{
  this->UnstructuredGrid->Delete();
  this->UnstructuredGridMapper->Delete();
  this->Actor->Delete();  //**valgrinderror
                              }

//vtkDataSet* vtkMimxUnstructuredGridActor::GetDataSet()
//{
//      return vtkDataSet::SafeDownCast(this->UnstructuredGrid);
//}
vtkUnstructuredGrid* vtkMimxUnstructuredGridActor::GetDataSet()
{
  return this->UnstructuredGrid;
}

int vtkMimxUnstructuredGridActor::MeshSeedFromAverageElementLength(double ElLengthX, double ElLengthY, double ElLengthZ)
{
  if (this->UnstructuredGrid->GetNumberOfCells() < 1 || this->UnstructuredGrid->GetNumberOfPoints() < 8)
    {
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
  this->EstimateMeshSeedsBasedOnAverageElementLength(ElLengthX, ElLengthY, ElLengthZ);
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
      Node *currchild = NULL;
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
              {
              this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1) = this->CheckIfCellsShareEdgeX(currpar->CellNum,neigh_cell_list->GetId(k),i,j);}
            else
              {
              if(currpar->EdgeNum == 2)
                {
                this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1) = this->CheckIfCellsShareEdgeY(currpar->CellNum,neigh_cell_list->GetId(k),i,j);}
              else
                {
                this->MeshSeedCheck(neigh_cell_list->GetId(k)+1,1) = this->CheckIfCellsShareEdgeZ(currpar->CellNum,neigh_cell_list->GetId(k),i,j);}}
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
              else
                {
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

          //                              currparpar = currchild->Parent;
          if (!count)
            {
            if(currpar->PeerPrev !=NULL)    currpar = currpar->PeerPrev;
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
      /*              for (int k=0; k < this->UnstructuredGrid->GetNumberOfCells(); k++)
                        {
                        //cout << this->MeshSeedLinks(3*i +j, k+1)<<"  ";
                        }
                        //cout<<endl;*/
      }
    }

  //ofstream FileOutput;
  //FileOutput.open("debug.txt",std::ios::out);
  //for (int i=0; i < this->UnstructuredGrid->GetNumberOfCells()*3; i++)
  //{
  //      for (int j=0; j<this->UnstructuredGrid->GetNumberOfCells(); j++) {
  //              FileOutput << this->MeshSeedLinks(i+1, j+1)<<"  ";
  //      }
  //      FileOutput<<endl;
  //}

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
    //              ptids_temp->Initialize();
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
void vtkMimxUnstructuredGridActor::EstimateMeshSeedsBasedOnAverageElementLength(
  double ElLengthX, double ElLengthY, double ElLengthZ)
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

      for (i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++) 
        {
        MeshSeedValues->SetTupleValue(i, dim);
        }
      }
    else
      {
      vtkIntArray *meshseedarray = vtkIntArray::New();
      meshseedarray->SetNumberOfComponents(3);
      meshseedarray->SetNumberOfTuples(this->UnstructuredGrid->GetNumberOfCells());
      meshseedarray->SetName("Mesh_Seed");
      dim[0] = 0; dim[1] = 0; dim[2] = 0;
      for (i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++)
        {
        meshseedarray->SetTupleValue(i,dim);
        }
      this->UnstructuredGrid->GetCellData()->AddArray(meshseedarray);
      meshseedarray->Delete();

      }

    vtkIntArray *MeshSeedValues = vtkIntArray::SafeDownCast(this->UnstructuredGrid->GetCellData()->
                                                            GetArray("Mesh_Seed"));
    vtkGenericCell *cell = vtkGenericCell::New();
    for(i =0; i < this->UnstructuredGrid->GetNumberOfCells(); i++)
      {
      cell->Initialize();
      this->UnstructuredGrid->GetCell(i, cell);
      MeshSeedValues->GetTupleValue(i, dim);
      // calculate the average length of X edges,
      // 0-3, 1-2, 4-7, 5-6
      double tempLength = 0.0;
      if(!dim[0])// if the X meshseed is not precalculated
        {
        double lengthX = 0.0;
        double a[3], b[3];
        this->UnstructuredGrid->GetPoint(cell->GetPointId(0),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(3),b);
        tempLength = sqrt(vtkMath::Distance2BetweenPoints(a,b));
        if(tempLength > lengthX) lengthX = tempLength;
        this->UnstructuredGrid->GetPoint(cell->GetPointId(1),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(2),b);
        tempLength = sqrt(vtkMath::Distance2BetweenPoints(a,b));
        if(tempLength > lengthX) lengthX = tempLength;
        this->UnstructuredGrid->GetPoint(cell->GetPointId(4),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(7),b);
        tempLength = sqrt(vtkMath::Distance2BetweenPoints(a,b));
        if(tempLength > lengthX) lengthX = tempLength;
        this->UnstructuredGrid->GetPoint(cell->GetPointId(5),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(6),b);
        tempLength = sqrt(vtkMath::Distance2BetweenPoints(a,b));
        if(tempLength > lengthX) lengthX = tempLength;
        dim[0] = int(lengthX/ElLengthX)+1;
        if(dim[0] == 1) dim[0] = dim[0] +1;
        MeshSeedValues->SetTupleValue(i, dim);
        //                              this->MeshSeedX->SetValue(i,int(lengthX/length)+1);
        }
      this->SetHigherNumberedBBoxMeshSeed(i,1);
      // calculate the average length of X edges,
      // 0-4, 1-5, 2-6, 3-7
      tempLength = 0.0;
      if(!dim[1])// if the X meshseed is not precalculated
        {
        double lengthY = 0.0;
        double a[3], b[3];
        this->UnstructuredGrid->GetPoint(cell->GetPointId(0),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(4),b);
        tempLength = sqrt(vtkMath::Distance2BetweenPoints(a,b));
        if(tempLength > lengthY) lengthY = tempLength;
        this->UnstructuredGrid->GetPoint(cell->GetPointId(1),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(5),b);
        tempLength = sqrt(vtkMath::Distance2BetweenPoints(a,b));
        if(tempLength > lengthY) lengthY = tempLength;
        this->UnstructuredGrid->GetPoint(cell->GetPointId(2),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(6),b);
        tempLength = sqrt(vtkMath::Distance2BetweenPoints(a,b));
        if(tempLength > lengthY) lengthY = tempLength;
        this->UnstructuredGrid->GetPoint(cell->GetPointId(3),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(7),b);
        tempLength = sqrt(vtkMath::Distance2BetweenPoints(a,b));
        if(tempLength > lengthY) lengthY = tempLength;
        dim[1] = int(lengthY/ElLengthY)+1;
        if(dim[1] == 1) dim[1] = dim[1] +1;
        MeshSeedValues->SetTupleValue(i, dim);
        //                              this->MeshSeedY->SetValue(i,int(lengthY/length)+1);
        }
      this->SetHigherNumberedBBoxMeshSeed(i,2);
      // calculate the average length of Z edges,
      // 0-1, 2-3, 4-5, 6-7
      tempLength = 0.0;
      if(!dim[2])// if the X meshseed is not precalculated
        {
        double lengthZ = 0.0;
        double a[3], b[3];
        this->UnstructuredGrid->GetPoint(cell->GetPointId(0),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(1),b);
        tempLength = sqrt(vtkMath::Distance2BetweenPoints(a,b));
        if(tempLength > lengthZ) lengthZ = tempLength;
        this->UnstructuredGrid->GetPoint(cell->GetPointId(2),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(3),b);
        tempLength = sqrt(vtkMath::Distance2BetweenPoints(a,b));
        if(tempLength > lengthZ) lengthZ = tempLength;
        this->UnstructuredGrid->GetPoint(cell->GetPointId(4),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(5),b);
        tempLength = sqrt(vtkMath::Distance2BetweenPoints(a,b));
        if(tempLength > lengthZ) lengthZ = tempLength;
        this->UnstructuredGrid->GetPoint(cell->GetPointId(6),a);
        this->UnstructuredGrid->GetPoint(cell->GetPointId(7),b);
        tempLength = sqrt(vtkMath::Distance2BetweenPoints(a,b));
        if(tempLength > lengthZ) lengthZ = tempLength;
        dim[2] = int(lengthZ/ElLengthZ)+1;
        if(dim[2] == 1) dim[2] = dim[2] +1;
        MeshSeedValues->SetTupleValue(i, dim);
        }
      this->SetHigherNumberedBBoxMeshSeed(i,3);
      }
    cell->Delete();
    //ofstream FileOutput;
    //FileOutput.open("meshseed.txt", std::ios::out);
    //cout <<"Mesh Seed"<<endl;
    for(i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++)
      {
      MeshSeedValues->GetTupleValue(i, dim);
//                      FileOutput <<dim[0]<<"  "<<dim[1]<<"  "<<dim[2]<<endl;
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

int vtkMimxUnstructuredGridActor::GetMeshSeed(int BoxNum, int Axis)
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
    int dim[3];
    MeshSeedValues->GetTupleValue(BoxNum, dim);
    return (dim[Axis]); 
    }
  else
    {
    return 0;
    }
}

void vtkMimxUnstructuredGridActor::ChangeMeshSeed(int BoxNum, int Axis, int SeedNum)
{
  if(!this->Links)        
    {
    this->BuildMeshSeedLinks();
    }

  // change the mesh seed of the BoxNum first
  if(!this->UnstructuredGrid->GetCellData()->GetArray("Mesh_Seed"))
    {
    this->EstimateMeshSeedsBasedOnAverageElementLength(1.0, 1.0, 1.0);
    }
  //
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
        
  //else{
  //      cout <<" No mesh seed values to change "<<endl;
  //}
  //cout <<"Mesh Seed"<<endl;
  //for(int i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++)
  //{
  //      MeshSeedValues->GetTupleValue(i, dim);
  //      cout <<dim[0]<<"  "<<dim[1]<<"  "<<dim[2]<<endl;
  //}
}

//void vtkMimxUnstructuredGridActor::SetDataType(int UserTypeData)
//{
//      this->DataType = UserTypeData;
//      if(this->DataType == ACTOR_FE_MESH)
//      {
//              if (!this->UnstructuredGrid->GetCellData()->GetArray("Fe_Mesh")) {
//                      vtkIntArray *intarray = vtkIntArray::New();
//                      intarray->SetNumberOfValues(this->UnstructuredGrid->GetNumberOfCells());
//                      for (int i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++) {
//                              intarray->SetValue(i,1);
//                      }
//                      intarray->SetName("Fe_Mesh");
//                      this->UnstructuredGrid->GetCellData()->AddArray(intarray);
//                      intarray->Delete();
//                      return;
//              }
//      }
//      else{
//              if(!this->UnstructuredGrid->GetCellData()->GetArray("Bounding_Box"))
//              {
//                      if (!this->UnstructuredGrid->GetCellData()->GetArray("Bounding_Box")) {
//                              vtkIntArray *intarray = vtkIntArray::New();
//                              intarray->SetNumberOfValues(this->UnstructuredGrid->GetNumberOfCells());
//                              for (int i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++) {
//                                      intarray->SetValue(i,1);
//                              }
//                              intarray->SetName("Bounding_Box");
//                              this->UnstructuredGrid->GetCellData()->AddArray(intarray);
//                              intarray->Delete();
//                              return;
//                      }
//              }
//      }
//}
//---------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::ChangeMeshSeedAvElLength(
  int BoxNum, int Axis, double ElLength)
{
  //if(!this->Links)      
  //{
  //      this->BuildMeshSeedLinks();
  //}
  if(ElLength <=0.0)
    {
    return;
    }
  vtkGenericCell *cell = vtkGenericCell::New();
  this->UnstructuredGrid->GetCell(BoxNum, cell);

  vtkIntArray *MeshSeedValues = vtkIntArray::SafeDownCast(
    this->UnstructuredGrid->GetCellData()->GetArray("Mesh_Seed"));
  if(!MeshSeedValues)
    {
    this->EstimateMeshSeedsBasedOnAverageElementLength(1.0, 1.0, 1.0);
    MeshSeedValues = vtkIntArray::SafeDownCast(
      this->UnstructuredGrid->GetCellData()->GetArray("Mesh_Seed"));
    }

  int dim[3];
  //if (MeshSeedValues == NULL)
  //{
  //  vtkIntArray *meshseedarray = vtkIntArray::New();
  //      meshseedarray->SetNumberOfComponents(3);
  //      meshseedarray->SetNumberOfTuples(this->UnstructuredGrid->GetNumberOfCells());
  //      meshseedarray->SetName("Mesh_Seed");
  //      dim[0] = 0; dim[1] = 0; dim[2] = 0;
  //      for (int i=0; i<this->UnstructuredGrid->GetNumberOfCells(); i++) {
  //              meshseedarray->SetTupleValue(i,dim);
  //      }
  //      this->UnstructuredGrid->GetCellData()->AddArray(meshseedarray);
  //      meshseedarray->Delete();
  //      MeshSeedValues = vtkIntArray::SafeDownCast(
  //        this->UnstructuredGrid->GetCellData()->GetArray("Mesh_Seed"));
  //}
        
  MeshSeedValues->GetTupleValue(BoxNum, dim);

  // calculate the average length of X edges,
  // 0-3, 1-2, 4-7, 5-6
  if(Axis == 0)// if the X meshseed is not precalculated
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
    //std::cout << "Old X Length " << dim[0] << std::endl;
    dim[0] = int(lengthX/ElLength)+1;
    if(dim[0] == 1) dim[0] = dim[0] +1;
    //std::cout << "New X Length " << dim[0] << std::endl;
    MeshSeedValues->SetTupleValue(BoxNum, dim);
    this->ChangeMeshSeed(BoxNum, Axis, dim[0]);
//              this->SetHigherNumberedBBoxMeshSeed(BoxNum,1);
    return;
    }
  // calculate the average length of X edges,
  // 0-4, 1-5, 2-6, 3-7
  if(Axis == 1)// if the X meshseed is not precalculated
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
    //std::cout << "Old Y Length " << dim[1] << std::endl;
    dim[1] = int(lengthY/ElLength)+1;
    //std::cout << "New Y Length " << dim[1] << std::endl;
    if(dim[1] == 1) dim[1] = dim[1] +1;
    MeshSeedValues->SetTupleValue(BoxNum, dim);
    this->ChangeMeshSeed(BoxNum, Axis, dim[1]);
//              this->SetHigherNumberedBBoxMeshSeed(BoxNum,2);
    return;
    }
  // calculate the average length of Z edges,
  // 0-1, 2-3, 4-5, 6-7
  if(Axis == 2)// if the X meshseed is not precalculated
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
    //std::cout << "Old Z Length " << dim[2] << std::endl;
    dim[2] = int(lengthZ/ElLength)+1;
    if(dim[2] == 1) dim[2] = dim[2] +1;
    //std::cout << "New Z Length " << dim[2] << std::endl;
    MeshSeedValues->SetTupleValue(BoxNum, dim);
    this->ChangeMeshSeed(BoxNum, Axis, dim[2]);
//              this->SetHigherNumberedBBoxMeshSeed(BoxNum,3);
    return;
    }

}
//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::DeleteNodeSet(const char *Name)
{

}
//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::DeleteElementSet(const char *Name)
{
  vtkFieldData *fielddata = this->UnstructuredGrid->GetFieldData();
  vtkCellData *celldata = this->UnstructuredGrid->GetCellData();

  vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
    fielddata->GetAbstractArray("Element_Set_Names"));

  if(!stringarray)        return;

  vtkDataArray *datasetarray;
        
  datasetarray = celldata->GetArray(Name);
  if(datasetarray)        celldata->RemoveArray(Name);

  char Young[256];
  strcpy(Young, Name);
  strcat(Young, "_Constant_Youngs_Modulus");
  datasetarray = fielddata->GetArray(Young);
  if(datasetarray)        fielddata->RemoveArray(Young);

  char Poisson[256];
  strcpy(Poisson, Name);
  strcat(Poisson, "_Constant_Poissons_Ratio");
  datasetarray = fielddata->GetArray(Poisson);
  if(datasetarray)        fielddata->RemoveArray(Poisson);
        
  char ImageBased[256];
  strcpy(ImageBased, Name);
  strcat(ImageBased, "_Image_Based_Material_Property");
  datasetarray = celldata->GetArray(ImageBased);
  if(datasetarray)        celldata->RemoveArray(ImageBased);
        
  strcat(ImageBased, "_ReBin");
  datasetarray = celldata->GetArray(ImageBased);
  if(datasetarray)        celldata->RemoveArray(ImageBased);

  vtkStringArray *temparray = vtkStringArray::New();
  temparray->DeepCopy(stringarray);
  stringarray->Initialize();
  int i;

  for (i=0; i<temparray->GetNumberOfTuples(); i++)
    {
    if(strcmp(temparray->GetValue(i), Name))
      {
      stringarray->InsertNextValue(temparray->GetValue(i));
      }
    }
  temparray->Delete();
}
//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::GetOutlineColor(double &red, double &green, double &blue)
{
  red = this->OutlineColor[0];
  green = this->OutlineColor[1];
  blue = this->OutlineColor[2];
}

//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::GetOutlineColor(double rgb[3])
{
  rgb[0] = this->OutlineColor[0];
  rgb[1] = this->OutlineColor[1];
  rgb[2] = this->OutlineColor[2];
}

//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::SetOutlineColor(double red, double green, double blue)
{
  this->OutlineColor[0] = red;
  this->OutlineColor[1] = green;
  this->OutlineColor[2] = blue;
  if (this->DisplayType == DisplayOutline)
    {
    this->Actor->GetProperty()->SetColor(red, green, blue);
    this->Actor->Modified();
    }
}

//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::SetOutlineColor(double rgb[3])
{
  this->SetOutlineColor(rgb[0], rgb[1], rgb[2]);
}
//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::SetFillColor(double red, double green, double blue)
{
  this->FillColor[0] = red;
  this->FillColor[1] = green;
  this->FillColor[2] = blue;
  if (this->DisplayType == DisplaySurface)
    {
    this->Actor->GetProperty()->SetColor(red, green, blue);
    this->Actor->Modified();
    }
}

//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::SetFillColor(double rgb[3])
{
  this->SetFillColor(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::GetFillColor(double &red, double &green, double &blue)
{
  red = this->FillColor[0];
  green = this->FillColor[1];
  blue = this->FillColor[2];
}

//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::GetFillColor(double rgb[3])
{
  rgb[0] = this->FillColor[0];
  rgb[1] = this->FillColor[1];
  rgb[2] = this->FillColor[2];
}

//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::SetDisplayType(int mode)
{
  switch ( mode )
    {
    case DisplaySurface:
      this->Actor->GetProperty()->SetRepresentationToSurface();
      this->Actor->GetProperty()->SetColor( this->FillColor );
      break;
    case DisplayOutline:
      this->Actor->GetProperty()->SetRepresentationToWireframe();
      this->Actor->GetProperty()->SetColor( this->OutlineColor );
      break;
    }
  this->DisplayType = mode;
}

//----------------------------------------------------------------------------------
int vtkMimxUnstructuredGridActor::GetDisplayType( )
{
  return this->DisplayType;
}

//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------------------


// added to support slicer integration
void vtkMimxUnstructuredGridActor::SaveVisibility(void) {this->SavedVisibility = (this->Actor->GetVisibility())?true:false;}
void vtkMimxUnstructuredGridActor::RestoreVisibility(void) {this->Actor->SetVisibility(this->SavedVisibility);}
void vtkMimxUnstructuredGridActor::Hide() {this->Actor->SetVisibility(0);}
void vtkMimxUnstructuredGridActor::Show() {this->Actor->SetVisibility(1);}

