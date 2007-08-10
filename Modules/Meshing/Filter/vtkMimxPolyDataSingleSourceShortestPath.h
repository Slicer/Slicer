/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxPolyDataSingleSourceShortestPath.h,v $
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

// .NAME vtkMimxPolyDataSingleSourceShortestPath - Single source shortest path on a polygonal mesh
// .SECTION Description
//
// vtkMimxPolyDataSingleSourceShortestPath is a filter that takes as input
// a polygonal mesh and performs a single source shortest path
// calculation. Most of the code is used from the work of Rasmus Paulsen
// email:  rrp(a)imm.dtu.dk, web:    www.imm.dtu.dk/~rrp/VTK

// The output of the filter is a polydata describing the shortest path. The user has to input the
// polydata on which the path need to be found. the start and end vertex need to be specified.
// The input polydata should be a single continuous region.

#ifndef __vtkMimxPolyDataSingleSourceShortestPath_h__
#define __vtkMimxPolyDataSingleSourceShortestPath_h__

#include "vtkFilter.h"

#include "vtkPolyDataAlgorithm.h"

class vtkDoubleArray;
class vtkIntArray;
class vtkIdList;
class vtkFloatArray;
class vtkPolyData;

class VTK_MIMXFILTER_EXPORT vtkMimxPolyDataSingleSourceShortestPath : public vtkPolyDataAlgorithm
{
public:
        vtkTypeRevisionMacro(vtkMimxPolyDataSingleSourceShortestPath,vtkPolyDataAlgorithm);
        
        static vtkMimxPolyDataSingleSourceShortestPath *New();
        
        void PrintSelf(ostream& os, vtkIndent indent);
        
        // Description:
        // The vertex at the start of the shortest path
        vtkGetMacro(StartVertex, vtkIdType);
        vtkSetMacro(StartVertex, vtkIdType);
        
        // Description:
        // The vertex at the end of the shortest path
        vtkGetMacro(EndVertex, vtkIdType);
        vtkSetMacro(EndVertex, vtkIdType);
        
        // Description: 
        // Stop when the end vertex is reached 
        // or calculate shortest path to all vertices
        vtkSetMacro(StopWhenEndReached, int);
        vtkGetMacro(StopWhenEndReached, int);

        // Description:
        // To force the connection between the vertices to be constant (in this case 1.0)
        // else distance b/w two vertices is calculated.
        vtkSetMacro(EqualWeights, int);
        vtkGetMacro(EqualWeights, int);
        vtkBooleanMacro(EqualWeights, int);

        // Description:
        // Id list of points in the closest path
        vtkGetMacro(IdList, vtkIdList*);


protected:
        vtkMimxPolyDataSingleSourceShortestPath();
        ~vtkMimxPolyDataSingleSourceShortestPath();

        virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

        vtkMimxPolyDataSingleSourceShortestPath(const vtkMimxPolyDataSingleSourceShortestPath&);
        void operator=(const vtkMimxPolyDataSingleSourceShortestPath&);
                
        // Build a graph description of the mesh
        void BuildAdjacency(vtkPolyData *pd);
        
        void DeleteAdjacency();
        
        // The cost going from vertex u to v
        // TODO: should be implemented as a user supplied
        // callback function
        double EdgeCost(vtkPolyData *pd, vtkIdType u, vtkIdType v);
        
        void init(vtkPolyData*);
        
        // structure the heap
        void Heapify(int i);
        
        // insert vertex v in heap. Weight is in d(v)
        void HeapInsert(int v);
        
        // extract vertex with min d(v)
        int HeapExtractMin();
        
        // Update heap when key d(v) has been decreased
        void HeapDecreaseKey(int v);
        
        void InitSingleSource(int startv);
        
        // Calculate shortest path from vertex startv to vertex endv
        void ShortestPath(int startv, int endv, vtkPolyData*);
        
        // Relax edge u,v with weight w
        void Relax(int u, int v, double w);
        
        // Backtrace the shortest path
        void TraceShortestPath(vtkPolyData *inPd, vtkPolyData *outPd,
                                                         vtkIdType startv, vtkIdType endv);
        
        // the number of vertices
        int n;
        
        // d(v) current summed weight for path to vertex v
        vtkFloatArray *d;
        
        // pre(v) predecessor of v
        vtkIntArray *pre;
        
        // f is the set of vertices wich has not a shortest path yet but has a path
        // ie. the front set (f(v) == 1 means that vertex v is in f)
        vtkIntArray *f;
        
        // s is the set of vertices with allready determined shortest path
        // s(v) == 1 means that vertex v is in s
        vtkIntArray *s;
        
        // the priority que (a binary heap) with vertex indices
        vtkIntArray *H;
        
        // The real number of elements in H != H.size()
        int Hsize;
        
        // p(v) the position of v in H (p and H are kindoff inverses)
        vtkIntArray *p;
        
        // The vertex ids on the shortest path
        vtkIdList *IdList;
        
        // The vertex at the start of the shortest path
        vtkIdType StartVertex;
        
        // The vertex at the end of the shortest path
        vtkIdType EndVertex;
        
        // Adjacency representation
        vtkIdList **Adj;
        
        int StopWhenEndReached;
        
        int UseScalarWeights;

        int EqualWeights;

        // Used to remember the size of the graph. If the filter is re-used.
        int AdjacencyGraphSize;
};

#endif

