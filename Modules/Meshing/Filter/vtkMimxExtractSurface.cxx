/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractSurface.cxx,v $
Language:  C++
Date:      $Date: 2008/05/13 20:13:44 $
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

#include "vtkMimxExtractSurface.h"

#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCleanPolyData.h"
#include "vtkDataSet.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkGarbageCollector.h"
#include "vtkGenericCell.h"
#include "vtkGeometryFilter.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxExtractSurface, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkMimxExtractSurface);

// Description:
// Instantiate object with InsideOut turned off.
vtkMimxExtractSurface::vtkMimxExtractSurface()
{
        this->CellIdList = NULL;
        this->FaceIdList = NULL;
}

//----------------------------------------------------------------------------
vtkMimxExtractSurface::~vtkMimxExtractSurface()
{
  if (this->CellIdList)
    this->CellIdList->Delete();
  if (this->FaceIdList)
    this->FaceIdList->Delete();
}

//----------------------------------------------------------------------------
int vtkMimxExtractSurface::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPolyData *output = vtkPolyData::SafeDownCast(
          outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if(!this->FaceIdList) 
  {
          vtkErrorMacro("Choose faces to be extruded");
          return 0;
  }
  if(this->CellIdList == NULL || this->FaceIdList == NULL)
  {
          vtkErrorMacro("Set cell ids and corresponding face ids to be extracted");
          return 0;
  }
  //
  if(this->CellIdList->GetNumberOfIds() <= 0 || this->FaceIdList->GetNumberOfIds() <= 0)
  {
          vtkErrorMacro("Cell ids and corresponding face ids to be extracted list empty");
          return 0;
  }
  //
  if(this->CellIdList->GetNumberOfIds() != this->FaceIdList->GetNumberOfIds())
  {
          vtkErrorMacro("Each cell should have a corresponding face associated with it");
          return 0;
  }

  //
  if(!input)
  {
          vtkErrorMacro("Input not set");
          return 0;
  }
  //
  int i, j;
  vtkPoints *Points = vtkPoints::New();
  vtkCellArray *CellArray = vtkCellArray::New();

  int numIds = this->CellIdList->GetNumberOfIds();
  //vtkIdList *facepointlist;
  for (i=0; i<numIds; i++)
  {
      vtkIdList *facepointlist = input->GetCell(this->CellIdList->GetId(i))
                        ->GetFace(this->FaceIdList->GetId(i))->GetPointIds();
      CellArray->InsertNextCell(facepointlist->GetNumberOfIds());
      for (j=0; j<facepointlist->GetNumberOfIds(); j++)
      {
          Points->InsertNextPoint(input->GetPoint(facepointlist->GetId(j)));
          CellArray->InsertCellPoint(Points->GetNumberOfPoints()-1);
      }
  }
  
  vtkPolyData *PolyData = vtkPolyData::New();
  PolyData->SetPoints(Points);
  PolyData->SetPolys(CellArray);

  Points->Delete();
  CellArray->Delete();

  vtkCleanPolyData *Clean = vtkCleanPolyData::New();
  Clean->SetInput(PolyData);
  Clean->SetTolerance(0.0);
  Clean->SetPointMerging(1);
  Clean->Update();

  output->DeepCopy(Clean->GetOutput());
  PolyData->Delete();
  Clean->Delete();

  return 1;
}
//---------------------------------------------------------------------------
unsigned long int vtkMimxExtractSurface::GetMTime()
{
  unsigned long mTime=this->Superclass::GetMTime();
  unsigned long time;

  if ( this->CellIdList != NULL )
    {
    time = this->CellIdList->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  if ( this->FaceIdList != NULL )
  {
          time = this->FaceIdList->GetMTime();
          mTime = ( time > mTime ? time : mTime );
  }
  return mTime;
}
//---------------------------------------------------------------------------
int vtkMimxExtractSurface::FillInputPortInformation(int, vtkInformation *info)
{
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
        return 1;
}
//----------------------------------------------------------------------------
void vtkMimxExtractSurface::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

