/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSubdivideCurve.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
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
#include "vtkMimxSubdivideCurve.h"

#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkCxxRevisionMacro(vtkMimxSubdivideCurve, "$Revision: 1.8 $");
vtkStandardNewMacro(vtkMimxSubdivideCurve);

// Description:

vtkMimxSubdivideCurve::vtkMimxSubdivideCurve()
{
}

vtkMimxSubdivideCurve::~vtkMimxSubdivideCurve()
{
}

int vtkMimxSubdivideCurve::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints *newPts = vtkPoints::New();

  vtkDebugMacro(<< "Starting resampling");
//
// Initialize and check input
//
  if (input->GetNumberOfPoints() < 2)
    {
    vtkErrorMacro(<<"Not enough number of points defining the curve");
    return 0;
    }
  if(this->NumberOfDivisions < 1)
  {
          vtkErrorMacro(<<"Invalid number of divisions");
  return 0;
  }
  
        this->SubDivideCurveBasedOnNumberOfDivisions(input->GetPoints(),
                newPts);
        output->SetPoints(newPts);
        newPts->Delete();
        this->GenerateCurveConnectivity(output);
  return 1;
}

double vtkMimxSubdivideCurve::MeasureCurveLength(vtkPoints *Points)
{
        double length;
        length = 0.0;
        for(int i=0; i<Points->GetNumberOfPoints()-1; i++)
        {
                double x1[3],x2[3];
                Points->GetPoint(i,x1); Points->GetPoint(i+1,x2);
                length = length+ sqrt(vtkMath::Distance2BetweenPoints(x1,x2));
                //cout <<length <<"  "<<sqrt(vtkMath::Distance2BetweenPoints(x1,x2))<<endl;
        }
        return length;
}

int vtkMimxSubdivideCurve::SubDivideCurveBasedOnNumberOfDivisions(vtkPoints* Pts, vtkPoints *NewPts)
{
        NewPts->Initialize();
        if(this->NumberOfDivisions == 1)
        {
                NewPts->InsertNextPoint(Pts->GetPoint(0));
                NewPts->InsertNextPoint(Pts->GetPoint(Pts->GetNumberOfPoints()-1));
                return 1;
        }
        // determine element length based on number of divisions
        double length = this->MeasureCurveLength(Pts);
        double elementlength = length/this->NumberOfDivisions;
        double distance;
        // calculate location of nodes
        int i = 0;
        double firstpoint[3], secondpoint[3], centerpoint[3];
        Pts->GetPoint(i++,firstpoint);
        Pts->GetPoint(i,secondpoint);
        NewPts->InsertNextPoint(firstpoint);
        do{
                NewPts->GetPoint(NewPts->GetNumberOfPoints()-1,centerpoint);
                Pts->GetPoint(i,secondpoint);
                distance = 0.0;
                distance = sqrt(vtkMath::Distance2BetweenPoints(centerpoint,secondpoint));
                if(distance <= elementlength)
                {
                        do{
                                if(i < Pts->GetNumberOfPoints()-1)
                                {
                                        Pts->GetPoint(i,centerpoint);
                                        Pts->GetPoint(++i,secondpoint);
                                        distance = distance + sqrt(vtkMath::Distance2BetweenPoints(centerpoint,secondpoint));
                                }
                                else    break;
                        }while(distance <= elementlength);
                }
                Pts->GetPoint(i-1,firstpoint);
                Pts->GetPoint(i,secondpoint);
                double distanceex = sqrt(vtkMath::Distance2BetweenPoints
                        (firstpoint,secondpoint));
                for(int j=0; j <3; j++)
                        centerpoint[j] = firstpoint[j] + ((elementlength - distance +
                        distanceex)/distanceex)*(secondpoint[j] - firstpoint[j]);
                NewPts->InsertNextPoint(centerpoint);
        }while(i < Pts->GetNumberOfPoints()-1);

        do{     
                NewPts->GetPoint(NewPts->GetNumberOfPoints()-1,firstpoint);
                Pts->GetPoint(Pts->GetNumberOfPoints()-1,secondpoint);
                distance = sqrt(vtkMath::Distance2BetweenPoints(firstpoint,secondpoint));
                if(distance > elementlength)
                {
                        for(int j=0; j <3; j++)
                        {
                                centerpoint[j] = firstpoint[j] + ((elementlength 
                                /distance)*(secondpoint[j] - firstpoint[j]));
                        }
                        NewPts->InsertNextPoint(centerpoint);
                }
        }while (sqrt(vtkMath::Distance2BetweenPoints(Pts->GetPoint(Pts->GetNumberOfPoints()-1),
                NewPts->GetPoint(NewPts->GetNumberOfPoints()-1))) > elementlength);
        if(NewPts->GetNumberOfPoints() == this->NumberOfDivisions)
        {
                NewPts->InsertNextPoint(Pts->GetPoint(Pts->GetNumberOfPoints()-1));
        }
        else
        {
                NewPts->InsertPoint(NewPts->GetNumberOfPoints()-1,
                        Pts->GetPoint(Pts->GetNumberOfPoints()-1));
        }
        //cout <<"El Length "<<elementlength<<endl;
        for(int i=0; i<NewPts->GetNumberOfPoints(); i++)
        {
                NewPts->GetPoint(i,centerpoint);
                //cout<<i<<"  "<<centerpoint[0]<<"  "<<centerpoint[1]<<"  "<<centerpoint[2]<<endl;
        }
        this->MeasureCurveLength(NewPts);
        return 1;
}

void vtkMimxSubdivideCurve::GenerateCurveConnectivity(vtkPolyData* Polydata)
{
        vtkCellArray * cellarray = vtkCellArray::New();
        for(int i=0; i < Polydata->GetNumberOfPoints()-1; i++)
        {
                cellarray->InsertNextCell(2);
                cellarray->InsertCellPoint(i);
                cellarray->InsertCellPoint(i+1);
        }
        Polydata->SetLines(cellarray);
        cellarray->Delete();
}
void vtkMimxSubdivideCurve::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << this->NumberOfDivisions << std::endl;
}
