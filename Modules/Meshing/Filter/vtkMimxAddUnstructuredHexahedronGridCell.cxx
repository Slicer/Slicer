/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxAddUnstructuredHexahedronGridCell.cxx,v $
Language:  C++
Date:      $Date: 2008/03/16 20:50:55 $
Version:   $Revision: 1.8 $

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

#include "vtkMimxAddUnstructuredHexahedronGridCell.h"

#include "vtkCell.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkTriangle.h"
#include "vtkUnstructuredGrid.h"
#include "vtkIntArray.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkCellData.h"

vtkCxxRevisionMacro(vtkMimxAddUnstructuredHexahedronGridCell, "$Revision: 1.8 $");
vtkStandardNewMacro(vtkMimxAddUnstructuredHexahedronGridCell);

// Description:

vtkMimxAddUnstructuredHexahedronGridCell::vtkMimxAddUnstructuredHexahedronGridCell()
{
        this->IdList = NULL;
        this->ExtrusionLength = 1.0;
        this->AverageElementLength = 1.0;
}

vtkMimxAddUnstructuredHexahedronGridCell::~vtkMimxAddUnstructuredHexahedronGridCell()
{
}

int vtkMimxAddUnstructuredHexahedronGridCell::RequestData(
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
          vtkErrorMacro("Enter the point numbers of the face to be extruded,  face list should contain 4 points");
          return 0;
  }
  if(IdList->GetNumberOfIds() !=4)
  {
          vtkErrorMacro("Face should contain only four points");
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
// Initialize and check input
//
  if(this->ExtrusionLength <= 0.0)
  {
          vtkErrorMacro("Extrusion Length should always be > 0");
          return 0;
  }
  int i, facenum;
  bool status = false;
        int count_ele = 0;
  do {
        for(i=0; i < 6; i++)
        {
                vtkCell *cell = input->GetCell(count_ele)->GetFace(i);
                if(cell->GetPointId(0) == IdList->GetId(0) || cell->GetPointId(1) == IdList->GetId(0) || cell->GetPointId(2) == IdList->GetId(0) || cell->GetPointId(3) == IdList->GetId(0))
                {
                        if(cell->GetPointId(0) == IdList->GetId(1) || cell->GetPointId(1) == IdList->GetId(1) || cell->GetPointId(2) == IdList->GetId(1) || cell->GetPointId(3) == IdList->GetId(1))
                        {
                                if(cell->GetPointId(0) == IdList->GetId(2) || cell->GetPointId(1) == IdList->GetId(2) || cell->GetPointId(2) == IdList->GetId(2) || cell->GetPointId(3) == IdList->GetId(2))
                                {
                                        if(cell->GetPointId(0) == IdList->GetId(3) || cell->GetPointId(1) == IdList->GetId(3) || cell->GetPointId(2) == IdList->GetId(3) || cell->GetPointId(3) == IdList->GetId(3))
                                        {
                                status = true;
                                break;
                                        }
                                }
                        }
                }
        }
        count_ele++;
  } while(count_ele < input->GetNumberOfCells() && !status);

  if(!status)
  {
          vtkErrorMacro("Invalid face points entered, check for correct face points input");
          return 0;
  }
  // if the input is valid
   facenum = i;
  //    get points based on the plane being divided
  //    lines accessed are in CW
  // generate the connectivity for the newly extruded cell
  
  // storage of output
  output->Allocate(input->GetNumberOfCells()+1,input->GetNumberOfCells()+1);
  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(0);
  for (int i=0; i < input->GetNumberOfPoints(); i++)
  {
          points->InsertNextPoint(input->GetPoint(i));
  }
  output->SetPoints(points);
  points->Delete();

  for(int i=0; i <input->GetNumberOfCells(); i++)
  {
          vtkIdList *locallist = vtkIdList::New();
          locallist->DeepCopy(input->GetCell(i)->GetPointIds());
          output->InsertNextCell(input->GetCellType(i), locallist);
          locallist->Delete();
  }
  //    calculate the direction of extrusion
  //    calculate the normals at all the 4 points
  // and average them
  double pt1[3], pt2[3], pt3[3], pt4[3];
  double normal[3], normal1[3], normal2[3], normal3[3], normal4[3];
  output->GetPoints()->GetPoint(output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(0),pt1);
  output->GetPoints()->GetPoint(output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(1),pt2);
  output->GetPoints()->GetPoint(output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(2),pt3);
  output->GetPoints()->GetPoint(output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(3),pt4);
  vtkTriangle::ComputeNormal(pt1, pt2, pt3, normal1);
  vtkTriangle::ComputeNormal(pt2, pt3, pt4, normal2);
  vtkTriangle::ComputeNormal(pt3, pt4, pt1, normal3);
  vtkTriangle::ComputeNormal(pt4, pt1, pt2, normal4);
  normal[0] = 0.0; normal[1] = 0.0; normal[2] = 0.0;
  normal[0] = normal1[0] + normal2[0] + normal3[0] + normal4[0];
  normal[1] = normal1[1] + normal2[1] + normal3[1] + normal4[1];
  normal[2] = normal1[2] + normal2[2] + normal3[2] + normal4[2];
  for(int i=0; i<3; i++)        normal[i] = normal[i]/4.0;

  output->GetPoints()->InsertNextPoint(pt1[0] + this->ExtrusionLength*normal[0],
          pt1[1] + this->ExtrusionLength*normal[1], pt1[2] + this->ExtrusionLength*normal[2]);
  output->GetPoints()->InsertNextPoint(pt2[0] + this->ExtrusionLength*normal[0],
          pt2[1] + this->ExtrusionLength*normal[1], pt2[2] + this->ExtrusionLength*normal[2]);
  output->GetPoints()->InsertNextPoint(pt3[0] + this->ExtrusionLength*normal[0],
          pt3[1] + this->ExtrusionLength*normal[1], pt3[2] + this->ExtrusionLength*normal[2]);
  output->GetPoints()->InsertNextPoint(pt4[0] + this->ExtrusionLength*normal[0],
          pt4[1] + this->ExtrusionLength*normal[1], pt4[2] + this->ExtrusionLength*normal[2]);

  vtkIdList *idlistnew = vtkIdList::New();
  idlistnew->SetNumberOfIds(8);
  // copy the meshseed values for the newly created mesh
  vtkIntArray *meshseedarray = NULL;
  vtkIntArray *inputmeshseed;
  if(input->GetCellData()->GetArray("Mesh_Seed"))
  {
                meshseedarray = vtkIntArray::New();
                meshseedarray->SetNumberOfComponents(3);
                meshseedarray->SetNumberOfTuples(input->GetNumberOfCells()+1);
                meshseedarray->SetName("Mesh_Seed");
                inputmeshseed = vtkIntArray::SafeDownCast(
                        input->GetCellData()->GetArray("Mesh_Seed"));
                for (int i=0; i<input->GetNumberOfCells(); i++)
                {
                        int dim[3];
                        inputmeshseed->GetTupleValue(i, dim);
                        meshseedarray->SetTupleValue(i,dim);
                }
  }
  //

  // numbering order depends on the face chosen 
  if(facenum == 0)
  {
          idlistnew->SetId(1, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(0));
          idlistnew->SetId(5,output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(1));
          idlistnew->SetId(6, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(2));
          idlistnew->SetId(2, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(3));
          idlistnew->SetId(0, output->GetNumberOfPoints()-4);
          idlistnew->SetId(4, output->GetNumberOfPoints()-3);
          idlistnew->SetId(7, output->GetNumberOfPoints()-2);
          idlistnew->SetId(3, output->GetNumberOfPoints()-1);
          output->InsertNextCell(12,idlistnew);
          output->Squeeze();
          idlistnew->Delete();
          this->CopyConstraintValues(input, output);
          if(meshseedarray)
          {
                  int dim[3];
                  inputmeshseed->GetTupleValue(count_ele-1, dim);
                  int dimmod[3];
                  dimmod[0] = dim[0]; dimmod[1] = dim[1]; 
                  dimmod[2] = static_cast<int>(this->ExtrusionLength/this->AverageElementLength);
                  meshseedarray->SetTupleValue(input->GetNumberOfCells(), dimmod);
                  output->GetCellData()->AddArray(meshseedarray);
                  meshseedarray->Delete();
          }
          return 1;
  }

  if(facenum == 1)
  {
          idlistnew->SetId(0, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(0));
          idlistnew->SetId(3, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(1));
          idlistnew->SetId(7, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(2));
          idlistnew->SetId(4, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(3));
          idlistnew->SetId(1, output->GetNumberOfPoints()-4);
          idlistnew->SetId(2, output->GetNumberOfPoints()-3);
          idlistnew->SetId(6, output->GetNumberOfPoints()-2);
          idlistnew->SetId(5, output->GetNumberOfPoints()-1);
          output->InsertNextCell(12,idlistnew);
          output->Squeeze();
          idlistnew->Delete();
          this->CopyConstraintValues(input, output);
          if(meshseedarray)
          {
                  int dim[3];
                  inputmeshseed->GetTupleValue(count_ele-1, dim);
                  int dimmod[3];
                  dimmod[0] = dim[0]; dimmod[1] = dim[1];
                  dimmod[2] = static_cast<int>(this->ExtrusionLength/this->AverageElementLength);
                  meshseedarray->SetTupleValue(input->GetNumberOfCells(), dimmod);
                  output->GetCellData()->AddArray(meshseedarray);
                  meshseedarray->Delete();
          }
          return 1;
  }
  if(facenum == 2)
  {
          idlistnew->SetId(3, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(0));
          idlistnew->SetId(2, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(1));
          idlistnew->SetId(6, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(2));
          idlistnew->SetId(7, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(3));
          idlistnew->SetId(0, output->GetNumberOfPoints()-4);
          idlistnew->SetId(1, output->GetNumberOfPoints()-3);
          idlistnew->SetId(5, output->GetNumberOfPoints()-2);
          idlistnew->SetId(4, output->GetNumberOfPoints()-1);
          output->InsertNextCell(12,idlistnew);
          output->Squeeze();
          idlistnew->Delete();
          this->CopyConstraintValues(input, output);
          if(meshseedarray)
          {
                  int dim[3];
                  inputmeshseed->GetTupleValue(count_ele-1, dim);
                  int dimmod[3];
                  dimmod[1] = dim[1]; dimmod[2] = dim[2];
                  dimmod[0] = static_cast<int>(this->ExtrusionLength/this->AverageElementLength);
                  meshseedarray->SetTupleValue(input->GetNumberOfCells(), dimmod);
                  output->GetCellData()->AddArray(meshseedarray);
                  meshseedarray->Delete();
          }
          return 1;
  }

  if(facenum == 3)
  {
          idlistnew->SetId(0, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(0));
          idlistnew->SetId(4, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(1));
          idlistnew->SetId(5, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(2));
          idlistnew->SetId(1, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(3));
          idlistnew->SetId(3, output->GetNumberOfPoints()-4);
          idlistnew->SetId(7, output->GetNumberOfPoints()-3);
          idlistnew->SetId(6, output->GetNumberOfPoints()-2);
          idlistnew->SetId(2, output->GetNumberOfPoints()-1);
          output->InsertNextCell(12,idlistnew);
          output->Squeeze();
          idlistnew->Delete();
          this->CopyConstraintValues(input, output);
          if(meshseedarray)
          {
                  int dim[3];
                  inputmeshseed->GetTupleValue(count_ele-1, dim);
                  int dimmod[3];
                  dimmod[1] = dim[1]; dimmod[2] = dim[2];
                  dimmod[0] = static_cast<int>(this->ExtrusionLength/this->AverageElementLength);
                  meshseedarray->SetTupleValue(input->GetNumberOfCells(), dimmod);
                  output->GetCellData()->AddArray(meshseedarray);
                  meshseedarray->Delete();
          }
          return 1;
  }

  if(facenum == 4)
  {
          idlistnew->SetId(4, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(0));
          idlistnew->SetId(7, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(1));
          idlistnew->SetId(6, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(2));
          idlistnew->SetId(5, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(3));
          idlistnew->SetId(0, output->GetNumberOfPoints()-4);
          idlistnew->SetId(3, output->GetNumberOfPoints()-3);
          idlistnew->SetId(2, output->GetNumberOfPoints()-2);
          idlistnew->SetId(1, output->GetNumberOfPoints()-1);
          output->InsertNextCell(12,idlistnew);
          output->Squeeze();
          idlistnew->Delete();
          this->CopyConstraintValues(input, output);
          if(meshseedarray)
          {
                  int dim[3];
                  inputmeshseed->GetTupleValue(count_ele-1, dim);
                  int dimmod[3];
                  dimmod[0] = dim[0]; dimmod[2] = dim[2];
                  dimmod[1] = static_cast<int>(this->ExtrusionLength/this->AverageElementLength);
                  meshseedarray->SetTupleValue(input->GetNumberOfCells(), dimmod);
                  output->GetCellData()->AddArray(meshseedarray);
                  meshseedarray->Delete();
          }
          return 1;
  }
  if(facenum == 5)
  {
          idlistnew->SetId(0, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(0));
          idlistnew->SetId(1, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(1));
          idlistnew->SetId(2, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(2));
          idlistnew->SetId(3, output->GetCell(count_ele-1)->GetFace(facenum)->GetPointId(3));
          idlistnew->SetId(4, output->GetNumberOfPoints()-4);
          idlistnew->SetId(5, output->GetNumberOfPoints()-3);
          idlistnew->SetId(6, output->GetNumberOfPoints()-2);
          idlistnew->SetId(7, output->GetNumberOfPoints()-1);
          output->InsertNextCell(12,idlistnew);
          output->Squeeze();
          idlistnew->Delete();
          this->CopyConstraintValues(input, output);
          if(meshseedarray)
          {
                  int dim[3];
                  inputmeshseed->GetTupleValue(count_ele-1, dim);
                  int dimmod[3];
                  dimmod[0] = dim[0]; dimmod[2] = dim[2];
                  dimmod[0] = static_cast<int>(this->ExtrusionLength/this->AverageElementLength);
                  meshseedarray->SetTupleValue(input->GetNumberOfCells(), dimmod);
                  output->GetCellData()->AddArray(meshseedarray);
                  meshseedarray->Delete();
          }
          return 1;
  }
 return 0;
  
}
//------------------------------------------------------------------------------------
void vtkMimxAddUnstructuredHexahedronGridCell::CopyConstraintValues(
        vtkUnstructuredGrid *input, vtkUnstructuredGrid *output)
{
        vtkIntArray *constrain = vtkIntArray::SafeDownCast(
                input->GetPointData()->GetArray("Constrain"));
        if(constrain)
        {
                vtkPoints *inpoints = input->GetPoints();
                vtkPoints *outpoints = output->GetPoints();
                vtkPointLocator *locator = vtkPointLocator::New();

                int i;
                vtkPoints *points = vtkPoints::New();
                locator->InitPointInsertion(points, outpoints->GetBounds());
                for (i=0; i<inpoints->GetNumberOfPoints(); i++)
                {
                        locator->InsertNextPoint(inpoints->GetPoint(i));
                }
                                        
                vtkIntArray *constrainout = vtkIntArray::New();
                constrainout->SetNumberOfValues(outpoints->GetNumberOfPoints());
                for (i=0; i<outpoints->GetNumberOfPoints(); i++)
                {
                        vtkIdType pos;
                        pos = locator->IsInsertedPoint(outpoints->GetPoint(i));
                        if (pos != -1)
                        {
                                constrainout->SetValue(i, constrain->GetValue(pos));
                        }
                        else{
                                constrainout->SetValue(i, 0);
                        }
                }
                constrainout->SetName("Constrain");
                output->GetPointData()->AddArray(constrainout);
                constrainout->Delete();
                locator->Delete();
                points->Delete();
        }
}
//------------------------------------------------------------------------------------
void vtkMimxAddUnstructuredHexahedronGridCell::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
