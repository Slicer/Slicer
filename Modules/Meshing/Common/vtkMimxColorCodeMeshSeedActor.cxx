/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxColorCodeMeshSeedActor.cxx,v $
Language:  C++
Date:      $Date: 2008/07/06 20:30:13 $
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
#include "vtkMimxColorCodeMeshSeedActor.h"

#include "vtkActor.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkIntArray.h"
#include "vtkLookupTable.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPropCollection.h"
#include "vtkUnstructuredGrid.h"
#include "vtkScalarBarActor.h"
#include "vtkTextProperty.h"
#include "vtkProperty.h"


vtkCxxRevisionMacro(vtkMimxColorCodeMeshSeedActor, "$Revision: 1.6 $");
vtkStandardNewMacro(vtkMimxColorCodeMeshSeedActor);

vtkMimxColorCodeMeshSeedActor::vtkMimxColorCodeMeshSeedActor()
{
  this->UpdateProps();
  this->Input = NULL;
  this->MeshSeedActor = NULL;
  this->ScalarBarActor = NULL;
  this->LineWidth = 1; 
  TextColor[0] = TextColor[1] = TextColor[2] = 1.0;
}

vtkMimxColorCodeMeshSeedActor::~vtkMimxColorCodeMeshSeedActor()
{
  if (this->MeshSeedActor)
    this->MeshSeedActor->Delete();
  if(this->ScalarBarActor)
    this->ScalarBarActor->Delete();
}

// Shallow copy of an actor.
void vtkMimxColorCodeMeshSeedActor::ShallowCopy(vtkProp *prop)
{
  vtkMimxColorCodeMeshSeedActor *a = vtkMimxColorCodeMeshSeedActor::SafeDownCast(prop);
  if ( a != NULL )
    {
    }

  // Now do superclass
  this->vtkProp3D::ShallowCopy(prop);
}

void vtkMimxColorCodeMeshSeedActor::GetActors(vtkPropCollection *ac)
{
  ac->AddItem(this->MeshSeedActor);
}

int vtkMimxColorCodeMeshSeedActor::RenderOpaqueGeometry(vtkViewport *vp)
{
  this->UpdateProps();

  return 1;
}

//-----------------------------------------------------------------------------
int vtkMimxColorCodeMeshSeedActor::RenderTranslucentPolygonalGeometry(vtkViewport *vp)
{
  this->UpdateProps();
  return 1;
}

//-----------------------------------------------------------------------------
// Description:
// Does this prop have some translucent polygonal geometry?
int vtkMimxColorCodeMeshSeedActor::HasTranslucentPolygonalGeometry()
{
  this->UpdateProps();
  return 1;
}

//-----------------------------------------------------------------------------
void vtkMimxColorCodeMeshSeedActor::ReleaseGraphicsResources(vtkWindow *win)
{
}

void vtkMimxColorCodeMeshSeedActor::GetBounds(double bounds[6])
{
  this->GetBounds();
}

// Get the bounds for this Actor as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
double *vtkMimxColorCodeMeshSeedActor::GetBounds()
{
  return this->Input->GetBounds();
}

unsigned long int vtkMimxColorCodeMeshSeedActor::GetMTime()
{
  unsigned long mTime = this->Superclass::GetMTime();
  return mTime;
}

unsigned long int vtkMimxColorCodeMeshSeedActor::GetRedrawMTime()
{
  unsigned long mTime = this->GetMTime();
  return mTime;
}

void vtkMimxColorCodeMeshSeedActor::UpdateProps()
{
}
//-----------------------------------------------------------------------------
void vtkMimxColorCodeMeshSeedActor::SetInput(vtkUnstructuredGrid *MeshSeedInput)
{
  this->Input = MeshSeedInput;
  if(!this->Input->GetCellData()->GetArray("Mesh_Seed"))
    {
    vtkErrorMacro("Mesh seed data absent. Mesh seed data should be present");
    this->Input = NULL;
    return;
    }
  // calculate the smallest and largest mesh seed value
  int dim[3];
  vtkIntArray *MeshSeedValues = vtkIntArray::SafeDownCast(
    this->Input->GetCellData()->GetArray("Mesh_Seed"));
  this->MinMeshSeed = VTK_INT_MAX;
  this->MaxMeshSeed = VTK_INT_MIN;
  int i,j;
  for (i=0; i<this->Input->GetNumberOfCells(); i++)
    {
    MeshSeedValues->GetTupleValue(i, dim);
    for (j=0; j<3; j++)
      {
      if(dim[j] < this->MinMeshSeed)
        this->MinMeshSeed = dim[j];
      if(dim[j] > this->MaxMeshSeed)
        this->MaxMeshSeed = dim[j];                     
      }       
    }
  this->MinMeshSeed --;
  this->MaxMeshSeed --;
  // extract edges for color coding
  // the unstructured grid is converted to polydata with each edge 
  // stored as a cell in polydata
  vtkIdType* pts=0;
  vtkIdType t=0;
  this->Input->GetCells()->InitTraversal();
  vtkPoints* points = vtkPoints::New();
  points->SetNumberOfPoints(this->Input->GetNumberOfPoints());
  for(i=0; i <this->Input->GetNumberOfPoints(); i++)
    points->SetPoint(i,this->Input->GetPoint(i));
  vtkPolyData *EdgePolyData = vtkPolyData::New();
  EdgePolyData->SetPoints(points);
  vtkCell* cell;
  vtkCellArray* edgelist = vtkCellArray::New();
  edgelist->InitTraversal();
  vtkIntArray *intarray = vtkIntArray::New();
  for(i=0; i < this->Input->GetNumberOfCells(); i++)
    {
    MeshSeedValues->GetTupleValue(i, dim);
    if(i >0)
      {
      cell = this->Input->GetCell(i);
      // loop through all the edges in the hexahedron cell
      for(j=0; j < cell->GetNumberOfEdges(); j++)
        {
        vtkCell* edgecell = cell->GetEdge(j);
        vtkIdList* pointlist = edgecell->GetPointIds();
        vtkIdType pt1 = pointlist->GetId(0);
        vtkIdType pt2 = pointlist->GetId(1);
        bool status = false;
        edgelist->InitTraversal();
        edgelist->GetNextCell(t,pts);
        do {
        // check if the edge is already present in the edge list
        if((pts[0] == pt1 && pts[1] == pt2) ||
           (pts[0] == pt2 && pts[1] == pt1))
          {
          status = true;
          }
        } while(!status && edgelist->GetNextCell(t,pts));
        if(!status)     
          {edgelist->InsertNextCell(2);
          edgelist->InsertCellPoint(pt1);
          edgelist->InsertCellPoint(pt2);
          if(j ==1 || j == 3 || j == 5 || j == 7)
            {
            intarray->InsertNextValue(dim[0]-1);
            }
          if(j == 0 || j == 2 || j == 4 || j == 6)
            {
            intarray->InsertNextValue(dim[2]-1);
            }
          if(j > 7 && j < 12)
            {
            intarray->InsertNextValue(dim[1]-1);
            }
          }
        }
      }
    else
      {
      cell = this->Input->GetCell(i);
      for(j=0; j < cell->GetNumberOfEdges(); j++)
        {
        vtkCell* edgecell = cell->GetEdge(j);
        vtkIdList* pointlist = edgecell->GetPointIds();
        vtkIdType pt1 = pointlist->GetId(0);
        vtkIdType pt2 = pointlist->GetId(1);
        edgelist->InsertNextCell(2);
        edgelist->InsertCellPoint(pt1);
        edgelist->InsertCellPoint(pt2);
        if(j ==1 || j == 3 || j == 5 || j == 7)
          {
          intarray->InsertNextValue(dim[0]-1);
          }
        if(j == 0 || j == 2 || j == 4 || j == 6)
          {
          intarray->InsertNextValue(dim[2]-1);
          }
        if(j > 7 && j < 12)
          {
          intarray->InsertNextValue(dim[1]-1);
          }
        }
      }
    }
  EdgePolyData->SetLines(edgelist);
  EdgePolyData->GetCellData()->SetScalars(intarray);
  intarray->Delete();
  vtkLookupTable *lut = vtkLookupTable::New();
  lut->SetTableRange(this->MinMeshSeed,this->MaxMeshSeed);
  lut->Build();
  vtkPolyDataMapper *EdgeMapper = vtkPolyDataMapper::New();
  EdgeMapper->SetLookupTable(lut);
  EdgeMapper->SetScalarRange(this->MinMeshSeed,this->MaxMeshSeed);
  EdgeMapper->SetInput(EdgePolyData);
  int numberOfColors = this->MaxMeshSeed - this->MinMeshSeed + 1;
  this->ScalarBarActor = vtkScalarBarActor::New();
  this->ScalarBarActor->SetLookupTable(lut);
  this->ScalarBarActor->SetTitle("# Seeds");
  this->ScalarBarActor->SetLabelFormat("%.0f");
  this->ScalarBarActor->SetMaximumNumberOfColors(numberOfColors);
  this->ScalarBarActor->SetNumberOfLabels(4);
  this->ScalarBarActor->SetWidth(0.075);
  this->ScalarBarActor->SetHeight(0.9);
  this->ScalarBarActor->SetPosition(0.01,0.1);
  
  vtkTextProperty *textProperty = this->ScalarBarActor->GetTitleTextProperty();
  textProperty->SetColor( TextColor );
  this->ScalarBarActor->SetTitleTextProperty( textProperty );
  this->ScalarBarActor->SetLabelTextProperty( textProperty );

  lut->Delete();
  if(this->MeshSeedActor)
    this->MeshSeedActor->Delete();
  this->MeshSeedActor = vtkActor::New();
  this->MeshSeedActor->SetMapper(EdgeMapper);
  this->MeshSeedActor->GetProperty()->SetLineWidth(this->LineWidth);
  EdgeMapper->Delete();
  EdgePolyData->Delete();
  edgelist->Delete();
  points->Delete();
}
//-----------------------------------------------------------------------------
void vtkMimxColorCodeMeshSeedActor::SetLabelTextColor(double color[3])
{
  TextColor[0] = color[0];
  TextColor[1] = color[1];
  TextColor[2] = color[2];
  
  if (this->ScalarBarActor)
    {
    vtkTextProperty *textProperty = this->ScalarBarActor->GetTitleTextProperty();
    textProperty->SetColor( TextColor );
    this->ScalarBarActor->SetTitleTextProperty( textProperty );
    this->ScalarBarActor->SetLabelTextProperty( textProperty );
    }
}

//-----------------------------------------------------------------------------
void vtkMimxColorCodeMeshSeedActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

