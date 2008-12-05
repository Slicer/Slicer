/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxPatchSurfaceMesh.cxx,v $
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

#include "vtkMimxPatchSurfaceMesh.h"

#include "vtkCellArray.h"
#include "vtkDelaunay2D.h"
#include "vtkGeometryFilter.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkPointLocator.h"
#include "vtkPoints.h"
#include "vtkPointSet.h"
#include "vtkPolyData.h"
#include "vtkPolyDataCollection.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxPatchSurfaceMesh, "$Revision: 1.7 $");
vtkStandardNewMacro(vtkMimxPatchSurfaceMesh);

// Construct with all types of clipping turned off.
vtkMimxPatchSurfaceMesh::vtkMimxPatchSurfaceMesh()
{
        this->ContourCollection = NULL;
}

vtkMimxPatchSurfaceMesh::~vtkMimxPatchSurfaceMesh()
{
        if (this->ContourCollection)
        {
                int numCol = this->ContourCollection->GetNumberOfItems();
                this->ContourCollection->InitTraversal();
                do 
                {
                        this->ContourCollection->GetNextItem()->Delete();
                        numCol--;
                } while(numCol != 0);
                this->ContourCollection->Delete();
        }
}

int vtkMimxPatchSurfaceMesh::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkGeometryFilter *pfilter = NULL;

  // if the input is an unstructured grid
        if(input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
                pfilter = vtkGeometryFilter::New();
                pfilter->SetInput(input);
                pfilter->Update();
      this->PolyDataExecute(pfilter->GetOutput(), output, outInfo);
          pfilter->Delete();
      return 1;
    }
        // if the input is a polydata
        this->PolyDataExecute(vtkPolyData::SafeDownCast(input), output, outInfo);
  return 1;
}


int vtkMimxPatchSurfaceMesh::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

void vtkMimxPatchSurfaceMesh::PrintSelf(ostream& os, vtkIndent indent)
{
 
}

void vtkMimxPatchSurfaceMesh::PolyDataExecute(vtkPolyData *input,
                                        vtkPolyData *output,
                                        vtkInformation *outInfo)
{
        input->BuildLinks();
        // to store the edge connectivity
        vtkIdList *StartNodeList = vtkIdList::New();
        vtkIdList *EndNodeList = vtkIdList::New();
        vtkIdList *idlist = vtkIdList::New();
        // where all the nodes which form the edge are stored
        vtkIdList *edgeidlist = vtkIdList::New();
        // list all the edges in the edgetable
        vtkIdType npts, *pts;
        for(int i=0; i<input->GetNumberOfCells();i++)
        {
                input->GetCellPoints(i,npts, pts);
                for(int j=0; j<npts-1; j++)
                {
                        idlist->Initialize();
                        input->GetCellEdgeNeighbors(i, pts[j], pts[j+1],idlist);
                        if(idlist->GetNumberOfIds() == 0)
                        {
                                if(!IsEdge(StartNodeList, EndNodeList, pts[j], pts[j+1]))
                                {
                                        //cout <<"invalid edge"<<endl;
                                        return;
                                }
                        }
                }
                input->GetCellEdgeNeighbors(i, pts[npts-1], pts[0],idlist);
                if(idlist->GetNumberOfIds() == 0)
                {
                        if(!IsEdge(StartNodeList, EndNodeList, pts[npts-1], pts[0]))
                        {
                                //cout <<"invalid edge"<<endl;
                                return;
                        }
                }
        }
        // there could be more than 1 contour generated
        // identify the contours
        // use an intarray to keep track of edges being used
        vtkIntArray *intarray = vtkIntArray::New();
        intarray->SetNumberOfValues(StartNodeList->GetNumberOfIds());
        for(int i=0; i<StartNodeList->GetNumberOfIds(); i++)
                intarray->SetValue(i,0);
        vtkIdType startpoint, endpoint, cellnum = 0,cellnumret;

        // to store the intermediate patch surface generated
        vtkPolyData *polydata = vtkPolyData::New();
        // to store points for output
        polydata->SetPoints(input->GetPoints());
        polydata->SetPolys(input->GetPolys());

        while (HaveAllEdgesUtilized(intarray) != -1)
        {
                bool status = false;
                vtkIdType numcurrpoints = polydata->GetNumberOfPoints();
                // to store points which in turn are fed into delaunay triangulation
                vtkPoints *points = vtkPoints::New();
                points->SetNumberOfPoints(1);
                points->SetPoint(0,input->GetPoint(StartNodeList->GetId(HaveAllEdgesUtilized(intarray))));
                // initialize the first edge in a contour
                startpoint = StartNodeList->GetId(HaveAllEdgesUtilized(intarray));
                endpoint = EndNodeList->GetId(HaveAllEdgesUtilized(intarray));
                intarray->SetValue(HaveAllEdgesUtilized(intarray),1);
                do
                {
                        cellnumret = GetCellContainingPoint(cellnum, StartNodeList, endpoint);
                        if(cellnumret == -1)
                        {
                                cellnumret = GetCellContainingPoint(cellnum, EndNodeList, endpoint);
                                if (cellnumret == -1)
                                {
                                        return ;
                                }
                                points->InsertNextPoint(input->GetPoint(EndNodeList->GetId(cellnumret)));
                                polydata->GetPoints()->InsertNextPoint(input->GetPoint(EndNodeList->GetId(cellnumret)));
                                endpoint = StartNodeList->GetId(cellnumret);
                        }
                        else{
                                points->InsertNextPoint(input->GetPoint(StartNodeList->GetId(cellnumret)));
                                polydata->GetPoints()->InsertNextPoint(input->GetPoint(EndNodeList->GetId(cellnumret)));
                                endpoint = EndNodeList->GetId(cellnumret);
                        }
                        if(!intarray->GetValue(cellnumret))
                                intarray->SetValue(cellnumret,1);
                        else
                                status = true;
                        cellnum = cellnumret;
                } while(!status);

                vtkPointSet *pointset = vtkPolyData::New();
                pointset->SetPoints(points);
                vtkDelaunay2D *del2d = vtkDelaunay2D::New();
                del2d->SetInput(pointset);
                del2d->Update();
                del2d->GetOutput()->GetPolys()->InitTraversal();
                while (del2d->GetOutput()->GetPolys()->GetNextCell(npts, pts))
                {
                        polydata->GetPolys()->InsertNextCell(npts);
                        for (int j=0; j<npts; j++)
                        {
                                polydata->GetPolys()->InsertCellPoint(pts[j]+numcurrpoints);
                        }
                }
                // store the contour as a polydata
                if(!this->ContourCollection)
                {
                        this->ContourCollection = vtkPolyDataCollection::New();
                }
                this->ContourCollection->AddItem(vtkPolyData::New());
                vtkPolyData::SafeDownCast(this->ContourCollection->GetItemAsObject(this->ContourCollection->GetNumberOfItems()-1))->SetPoints(points);
                vtkCellArray *cellarray = vtkCellArray::New();
                for(int i=0; i<points->GetNumberOfPoints()-1; i++)
                {
                        cellarray->InsertNextCell(2);
                        cellarray->InsertCellPoint(i);
                        cellarray->InsertCellPoint(i+1);
                }
                cellarray->InsertNextCell(2);
                cellarray->InsertCellPoint(points->GetNumberOfPoints()-1);
                cellarray->InsertCellPoint(0);
                vtkPolyData::SafeDownCast(this->ContourCollection->GetItemAsObject(this->ContourCollection->GetNumberOfItems()-1))->SetLines(cellarray);
                cellarray->Delete();
                points->Delete();
                pointset->Delete();
                del2d->Delete();
        }
        this->NumberOfContours = this->ContourCollection->GetNumberOfItems();
        // merge duplicate points
        vtkPointLocator *locator = vtkPointLocator::New();
        vtkPoints *points = vtkPoints::New();
        points->Allocate(polydata->GetNumberOfPoints());
        output->Allocate(polydata->GetNumberOfCells(), polydata->GetNumberOfCells());
        locator->InitPointInsertion (points, polydata->GetPoints()->GetBounds());
        vtkIdList *idlistpoly;
        double x[3];
        vtkIdType tempid;
        for(int i=0; i < polydata->GetNumberOfCells(); i++)
        {
                idlistpoly = polydata->GetCell(i)->GetPointIds();
                for(int j=0; j < idlistpoly->GetNumberOfIds(); j++)
                {
                        polydata->GetPoints()->GetPoint(idlistpoly->GetId(j),x);                 
                        locator->InsertUniquePoint(x,tempid);
                        idlistpoly->SetId(j,tempid);
                }
                output->InsertNextCell(polydata->GetCellType(i), idlistpoly);
        }
        points->Squeeze();
        output->SetPoints(points);
        output->Squeeze();
        points->Delete();
        locator->Delete();
        StartNodeList->Delete();
        EndNodeList->Delete();
        idlist->Delete();
        intarray->Delete();
        polydata->Delete();
}

int vtkMimxPatchSurfaceMesh::IsEdge(vtkIdList *startnodelist, vtkIdList *endnodelist, vtkIdType pt1, vtkIdType pt2)
{
        for (int i=0; i<startnodelist->GetNumberOfIds(); i++)
        {
                if(startnodelist->GetId(i) == pt1 || startnodelist->GetId(i) == pt2)
                {
                        if(endnodelist->GetId(i) == pt1 || endnodelist->GetId(i) == pt2)
                                return 0;
                }

        }
        startnodelist->InsertNextId(pt1);
        endnodelist->InsertNextId(pt2);
        return 1;
}

int vtkMimxPatchSurfaceMesh::GetCellContainingPoint(vtkIdType cellnum, vtkIdList *nodelist, vtkIdType pt)
{
        for (int i=0; i<nodelist->GetNumberOfIds(); i++)
        {
                if(cellnum != i)
                {
                        if(nodelist->GetId(i) == pt) return i;
                }
        }
        return -1;
}

int vtkMimxPatchSurfaceMesh::HaveAllEdgesUtilized(vtkIntArray *arr)
{
        for(int i=0; i <arr->GetSize(); i++)
        {
                if(!arr->GetValue(i))   return i;
        }
        return -1;
}

vtkPolyData* vtkMimxPatchSurfaceMesh::GetContour(int ContourNum)
{
        if(ContourNum >= 0 && ContourNum < this->ContourCollection->GetNumberOfItems())
        {
                return vtkPolyData::SafeDownCast(this->ContourCollection->GetItemAsObject(ContourNum));
        }
        else{
                vtkErrorMacro("Invalid contour number");
                return NULL;
        }
}
