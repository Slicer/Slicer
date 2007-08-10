/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxButterflyMeshFromBounds.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
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

#include "vtkMimxButterflyMeshFromBounds.h"

#include "vtkActor.h"
#include "vtkDataSetMapper.h"
#include "vtkHexahedron.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxButterflyMeshFromBounds, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkMimxButterflyMeshFromBounds);

//----------------------------------------------------------------------------

vtkMimxButterflyMeshFromBounds::vtkMimxButterflyMeshFromBounds()
{
        // default bounds
        this->Bounds[0] = 0; this->Bounds[1] = 1; 
        this->Bounds[2] = 0; this->Bounds[3] = 1; 
        this->Bounds[4] = 0; this->Bounds[5] = 1; 
        this->SetNumberOfInputPorts(0);
        this->NumberOfLayers = 1;
}

vtkMimxButterflyMeshFromBounds::~vtkMimxButterflyMeshFromBounds()
{
}

//----------------------------------------------------------------------------
int vtkMimxButterflyMeshFromBounds::RequestData(
        vtkInformation *vtkNotUsed(request),
        vtkInformationVector **inputVector,
        vtkInformationVector *outputVector)
{
        // get the info objects
        vtkInformation *outInfo = outputVector->GetInformationObject(0);

        vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
                outInfo->Get(vtkDataObject::DATA_OBJECT()));

        vtkPoints* points = vtkPoints::New();
        if(NumberOfLayers == 2)
        {
                points->SetNumberOfPoints(24);
                double delta[3];
                delta[0] = this->Bounds[1] - this->Bounds[0];
                delta[1] = this->Bounds[3] - this->Bounds[2];
                delta[2] = this->Bounds[5] - this->Bounds[4];
                // set all the 24 points for a two layer butterfly mesh
                points->SetPoint(0, this->Bounds[0], this->Bounds[2], this->Bounds[5]);
                points->SetPoint(1, this->Bounds[1], this->Bounds[2], this->Bounds[5]);
                points->SetPoint(2, this->Bounds[1], this->Bounds[2], this->Bounds[4]);
                points->SetPoint(3, this->Bounds[0], this->Bounds[2], this->Bounds[4]);
                points->SetPoint(4, this->Bounds[0]+delta[0]/6.0 , 
                        this->Bounds[2], this->Bounds[5]-delta[2]/6.0);
                points->SetPoint(5, this->Bounds[1]-delta[0]/6.0, 
                        this->Bounds[2], this->Bounds[5]-delta[2]/6.0);
                points->SetPoint(6, this->Bounds[1]-delta[0]/6.0, 
                        this->Bounds[2], this->Bounds[4]+delta[2]/6.0);
                points->SetPoint(7, this->Bounds[0]+delta[0]/6.0, 
                        this->Bounds[2], this->Bounds[4]+delta[2]/6.0);

                points->SetPoint(8, this->Bounds[0]+delta[0]/3.0 , 
                        this->Bounds[2], this->Bounds[5]-delta[2]/3.0);
                points->SetPoint(9, this->Bounds[1]-delta[0]/3.0, 
                        this->Bounds[2], this->Bounds[5]-delta[2]/3.0);
                points->SetPoint(10, this->Bounds[1]-delta[0]/3.0, 
                        this->Bounds[2], this->Bounds[4]+delta[2]/3.0);
                points->SetPoint(11, this->Bounds[0]+delta[0]/3.0, 
                        this->Bounds[2], this->Bounds[4]+delta[2]/3.0);

                points->SetPoint(12, this->Bounds[0], this->Bounds[3], this->Bounds[5]);
                points->SetPoint(13, this->Bounds[1], this->Bounds[3], this->Bounds[5]);
                points->SetPoint(14, this->Bounds[1], this->Bounds[3], this->Bounds[4]);
                points->SetPoint(15, this->Bounds[0], this->Bounds[3], this->Bounds[4]);
                points->SetPoint(16, this->Bounds[0]+delta[0]/6.0 , 
                        this->Bounds[3], this->Bounds[5]-delta[2]/6.0);
                points->SetPoint(17, this->Bounds[1]-delta[0]/6.0, 
                        this->Bounds[3], this->Bounds[5]-delta[2]/6.0);
                points->SetPoint(18, this->Bounds[1]-delta[0]/6.0, 
                        this->Bounds[3], this->Bounds[4]+delta[2]/6.0);
                points->SetPoint(19, this->Bounds[0]+delta[0]/6.0, 
                        this->Bounds[3], this->Bounds[4]+delta[2]/6.0);

                points->SetPoint(20, this->Bounds[0]+delta[0]/3.0 , 
                        this->Bounds[3], this->Bounds[5]-delta[2]/3.0);
                points->SetPoint(21, this->Bounds[1]-delta[0]/3.0, 
                        this->Bounds[3], this->Bounds[5]-delta[2]/3.0);
                points->SetPoint(22, this->Bounds[1]-delta[0]/3.0, 
                        this->Bounds[3], this->Bounds[4]+delta[2]/3.0);
                points->SetPoint(23, this->Bounds[0]+delta[0]/3.0, 
                        this->Bounds[3], this->Bounds[4]+delta[2]/3.0);

                //        set element connectivity
                output->Allocate(9,9);

                vtkIdList *idlist = vtkIdList::New();
                idlist->SetNumberOfIds(8);
                idlist->SetId(0,4);        idlist->SetId(1,0);        idlist->SetId(2,1);        idlist->SetId(3,5);        
                idlist->SetId(4,16);        idlist->SetId(5,12);        idlist->SetId(6,13);        idlist->SetId(7,17);                        
                output->InsertNextCell(12, idlist);
                idlist->Initialize();                        idlist->SetNumberOfIds(8);

                idlist->SetId(0,5);        idlist->SetId(1,1);        idlist->SetId(2,2);        idlist->SetId(3,6);        
                idlist->SetId(4,17);        idlist->SetId(5,13);        idlist->SetId(6,14);        idlist->SetId(7,18);                        
                output->InsertNextCell(12, idlist);
                idlist->Initialize();                        idlist->SetNumberOfIds(8);

                idlist->SetId(0,6);        idlist->SetId(1,2);        idlist->SetId(2,3);        idlist->SetId(3,7);        
                idlist->SetId(4,18);        idlist->SetId(5,14);        idlist->SetId(6,15);        idlist->SetId(7,19);                        
                output->InsertNextCell(12, idlist);
                idlist->Initialize();                        idlist->SetNumberOfIds(8);

                idlist->SetId(0,7);        idlist->SetId(1,3);        idlist->SetId(2,0);        idlist->SetId(3,4);        
                idlist->SetId(4,19);        idlist->SetId(5,15);        idlist->SetId(6,12);        idlist->SetId(7,16);                        
                output->InsertNextCell(12, idlist);
                idlist->Initialize();                        idlist->SetNumberOfIds(8);

                idlist->SetId(0,8);        idlist->SetId(1,4);        idlist->SetId(2,5);        idlist->SetId(3,9);        
                idlist->SetId(4,20);        idlist->SetId(5,16);        idlist->SetId(6,17);        idlist->SetId(7,21);                        
                output->InsertNextCell(12, idlist);
                idlist->Initialize();                        idlist->SetNumberOfIds(8);

                idlist->SetId(0,9);        idlist->SetId(1,5);        idlist->SetId(2,6);        idlist->SetId(3,10);        
                idlist->SetId(4,21);        idlist->SetId(5,17);        idlist->SetId(6,18);        idlist->SetId(7,22);                        
                output->InsertNextCell(12, idlist);
                idlist->Initialize();                        idlist->SetNumberOfIds(8);

                idlist->SetId(0,10);        idlist->SetId(1,6);        idlist->SetId(2,7);        idlist->SetId(3,11);        
                idlist->SetId(4,22);        idlist->SetId(5,18);        idlist->SetId(6,19);        idlist->SetId(7,23);                        
                output->InsertNextCell(12, idlist);
                idlist->Initialize();                        idlist->SetNumberOfIds(8);

                idlist->SetId(0,11);        idlist->SetId(1,7);        idlist->SetId(2,4);        idlist->SetId(3,8);        
                idlist->SetId(4,23);        idlist->SetId(5,19);        idlist->SetId(6,16);        idlist->SetId(7,20);                        
                output->InsertNextCell(12, idlist);
                idlist->Initialize();                        idlist->SetNumberOfIds(8);

                idlist->SetId(0,11);        idlist->SetId(1,8);        idlist->SetId(2,9);        idlist->SetId(3,10);        
                idlist->SetId(4,23);        idlist->SetId(5,20);        idlist->SetId(6,21);        idlist->SetId(7,22);                        
                output->InsertNextCell(12, idlist);

                idlist->Delete();        
                        }

        output->SetPoints(points);

        points->Delete();
    return 1;
}

//----------------------------------------------------------------------------
void vtkMimxButterflyMeshFromBounds::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMimxButterflyMeshFromBounds::SetBounds(double bounds[6])
{
        for(int i=0; i<6; i++)        { this->Bounds[i] = bounds[i];
        }
}
