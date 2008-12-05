/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxApplyNodeElementNumbers.cxx,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.3 $

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

#include "ApplyNodeElementNumbers.h"

#include "vtkCellData.h"
#include "vtkIntArray.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"

ApplyNodeElementNumbers::ApplyNodeElementNumbers()
{
        this->UGrid = NULL;
        this->StartingElementNumber = 1;
        this->StartingNodeNumber = 1;
        this->ElementSetName = NULL;
}

ApplyNodeElementNumbers::~ApplyNodeElementNumbers()
{
}

void ApplyNodeElementNumbers::SetUnstructuredGrid(vtkUnstructuredGrid *UnsGrid)
{
        this->UGrid = UnsGrid;
}

void ApplyNodeElementNumbers::SetStartingElementNumber(int EleNum)
{
        this->StartingElementNumber = EleNum;
}

void ApplyNodeElementNumbers::SetStartingNodeNumber(int NodeNum)
{
        this->StartingNodeNumber = NodeNum;
}

void ApplyNodeElementNumbers::ApplyElementNumbers()
{
        // if an array already present
        int i;
        if(UGrid->GetCellData()->GetArray("Element_Numbers"))
        {
                vtkIntArray *elementarray = 
                        vtkIntArray::SafeDownCast(UGrid->GetCellData()->GetArray("Element_Numbers"));
                for (i=0; i<UGrid->GetNumberOfCells(); i++) {
                        elementarray->SetValue(i, i+this->StartingElementNumber);
                }
        }
        else
        {
                vtkIntArray *elementarray = vtkIntArray::New();
                elementarray->SetNumberOfValues(UGrid->GetNumberOfCells());

                for (i=0; i<UGrid->GetNumberOfCells(); i++) {
                        elementarray->SetValue(i, i+this->StartingElementNumber);
                }
                elementarray->SetName("Element_Numbers");
                UGrid->GetCellData()->AddArray(elementarray);
                elementarray->Delete();
        }
        if(!strcmp(this->ElementSetName, ""))
        {
                this->ElementSetName = "Element_Set_";
        }
                if(UGrid->GetCellData()->GetArray(ElementSetName))
                {
                        vtkIntArray *elementarray = 
                                vtkIntArray::SafeDownCast(UGrid->GetCellData()->GetArray(ElementSetName));
                        for (i=0; i<UGrid->GetNumberOfCells(); i++) {
                                elementarray->SetValue(i, 1);
                        }
                }
                else
                {
                        vtkIntArray *elementarray = vtkIntArray::New();
                        elementarray->SetNumberOfValues(UGrid->GetNumberOfCells());

                        for (i=0; i<UGrid->GetNumberOfCells(); i++) {
                                elementarray->SetValue(i, 1);
                        }
                        elementarray->SetName(ElementSetName);
                        UGrid->GetCellData()->AddArray(elementarray);
                        elementarray->Delete();
                }

}

void ApplyNodeElementNumbers::ApplyNodeNumbers()
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
        }
}

void ApplyNodeElementNumbers::SetElementSetName(const char *ElSetName)
{
        this->ElementSetName = ElSetName;
}
