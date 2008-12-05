/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSTLSource.cxx,v $
Language:  C++
Date:      $Date: 2007/09/20 20:40:11 $
Version:   $Revision: 1.10 $

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

#include "vtkMimxSTLSource.h"

#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkSTLWriter.h"
#include "vtkLookupTable.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkCellLocator.h"
#include "vtkScalarsToColors.h"
#include "vtkProperty.h"
#include "vtkPolyDataMapper.h"
#include "vtkMath.h"

/////////////////////////////////
vtkCxxRevisionMacro(vtkMimxSTLSource, "$Revision: 1.10 $");
vtkStandardNewMacro(vtkMimxSTLSource);

vtkMimxSTLSource::vtkMimxSTLSource()
{
        this->PolyData = vtkPolyData::New();
        this->PolyDataMapper = vtkPolyDataMapper::New();
        this->Actor = vtkActor::New();
        this->PolyDataMapper->SetInput(this->PolyData);
        this->Actor->SetMapper(this->PolyDataMapper);
        this->FileName = NULL;
        this->DoubleArray = vtkDoubleArray::New();
        for(int i=0; i<3; i++)  this->PrevColor[i] = 1.0;
}

vtkMimxSTLSource::~vtkMimxSTLSource()
{
        this->Actor->Delete();
        if(this->FileName)      delete [] this->FileName;
        if(this->DoubleArray)   this->DoubleArray->Delete();
        this->PolyData->Delete();
        this->PolyDataMapper->Delete();
}

void vtkMimxSTLSource::Execute()
{
        vtkPolyData* output = this->GetOutput();
        this->STL->SetFileName(this->FileName);
        this->PolyDataMapper->SetInput(this->PolyData);
        this->Actor->SetMapper(this->PolyDataMapper);
}

//void vtkSTLSource::ReadSTLFile()
//{
//      this->STL->SetFileName(this->FileName);
//      this->STL->Update();
//      this->PolyData->DeepCopy(this->STL->GetOutput());
//      this->PolyData->Modified();
////    this->PolyDataMapper->SetInput(this->PolyData);
////    this->Actor->SetMapper(this->PolyDataMapper);
//      this->Actor->GetProperty()->SetColor(1.0,1.0,1.0);
//}

vtkActor* vtkMimxSTLSource::GetActor(){return this->Actor;}


void vtkMimxSTLSource::ClosestPointDistanceMap(vtkMimxSTLSource* Target)
{
        vtkLookupTable* Lut = vtkLookupTable::New();
        vtkCellLocator *CellLocator = vtkCellLocator::New();
        CellLocator->SetDataSet(Target->PolyData);
        CellLocator->BuildLocator();
        this->DoubleArray->SetNumberOfValues(this->PolyData->GetNumberOfPoints());
        vtkIdType CellId;
        int SubId;
        double x[3], ClosestPoint[3];
        double dist,distmax,cumdist = 0.0;
        ofstream FileOutput;
        FileOutput.open("out.txt",std::ios::out);
        for(int i=0; i <this->PolyData->GetNumberOfPoints(); i++)
        {
                PolyData->GetPoint(i,x);
                CellLocator->FindClosestPoint(x,ClosestPoint,CellId,SubId,dist);
                dist = sqrt(dist);
                DoubleArray->SetValue(i,dist);
                FileOutput <<i<<"  "<<dist<<endl;
                if(i==0)        distmax = dist;
                else    if(distmax < dist) distmax = dist;
                cumdist = cumdist + dist;
        }
        
        cumdist = cumdist/this->PolyData->GetNumberOfPoints();
        //cout <<"Average Distance = "<<cumdist<<endl;

        Lut->SetHueRange(0.667,0.0);

        this->PolyData->GetPointData()->SetScalars(DoubleArray);
        this->PolyDataMapper->SetInput(this->PolyData);
        this->PolyDataMapper->SetLookupTable(Lut);

        this->PolyDataMapper->SetScalarRange(0,distmax);
        this->Actor->SetMapper(this->PolyDataMapper);
        DoubleArray->Delete();
        CellLocator->Delete();
        Lut->Delete();

}

vtkPolyDataMapper* vtkMimxSTLSource::GetPolyDataMapper(){ return this->PolyDataMapper;
}

void vtkMimxSTLSource::ClosestPointDistanceMapMod(vtkMimxSTLSource* Target, double max)
{

        vtkLookupTable* Lut = vtkLookupTable::New();
        Lut->SetNumberOfColors(this->PolyData->GetNumberOfPoints());
        Lut->Build();
        vtkDoubleArray* DoubleArrayStore = vtkDoubleArray::New();
        DoubleArrayStore->SetNumberOfValues(this->PolyData->GetNumberOfPoints());



        for(int i=0; i <this->PolyData->GetNumberOfPoints(); i++)
        {
                double dist = this->DoubleArray->GetValue(i);
                if(dist <=max/2.0)
                {
                        Lut->SetTableValue(i,0.0,dist*2.0,(max/2.0-dist)*2.0,1.0);
                }
                if(dist >max/2.0 && dist <=max)
                {
                        Lut->SetTableValue(i,(dist-max/2.0)*2.0,(max-dist)*2.0,0.0,1.0);
                }
                if(dist >max)   Lut->SetTableValue(i,1.0,1.0,1.0,1.0);
                DoubleArrayStore->SetValue(i,i);
        }
        this->PolyData->GetPointData()->SetScalars(DoubleArrayStore);
        this->PolyDataMapper->SetInput(this->PolyData);
        Lut->SetTableRange(0,max);
        this->PolyDataMapper->SetLookupTable(Lut);
        this->PolyDataMapper->SetScalarRange
                (0,this->PolyData->GetNumberOfPoints());
        DoubleArrayStore->Delete();
        Lut->Delete();
}

void vtkMimxSTLSource::GenerateLookupTableMod(vtkScalarBarActor* ScalarBarActor,double max)
{
        vtkLookupTable *lut = vtkLookupTable::New();
        lut->SetNumberOfColors(40);
        lut->Build();
        double red = 0.0;
        double green = 0.0;
        double blue = 1.0;
        int i;
        for (i=0; i<20; i++)
        {
                lut->SetTableValue(i, red,green,blue,1.0);
                green = green + 0.05;
                blue = blue - 0.05;
        }
        for (i=20; i<40; i++)
        {
                lut->SetTableValue(i, red,green,blue,1.0);
                green = green - 0.05;
                red = red + 0.05;
        }
        lut->SetTableRange(0,max);
        ScalarBarActor->SetLookupTable(lut);
        //      for(i =20; i < 45; i++) lut->SetTableValue(i, 1.0,1.0,1.0,1.0);
}

void vtkMimxSTLSource::ClosestPointDistanceMapGrayScale(vtkMimxSTLSource* Target)
{
        vtkLookupTable* Lut = vtkLookupTable::New();
        vtkCellLocator *CellLocator = vtkCellLocator::New();
        CellLocator->SetDataSet(Target->PolyData);
        CellLocator->BuildLocator();
        this->DoubleArray->SetNumberOfValues(this->PolyData->GetNumberOfPoints());
        vtkIdType CellId;
        int SubId;
        double x[3], ClosestPoint[3];
        double dist,distmax,cumdist = 0.0;
        ofstream FileOutput;
        FileOutput.open("out.txt",std::ios::out);
        for(int i=0; i <this->PolyData->GetNumberOfPoints(); i++)
        {
                PolyData->GetPoint(i,x);
                CellLocator->FindClosestPoint(x,ClosestPoint,CellId,SubId,dist);
                dist = sqrt(dist);
                DoubleArray->SetValue(i,dist);
                FileOutput <<i<<"  "<<dist<<endl;
                if(i==0)        distmax = dist;
                else    if(distmax < dist) distmax = dist;
                cumdist = cumdist + dist;
        }

        cumdist = cumdist/this->PolyData->GetNumberOfPoints();
        cout <<"Average Distance = "<<cumdist<<endl;

        ////
        Lut->SetHueRange(0.0,0.0);
        Lut->SetSaturationRange(0.0,0.0);
        Lut->SetValueRange(0,1);
        //
        this->PolyData->GetPointData()->SetScalars(DoubleArray);
        this->PolyDataMapper->SetInput(this->PolyData);
        this->PolyDataMapper->SetLookupTable(Lut);

        this->PolyDataMapper->SetScalarRange(0,distmax);
        this->Actor->SetMapper(this->PolyDataMapper);
        DoubleArray->Delete();
        CellLocator->Delete();
        Lut->Delete();

}
void vtkMimxSTLSource::ClosestPointDistanceMapModGrayScale(vtkMimxSTLSource* Target, double max)
{

        vtkLookupTable* Lut = vtkLookupTable::New();
        Lut->SetNumberOfColors(this->PolyData->GetNumberOfPoints());
        Lut->Build();
        vtkDoubleArray* DoubleArrayStore = vtkDoubleArray::New();
        DoubleArrayStore->SetNumberOfValues(this->PolyData->GetNumberOfPoints());



        for(int i=0; i <this->PolyData->GetNumberOfPoints(); i++)
        {
                double dist = this->DoubleArray->GetValue(i);
                if(dist < max)
                {
                        Lut->SetTableValue(i,dist/max,dist/max,dist/max,1.0);
                }
                else
                if(dist >= max) Lut->SetTableValue(i,1.0,1.0,1.0,1.0);
                DoubleArrayStore->SetValue(i,i);
        }
        this->PolyData->GetPointData()->SetScalars(DoubleArrayStore);
        this->PolyDataMapper->SetInput(this->PolyData);
        Lut->SetTableRange(0,max);
        this->PolyDataMapper->SetLookupTable(Lut);
        this->PolyDataMapper->SetScalarRange
                (0,this->PolyData->GetNumberOfPoints());
        DoubleArrayStore->Delete();
        Lut->Delete();
}

void vtkMimxSTLSource::GenerateLookupTableModGrayScale(vtkScalarBarActor* ScalarBarActor,double max)
{
        vtkLookupTable *lut = vtkLookupTable::New();
        lut->SetNumberOfColors(40);
        lut->Build();
        double intensity = 0.0;
        int i;
        for (i=0; i<40; i++)
        {
                lut->SetTableValue(i, intensity,intensity,intensity,1.0);
                intensity = intensity + 0.0125;
        }
        
        lut->SetTableRange(0,max);
        ScalarBarActor->SetLookupTable(lut);
}

void vtkMimxSTLSource::Copy(vtkPolyData* Source)
{
        this->PolyData = vtkPolyData::New();
        this->PolyData->DeepCopy(Source);
        this->PolyDataMapper->SetInput(this->PolyData);
        this->Actor->SetMapper(this->PolyDataMapper);
        this->Actor->GetProperty()->SetColor(1.0,0.0,0.0);
}

void vtkMimxSTLSource::SetPrevColor(double color[3])
{
        for(int i=0; i<3; i++)  this->PrevColor[i] = color[i];
}

void vtkMimxSTLSource::GetPrevColor(double color[3])
{
        for(int i=0; i<3; i++)  color[i] = this->PrevColor[i];
}
vtkPolyData* vtkMimxSTLSource::GetPolyData()
{
        return this->PolyData;
}

void vtkMimxSTLSource::Translate(double X, double Y, double Z)
{
        cout<<"Translate by : "<<X<<"  "<<Y<<"  "<<Z<<endl;
        vtkTransform* trans = vtkTransform::New();
        vtkPoints* mPointsStore = vtkPoints::New();
        trans->Translate(X,Y,Z);
        trans->TransformPoints(this->PolyData->GetPoints(),mPointsStore);
        this->PolyData->GetPoints()->DeepCopy(mPointsStore);
        this->PolyDataMapper->Update();
        trans->Delete();
        mPointsStore->Delete();

}

void vtkMimxSTLSource::Scale(double Center[3], double ScaleFactor)
{
        cout<<"Scale by : "<<ScaleFactor<<endl;
        vtkTransform* trans = vtkTransform::New();
        vtkPoints* mPointsStore = vtkPoints::New();
        trans->PostMultiply();
        trans->Translate(-Center[0], -Center[1], -Center[2]);
        trans->Scale(ScaleFactor,ScaleFactor,ScaleFactor);
        trans->Translate(Center);
        trans->TransformPoints(this->PolyData->GetPoints(),mPointsStore);
        this->PolyData->GetPoints()->DeepCopy(mPointsStore);
        this->PolyDataMapper->Update();
        trans->Delete();
        mPointsStore->Delete();
}

void vtkMimxSTLSource::Rotate(double Center[3], double Angle, int Axis)
{
        cout<<"Rotate  : "<<Angle<<endl;
        vtkTransform* trans = vtkTransform::New();
        vtkPoints* mPointsStore = vtkPoints::New();
        trans->PostMultiply();
        trans->Translate(-Center[0], -Center[1], -Center[2]);
        if(Axis == 1)   trans->RotateX(Angle);
        else{
                if(Axis == 2)   trans->RotateY(Angle);
                else    trans->RotateZ(Angle);
        }
        trans->Translate(Center);
        trans->TransformPoints(this->PolyData->GetPoints(),mPointsStore);
        this->PolyData->GetPoints()->DeepCopy(mPointsStore);
        this->PolyDataMapper->Update();
        trans->Delete();
        mPointsStore->Delete();
}
