/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxBoundingBoxToStructuredGrids.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.7 $

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

#include "vtkMimxBoundingBoxToStructuredGrids.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCollection.h"
#include "vtkExecutive.h"
#include "vtkHexahedron.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxBoundingBoxToStructuredGrids, "$Revision: 1.7 $");
vtkStandardNewMacro(vtkMimxBoundingBoxToStructuredGrids);

// Construct object to extract all of the input data.
vtkMimxBoundingBoxToStructuredGrids::vtkMimxBoundingBoxToStructuredGrids()
{
        this->SetNumberOfOutputPorts(0);
        this->StructuredGrid = vtkCollection::New();
}

vtkMimxBoundingBoxToStructuredGrids::~vtkMimxBoundingBoxToStructuredGrids()
{
        for (int i = 0; i < this->StructuredGrid->GetNumberOfItems(); i++)
        {
                ((vtkStructuredGrid*)(this->StructuredGrid->GetItemAsObject(i)))->Delete();
        }
        this->StructuredGrid->Delete();
}


int vtkMimxBoundingBoxToStructuredGrids::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
 
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  

 // vtkCellData *cd=input->GetCellData();

  int numPts = input->GetNumberOfPoints();
  int numCells = input->GetNumberOfCells();
        
  if(numPts <=0 || numCells <= 0){vtkErrorMacro("Invalid Data")};
  
  // check if mesh seed data exists
  if(!input->GetCellData()->GetArray("Mesh_Seed"))
  {
          vtkErrorMacro("Mesh seed data needed");
          return 0;
  }

//        need to find out how to write a filter with one input and multiple outputs
//  creation of the output is a temporary arrangement
  vtkIdType npts;
  vtkIdType *pts;
  // travese through all the cells
  input->GetCells()->InitTraversal();
  // traverse through all the structured hexahedral mesh
  // whose nodes to be computed
  vtkHexahedron* hexahedron = vtkHexahedron::New();

  //if(this->MeshSeed->GetNumberOfComponents() != 3)
  //{
         // vtkErrorMacro("Number of components in the mesh seed should be 3");
         // return 0;
  //}

 /* if( input->GetCellData()->GetArray("Mesh_Seed")->GetNumber != numCells)
  {
        vtkErrorMacro(" Size of the mesh seed array does not match the number of cells in the input");
        return 0;
  }*/
  int dimtemp[3], dim[3];
  double shape[3];

        for(int i=0; i <numCells; i++)
        {
                this->StructuredGrid->AddItem((vtkObject*) vtkStructuredGrid::New());
                vtkIntArray::SafeDownCast(input->GetCellData()->GetArray("Mesh_Seed"))->GetTupleValue(i, dimtemp);
                dim[0] = dimtemp[2];
                dim[1] = dimtemp[0];
                dim[2] = dimtemp[1];
                
                ((vtkStructuredGrid*)(this->StructuredGrid->GetItemAsObject(i)))->SetDimensions(dim);

                        input->GetCells()->GetNextCell(npts, pts);
                        if(npts != 8){ vtkErrorMacro("Input should contain only Hexahedron cell");
                                                        return 0;}
                        for(int j=0; j < npts; j++)
                                hexahedron->GetPoints()->SetPoint(j,input->GetPoint(pts[j]));

                        // to calculate the parametric coordinates;
                        for(int j=0; j < 3; j++)        shape[j] = 1.0/(static_cast<double>(dim[j]-1));
                        // compute the points for the structured grid corresponding to given cell
                        vtkPoints* points = vtkPoints::New();
                        points->SetNumberOfPoints(dim[0]*dim[1]*dim[2]);
                        for(int y=0; y <dim[2]; y++)
                        {
                                for(int x=0; x<dim[1]; x++)
                                {
                                        for(int z=0; z < dim[0]; z++)
                                        {
                                                double shape_loc[3];
                                                shape_loc[0] = z*shape[0];
                                                shape_loc[1] = x*shape[1];
                                                shape_loc[2] = y*shape[2];
                                                int subid; double interp_loc[3], weights[8];
                                                // use isoparametric shape functions to evaluate
                                                // physical coordinates
                                                hexahedron->EvaluateLocation(subid,shape_loc,interp_loc,weights);
                                                points->SetPoint(y*dim[0]*dim[1]+x*dim[0]+z,interp_loc);
                                        }
                                }
                        }
                        ((vtkStructuredGrid*)(this->StructuredGrid->GetItemAsObject(i)))->SetPoints(points);
                        // scalar pointdata, mainly used to store whether a point should be moved
                        // during smoothing.
                        // 1 indicates point can be moved during smoothing and 0 otherwise
                        //vtkIntArray *intarray = vtkIntArray::New();
                        //intarray->SetNumberOfComponents(1);
                        //intarray->SetNumberOfValues(dim[0]*dim[1]*dim[2]);
                        //int status[1];
                        //status[0] = 1;
                        //for(int m=0; m < dim[0]*dim[1]*dim[2]; m++)        intarray->SetTupleValue(m,status);

                        //((vtkStructuredGrid*)(this->StructuredGrid->GetItemAsObject(i)))->GetPointData()->SetScalars(intarray);
                        //intarray->Delete();
                        points->Delete();
                }

  
  return 1;
}

int vtkMimxBoundingBoxToStructuredGrids::FillInputPortInformation(int port,
                                                                                                                          vtkInformation *info)
{
        if(port == 0)
        {
                info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
                
                return 1;
        }
        else
        {
                vtkErrorMacro("Only one input is allowed");
        }
        return 0;
}

vtkStructuredGrid * vtkMimxBoundingBoxToStructuredGrids::GetStructuredGrid(vtkIdType GridNum)
{
        if( GridNum > this->StructuredGrid->GetNumberOfItems()-1 || GridNum < 0)
        {
                vtkErrorMacro("Invalid Structured Grid Number");
                return NULL;
        }

        return ((vtkStructuredGrid*)(this->StructuredGrid->GetItemAsObject(GridNum)));
}

void vtkMimxBoundingBoxToStructuredGrids::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
