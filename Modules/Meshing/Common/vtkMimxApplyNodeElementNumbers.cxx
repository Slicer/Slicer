/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxApplyNodeElementNumbers.cxx,v $
Language:  C++
Date:      $Date: 2008/03/21 20:10:49 $
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

#include "vtkMimxApplyNodeElementNumbers.h"

#include "vtkCellData.h"
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"

vtkMimxApplyNodeElementNumbers::vtkMimxApplyNodeElementNumbers()
{
        this->UGrid = NULL;
        this->StartingElementNumber = 1;
        this->StartingNodeNumber = 1;
        this->ElementSetName = NULL;
        this->NodeSetName = NULL;
}

vtkMimxApplyNodeElementNumbers::~vtkMimxApplyNodeElementNumbers()
{
}

void vtkMimxApplyNodeElementNumbers::SetUnstructuredGrid(vtkUnstructuredGrid *UnsGrid)
{
        this->UGrid = UnsGrid;
}

void vtkMimxApplyNodeElementNumbers::SetStartingElementNumber(int EleNum)
{
        this->StartingElementNumber = EleNum;
}

void vtkMimxApplyNodeElementNumbers::SetStartingNodeNumber(int NodeNum)
{
        this->StartingNodeNumber = NodeNum;
}

void vtkMimxApplyNodeElementNumbers::ApplyElementNumbers()
{
        //// if an array already present
        //int i;
        //if(UGrid->GetCellData()->GetArray("Element_Numbers"))
        //{
        //      vtkIntArray *elementarray = 
        //              vtkIntArray::SafeDownCast(UGrid->GetCellData()->GetArray("Element_Numbers"));
        //      for (i=0; i<UGrid->GetNumberOfCells(); i++) {
        //              elementarray->SetValue(i, i+this->StartingElementNumber);
        //      }
        //}
        //else
        //{
        //      vtkIntArray *elementarray = vtkIntArray::New();
        //      elementarray->SetNumberOfValues(UGrid->GetNumberOfCells());

        //      for (i=0; i<UGrid->GetNumberOfCells(); i++) {
        //              elementarray->SetValue(i, i+this->StartingElementNumber);
        //      }
        //      elementarray->SetName("Element_Numbers");
        //      UGrid->GetCellData()->AddArray(elementarray);
        //      elementarray->Delete();
        //}

        //// to change the name of the element set
        //// delete all the celldata except Element_Numbers
        //if(this->ElementSetName)
        //{
        //      int numarrays = UGrid->GetCellData()->GetNumberOfArrays();
        //      for (i = 0; i < numarrays; i++)
        //      {
        //              if(strcmp(UGrid->GetCellData()->GetArray(i)->GetName(), "Element_Numbers"))
        //              {
        //                      this->UGrid->GetCellData()->RemoveArray(
        //                              this->UGrid->GetCellData()->GetArray(i)->GetName());
        //              }
        //      }
        //      vtkIntArray *elementarray = vtkIntArray::New();
        //      elementarray->SetNumberOfValues(UGrid->GetNumberOfCells());

        //      for (i=0; i<UGrid->GetNumberOfCells(); i++) {
        //              elementarray->SetValue(i, 1);
        //      }
        //      elementarray->SetName(ElementSetName);
        //      UGrid->GetCellData()->AddArray(elementarray);
        //      elementarray->Delete();
        //}

        int i;
        vtkIntArray *elementarray = vtkIntArray::SafeDownCast(
                UGrid->GetCellData()->GetArray("Element_Numbers"));

        if(!elementarray)
        {
                elementarray = vtkIntArray::New();
                elementarray->SetNumberOfValues(UGrid->GetNumberOfCells());
                for (i=0; i<this->UGrid->GetNumberOfCells(); i++)
                {
                        elementarray->SetValue(i, i+this->StartingElementNumber);
                }
                // for the element numbers
                elementarray->SetName("Element_Numbers");
                UGrid->GetCellData()->AddArray(elementarray);
                elementarray->Delete();
                //
                elementarray = vtkIntArray::New();
                elementarray->SetNumberOfValues(UGrid->GetNumberOfCells());
                // for the element set
                for (i=0; i<UGrid->GetNumberOfCells(); i++) {
                        elementarray->SetValue(i, 1);
                }
                elementarray->SetName(ElementSetName);
                UGrid->GetCellData()->AddArray(elementarray);
                elementarray->Delete();
        }
        else
        {
                for (i=0; i<this->UGrid->GetNumberOfCells(); i++)
                {
                        elementarray->SetValue(i, i+this->StartingElementNumber);
                }
        }
}

void vtkMimxApplyNodeElementNumbers::ApplyNodeNumbers()
{
        // if an array already present
        int i;
        if(UGrid->GetPointData()->GetArray("Node_Numbers"))
        {
                vtkIntArray *nodearray = 
                        vtkIntArray::SafeDownCast(UGrid->GetPointData()->GetArray("Node_Numbers"));
                for (i=0; i<UGrid->GetNumberOfPoints(); i++) {
                        nodearray->SetValue(i, i+this->StartingNodeNumber);
                }
        }
        else
        {
                vtkIntArray *nodearray = vtkIntArray::New();
                nodearray->SetNumberOfValues(UGrid->GetNumberOfPoints());

                for (i=0; i<UGrid->GetNumberOfPoints(); i++) {
                        nodearray->SetValue(i, i+this->StartingNodeNumber);
                }
                nodearray->SetName("Node_Numbers");
                UGrid->GetPointData()->AddArray(nodearray);
                nodearray->Delete();

                nodearray = vtkIntArray::New();
                nodearray->SetNumberOfValues(UGrid->GetNumberOfPoints());
                // for the element set
                for (i=0; i<UGrid->GetNumberOfPoints(); i++) {
                        nodearray->SetValue(i, 1);
                }
                nodearray->SetName(NodeSetName);
                UGrid->GetPointData()->AddArray(nodearray);
                nodearray->Delete();
        }
}
//-------------------------------------------------------------------------------------
void vtkMimxApplyNodeElementNumbers::SetElementSetName(const char *ElSetName)
{
        this->ElementSetName = ElSetName;
}
//--------------------------------------------------------------------------------------
void vtkMimxApplyNodeElementNumbers::SetNodeSetName(const char *NSetName)
{
        this->NodeSetName = NSetName;
}
//--------------------------------------------------------------------------------------
