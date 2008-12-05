/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxBoundaryConditionActor.cxx,v $
Language:  C++
Date:      $Date: 2008/08/10 00:48:34 $
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

#include "vtkMimxBoundaryConditionActor.h"

#include "mimxLinkedList.h"
#include "vtkActor.h"
#include "vtkArrowSource.h"
#include "vtkConeSource.h"
#include "vtkFloatArray.h"
#include "vtkPointSet.h"
#include "vtkGlyph3D.h"
#include "vtkPolyDataMapper.h"
#include "vtkFieldData.h"
#include "vtkCellData.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkUnstructuredGrid.h"
#include "vtkRenderer.h"
#include "vtkPointData.h"
#include "vtkStringArray.h"


vtkCxxRevisionMacro(vtkMimxBoundaryConditionActor, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkMimxBoundaryConditionActor);


//----------------------------------------------------------------------------------------
vtkMimxBoundaryConditionActor::vtkMimxBoundaryConditionActor()
{
  this->Renderer = NULL;
  this->BoundaryConditionDisplayList.clear();
  this->GlyphScale = 1.0;
  this->UnstructuredGrid = vtkUnstructuredGrid::New();
  this->Step = 0;
  this->GlyphSize = 1.0;
  this->ScaleFactor = 1.0;
}

//----------------------------------------------------------------------------------------
vtkMimxBoundaryConditionActor::~vtkMimxBoundaryConditionActor()
{
        while (!BoundaryConditionDisplayList.empty())
  {
    BoundaryConditionProperty *currentProp = BoundaryConditionDisplayList.front();
    if (this->Renderer)
    {
      this->Renderer->RemoveViewProp(currentProp->GlyphActor);
    }
    if (currentProp->GlyphActor) currentProp->GlyphActor->Delete();
    if (currentProp->Glyph) currentProp->Glyph->Delete();
    BoundaryConditionDisplayList.pop_front();
  }
  
        this->UnstructuredGrid->Delete();
}

//----------------------------------------------------------------------------------------
vtkUnstructuredGrid* vtkMimxBoundaryConditionActor::GetDataSet()
{
  return this->UnstructuredGrid;
}

//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::SetDataSet(vtkUnstructuredGrid *mesh)
{
 
  while (!BoundaryConditionDisplayList.empty())
  {
    BoundaryConditionProperty *currentProp = BoundaryConditionDisplayList.front();
    if (this->Renderer)
    {
      this->Renderer->RemoveViewProp(currentProp->GlyphActor);
    }
    if (currentProp->GlyphActor) currentProp->GlyphActor->Delete();
    if (currentProp->Glyph) currentProp->Glyph->Delete();
    BoundaryConditionDisplayList.pop_front();
  }
  
  this->UnstructuredGrid = mesh;
  this->Step = 0;
}

//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::SetStep( int stepNumber )
{
 
  this->Step = stepNumber;
  this->DeleteAllBoundaryConditionActors();
  this->CreateStepActors(); 
}

//----------------------------------------------------------------------------------------
int vtkMimxBoundaryConditionActor::GetStep( )
{
  return this->Step; 
}

//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::SetGlyphSize( double size )
{
 /* this->GlyphSize = size;
  std::list<BoundaryConditionProperty*>::iterator it;
  
  for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
  {
    BoundaryConditionProperty *currentProp = *it;
  }
  this->UpdateGlyphSize();*/
}

//----------------------------------------------------------------------------------------
double vtkMimxBoundaryConditionActor::GetGlyphSize( )
{
  return this->GlyphSize;
}

//----------------------------------------------------------------------------------------
double vtkMimxBoundaryConditionActor::GetGlyphScale( )
{
  return this->GlyphScale; 
}

//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::CreateStepActors( )
{
  char stepString[128];
  sprintf(stepString, "Step_%d_", this->Step);

  char StepNum[16];
  sprintf(StepNum, "%d", this->Step);

  const char *BCType;
  const char *NodeSetName;

  vtkStringArray *nodesetnames = vtkStringArray::SafeDownCast(
          this->UnstructuredGrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));
  if(!nodesetnames)     return;

  vtkFieldData *fieldData = this->UnstructuredGrid->GetFieldData();
  if ( fieldData )
  {
    for (int i=0;i<fieldData->GetNumberOfArrays();i++)
    {
      std::string arrayName = fieldData->GetArrayName(i);
      if (  arrayName.find(stepString) == 0 )
      {
                  BCType = NULL;
                  if(strstr(arrayName.c_str(), "Displacement")) BCType = "Displacement";
                  if(strstr(arrayName.c_str(), "Force"))        BCType = "Force";
                  if(strstr(arrayName.c_str(), "Moment"))       BCType = "Moment";
                  if(strstr(arrayName.c_str(), "Rotation"))     BCType = "Rotation";
                  if(BCType)
                  {
                          for (int j=0; j<nodesetnames->GetNumberOfTuples(); j++)
                          {
                                  if(arrayName.find(nodesetnames->GetValue(j)))
                                  {
                                          NodeSetName = nodesetnames->GetValue(j);
                                          this->CreateBoundaryConditionActor(StepNum, NodeSetName, BCType);
                                  }
                          }
                  }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::UpdateGlyphSize( )
{  
  std::list<BoundaryConditionProperty*>::iterator it;
  
  for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
  {
    BoundaryConditionProperty *currentProp = *it;
 
    if (currentProp->bcValue == 0.0 && !strcmp(currentProp->bcType, "Displacement"))    
        {
                vtkPointSet *pointSet = this->GetPointSetOfNodeSet(currentProp->nodeSet);

                vtkFloatArray *vectorArray = vtkFloatArray::New();
                vectorArray->SetNumberOfComponents(3);
                vectorArray->SetNumberOfTuples(pointSet->GetNumberOfPoints());
                float direction[3];

                int bcAxis;
                if(!strcmp(currentProp->Axis, "X"))     bcAxis = 0;
                else if(!strcmp(currentProp->Axis, "Y"))        bcAxis = 1;
                else bcAxis = 2;

                direction[0] = 0.0; direction[1] = 0.0; direction[2] = 0.0;
                if (currentProp->bcValue >= 0.0)
                {
                        direction[bcAxis] = 1.0;
                }
                else
                {
                        direction[bcAxis] = -1.0;
                }

                for (int i=0; i<pointSet->GetNumberOfPoints(); i++)
                {
                        vectorArray->SetTuple(i, direction);
                }
                pointSet->GetPointData()->SetVectors(vectorArray);
                
                vtkConeSource *coneSource = vtkConeSource::New();
                coneSource->SetHeight(coneSource->GetHeight()/3.0);
                coneSource->SetRadius(coneSource->GetRadius()/3.0);
                currentProp->Glyph->SetInput(pointSet);
                currentProp->Glyph->SetSource(coneSource->GetOutput());
                currentProp->Glyph->SetScaleFactor(this->ScaleFactor);

                double offSet = coneSource->GetHeight()*this->ScaleFactor/2.0;
                for (int i=0; i<pointSet->GetNumberOfPoints(); i++)
                {
                        double point[3];
                        pointSet->GetPoint(i, point);
                        point[bcAxis] = point[bcAxis] - offSet;
                        pointSet->GetPoints()->SetPoint(i, point);
                }
                pointSet->Delete();
                vectorArray->Delete();
                coneSource->Delete();
        }
        else 
        {
                currentProp->Glyph->SetScaleFactor(this->ScaleFactor*currentProp->AxisFactor);
        }
        currentProp->Glyph->Modified();
        currentProp->Glyph->Update();
  }
  if (this->Renderer)
          this->Renderer->Render();
}
//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::CreateBoundaryConditionActor( const char *StepNum, 
                                                                                                                                 const char *NodeSetName, const char *BCType)
{
        char bcListName[128];
        strcpy(bcListName, "Step_");    strcat(bcListName, StepNum);    strcat(bcListName, "_");
        strcat(bcListName, NodeSetName);        strcat(bcListName, "_");        strcat(bcListName, BCType);
        strcat(bcListName, "_");

        char bcListNameX[128], bcListNameY[128], bcListNameZ[128];
        strcpy(bcListNameX, bcListName);        strcat(bcListNameX, "X");
        strcpy(bcListNameY, bcListName);        strcat(bcListNameY, "Y");
        strcpy(bcListNameZ, bcListName);        strcat(bcListNameZ, "Z");

        double xmag = VTK_DOUBLE_MAX, ymag = VTK_DOUBLE_MAX, zmag = VTK_DOUBLE_MAX;
        double xmagabs = VTK_DOUBLE_MAX, ymagabs = VTK_DOUBLE_MAX, zmagabs = VTK_DOUBLE_MAX;
        double max = VTK_DOUBLE_MAX;
        
        double ValX = this->GetValue(bcListNameX);
        double ValY = this->GetValue(bcListNameY);
        double ValZ = this->GetValue(bcListNameZ);

        if(ValX != VTK_DOUBLE_MAX)
        {
                xmagabs = fabs(ValX);
                xmag = ValX;
                max = xmagabs;  
        }
        if(ValY != VTK_DOUBLE_MAX)
        {
                ymagabs = fabs(ValY);
                ymag = ValY;
                if(max == VTK_DOUBLE_MAX)
                {
                        max = ymagabs;
                }
                else
                {
                        if(max < ymagabs)
                        {
                                max = ymagabs;
                        }
                }
        }
        if(ValZ != VTK_DOUBLE_MAX)
        {
                zmagabs = fabs(ValZ);
                zmag = ValZ;
                if(max == VTK_DOUBLE_MAX)
                {
                        max = zmagabs;
                }
                else
                {
                        if(max < zmagabs)
                        {
                                max = zmagabs;
                        }
                }
        }

        double factorX = 0.0, factorY = 0.0, factorZ = 0.0;
        if(max != VTK_DOUBLE_MAX && max != 0.0)
        {
                if(xmagabs != VTK_DOUBLE_MAX)
                {
                        factorX = xmagabs/max;
                }
                if(ymagabs != VTK_DOUBLE_MAX)
                {
                        factorY = ymagabs/max;
                }
                if(zmagabs != VTK_DOUBLE_MAX)
                {
                        factorZ = zmagabs/max;
                }
        }

        if(xmagabs != VTK_DOUBLE_MAX)
                this->CreateIndividualBoundaryConditionActor(StepNum, NodeSetName, BCType, "X", factorX, ValX, bcListNameX);
        if(ymagabs != VTK_DOUBLE_MAX)
                this->CreateIndividualBoundaryConditionActor(StepNum, NodeSetName, BCType, "Y", factorY, ValY, bcListNameY);
        if(zmagabs != VTK_DOUBLE_MAX)
                this->CreateIndividualBoundaryConditionActor(StepNum, NodeSetName, BCType, "Z", factorZ, ValZ, bcListNameZ);
}

//----------------------------------------------------------------------------------------
int vtkMimxBoundaryConditionActor::HasBoundaryConditionActor( std::string bcName )
{
  std::list<BoundaryConditionProperty*>::iterator it;
  
  for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
  {
    BoundaryConditionProperty *currentProp = *it;
    
    if (strstr(currentProp->name, bcName.c_str()))
    {
      return 1;
    }
  }
  
  return 0;
}
//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::DeleteBoundaryConditionActor( std::string bcName )
{
  std::list<BoundaryConditionProperty*>::iterator it;
  
  for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
  {
    BoundaryConditionProperty *currentProp = *it;
    
        if (strstr(currentProp->name, bcName.c_str()))
        {
      if (this->Renderer)
      {
        this->Renderer->RemoveViewProp(currentProp->GlyphActor);
      }
      if (currentProp->GlyphActor) currentProp->GlyphActor->Delete();
      if (currentProp->Glyph) currentProp->Glyph->Delete();
      BoundaryConditionDisplayList.erase(it);
      break;
    }
  }

  for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
  {
          BoundaryConditionProperty *currentProp = *it;

          if (strstr(currentProp->name, bcName.c_str()))
          {
                  if (this->Renderer)
                  {
                          this->Renderer->RemoveViewProp(currentProp->GlyphActor);
                  }
                  if (currentProp->GlyphActor) currentProp->GlyphActor->Delete();
                  if (currentProp->Glyph) currentProp->Glyph->Delete();
                  BoundaryConditionDisplayList.erase(it);
                  break;
          }
  }

  for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
  {
          BoundaryConditionProperty *currentProp = *it;

          if (strstr(currentProp->name, bcName.c_str()))
          {
                  if (this->Renderer)
                  {
                          this->Renderer->RemoveViewProp(currentProp->GlyphActor);
                  }
                  if (currentProp->GlyphActor) currentProp->GlyphActor->Delete();
                  if (currentProp->Glyph) currentProp->Glyph->Delete();
                  BoundaryConditionDisplayList.erase(it);
                  break;
          }
  }
}

//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::DeleteAllBoundaryConditionActors( )
{
  while (!BoundaryConditionDisplayList.empty())
  {
    BoundaryConditionProperty *currentProp = BoundaryConditionDisplayList.front();
    if (this->Renderer)
    {
      this->Renderer->RemoveViewProp(currentProp->GlyphActor);
    }
    if (currentProp->GlyphActor) currentProp->GlyphActor->Delete();
    if (currentProp->Glyph) currentProp->Glyph->Delete();
    BoundaryConditionDisplayList.pop_front();
  }
}

//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::AddBoundaryConditionActor( const char *StepNum, const char *NodeSetName, 
                                                                                                                          const char *BCType, const char *BCName )
{
        char bcListName[128];
        strcpy(bcListName, "Step_");    strcat(bcListName, StepNum);    strcat(bcListName, "_");
        strcat(bcListName, NodeSetName);        strcat(bcListName, "_");        strcat(bcListName, BCType);
        strcat(bcListName, "_");
        if (this->HasBoundaryConditionActor(bcListName) )
                this->DeleteBoundaryConditionActor(bcListName);
    
  this->CreateBoundaryConditionActor(StepNum, NodeSetName, BCType);
}
//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::ShowAllStepActors( int stepNumber )
{
  char stepString[128];
  sprintf(stepString, "Step_%d_", stepNumber);
  this->Step = stepNumber;
  
  std::list<BoundaryConditionProperty*>::iterator it;
  for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
  {
    BoundaryConditionProperty *currentProp = *it;
    
    if (strstr(currentProp->name, stepString))
    {
      //std::cout << "Update Visibility on : " << currentProp->name << std::endl;
      currentProp->GlyphActor->SetVisibility(1);
          currentProp->IsVisible = true;
    }
  }
  
  if (this->Renderer)
    this->Renderer->Render();
  
}
//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::HideAllStepActors( int stepNumber )
{
  char stepString[128];
  sprintf(stepString, "Step_%d_", stepNumber);
  this->Step = stepNumber;
  
  std::list<BoundaryConditionProperty*>::iterator it;
  for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
  {
    BoundaryConditionProperty *currentProp = *it;
    
    if (strstr(currentProp->name, stepString))
    {
      //std::cout << "Update Visibility off : " << currentProp->name << std::endl;
      currentProp->GlyphActor->SetVisibility(0);
          currentProp->IsVisible = false;
    }
  }
  
  if (this->Renderer)
    this->Renderer->Render();
  
}

//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::ShowBoundaryCondition(int step, const char *setName, int type)
{
  char stepString[128];
  sprintf(stepString, "Step_%d_%s_", step, setName);
  if (type == vtkMimxBoundaryConditionActor::Force)
  {
    strcat(stepString, "Force_");
  }
  else if (type == vtkMimxBoundaryConditionActor::Displacement)
  {
    strcat(stepString, "Displacement_");
  }
  else if (type == vtkMimxBoundaryConditionActor::Rotation)
  {
    strcat(stepString, "Rotation_");
  }
  else
  {
    strcat(stepString, "Moment_");
  }
  
  std::list<BoundaryConditionProperty*>::iterator it;
  for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
  {
    BoundaryConditionProperty *currentSet = *it;
    
    if (strstr(currentSet->name, stepString))
    {
                currentSet->IsVisible = true;
      currentSet->GlyphActor->SetVisibility(1);
    }
  }
  
  if (this->Renderer)
    this->Renderer->Render();
  
}
//----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::HideBoundaryCondition(int step, const char *setName, int type)
{
  char stepString[128];
  sprintf(stepString, "Step_%d_%s_", step, setName);
  if (type == vtkMimxBoundaryConditionActor::Force)
  {
    strcat(stepString, "Force_");
  }
  else if (type == vtkMimxBoundaryConditionActor::Displacement)
  {
    strcat(stepString, "Displacement_");
  }
  else if (type == vtkMimxBoundaryConditionActor::Rotation)
  {
    strcat(stepString, "Rotation_");
  }
  else
  {
    strcat(stepString, "Moment_");
  }
  
  std::list<BoundaryConditionProperty*>::iterator it;
  for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
  {
    BoundaryConditionProperty *currentSet = *it;
    
    if (strstr(currentSet->name, stepString))
    {
                currentSet->IsVisible = false;
      currentSet->GlyphActor->SetVisibility(0);
    }
  }
  
  if (this->Renderer)
    this->Renderer->Render();
  
}
//----------------------------------------------------------------------------------------
int vtkMimxBoundaryConditionActor::GetBoundaryConditionVisibility(int step, const char *setName, int type)
{
  char stepString[128];
  sprintf(stepString, "Step_%d_%s_", step, setName);
  if (type == vtkMimxBoundaryConditionActor::Force)
  {
    strcat(stepString, "Force_");
  }
  else if (type == vtkMimxBoundaryConditionActor::Displacement)
  {
    strcat(stepString, "Displacement_");
  }
  else if (type == vtkMimxBoundaryConditionActor::Rotation)
  {
    strcat(stepString, "Rotation_");
  }
  else
  {
    strcat(stepString, "Moment_");
  }
  
  std::list<BoundaryConditionProperty*>::iterator it;
  for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
  {
    BoundaryConditionProperty *currentSet = *it;
    
    if (strstr(currentSet->name, stepString))
    {
      return currentSet->GlyphActor->GetVisibility();
    }
  }
  
  return 0;
  
}


//--------------------------------------------------------------------------------------------------------------------
BoundaryConditionProperty* vtkMimxBoundaryConditionActor::GetBoundaryCondition(const char *bcLabel)
{
        std::list<BoundaryConditionProperty*>::iterator it;
        for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
        {
                BoundaryConditionProperty *currentProp = *it;
                if (strstr(currentProp->name, bcLabel))
                {
                        return currentProp;
                }
        }
        return NULL;
}

//----------------------------------------------------------------------------------
vtkPointSet* vtkMimxBoundaryConditionActor::GetPointSetOfNodeSet(const char* NodeSetName)
{
        if (!this->UnstructuredGrid->GetPointData()->GetArray(NodeSetName))     return NULL;
        vtkIntArray *intarray = vtkIntArray::SafeDownCast(
                this->UnstructuredGrid->GetPointData()->GetArray(NodeSetName));

        vtkPoints *points = vtkPoints::New();
        vtkUnstructuredGrid *pointSetOfNodeSet = vtkUnstructuredGrid::New();
        
        int numPoints = this->UnstructuredGrid->GetNumberOfPoints();

        for (int i=0; i<numPoints; i++)
        {
                if(intarray->GetValue(i))
                {
                        points->InsertNextPoint(this->UnstructuredGrid->GetPoint(i));
                }
        }
        pointSetOfNodeSet->SetPoints(points);
        points->Delete();
        return pointSetOfNodeSet;
}

//----------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::SetRenderer(vtkRenderer *renderer)
{
  this->Renderer = renderer;
  
  std::list<BoundaryConditionProperty*>::iterator it;
  for ( it=this->BoundaryConditionDisplayList.begin(); it != this->BoundaryConditionDisplayList.end(); it++ )
  {
    BoundaryConditionProperty *currentSet = *it;
    
    if (this->Renderer)
    {
          this->Renderer->AddViewProp(currentSet->GlyphActor);
          currentSet->GlyphActor->SetVisibility(0);
          currentSet->IsVisible = false;
    }
  }
  
}
//-----------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::SetScaleFactor(double sFactor)
{
        this->ScaleFactor = sFactor;
        this->UpdateGlyphSize();
}
//----------------------------------------------------------------------------------
vtkRenderer* vtkMimxBoundaryConditionActor::GetRenderer( )
{
  return (this->Renderer);
}
//-----------------------------------------------------------------------------------
double vtkMimxBoundaryConditionActor::GetValue(const char *BCName)
{
        // list all the three values.
        vtkDataArray *dataarray = this->UnstructuredGrid->GetFieldData()->GetArray(BCName);
        double Value = VTK_DOUBLE_MAX;
        if(dataarray)
        {
                Value = vtkFloatArray::SafeDownCast(dataarray)->GetValue(0);
        }
        return Value;
}
//-----------------------------------------------------------------------------------------
void vtkMimxBoundaryConditionActor::CreateIndividualBoundaryConditionActor(
        const char *StepNum, const char *NodeSetName, const char *BCType, 
        const char *Direction, double axisFactor, double bcValue, const char *BCName)
{
        vtkPointSet *pointSet = this->GetPointSetOfNodeSet(NodeSetName);

        BoundaryConditionProperty *bcProperty = new BoundaryConditionProperty;
        strcpy(bcProperty->name, BCName);
        bcProperty->IsVisible = false;
        strcpy(bcProperty->bcType, BCType);
        strcpy(bcProperty->Axis, Direction);
        strcpy(bcProperty->StepNumber, StepNum);
        strcpy(bcProperty->nodeSet, NodeSetName);
        bcProperty->bcValue = bcValue;
    bcProperty->AxisFactor = axisFactor;

        vtkArrowSource *arrowSource = vtkArrowSource::New();
        vtkConeSource *coneSource = vtkConeSource::New();
        coneSource->SetHeight(coneSource->GetHeight()/3.0);
        coneSource->SetRadius(coneSource->GetRadius()/3.0);

        bcProperty->GlyphActor = vtkActor::New();

        vtkFloatArray *vectorArray = vtkFloatArray::New();
        vectorArray->SetNumberOfComponents(3);
        vectorArray->SetNumberOfTuples(pointSet->GetNumberOfPoints());
        float direction[3];

        int bcAxis;
        if(!strcmp(Direction, "X"))     bcAxis = 0;
        else if(!strcmp(Direction, "Y"))        bcAxis = 1;
        else bcAxis = 2;

        direction[0] = 0.0; direction[1] = 0.0; direction[2] = 0.0;
        if (bcValue >= 0.0)
        {
                direction[bcAxis] = 1.0;
        }
        else
        {
                direction[bcAxis] = -1.0;
        }

        for (int i=0; i<pointSet->GetNumberOfPoints(); i++)
        {
                vectorArray->SetTuple(i, direction);
        }
        pointSet->GetPointData()->SetVectors(vectorArray);

        bcProperty->Glyph = vtkGlyph3D::New();
        bcProperty->Glyph->SetInput(pointSet);

        if (bcValue == 0.0 && !strcmp(bcProperty->bcType, "Displacement"))      
        {
                bcProperty->AxisFactor = 1.0;
                axisFactor = bcProperty->AxisFactor;
                bcProperty->Glyph->SetSource(coneSource->GetOutput());
                bcProperty->Glyph->SetScaleFactor(this->ScaleFactor);
                
                double offSet = coneSource->GetHeight()*this->ScaleFactor/2.0;
                for (int i=0; i<pointSet->GetNumberOfPoints(); i++)
                {
                        double point[3];
                        pointSet->GetPoint(i, point);
                        point[bcAxis] = point[bcAxis] - offSet;
                        pointSet->GetPoints()->SetPoint(i, point);
                }
        }
        else 
        {
                bcProperty->Glyph->SetSource(arrowSource->GetOutput());
                bcProperty->Glyph->SetScaleFactor(this->ScaleFactor*axisFactor);
        }

        bcProperty->Glyph->SetVectorMode(1);
        bcProperty->Glyph->SetVectorModeToUseVector();
        bcProperty->Glyph->Update();

        vtkPolyDataMapper *glyphMapper = vtkPolyDataMapper::New();
        glyphMapper->SetInput(bcProperty->Glyph->GetOutput());

        bcProperty->GlyphActor->SetMapper(glyphMapper);
        if (bcAxis == vtkMimxBoundaryConditionActor::Xaxis)
        {
                bcProperty->GlyphActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
        }
        else if (bcAxis == vtkMimxBoundaryConditionActor::Yaxis)
        {
                bcProperty->GlyphActor->GetProperty()->SetColor(0.0, 1.0, 0.0);
        }
        else
        {
                bcProperty->GlyphActor->GetProperty()->SetColor(0.0, 0.0, 1.0);
        }
        arrowSource->Delete();
        coneSource->Delete();
        glyphMapper->Delete();
        vectorArray->Delete();
        pointSet->Delete();

        if (this->Renderer)
        {
                this->Renderer->AddViewProp(bcProperty->GlyphActor);
                bcProperty->GlyphActor->SetVisibility(0);
                bcProperty->IsVisible = false;
        }

        this->BoundaryConditionDisplayList.push_back( bcProperty );

}
//-------------------------------------------------------------------------------------------
