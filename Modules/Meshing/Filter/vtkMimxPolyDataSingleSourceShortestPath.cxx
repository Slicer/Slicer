/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxPolyDataSingleSourceShortestPath.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
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

#include "vtkMimxPolyDataSingleSourceShortestPath.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkFloatArray.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"

vtkCxxRevisionMacro(vtkMimxPolyDataSingleSourceShortestPath, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkMimxPolyDataSingleSourceShortestPath);

//----------------------------------------------------------------------------
vtkMimxPolyDataSingleSourceShortestPath::vtkMimxPolyDataSingleSourceShortestPath()
{
        this->IdList = vtkIdList::New();
        this->d          = vtkFloatArray::New();
        this->pre        = vtkIntArray::New();
        this->f          = vtkIntArray::New();
        this->s          = vtkIntArray::New();
        this->H          = vtkIntArray::New();
        this->p          = vtkIntArray::New();
        this->Hsize  = 0;
        this->StartVertex = 0;
        this->EndVertex   = 0;  
        this->StopWhenEndReached = 0;
        this->UseScalarWeights = 0;
        this->Adj = NULL;
        this->n = 0;
        this->AdjacencyGraphSize = 0;
        this->EqualWeights = 0;
}


vtkMimxPolyDataSingleSourceShortestPath::~vtkMimxPolyDataSingleSourceShortestPath()
{
        if (this->IdList)
                this->IdList->Delete();
        if (this->d)
                this->d->Delete();
        if (this->pre)
                this->pre->Delete();
        if (this->f)
                this->f->Delete();
        if (this->s)
                this->s->Delete();
        if (this->H)
                this->H->Delete();
        if (this->p)
                this->p->Delete();

        DeleteAdjacency();
}



int vtkMimxPolyDataSingleSourceShortestPath::RequestData(
                vtkInformation *vtkNotUsed(request),
                vtkInformationVector **inputVector,
                vtkInformationVector *outputVector)
{
        // get the info objects
        vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
        vtkInformation *outInfo = outputVector->GetInformationObject(0);

        vtkPolyData *input;
        if(inInfo)      input = vtkPolyData::SafeDownCast(
                inInfo->Get(vtkDataObject::DATA_OBJECT()));
        else
        {
                vtkErrorMacro("Input polydata needs to be set");
                return 0;
        }
        vtkIdType numPts, numCells;
        numPts = input->GetNumberOfPoints();
        numCells = input->GetNumberOfCells();
        if(numPts < 2 || numCells < 1){vtkErrorMacro("Not enough number of input points or cells");
                                                                        return 0;}
        input->BuildLinks();
        vtkIdType *cells;
        unsigned short ncells;
        input->GetPointCells(this->StartVertex, ncells, cells);
        if(ncells == 0){        vtkErrorMacro("invalid start vertex"); return 0;}
        input->GetPointCells(this->EndVertex, ncells, cells);
        if(ncells == 0){        vtkErrorMacro("invalid end vertex"); return 0;}

        vtkDebugMacro(<< "vtkPolyDataSingleSourceShortestPath finding shortest path");

        init(input);

        ShortestPath(this->StartVertex, this->EndVertex, input);
        
        vtkPolyData *output = vtkPolyData::SafeDownCast(
                outInfo->Get(vtkDataObject::DATA_OBJECT()));

        TraceShortestPath(input, output, this->StartVertex, this->EndVertex);
        return 1;
}

void vtkMimxPolyDataSingleSourceShortestPath::init(vtkPolyData *input)
{
        BuildAdjacency(input);

        IdList->Reset();

        this->n = input->GetNumberOfPoints();

        this->d->SetNumberOfComponents(1);
        this->d->SetNumberOfTuples(this->n);
        this->pre->SetNumberOfComponents(1);
        this->pre->SetNumberOfTuples(this->n);
        this->f->SetNumberOfComponents(1);
        this->f->SetNumberOfTuples(this->n);
        this->s->SetNumberOfComponents(1);
        this->s->SetNumberOfTuples(this->n);
        this->p->SetNumberOfComponents(1);
        this->p->SetNumberOfTuples(this->n);

        // The heap has elements from 1 to n
        this->H->SetNumberOfComponents(1);
        this->H->SetNumberOfTuples(this->n+1);

        Hsize = 0;
}

void vtkMimxPolyDataSingleSourceShortestPath::DeleteAdjacency()
{
        const int npoints = this->AdjacencyGraphSize;

        if (this->Adj)
        {
                for (int i = 0; i < npoints; i++)
                {
                        this->Adj[i]->Delete();
                }
                delete [] this->Adj;
        }
        this->Adj = NULL;
}

// The edge cost function should be implemented as a callback function to
// allow more advanced weighting
double vtkMimxPolyDataSingleSourceShortestPath::EdgeCost(vtkPolyData *pd, vtkIdType u, vtkIdType v)
{
                double p1[3];
                pd->GetPoint(u,p1);
                double p2[3];
                pd->GetPoint(v,p2);
        double w;
        if( !this->EqualWeights){
                w = sqrt(vtkMath::Distance2BetweenPoints(p1, p2));}
        else w = 1.0;
                
        return w;
}


// This is probably a horribly inefficient way to do it.
void vtkMimxPolyDataSingleSourceShortestPath::BuildAdjacency(vtkPolyData *pd)
{
        int i;

        int npoints = pd->GetNumberOfPoints();
        int ncells = pd->GetNumberOfCells();

        DeleteAdjacency();

        this->Adj = new vtkIdList*[npoints];

        // Remember size, so it can be deleted again
        this->AdjacencyGraphSize = npoints;

        for (i = 0; i < npoints; i++)
        {
                this->Adj[i] = vtkIdList::New();
        }

        for (i = 0; i < ncells; i++)
        {
                // Possible types
                //              VTK_VERTEX, VTK_POLY_VERTEX, VTK_LINE, VTK_POLY_LINE,VTK_TRIANGLE, VTK_QUAD, VTK_POLYGON, or VTK_TRIANGLE_STRIP.

                vtkIdType ctype = pd->GetCellType(i);

                // Until now only handle polys and triangles
                // TODO: All types
                if (ctype == VTK_POLYGON || ctype == VTK_TRIANGLE || ctype == VTK_LINE || ctype == VTK_QUAD)
                {
                        vtkIdType *pts;
                        vtkIdType npts;
                        pd->GetCellPoints (i, npts, pts);

                        vtkIdType u = pts[0];
                        vtkIdType v = pts[npts-1];

                        Adj[u]->InsertUniqueId(v);
                        Adj[v]->InsertUniqueId(u);
                        for (int j = 0; j < npts-1; j++)
                        {
                                vtkIdType u = pts[j];
                                vtkIdType v = pts[j+1];
                                Adj[u]->InsertUniqueId(v);
                                Adj[v]->InsertUniqueId(u);
                        }
                }
        }
}

void vtkMimxPolyDataSingleSourceShortestPath::TraceShortestPath(
        vtkPolyData *inPd, 
        vtkPolyData *outPd,
        vtkIdType startv, vtkIdType endv)
{
        vtkPoints *points = vtkPoints::New();
        vtkCellArray *lines = vtkCellArray::New();

        // n is far to many. Adjusted later
        lines->InsertNextCell(this->n);

        // trace backward
        int npoints = 0;
        int v = endv;
        double pt[3];
        vtkIdType id;
        while (v != startv)
        {
                IdList->InsertNextId(v);
//              //cout << v<<endl;              
                inPd->GetPoint(v,pt);
                id = points->InsertNextPoint(pt);
                lines->InsertCellPoint(id);
                npoints++;

                v = this->pre->GetValue(v);
        }
        IdList->InsertNextId(v);
        
        vtkIdList *tempid = vtkIdList::New();
        tempid->DeepCopy(IdList);
        IdList->Initialize();
        IdList->SetNumberOfIds(tempid->GetNumberOfIds());
        for (int i=0; i<tempid->GetNumberOfIds(); i++)
        {
                IdList->SetId(tempid->GetNumberOfIds()-1-i, tempid->GetId(i));
        }

        inPd->GetPoint(v,pt);
        id = points->InsertNextPoint(pt);
        lines->InsertCellPoint(id);
        npoints++;

        lines->UpdateCellCount(npoints);
        outPd->SetPoints(points);
        points->Delete();
        outPd->SetLines(lines);
        lines->Delete();
}


void vtkMimxPolyDataSingleSourceShortestPath::InitSingleSource(int startv)
{
        for (int v = 0; v < this->n; v++)
        {
                // d will be updated with first visit of vertex
                this->d->SetValue(v, -1);
                this->pre->SetValue(v, -1);
                this->s->SetValue(v, 0);
                this->f->SetValue(v, 0);
        }

        this->d->SetValue(startv, 0);
}


void vtkMimxPolyDataSingleSourceShortestPath::Relax(int u, int v, double w)
{
        if (this->d->GetValue(v) > this->d->GetValue(u) + w)
        {
                this->d->SetValue(v, this->d->GetValue(u) + w);
                this->pre->SetValue(v, u);

                HeapDecreaseKey(v);
        }
}

void vtkMimxPolyDataSingleSourceShortestPath::ShortestPath(int startv, int endv, vtkPolyData *input)
{
        int i, u, v;

        InitSingleSource(startv);

        HeapInsert(startv);
        

        this->f->SetValue(startv, 1);

        

        int stop = 0;
        while ((u = HeapExtractMin()) >= 0 && !stop)
        {
                // u is now in s since the shortest path to u is determined
                this->s->SetValue(u, 1);
                

                // remove u from the front set
                this->f->SetValue(u, 0);
                

                if (u == endv && StopWhenEndReached)
                        stop = 1;

                // Update all vertices v adjacent to u
                for (i = 0; i < Adj[u]->GetNumberOfIds(); i++)
                {
                        v = Adj[u]->GetId(i);

                        // s is the set of vertices with determined shortest path...do not use them again
                        if (!this->s->GetValue(v))
                        {
                                // Only relax edges where the end is not in s and edge is in the front set
                                double w = EdgeCost(input, u, v);

                                if (this->f->GetValue(v))
                                {
                                        Relax(u, v, w);
                                        
                                }
                                // add edge v to front set
                                else
                                {
                                        this->f->SetValue(v, 1);
                                        this->d->SetValue(v, this->d->GetValue(u) + w);

                                        // Set Predecessor of v to be u
                                        this->pre->SetValue(v, u);
                                                                        
                                        HeapInsert(v);
                                        
                                }
                        }
                }
        }
}


void vtkMimxPolyDataSingleSourceShortestPath::Heapify(int i)
{
        // left node
        int l = i * 2;

        // right node
        int r = i * 2 + 1;

        int smallest = -1;

        // The value of element v is d(v)
        // the heap stores the vertex numbers
        if (l <= Hsize && this->d->GetValue(this->H->GetValue(l)) < this->d->GetValue(this->H->GetValue(i)))
                smallest = l;
        else
                smallest = i;

        if (r <= Hsize && this->d->GetValue(this->H->GetValue(r))< this->d->GetValue(this->H->GetValue(smallest)))
                smallest = r;

        if (smallest != i)
        {
                int t = this->H->GetValue(i);

                this->H->SetValue(i, this->H->GetValue(smallest));

                // where is H(i)
                this->p->SetValue(this->H->GetValue(i), i);

                // H and p is kinda inverse
                this->H->SetValue(smallest, t);
                this->p->SetValue(t, smallest);

                Heapify(smallest);
        }
}

// Insert vertex v. Weight is given in d(v)
// H has indices 1..n
void vtkMimxPolyDataSingleSourceShortestPath::HeapInsert(int v)
{
        if (Hsize >= this->H->GetNumberOfTuples()-1)
                return;

        Hsize++;
        int i = Hsize;

        while (i > 1 && this->d->GetValue(this->H->GetValue(i/2)) > this->d->GetValue(v))
        {
                this->H->SetValue(i, this->H->GetValue(i/2));
                this->p->SetValue(this->H->GetValue(i), i);
                i /= 2;
        }
        // H and p is kinda inverse
        this->H->SetValue(i, v);
        this->p->SetValue(v, i);
}

int vtkMimxPolyDataSingleSourceShortestPath::HeapExtractMin()
{
        if (Hsize == 0)
                return -1;

        int minv = this->H->GetValue(1);
        this->p->SetValue(minv, -1);

        this->H->SetValue(1, this->H->GetValue(Hsize));
        this->p->SetValue(this->H->GetValue(1), 1);

        Hsize--;
        Heapify(1);

        return minv;
}

void vtkMimxPolyDataSingleSourceShortestPath::HeapDecreaseKey(int v)
{
        // where in H is vertex v
        int i = this->p->GetValue(v);
        if (i < 1 || i > Hsize)
                return;

        while (i > 1 && this->d->GetValue(this->H->GetValue(i/2)) > this->d->GetValue(v))
        {
                this->H->SetValue(i, this->H->GetValue(i/2));
                this->p->SetValue(this->H->GetValue(i), i);
                i /= 2;
        }

        // H and p is kinda inverse
        this->H->SetValue(i, v);
        this->p->SetValue(v, i);
}


//void vtkMimxPolyDataSingleSourceShortestPath::Print()
//{
//      ofstream Fileoutput;
//      Fileoutput.open("debug.txt");
//      int i;
//
//      Fileoutput <<"d"<<endl;
//      for(i = 0; i< this->d->GetNumberOfTuples(); i++)
//      {
//              Fileoutput << this->d->GetValue(i)<<"  ";
//      }
//      Fileoutput <<endl;
//
//      Fileoutput <<"pre"<<endl;
//      for(i = 0; i< this->pre->GetNumberOfTuples(); i++)
//      {
//              Fileoutput << this->pre->GetValue(i)<<"  ";
//      }
//      Fileoutput <<endl;
//
//      Fileoutput <<"f"<<endl;
//      for(i = 0; i< this->f->GetNumberOfTuples(); i++)
//      {
//              Fileoutput << this->f->GetValue(i)<<"  ";
//      }
//      Fileoutput <<endl;
//
//      Fileoutput <<"s"<<endl;
//      for(i = 0; i< this->s->GetNumberOfTuples(); i++)
//      {
//              Fileoutput << this->s->GetValue(i)<<"  ";
//      }
//      Fileoutput <<endl;
//
//      Fileoutput <<"H"<<endl;
//      for(i = 0; i< this->H->GetNumberOfTuples(); i++)
//      {
//              Fileoutput << this->H->GetValue(i)<<"  ";
//      }
//      Fileoutput <<endl;
//
//      Fileoutput <<"p"<<endl;
//      for(i = 0; i< this->p->GetNumberOfTuples(); i++)
//      {
//              Fileoutput << this->p->GetValue(i)<<"  ";
//      }
//      Fileoutput <<endl;
//
//}
void vtkMimxPolyDataSingleSourceShortestPath::PrintSelf(ostream& os, vtkIndent indent)
{
        this->Superclass::PrintSelf(os,indent);

}
