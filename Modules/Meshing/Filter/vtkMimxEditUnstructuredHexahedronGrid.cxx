/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxEditUnstructuredHexahedronGrid.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
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

#include "vtkMimxEditUnstructuredHexahedronGrid.h"
#include "vtkCellData.h"
#include "vtkThreshold.h"
#include "vtkIdList.h"
#include "vtkUnstructuredGrid.h"
#include "vtkIntArray.h"
#include "vtkCellArray.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkTriangle.h"
#include "vtkMergeCells.h"


vtkCxxRevisionMacro(vtkMimxEditUnstructuredHexahedronGrid, "$Revision: 1.6 $");
vtkStandardNewMacro(vtkMimxEditUnstructuredHexahedronGrid);


// Constructs with initial  values.
vtkMimxEditUnstructuredHexahedronGrid::vtkMimxEditUnstructuredHexahedronGrid()
{
        IsInitialized = 0;
        this->ProjectionMethodCell = NULL;
}

// Destroy any allocated memory.
vtkMimxEditUnstructuredHexahedronGrid::~vtkMimxEditUnstructuredHexahedronGrid()
{
        if(!this->ProjectionMethodCell) ProjectionMethodCell->Delete();
}

void vtkMimxEditUnstructuredHexahedronGrid::Execute()
{
        //      check for initialization 
        if(!IsInitialized)
        {
                //      create a new array storing information to be used on projection method
                this->ProjectionMethodCell = vtkIdList::New();
                IsInitialized = 1;
                this->ProjectionMethodCell->SetNumberOfIds(
                        (this->GetUGrid()->GetNumberOfCells()));
                //      initialize all the values for closest point projection
                for(int i=0; i < this->GetUGrid()->GetNumberOfCells(); i++)
                        this->ProjectionMethodCell->SetId(i,VTKPROJECTION_CLOSESTPOINT);
//              this->GetUGrid()->Initialize();
//              this->GetUGrid()->DeepCopy(this->GetUGrid());
        }
/*      else
        {
                //  modifying the array storing the information of projection
                //  two options exist - increase or decrease in number of cells
                //      hence check for in number of cells in input and output

                //      copy the modified projection method storage based on the cell deleted
                //      handles only one cell at a time
                //  bump up the location
                int x = this->GetUGrid()->GetNumberOfCells();
                int y = this->GetUGrid()->GetNumberOfCells();
                if(this->GetUGrid()->GetNumberOfCells() > this->GetUGrid()->GetNumberOfCells())
                {
                        if(this->CellToBeDeleted != this->GetUGrid()->GetNumberOfCells()-1)
                        {
                                for(int i=this->CellToBeDeleted; i < this->GetUGrid()->GetNumberOfCells()-1;i++)
                                {
                                        this->ProjectionMethodCell->SetId(i,this->ProjectionMethodCell->GetId(i+1));
                                }
                        }
                        //      delete the last value
                        this->ProjectionMethodCell->SetNumberOfIds(
                                this->ProjectionMethodCell->GetNumberOfIds()-1);
                        this->ProjectionMethodCell->Squeeze();
                }
                else
                {
                        this->ProjectionMethodCell->SetId
                                (this->ProjectionMethodCell->GetNumberOfIds(),VTKPROJECTION_CLOSESTPOINT);
                }
        }*/
}

// Delete cell based on the cell number.
// The cell is deleted from the list. the cells are renumbered and points not used
// are also deleted
void vtkMimxEditUnstructuredHexahedronGrid::DeleteCell(vtkIdType cellchosen)
{
        this->CellToBeDeleted = cellchosen;
        // declare an array to be used as scalar
        vtkIntArray* scalararray = vtkIntArray::New();
        //      an array for the number of cells
        scalararray->SetNumberOfValues(this->GetUGrid()->GetNumberOfCells());
        //      set the values initially to zero
        for(int i=0; i < this->GetUGrid()->GetNumberOfCells(); i++)     scalararray->SetValue(i,0);
        //      set the value of the cell to be deleted to 2
        scalararray->SetValue(cellchosen,2);
        //      check if the scalar data for the cells have been initialized previously
        if(this->GetUGrid()->GetCellData()->GetScalars())
        {
                this->GetUGrid()->GetCellData()->GetScalars()->Initialize();
        }
        //      set the scalars
        this->GetUGrid()->GetCellData()->SetScalars(scalararray);
        //      for the deleting the cell and renumbering
        //      copy the input to the output before renumbering
        this->GetUGrid()->Initialize();
        this->GetUGrid()->DeepCopy(this->GetUGrid());
        vtkThreshold* threshold = vtkThreshold::New();
        //      input the grid to be edited
        threshold->SetInput(this->GetUGrid());
        //      the cell to be deleted has a scalar value of 2
        threshold->ThresholdBetween(0,1);
        //      specify the array to process
        threshold->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_CELLS,
                vtkDataSetAttributes::SCALARS);
        threshold->Modified();
        //      store the modified grid to the output
        this->GetUGrid()->Initialize();
        this->GetUGrid()->DeepCopy(threshold->GetInput());
        scalararray->Delete();
        threshold->Delete();
}

// split cells depending on the edge chosen to split
void vtkMimxEditUnstructuredHexahedronGrid::SplitCell(vtkIdType* ptlist)
{

        vtkIdType pt1 = ptlist[0];
        vtkIdType pt2 = ptlist[1];

        vtkIdType* pts=0;
        vtkIdType t=0;
        vtkIdType pts_replace[8];
        vtkUnstructuredGrid* input = this->GetUGrid();
        this->GetUGrid()->GetCells()->InitTraversal();
        bool status = false;
        int count_ele = 0;
        // to check along which axis the needs to be split
        bool case_i = false, case_j = false, case_k = false;
        //      check whether the points belong to a hexahedron
        while (!status) 
        {
                this->GetUGrid()->GetCells()->GetNextCell(t,pts);
                if(pts[0] == pt1 || pts[1] == pt1 || pts[2] == pt1 || pts[3] == pt1 ||
                        pts[4] == pt1 || pts[5] == pt1 || pts[6] == pt1 || pts[7] == pt1)
                {
                        if(pts[0] == pt2 || pts[1] == pt2 || pts[2] == pt2 || pts[3] == pt2 ||
                                pts[4] == pt2 || pts[5] == pt2 || pts[6] == pt2 || pts[7] == pt2)
                        {
                                status = true;
                                if(((pts[0] == pt1) && pts[3] == pt2) || ((pts[3] == pt1) && pts[0] == pt2) 
                                        || ((pts[1] == pt1) && pts[2] == pt2) || ((pts[2] == pt1) && pts[1] == pt2)
                                        || ((pts[4] == pt1) && pts[7] == pt2) || ((pts[7] == pt1) && pts[4] == pt2)
                                        || ((pts[5] == pt1) && pts[6] == pt2) || ((pts[6] == pt1) && pts[5] == pt2))
                                {
                                        case_i = true;
                                }
                                else
                                {
                                        if(((pts[0] == pt1) && pts[4] == pt2) || ((pts[4] == pt1) && pts[0] == pt2) 
                                                || ((pts[1] == pt1) && pts[5] == pt2) || ((pts[5] == pt1) && pts[1] == pt2)
                                                || ((pts[2] == pt1) && pts[6] == pt2) || ((pts[6] == pt1) && pts[2] == pt2)
                                                || ((pts[3] == pt1) && pts[7] == pt2) || ((pts[3] == pt1) && pts[7] == pt2))
                                        {
                                                case_j = true;
                                        }
                                        else
                                        {
                                                case_k = true;
                                        }
                                }
                        }
                }
                count_ele++;
        }
        double x1[3], x2[3];
        //      get points based on the plane being divided
        //      lines accessed are in CW
        // generate the connectivity to replace the original cell with
        //      the cell closest to the origin
        for(int i=0; i <8; i++) pts_replace[i] = pts[i];
        //      list for the new cell being inserted into the grid
        vtkIdList* idlist = vtkIdList::New();
        idlist->SetNumberOfIds(8);
        for(int i=0; i <8; i++) idlist->SetId(i,pts[i]);

        if(case_i)
        {
                // calculate the coordinates of the points
                // point is located at the center of the line chosen
                this->GetUGrid()->GetPoint(pts[0],x1);  this->GetUGrid()->GetPoint(pts[3],x2);
                this->GetUGrid()->GetPoints()->InsertNextPoint(
                        (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                this->GetUGrid()->GetPoint(pts[4],x1);  this->GetUGrid()->GetPoint(pts[7],x2);
                this->GetUGrid()->GetPoints()->InsertNextPoint(
                        (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                this->GetUGrid()->GetPoint(pts[5],x1);  this->GetUGrid()->GetPoint(pts[6],x2);
                this->GetUGrid()->GetPoints()->InsertNextPoint(
                        (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                this->GetUGrid()->GetPoint(pts[1],x1);  this->GetUGrid()->GetPoint(pts[2],x2);
                this->GetUGrid()->GetPoints()->InsertNextPoint(
                        (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                pts_replace[2] = this->GetUGrid()->GetPoints()->GetNumberOfPoints()-1;
                pts_replace[3] = this->GetUGrid()->GetPoints()->GetNumberOfPoints()-4;
                pts_replace[6] = this->GetUGrid()->GetPoints()->GetNumberOfPoints()-2;
                pts_replace[7] = this->GetUGrid()->GetPoints()->GetNumberOfPoints()-3;
                this->GetUGrid()->ReplaceCell(count_ele-1,8,pts_replace);
                // generate the connectivity to insert cell farther from the origin
                //      into grid 
                idlist->SetId(0,this->GetUGrid()->GetPoints()->GetNumberOfPoints()-4);
                idlist->SetId(1,this->GetUGrid()->GetPoints()->GetNumberOfPoints()-1);
                idlist->SetId(4,this->GetUGrid()->GetPoints()->GetNumberOfPoints()-3);
                idlist->SetId(5,this->GetUGrid()->GetPoints()->GetNumberOfPoints()-2);

        }

        if(case_j)
        {
                // calculate the coordinates of the points
                // point is located at the center of the line chosen
                this->GetUGrid()->GetPoint(pts[0],x1);  this->GetUGrid()->GetPoint(pts[4],x2);
                this->GetUGrid()->GetPoints()->InsertNextPoint(
                        (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                this->GetUGrid()->GetPoint(pts[1],x1);  this->GetUGrid()->GetPoint(pts[5],x2);
                this->GetUGrid()->GetPoints()->InsertNextPoint(
                        (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                this->GetUGrid()->GetPoint(pts[2],x1);  this->GetUGrid()->GetPoint(pts[6],x2);
                this->GetUGrid()->GetPoints()->InsertNextPoint(
                        (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                this->GetUGrid()->GetPoint(pts[3],x1);  this->GetUGrid()->GetPoint(pts[7],x2);
                this->GetUGrid()->GetPoints()->InsertNextPoint(
                        (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                pts_replace[4] = this->GetUGrid()->GetPoints()->GetNumberOfPoints()-4;
                pts_replace[5] = this->GetUGrid()->GetPoints()->GetNumberOfPoints()-3;
                pts_replace[6] = this->GetUGrid()->GetPoints()->GetNumberOfPoints()-2;
                pts_replace[7] = this->GetUGrid()->GetPoints()->GetNumberOfPoints()-1;
                this->GetUGrid()->ReplaceCell(count_ele-1,8,pts_replace);
                // generate the connectivity to insert cell farther from the origin
                //      into grid 
                idlist->SetId(0,this->GetUGrid()->GetPoints()->GetNumberOfPoints()-4);
                idlist->SetId(1,this->GetUGrid()->GetPoints()->GetNumberOfPoints()-3);
                idlist->SetId(2,this->GetUGrid()->GetPoints()->GetNumberOfPoints()-2);
                idlist->SetId(3,this->GetUGrid()->GetPoints()->GetNumberOfPoints()-1);

        }

        if(case_k)
        {
                // calculate the coordinates of the points
                // point is located at the center of the line chosen
                this->GetUGrid()->GetPoint(pts[0],x1);  this->GetUGrid()->GetPoint(pts[1],x2);
                this->GetUGrid()->GetPoints()->InsertNextPoint(
                        (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                this->GetUGrid()->GetPoint(pts[3],x1);  this->GetUGrid()->GetPoint(pts[2],x2);
                this->GetUGrid()->GetPoints()->InsertNextPoint(
                        (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                this->GetUGrid()->GetPoint(pts[7],x1);  this->GetUGrid()->GetPoint(pts[6],x2);
                this->GetUGrid()->GetPoints()->InsertNextPoint(
                        (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                this->GetUGrid()->GetPoint(pts[4],x1);  this->GetUGrid()->GetPoint(pts[5],x2);
                this->GetUGrid()->GetPoints()->InsertNextPoint(
                        (x1[0]+x2[0])/2.0, (x1[1]+x2[1])/2.0, (x1[2]+x2[2])/2.0);
                pts_replace[1] = this->GetUGrid()->GetPoints()->GetNumberOfPoints()-4;
                pts_replace[2] = this->GetUGrid()->GetPoints()->GetNumberOfPoints()-3;
                pts_replace[5] = this->GetUGrid()->GetPoints()->GetNumberOfPoints()-1;
                pts_replace[6] = this->GetUGrid()->GetPoints()->GetNumberOfPoints()-2;
                this->GetUGrid()->ReplaceCell(count_ele-1,8,pts_replace);
                // generate the connectivity to insert cell farther from the origin
                //      into grid 
                idlist->SetId(0,this->GetUGrid()->GetPoints()->GetNumberOfPoints()-4);
                idlist->SetId(3,this->GetUGrid()->GetPoints()->GetNumberOfPoints()-3);
                idlist->SetId(4,this->GetUGrid()->GetPoints()->GetNumberOfPoints()-1);
                idlist->SetId(7,this->GetUGrid()->GetPoints()->GetNumberOfPoints()-2);
        }
        
        for(int i=0; i <8; i++)
        {
                pt1 = idlist->GetId(i);
        }
        this->UGrid->InsertNextCell(12,idlist);
        this->UGrid->GetCells()->Modified();
        this->UGrid->Modified();
//      this->UGrid->BuildLinks();
//      vtkUnstructuredGridWriter* writer = vtkUnstructuredGridWriter::New();
//      writer->SetFileName("debug1.txt");
//      writer->SetInput(this->GetUGrid());
//      writer->Write();
        idlist->Delete();
        }

void vtkMimxEditUnstructuredHexahedronGrid::AddCell(vtkIdType* ptlist)
{
        vtkIdType* pts=0;
        vtkIdType t=0;
        vtkUnstructuredGrid* input = this->GetUGrid();
        this->GetUGrid()->GetCells()->InitTraversal();
        vtkCell *face;
        int cell_count = 0;
        int face_count;
        bool status = false;
        while (!status) 
        {
                face_count = 0;
                do {
                        face = this->GetUGrid()->GetCell(cell_count)->                          GetFace(face_count++);
                        int pt0 = face->GetPointId(0);  int pt1 = face->GetPointId(1);  
                        int pt2 = face->GetPointId(2);  int pt3 = face->GetPointId(3);  
                        if(face->GetPointId(0) == ptlist[0] || face->GetPointId(1) == ptlist[0]
                                || face->GetPointId(2) == ptlist[0] || face->GetPointId(3) == ptlist[0])
                        {
                                if(face->GetPointId(0) == ptlist[1] || face->GetPointId(1) == ptlist[1]
                                        || face->GetPointId(2) == ptlist[1] || face->GetPointId(3) == ptlist[1])
                                {
                                        if(face->GetPointId(0) == ptlist[2] || face->GetPointId(1) == ptlist[2]
                                                || face->GetPointId(2) == ptlist[2] || face->GetPointId(3) == ptlist[2])
                                        {
                                                if(face->GetPointId(0) == ptlist[3] || face->GetPointId(1) == ptlist[3]
                                                        || face->GetPointId(2) == ptlist[3] || face->GetPointId(3) == ptlist[3])
                                                {
                                                        status = true;
                                                }
                                        }
                                }
                        }

                } while(!status && face_count < 6);
                cell_count++;
        }
        // calculate normal along which the cell has to be extruded
        double pt1[3], pt2[3], pt3[3], pt4[3];
        double normal[3], normal1[3], normal2[3], normal3[3], normal4[3];
        this->GetUGrid()->GetPoint(face->GetPointId(0),pt1);
        this->GetUGrid()->GetPoint(face->GetPointId(1),pt2);
        this->GetUGrid()->GetPoint(face->GetPointId(2),pt3);
        this->GetUGrid()->GetPoint(face->GetPointId(3),pt4);
        vtkTriangle::ComputeNormal(pt1, pt2, pt3, normal1);
        vtkTriangle::ComputeNormal(pt2, pt3, pt4, normal2);
        vtkTriangle::ComputeNormal(pt3, pt4, pt1, normal3);
        vtkTriangle::ComputeNormal(pt4, pt1, pt2, normal4);
        normal[0] = 0.0;
        normal[0] = normal1[0] + normal2[0] + normal3[0] + normal4[0];
        normal[1] = normal1[1] + normal2[1] + normal3[1] + normal4[1];
        normal[2] = normal1[2] + normal2[2] + normal3[2] + normal4[2];
        for(int i=0; i<3; i++)  normal[i] = normal[i]/4.0;

        this->UGrid->GetPoints()->InsertNextPoint(pt1[0] + 5.0*normal[0],
                pt1[1] + 5.0*normal[1], pt1[2] + 5.0*normal[2]);
        this->UGrid->GetPoints()->InsertNextPoint(pt2[0] + 5.0*normal[0],
                pt2[1] + 5.0*normal[1], pt2[2] + 5.0*normal[2]);
        this->UGrid->GetPoints()->InsertNextPoint(pt3[0] + 5.0*normal[0],
                pt3[1] + 5.0*normal[1], pt3[2] + 5.0*normal[2]);
        this->UGrid->GetPoints()->InsertNextPoint(pt4[0] + 5.0*normal[0],
                pt4[1] + 5.0*normal[1], pt4[2] + 5.0*normal[2]);
        // set idlist for the new cell being added
        vtkIdType idlist[8];
//      idlist->SetNumberOfIds(8);
        // numbering order depends on the face chosen   
        if(face_count == 1)
        {
                //idlist->SetId(1, face->GetPointId(0));
                //idlist->SetId(5, face->GetPointId(1));
                //idlist->SetId(6, face->GetPointId(2));
                //idlist->SetId(2, face->GetPointId(3));
                //idlist->SetId(0, this->UGrid->GetNumberOfPoints()-4);
                //idlist->SetId(4, this->UGrid->GetNumberOfPoints()-3);
                //idlist->SetId(7, this->UGrid->GetNumberOfPoints()-2);
                //idlist->SetId(3, this->UGrid->GetNumberOfPoints()-1);
                idlist[1] = face->GetPointId(0);
                idlist[5] = face->GetPointId(1);
                idlist[6] = face->GetPointId(2);
                idlist[2] = face->GetPointId(3);
                idlist[0] = this->UGrid->GetNumberOfPoints()-4;
                idlist[4] = this->UGrid->GetNumberOfPoints()-3;
                idlist[7] = this->UGrid->GetNumberOfPoints()-2;
                idlist[3] = this->UGrid->GetNumberOfPoints()-1;

        }

        if(face_count == 2)
        {
        /*      idlist->SetId(0, face->GetPointId(0));
                idlist->SetId(3, face->GetPointId(1));
                idlist->SetId(7, face->GetPointId(2));
                idlist->SetId(4, face->GetPointId(3));
                idlist->SetId(1, this->UGrid->GetNumberOfPoints()-4);
                idlist->SetId(2, this->UGrid->GetNumberOfPoints()-3);
                idlist->SetId(6, this->UGrid->GetNumberOfPoints()-2);
                idlist->SetId(5, this->UGrid->GetNumberOfPoints()-1);*/
                idlist[0] = face->GetPointId(0);
                idlist[3] = face->GetPointId(1);
                idlist[7] = face->GetPointId(2);
                idlist[4] = face->GetPointId(3);
                idlist[1] = this->UGrid->GetNumberOfPoints()-4;
                idlist[2] = this->UGrid->GetNumberOfPoints()-3;
                idlist[6] = this->UGrid->GetNumberOfPoints()-2;
                idlist[5] = this->UGrid->GetNumberOfPoints()-1;

        }
        if(face_count == 3)
        {
                //idlist->SetId(3, face->GetPointId(0));
                //idlist->SetId(2, face->GetPointId(1));
                //idlist->SetId(6, face->GetPointId(2));
                //idlist->SetId(7, face->GetPointId(3));
                //idlist->SetId(0, this->UGrid->GetNumberOfPoints()-4);
                //idlist->SetId(1, this->UGrid->GetNumberOfPoints()-3);
                //idlist->SetId(5, this->UGrid->GetNumberOfPoints()-2);
                //idlist->SetId(4, this->UGrid->GetNumberOfPoints()-1);
                idlist[3] = face->GetPointId(0);
                idlist[2] = face->GetPointId(1);
                idlist[6] = face->GetPointId(2);
                idlist[7] = face->GetPointId(3);
                idlist[0] = this->UGrid->GetNumberOfPoints()-4;
                idlist[1] = this->UGrid->GetNumberOfPoints()-3;
                idlist[5] = this->UGrid->GetNumberOfPoints()-2;
                idlist[4] = this->UGrid->GetNumberOfPoints()-1;

        }
        if(face_count == 4)
        {
                //idlist->SetId(0, face->GetPointId(0));
                //idlist->SetId(4, face->GetPointId(1));
                //idlist->SetId(5, face->GetPointId(2));
                //idlist->SetId(1, face->GetPointId(3));
                //idlist->SetId(3, this->UGrid->GetNumberOfPoints()-4);
                //idlist->SetId(7, this->UGrid->GetNumberOfPoints()-3);
                //idlist->SetId(6, this->UGrid->GetNumberOfPoints()-2);
                //idlist->SetId(2, this->UGrid->GetNumberOfPoints()-1);
                idlist[0] = face->GetPointId(0);
                idlist[4] = face->GetPointId(1);
                idlist[5] = face->GetPointId(2);
                idlist[1] = face->GetPointId(3);
                idlist[3] = this->UGrid->GetNumberOfPoints()-4;
                idlist[7] = this->UGrid->GetNumberOfPoints()-3;
                idlist[6] = this->UGrid->GetNumberOfPoints()-2;
                idlist[2] = this->UGrid->GetNumberOfPoints()-1;

        }
        if(face_count == 5)
        {
                //idlist->SetId(4, face->GetPointId(0));
                //idlist->SetId(7, face->GetPointId(1));
                //idlist->SetId(6, face->GetPointId(2));
                //idlist->SetId(5, face->GetPointId(3));
                //idlist->SetId(0, this->UGrid->GetNumberOfPoints()-4);
                //idlist->SetId(3, this->UGrid->GetNumberOfPoints()-3);
                //idlist->SetId(2, this->UGrid->GetNumberOfPoints()-2);
                //idlist->SetId(1, this->UGrid->GetNumberOfPoints()-1);
                idlist[4] = face->GetPointId(0);
                idlist[7] = face->GetPointId(1);
                idlist[6] = face->GetPointId(2);
                idlist[5] = face->GetPointId(3);
                idlist[0] = this->UGrid->GetNumberOfPoints()-4;
                idlist[3] = this->UGrid->GetNumberOfPoints()-3;
                idlist[2] = this->UGrid->GetNumberOfPoints()-2;
                idlist[1] = this->UGrid->GetNumberOfPoints()-1;

        }
        if(face_count == 6)
        {
                //idlist->SetId(0, face->GetPointId(0));
                //idlist->SetId(1, face->GetPointId(1));
                //idlist->SetId(2, face->GetPointId(2));
                //idlist->SetId(3, face->GetPointId(3));
                //idlist->SetId(4, this->UGrid->GetNumberOfPoints()-4);
                //idlist->SetId(5, this->UGrid->GetNumberOfPoints()-3);
                //idlist->SetId(6, this->UGrid->GetNumberOfPoints()-2);
                //idlist->SetId(7, this->UGrid->GetNumberOfPoints()-1); 
                idlist[0] = face->GetPointId(0);
                idlist[1] = face->GetPointId(1);
                idlist[2] = face->GetPointId(2);
                idlist[3] = face->GetPointId(3);
                idlist[4] = this->UGrid->GetNumberOfPoints()-4;
                idlist[5] = this->UGrid->GetNumberOfPoints()-3;
                idlist[6] = this->UGrid->GetNumberOfPoints()-2;
                idlist[7] = this->UGrid->GetNumberOfPoints()-1;

        }
//      this->GetUGrid()->InsertNextCell(12,idlist);
        this->UGrid->InsertNextCell(12,8,idlist);
        this->GetUGrid()->Modified();
//      this->UGrid->BuildLinks();
}

void vtkMimxEditUnstructuredHexahedronGrid::MergeNodes(double Tol)
{
        vtkMergeCells* mergecells = vtkMergeCells::New();
        vtkUnstructuredGrid *tempgrid = vtkUnstructuredGrid::New();
        vtkCell* cell;
        mergecells->SetUnstructuredGrid(tempgrid);
        mergecells->SetPointMergeTolerance(Tol);
        mergecells->SetTotalNumberOfDataSets(1);
        mergecells->SetTotalNumberOfCells(this->UGrid->GetNumberOfCells());
        mergecells->SetTotalNumberOfPoints(this->UGrid->GetNumberOfPoints());
        mergecells->MergeDataSet(this->UGrid);
        this->UGrid->Initialize();
        this->UGrid->DeepCopy(tempgrid);
        vtkIdType max_val = 0;
        for(int i=0; i < this->UGrid->GetNumberOfCells(); i++)
        {
                cell = this->UGrid->GetCell(i);
                vtkIdList* idlist = cell->GetPointIds();

                for(int j=0; j <8; j++)
                {
                        vtkIdType val = idlist->GetId(j);
                        if(val > max_val)       max_val = val;
                }
        }
        this->UGrid->GetPoints()->Squeeze();
        this->UGrid->GetPoints()->SetNumberOfPoints(max_val+1);
        this->UGrid->Modified();
        //vtkUnstructuredGridWriter* writer = vtkUnstructuredGridWriter::New();
        //writer->SetInput(this->UGrid);
        //writer->SetFileName("debug.txt");
        //writer->Write();
        tempgrid->Delete();
}

// 0 for x, 1 for y and 2 for z
// 0 for beginning and 1 for end
void vtkMimxEditUnstructuredHexahedronGrid::Mirror(int axis, int end)
{
        int numcells = this->UGrid->GetNumberOfCells();
        int numnodes = this->UGrid->GetNumberOfPoints();
        double x[3];
        double offset;
        double bounds[6];
        this->UGrid->GetBounds(bounds);
        offset = bounds[2*axis+end];
        vtkIdList *idlist = vtkIdList::New();
        for(int i = 0; i < numnodes; i++)
        {

                        this->UGrid->GetPoint(i,x);
                        x[axis] = 2.0*offset - x[axis];
                        this->UGrid->GetPoints()->InsertNextPoint(x);
        }
        for(int i=0; i <numcells; i++)
        {
                idlist->Initialize();
                idlist->SetNumberOfIds(8);
                for(int j=0; j<8; j++)
                {
                        idlist->SetId(j,this->UGrid->GetCell(i)->GetPointId(j)+numnodes);
                }
                this->UGrid->InsertNextCell(12, idlist);
        }
        this->UGrid->Modified();
//      this->UGrid->BuildLinks();
}
