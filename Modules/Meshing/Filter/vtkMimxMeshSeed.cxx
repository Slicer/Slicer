/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxMeshSeed.cxx,v $
Language:  C++
Date:      $Date: 2007/10/18 21:38:00 $
Version:   $Revision: 1.2 $

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

#include "vtkMimxMeshSeed.h"

#include "mimxLinkedList.h"

#include "vtkCellData.h"
#include "vtkCellTypes.h"
#include "vtkGenericCell.h"
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkUnstructuredGrid.h"

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

vtkMimxMeshSeed::vtkMimxMeshSeed()
{
        this->UGrid = NULL;
        this->MeshSeedValues = NULL;
        this->Links = 0;
}

vtkMimxMeshSeed::~vtkMimxMeshSeed()
{

}

int vtkMimxMeshSeed::MeshSeedFromAverageElementLength(double ElLength)
{
        if(!UGrid || ! MeshSeedValues)
        {
                //cout<<"User should allocate memory to the UGrid and the Array ";
                return 0;
        }
        if (UGrid->GetNumberOfCells() < 1 || UGrid->GetNumberOfPoints() < 8) {
                //cout<<"Invalid input grid, check number of points and cells ";
                return 0;
        }
        // check input cell type
        vtkCellTypes *celltypes = vtkCellTypes::New();
        UGrid->GetCellTypes(celltypes);
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
        MeshSeedValues->Initialize();
        this->EstimateMeshSeedsBasedOnAverageElementLength(ElLength);
        return 1;
}

void vtkMimxMeshSeed::BuildMeshSeedLinks()
{
        this->Links = 1;
        this->MeshSeedLinks.SetSize(this->UGrid->GetNumberOfCells()*3,
                this->UGrid->GetNumberOfCells());
        this->MeshSeedLinks.Set(0);
        this->MeshSeedCheck.SetSize(this->UGrid->GetNumberOfCells(), 1);
        vtkIdList *neigh_cell_list = vtkIdList::New();

        CLinkedList<Node*> *meshseedtree =  new CLinkedList<Node*>;

        // starting point for the tree to be constructed
        for (int i=0; i<this->UGrid->GetNumberOfCells(); i++)
        {
                cout<<i<<endl;
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
                        }
        }
        //ofstream FileOutput;
        //FileOutput.open("debug.txt",std::ios::out);
        //for (int i=0; i < this->UGrid->GetNumberOfCells()*3; i++)
        //{
        //      for (int j=0; j<this->UGrid->GetNumberOfCells(); j++) {
        //              FileOutput << this->MeshSeedLinks(i+1, j+1)<<"  ";
        //      }
        //      FileOutput<<endl;
        //}

        neigh_cell_list->Delete();
        delete meshseedtree;
}


void vtkMimxMeshSeed::SetUnstructuredGrid(vtkUnstructuredGrid* UnsGrid)
{
        this->UGrid = UnsGrid;
        this->Links = 0;
}

void vtkMimxMeshSeed::SetMeshSeedValues(vtkIntArray* IntArray)
{
        this->MeshSeedValues = IntArray;
        this->Links = 0;
}

void vtkMimxMeshSeed::GetCellNeighbors(vtkIdType cellnum, vtkIdList *cellids)
{
        cellids->Initialize();
        vtkIdList *ptids = vtkIdList::New();
        vtkIdList *ptids_temp = vtkIdList::New();
        vtkIdList *cellids_temp = vtkIdList::New();
        this->UGrid->GetCellPoints(cellnum,ptids);
        for(int i=0; i < ptids->GetNumberOfIds(); i++)
        {
                //              ptids_temp->Initialize();
                ptids_temp->SetNumberOfIds(1);
                ptids_temp->SetId(0,ptids->GetId(i));
                cellids_temp->Initialize();
                this->UGrid->GetCellNeighbors(cellnum,ptids_temp, cellids_temp);
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

int vtkMimxMeshSeed::CheckIfCellsShareEdgeX(int cellnum1, int cellnum2, int parcell, int edgenum)
{
        bool status1 = false;
        bool status2 = false;
        vtkGenericCell *cell = vtkGenericCell::New();
        this->UGrid->GetCell(cellnum1, cell);
        vtkGenericCell *cell_comp = vtkGenericCell::New();
        this->UGrid->GetCell(cellnum2, cell_comp);
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

int vtkMimxMeshSeed::CheckIfCellsShareEdgeY(int cellnum1, int cellnum2, int parcell, int edgenum)
{
        bool status1 = false;
        bool status2 = false;
        vtkGenericCell *cell = vtkGenericCell::New();
        this->UGrid->GetCell(cellnum1, cell);
        vtkGenericCell *cell_comp = vtkGenericCell::New();
        this->UGrid->GetCell(cellnum2, cell_comp);
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
void vtkMimxMeshSeed::EstimateMeshSeedsBasedOnAverageElementLength(double ElLength)
{
        if(!this->Links)        
        {
                this->BuildMeshSeedLinks();
        }

        if(this->UGrid)
        {
                this->MeshSeedValues->Initialize();
                this->MeshSeedValues->SetNumberOfValues(0);
                this->MeshSeedValues->SetNumberOfComponents(3);
                int dim[3];
                // initialize mesh seeds
                dim[0] = 0; dim[1] = 0; dim[2] = 0;
                for(int i=0; i < this->UGrid->GetNumberOfCells(); i++)
                {
                        this->MeshSeedValues->InsertNextTupleValue(dim);
                }

                vtkGenericCell *cell = vtkGenericCell::New();
                for(int i =0; i < this->UGrid->GetNumberOfCells(); i++)
                {
                        cell->Initialize();
                        this->UGrid->GetCell(i, cell);
                        this->MeshSeedValues->GetTupleValue(i, dim);
                        // calculate the average length of X edges,
                        // 0-3, 1-2, 4-7, 5-6
                        if(!dim[0])// if the X meshseed is not precalculated
                        {
                                double lengthX = 0.0;
                                double a[3], b[3];
                                this->UGrid->GetPoint(cell->GetPointId(0),a);
                                this->UGrid->GetPoint(cell->GetPointId(3),b);
                                lengthX = lengthX + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->UGrid->GetPoint(cell->GetPointId(1),a);
                                this->UGrid->GetPoint(cell->GetPointId(2),b);
                                lengthX = lengthX + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->UGrid->GetPoint(cell->GetPointId(4),a);
                                this->UGrid->GetPoint(cell->GetPointId(7),b);
                                lengthX = lengthX + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->UGrid->GetPoint(cell->GetPointId(5),a);
                                this->UGrid->GetPoint(cell->GetPointId(6),b);
                                lengthX = lengthX + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                lengthX = lengthX/4.0;
                                dim[0] = int(lengthX/ElLength)+1;
                                if(dim[0] == 1) dim[0] = dim[0] +1;
                                this->MeshSeedValues->SetTupleValue(i, dim);
//                              this->MeshSeedX->SetValue(i,int(lengthX/length)+1);
                        }
                        this->SetHigherNumberedBBoxMeshSeed(i,1);
                        // calculate the average length of X edges,
                        // 0-4, 1-5, 2-6, 3-7
                        if(!dim[1])// if the X meshseed is not precalculated
                        {
                                double lengthY = 0.0;
                                double a[3], b[3];
                                this->UGrid->GetPoint(cell->GetPointId(0),a);
                                this->UGrid->GetPoint(cell->GetPointId(4),b);
                                lengthY = lengthY + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->UGrid->GetPoint(cell->GetPointId(1),a);
                                this->UGrid->GetPoint(cell->GetPointId(5),b);
                                lengthY = lengthY + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->UGrid->GetPoint(cell->GetPointId(2),a);
                                this->UGrid->GetPoint(cell->GetPointId(6),b);
                                lengthY = lengthY + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->UGrid->GetPoint(cell->GetPointId(3),a);
                                this->UGrid->GetPoint(cell->GetPointId(7),b);
                                lengthY = lengthY + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                lengthY = lengthY/4.0;
                                dim[1] = int(lengthY/ElLength)+1;
                                if(dim[1] == 1) dim[1] = dim[1] +1;
                                this->MeshSeedValues->SetTupleValue(i, dim);
//                              this->MeshSeedY->SetValue(i,int(lengthY/length)+1);
                        }
                        this->SetHigherNumberedBBoxMeshSeed(i,2);
                        // calculate the average length of Z edges,
                        // 0-1, 2-3, 4-5, 6-7
                        if(!dim[2])// if the X meshseed is not precalculated
                        {
                                double lengthZ = 0.0;
                                double a[3], b[3];
                                this->UGrid->GetPoint(cell->GetPointId(0),a);
                                this->UGrid->GetPoint(cell->GetPointId(1),b);
                                lengthZ = lengthZ + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->UGrid->GetPoint(cell->GetPointId(2),a);
                                this->UGrid->GetPoint(cell->GetPointId(3),b);
                                lengthZ = lengthZ + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->UGrid->GetPoint(cell->GetPointId(4),a);
                                this->UGrid->GetPoint(cell->GetPointId(5),b);
                                lengthZ = lengthZ + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                this->UGrid->GetPoint(cell->GetPointId(6),a);
                                this->UGrid->GetPoint(cell->GetPointId(7),b);
                                lengthZ = lengthZ + sqrt(vtkMath::Distance2BetweenPoints(a,b));
                                lengthZ = lengthZ/4.0;
                                dim[2] = int(lengthZ/ElLength)+1;
                                if(dim[2] == 1) dim[2] = dim[2] +1;
                                this->MeshSeedValues->SetTupleValue(i, dim);
//                              this->MeshSeedZ->SetValue(i,int(lengthZ/length)+1);
                        }
                        this->SetHigherNumberedBBoxMeshSeed(i,3);
                }
                cell->Delete();
                //cout <<"Mesh Seed"<<endl;
                for(int i=0; i<this->UGrid->GetNumberOfCells(); i++)
                {
                        this->MeshSeedValues->GetTupleValue(i, dim);
                        //cout <<dim[0]<<"  "<<dim[1]<<"  "<<dim[2]<<endl;
                }
                if(!this->UGrid->GetCellData()->GetVectors())
                {
                        this->UGrid->GetCellData()->SetVectors(this->MeshSeedValues);
                }
        }
}


int vtkMimxMeshSeed::CheckIfCellsShareEdgeZ(int cellnum1, int cellnum2, int parcell, int edgenum)
{
        bool status1 = false;
        bool status2 = false;
        vtkGenericCell *cell = vtkGenericCell::New();
        this->UGrid->GetCell(cellnum1, cell);
        vtkGenericCell *cell_comp = vtkGenericCell::New();
        this->UGrid->GetCell(cellnum2, cell_comp);
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

int vtkMimxMeshSeed::WhichEdgeOfCellBeingCompared(vtkIdType vertex1, vtkIdType vertex2)
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

void vtkMimxMeshSeed::SetHigherNumberedBBoxMeshSeed(int boxnum, int axis)
{
        int dim[3], dimmod[3];

        for(int i = boxnum+1; i < this->UGrid->GetNumberOfCells(); i++)
        {
                int j = this->MeshSeedLinks(boxnum*3+axis,i+1);
                if(j)
                {
                        this->MeshSeedValues->GetTupleValue(boxnum, dim);
                        this->MeshSeedValues->GetTupleValue(i, dimmod);
                        
                        dimmod[j-1] = dim[axis-1];
                        this->MeshSeedValues->SetTupleValue(i, dimmod);
                        //if(axis == 1)
                        //{
                        //      if(j==1)        
                        //      {/*this->MeshSeedX->SetValue(i,
                        //      this->MeshSeedX->GetValue(boxnum));*/
                        //              dimmod[0] = dim[0];
                        //              this->MeshSeedValues->SetTuple(i, dimmod);
                        //      break;
                        //      }
                        //      if(j==2)        
                        //      {/*this->MeshSeedY->SetValue(i,
                        //      this->MeshSeedX->GetValue(boxnum));*/
                        //              dimmod[1] = dim[0];
                        //              this->MeshSeedValues->SetTuple(i, dimmod);
                        //      break;
                        //      }
                        //      if(j==3)        
                        //      {/*this->MeshSeedZ->SetValue(i,
                        //      this->MeshSeedX->GetValue(boxnum));*/
                        //              dimmod[2] = dim[0];
                        //              this->MeshSeedValues->SetTuple(i, dimmod);
                        //      break;
                        //      }
                        //      break;
                        //}
                        //if(axis == 2)
                        //{
                        //      if(j==1)        
                        //      {/*this->MeshSeedX->SetValue(i,
                        //      this->MeshSeedY->GetValue(boxnum));*/
                        //              dimmod[0] = dim[0];
                        //              this->MeshSeedValues->SetTuple(i, dimmod);
                        //      break;
                        //      }
                        //      if(j==2)        
                        //      {this->MeshSeedY->SetValue(i,
                        //      this->MeshSeedY->GetValue(boxnum));
                        //      break;
                        //      }
                        //      if(j==3)        
                        //      {this->MeshSeedZ->SetValue(i,
                        //      this->MeshSeedY->GetValue(boxnum));
                        //      break;
                        //      }       
                        //      break;
                        //}
                        //if(axis == 3)
                        //{
                        //      if(j==1)        
                        //      {this->MeshSeedX->SetValue(i,
                        //      this->MeshSeedZ->GetValue(boxnum));
                        //      break;
                        //      }
                        //      if(j==2)        
                        //      {this->MeshSeedY->SetValue(i,
                        //      this->MeshSeedZ->GetValue(boxnum));
                        //      break;
                        //      }
                        //      if(j==3)        
                        //      {this->MeshSeedZ->SetValue(i,
                        //      this->MeshSeedZ->GetValue(boxnum));
                        //      break;
                        //      }       
                        //      break;
                        //}
                }
        }
}

void vtkMimxMeshSeed::ChangeMeshSeed(int BoxNum, int Axis, int SeedNum)
{
        if(!this->Links)        
        {
                this->BuildMeshSeedLinks();
        }

        // change the mesh seed of the BoxNum first
        int dim[3], dimmod[3];
        this->MeshSeedValues->GetTupleValue(BoxNum, dim);
        dim[Axis] = SeedNum;
        this->MeshSeedValues->SetTupleValue(BoxNum, dim);

        // change all the other relevant boundingbox mesh seeds
        for(int i = 0; i < this->UGrid->GetNumberOfCells(); i++)
        {
                int j = this->MeshSeedLinks(BoxNum*3+Axis+1,i+1);
                if(j)
                {
                        this->MeshSeedValues->GetTupleValue(BoxNum, dim);
                        this->MeshSeedValues->GetTupleValue(i, dimmod);

                        dimmod[j-1] = dim[Axis];
                        this->MeshSeedValues->SetTupleValue(i, dimmod);
                }
        }
        cout <<"Mesh Seed"<<endl;
        for(int i=0; i<this->UGrid->GetNumberOfCells(); i++)
        {
                this->MeshSeedValues->GetTupleValue(i, dim);
                cout <<dim[0]<<"  "<<dim[1]<<"  "<<dim[2]<<endl;
        }
}
