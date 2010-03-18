/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxGeometryFilter.cxx,v $
Language:  C++
Date:      $Date: 2008/08/14 05:01:51 $
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

#include "vtkMimxGeometryFilter.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkIdTypeArray.h"
#include "vtkExecutive.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkGeometryFilter.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxGeometryFilter, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkMimxGeometryFilter);

// Construct object to Set all of the input data.
vtkMimxGeometryFilter::vtkMimxGeometryFilter()
{
}

vtkMimxGeometryFilter::~vtkMimxGeometryFilter()
{
}

int vtkMimxGeometryFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        // get the info objects
        vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

        vtkInformation *outInfo = outputVector->GetInformationObject(0);
        
        vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
                inInfo->Get(vtkDataObject::DATA_OBJECT()));

        vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
        
        int numNodes = input->GetNumberOfPoints();
        int numCells = input->GetNumberOfCells();

        if(numNodes <= 0 || numCells <= 0){vtkErrorMacro("Invalid input");
        return 0;}

        int i, j, k;

        vtkIntArray *orArray = vtkIntArray::New();
        orArray->SetName("mimxCellIds");
        for (i=0; i< input->GetNumberOfCells(); i++)
        {
                orArray->InsertNextValue(i);
        }
        input->GetCellData()->AddArray(orArray);
        orArray->Delete();

        vtkGeometryFilter *geofil = vtkGeometryFilter::New();
        geofil->SetInput(input);
        geofil->Update();
        vtkPolyData *polydata = geofil->GetOutput();    
        vtkIntArray *faceNumArray = vtkIntArray::New();
        faceNumArray->SetName("Face_Numbers");
        faceNumArray->SetNumberOfValues(polydata->GetNumberOfCells());
        vtkIntArray *orIds = vtkIntArray::SafeDownCast(
                polydata->GetCellData()->GetArray("mimxCellIds"));
        
        for (i=0; i<polydata->GetNumberOfCells(); i++)
        {
                faceNumArray->SetValue(i, -1);
                vtkIdList *surfaceIds = vtkIdList::New();
                polydata->GetCellPoints(i, surfaceIds);
                int numFaces = input->GetCell(orIds->GetValue(i))->GetNumberOfFaces();
                for (j=0; j<numFaces; j++)
                {
                        vtkIdList *faceIds = 
                                input->GetCell(orIds->GetValue(i))->GetFace(j)->GetPointIds();
                        bool status = true;
                        for (k=0; k<faceIds->GetNumberOfIds(); k++)
                        {
                                if(surfaceIds->IsId(faceIds->GetId(k)) == -1)
                                {
                                        status = false;
                                        break;
                                }
                        }
                        if(status)
                        {
                                faceNumArray->SetValue(i, j);
                                break;
                        }
                }
                surfaceIds->Delete();
        }
        polydata->GetCellData()->AddArray(faceNumArray);
        output->DeepCopy(polydata);
        input->GetCellData()->RemoveArray("mimxCellIds");
        geofil->Delete();
        faceNumArray->Delete();
        return 1;
}

int vtkMimxGeometryFilter::FillInputPortInformation(int port,   vtkInformation *info)
{
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
        return 1;
}

void vtkMimxGeometryFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
