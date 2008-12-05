/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSubdivideBoundingBox.cxx,v $
Language:  C++
Date:      $Date: 2007/12/01 02:44:59 $
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

#include "vtkMimxSubdivideBoundingBox.h"

#include "vtkCell.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointLocator.h"
#include "vtkPoints.h"
#include "vtkTriangle.h"
#include "vtkUnstructuredGrid.h"
#include "vtkHexahedron.h"

vtkCxxRevisionMacro(vtkMimxSubdivideBoundingbox, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkMimxSubdivideBoundingbox);

// Description:

vtkMimxSubdivideBoundingbox::vtkMimxSubdivideBoundingbox()
{
        this->CellNum = -1;
}

vtkMimxSubdivideBoundingbox::~vtkMimxSubdivideBoundingbox()
{
}

int vtkMimxSubdivideBoundingbox::RequestData(
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
 
  if(this->CellNum < 0 || this->CellNum >= input->GetNumberOfCells())
  {
          vtkErrorMacro("Cell number to be deleted incorrect");
          return 0;
  }

  vtkDebugMacro(<< "Check if the face points input are valid ?");
  vtkIdType numCells = input->GetNumberOfCells();
  vtkIdType numPts = input->GetNumberOfPoints();
  if(numCells < 1 || numPts <8)
  {
          vtkErrorMacro("Invalid input data");
          return 0;
  }
  
  // storage of output
  output->Allocate(input->GetNumberOfCells()-1,input->GetNumberOfCells()-1);
  vtkPoints *points = vtkPoints::New();
  points->DeepCopy(input->GetPoints());
  vtkIdList *idlist;
  int i, j, k;
  double x[3];
//  vtkIdType tempid;
  for(i=0; i <input->GetNumberOfCells(); i++)
  {
          if(i != this->CellNum)
          {
                  idlist = vtkIdList::New();
                  idlist->DeepCopy(input->GetCell(i)->GetPointIds());
                  output->InsertNextCell(12, idlist);
                  idlist->Delete();
          }
  }




  // go to the cell number to be subdivided
  double ScaleFactor = 0.5;

  idlist = input->GetCell(this->CellNum)->GetPointIds();
  
          if(idlist->GetNumberOfIds() != 8){ vtkErrorMacro("Input should contain only Hexahedron cell");
          return 0;}
          vtkHexahedron* hexahedron = vtkHexahedron::New();

          double center[3];     center[0] = 0.0; center[1] = 0.0; center[2] = 0.0;

          for(j=0; j < 8; j++)
          {
                  input->GetPoint(idlist->GetId(j),x);
                  for (k=0; k <3; k++)
                  {
                          center[k]+=x[k];
                  }
                  hexahedron->GetPoints()->SetPoint(j,input->GetPoint(idlist->GetId(j)));
          }

          for (k=0; k<3; k++)   center[k]=center[k]/8.0;

          // small hexahedron is shrunk based on user input shrink factor
          for (j=0; j <8; j++)
          {
                  hexahedron->GetPoints()->GetPoint(j,x);
                  double temp[3];
                  for (k=0; k<3; k++)   
                  {temp[k] = x[k] - center[k];
                  temp[k]*=ScaleFactor;
                  temp[k]+=center[k];
                  }
                  points->InsertNextPoint(temp);
          }
          hexahedron->Delete(); 
          vtkIdList *largelist = input->GetCell(this->CellNum)->GetPointIds();
          vtkIdList *smalllist = vtkIdList::New();
          smalllist->SetNumberOfIds(8);
          for (i=0; i<8;i++)
          {
                  smalllist->SetId(i, input->GetNumberOfPoints()+i);
          }
          // crete 7 sub-divisions;
          vtkIdList *hypecubeidlist;
          hypecubeidlist = vtkIdList::New();


          // cube 1;
          // set the points for hexahedron belonging to hypercubes
          hypecubeidlist->SetNumberOfIds(8);
          hypecubeidlist->SetId(0, largelist->GetId(0));
          hypecubeidlist->SetId(1, largelist->GetId(1));
          hypecubeidlist->SetId(2, smalllist->GetId(1));
          hypecubeidlist->SetId(3, smalllist->GetId(0));
          hypecubeidlist->SetId(4, largelist->GetId(4));
          hypecubeidlist->SetId(5, largelist->GetId(5));
          hypecubeidlist->SetId(6, smalllist->GetId(5));
          hypecubeidlist->SetId(7, smalllist->GetId(4));
          output->InsertNextCell(12,hypecubeidlist);
          hypecubeidlist->Initialize();



          // cube 2
          // set the points for hexahedron belonging to hypercubes
                hypecubeidlist->SetNumberOfIds(8);
                hypecubeidlist->SetId(0, smalllist->GetId(3));
                hypecubeidlist->SetId(1, smalllist->GetId(2));
                hypecubeidlist->SetId(2, largelist->GetId(2));
                hypecubeidlist->SetId(3, largelist->GetId(3));
                hypecubeidlist->SetId(4, smalllist->GetId(7));
                hypecubeidlist->SetId(5, smalllist->GetId(6));
                hypecubeidlist->SetId(6, largelist->GetId(6));
                hypecubeidlist->SetId(7, largelist->GetId(7));
                output->InsertNextCell(12,hypecubeidlist);
                hypecubeidlist->Initialize();


          // cube 3
          
                // set the points for hexahedron belonging to hypercubes
                hypecubeidlist->SetNumberOfIds(8);
                hypecubeidlist->SetId(0, largelist->GetId(0));
                hypecubeidlist->SetId(1, smalllist->GetId(0));
                hypecubeidlist->SetId(2, smalllist->GetId(3));
                hypecubeidlist->SetId(3, largelist->GetId(3));
                hypecubeidlist->SetId(4, largelist->GetId(4));
                hypecubeidlist->SetId(5, smalllist->GetId(4));
                hypecubeidlist->SetId(6, smalllist->GetId(7));
                hypecubeidlist->SetId(7, largelist->GetId(7));
                output->InsertNextCell(12,hypecubeidlist);
                hypecubeidlist->Initialize();


          
          // cube 4
          
                // set the points for hexahedron belonging to hypercubes
                hypecubeidlist->SetNumberOfIds(8);
                hypecubeidlist->SetId(0, smalllist->GetId(1));
                hypecubeidlist->SetId(1, largelist->GetId(1));
                hypecubeidlist->SetId(2, largelist->GetId(2));
                hypecubeidlist->SetId(3, smalllist->GetId(2));
                hypecubeidlist->SetId(4, smalllist->GetId(5));
                hypecubeidlist->SetId(5, largelist->GetId(5));
                hypecubeidlist->SetId(6, largelist->GetId(6));
                hypecubeidlist->SetId(7, smalllist->GetId(6));
                output->InsertNextCell(12,hypecubeidlist);
                hypecubeidlist->Initialize();


          

          
          // cube 5
          
                hypecubeidlist->SetNumberOfIds(8);
                hypecubeidlist->SetId(0, largelist->GetId(0));
                hypecubeidlist->SetId(1, largelist->GetId(1));
                hypecubeidlist->SetId(2, largelist->GetId(2));
                hypecubeidlist->SetId(3, largelist->GetId(3));
                hypecubeidlist->SetId(4, smalllist->GetId(0));
                hypecubeidlist->SetId(5, smalllist->GetId(1));
                hypecubeidlist->SetId(6, smalllist->GetId(2));
                hypecubeidlist->SetId(7, smalllist->GetId(3));
                output->InsertNextCell(12,hypecubeidlist);
                hypecubeidlist->Initialize();
          

          
          // cube 6

          
                hypecubeidlist->SetNumberOfIds(8);
                hypecubeidlist->SetId(0, smalllist->GetId(4));
                hypecubeidlist->SetId(1, smalllist->GetId(5));
                hypecubeidlist->SetId(2, smalllist->GetId(6));
                hypecubeidlist->SetId(3, smalllist->GetId(7));
                hypecubeidlist->SetId(4, largelist->GetId(4));
                hypecubeidlist->SetId(5, largelist->GetId(5));
                hypecubeidlist->SetId(6, largelist->GetId(6));
                hypecubeidlist->SetId(7, largelist->GetId(7));
                output->InsertNextCell(12,hypecubeidlist);
                hypecubeidlist->Initialize();

          

          

          // cube 7

                hypecubeidlist->SetNumberOfIds(8);
                hypecubeidlist->SetId(0, smalllist->GetId(0));
                hypecubeidlist->SetId(1, smalllist->GetId(1));
                hypecubeidlist->SetId(2, smalllist->GetId(2));
                hypecubeidlist->SetId(3, smalllist->GetId(3));
                hypecubeidlist->SetId(4, smalllist->GetId(4));
                hypecubeidlist->SetId(5, smalllist->GetId(5));
                hypecubeidlist->SetId(6, smalllist->GetId(6));
                hypecubeidlist->SetId(7, smalllist->GetId(7));
                output->InsertNextCell(12,hypecubeidlist);
                hypecubeidlist->Delete();
                smalllist->Delete();


            



  points->Squeeze();
  output->SetPoints(points);
  output->Squeeze();
  points->Delete();
 return 1;

}

void vtkMimxSubdivideBoundingbox::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
