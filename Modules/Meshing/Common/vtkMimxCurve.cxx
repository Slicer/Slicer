/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxCurve.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.4 $

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

#include "vtkMimxCurve.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkTransform.h"
/////////////////////////////////
vtkCxxRevisionMacro(vtkMimxCurve, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkMimxCurve);

vtkMimxCurve::vtkMimxCurve()
{
  this->PolyDataMapper = vtkPolyDataMapper::New();
  this->Actor = vtkActor::New();
  this->FileName = NULL;
  this->Points = vtkPoints::New();
  this->CellArray = vtkCellArray::New();
  this->PolyData = vtkPolyData::New();
  this->PolyDataMapper = vtkPolyDataMapper::New();
  PolyData->SetPoints(Points);
//  PolyData->SetPolys(CellArray);
  PolyData->SetLines(CellArray);
  PolyDataMapper->SetInput(PolyData);
  Actor->SetMapper(PolyDataMapper);
  Actor->GetProperty()->SetColor(0.0,0.0,0.0);
  this->Actor->GetProperty()->SetLineWidth(4.0);
  for(int i=0; i<3; i++)  this->PrevColor[i] = 0.0;
}

vtkMimxCurve::~vtkMimxCurve()
{
  this->PolyDataMapper->Delete();
  this->Actor->Delete();
  if(this->FileName)  delete [] this->FileName;
}
void vtkMimxCurve::Execute()
{
}

vtkActor* vtkMimxCurve::GetActor()
{
  return this->Actor;
}

void vtkMimxCurve::SetPrevColor(double color[3])
{
  for(int i=0; i<3; i++)  this->PrevColor[i] = color[i];
}

void vtkMimxCurve::GetPrevColor(double color[3])
{
  for(int i=0; i<3; i++)  color[i] = this->PrevColor[i];
}

vtkPoints* vtkMimxCurve::GetPoints()
{
  return this->Points;
}

void vtkMimxCurve::Translate(double X, double Y, double Z)
{
  cout<<"Translate by : "<<X<<"  "<<Y<<"  "<<Z<<endl;
  vtkTransform* trans = vtkTransform::New();
  vtkPoints* mPointsStore = vtkPoints::New();
  trans->Translate(X,Y,Z);
  trans->TransformPoints(this->Points,mPointsStore);
  this->Points->DeepCopy(mPointsStore);
  this->PolyDataMapper->Update();
  trans->Delete();
  mPointsStore->Delete();

}

void vtkMimxCurve::Scale(double Center[3], double ScaleFactor)
{
  cout<<"Scale by : "<<ScaleFactor<<endl;
  vtkTransform* trans = vtkTransform::New();
  vtkPoints* mPointsStore = vtkPoints::New();
  trans->PostMultiply();
  trans->Translate(-Center[0], -Center[1], -Center[2]);
  trans->Scale(ScaleFactor,ScaleFactor,ScaleFactor);
  trans->Translate(Center);
  trans->TransformPoints(this->Points,mPointsStore);
  this->Points->DeepCopy(mPointsStore);
  this->PolyDataMapper->Update();
  trans->Delete();
  mPointsStore->Delete();
}

void vtkMimxCurve::CalculateCenter(double Center[3])
{
  double x[3];
  Center[0] = 0.0; Center[1] = 0.0; Center[2] = 0.0;
  for(int i=0; i < this->Points->GetNumberOfPoints(); i++)
  {
    this->Points->GetPoint(i,x);
    for(int j=0; j<3; j++)  Center[j] += x[j];
  }
  for(int j=0; j<3; j++)  Center[j] = Center[j]/
    this->Points->GetNumberOfPoints();
}
