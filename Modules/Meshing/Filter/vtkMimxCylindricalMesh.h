/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxCylindricalMesh.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
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

// .NAME vtkMimxCylindricalMesh - appends one or more datasets together into a single unstructured grid
// .SECTION Description
// vtkMimxCylindricalMesh is a filter that appends one of more datasets into a single
// unstructured grid. All geometry is extracted and appended, but point 
// attributes (i.e., scalars, vectors, normals, field data, etc.) are extracted 
// and appended only if all datasets have the point attributes available. 
// (For example, if one dataset has scalars but another does not, scalars will 
// not be appended.)

// .SECTION See Also
// vtkAppendPolyData

#ifndef __vtkMimxCylindricalMesh_h
#define __vtkMimxCylindricalMesh_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridAlgorithm.h"

class vtkCellLocator;
class vtkClosestPointProjection;
class vtkDataSetCollection;
class vtkPlaneWidget;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataCollection;
class vtkStructuredGrid;


class VTK_MIMXFILTER_EXPORT vtkMimxCylindricalMesh : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkMimxCylindricalMesh *New();

  vtkTypeRevisionMacro(vtkMimxCylindricalMesh,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
 /* int SetSuperiorDividingPoints(vtkPoints *Points);
  int SetInferiorDividingPoints(vtkPoints *Points);*/
  //int SetMeshSeed(int MSeed[4]);
  void GetStructuredGrid(vtkStructuredGrid* SGrid);
  void SetCenter(double X[3]);
  void SetSource(vtkPolyData* PolyData);
  void SetCurve(int CurveNum, vtkPolyData* PolyData);
  void ReverseEdgeDirection(int EdgeNum);
  void SetEdgeScalars(int EdgeNum, int ScalarValue);
  void SetFaceScalars(int FaeNum, int ScalarValue);
  vtkSetMacro(Plane1, vtkPlaneWidget*);
  vtkSetMacro(Plane2, vtkPlaneWidget*);
  vtkSetMacro(Plane3, vtkPlaneWidget*);
  vtkSetMacro(Plane4, vtkPlaneWidget*);
  void SetBoundingBox(vtkUnstructuredGrid *BoundingBox);
  enum Constraint
  {
          Edge = 0,
          OuterFace
  };
protected:
  vtkMimxCylindricalMesh();
  ~vtkMimxCylindricalMesh();

  // Usual data generation method
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  virtual int FillInputPortInformation(int port, vtkInformation *info);
 /* vtkPoints *SuperiorDividingPoints;
  vtkPoints *InferiorDividingPoints;*/
  vtkStructuredGrid *StructuredSolidGrid[9];
  vtkStructuredGrid *StructuredPlanarGrid[40];
  double Center[3];
  vtkPolyData *EdgePolyData[60];
  void EdgeSubdivideProject(int EdgeNum, int NumDiv, vtkCellLocator* Locator);
  void GenerateUnknownEdge(int EdgeNum, int FirstEdge, int SecondEdge, int BeginOrEnd);
   void GenerateUnknownEdgeFromPoints(int EdgeNum, double *FirstPoint, double *SecondPoint);
  void GenerateFaceProject(int FaceNum, int Edge0, int Edge1, int Edge2,
          int Edge3, vtkCellLocator* Locator);
  int MeshSeed[5];
  void ClosestPointProjection(vtkPoints *Points, vtkCellLocator *Locator);
  void GenerateSolidMesh(int BoxNum, int FaceArray[6]);
  void DivideCurve(int CurveNum, vtkPolyData *PolyData, vtkUnstructuredGrid *UGrid);
  void MergeSGridToUGrid(vtkPolyData *Source);
  double MeasureCurveLength(vtkPolyData *PolyData, int CalculateClosedCurve);
  vtkPlaneWidget *Plane1, *Plane2, *Plane3, *Plane4;
  double SupCenter[3];
  double InfCenter[3];
  void CalculateCenter(vtkPolyData *curve);
 private:
  vtkMimxCylindricalMesh(const vtkMimxCylindricalMesh&);  // Not implemented.
  void operator=(const vtkMimxCylindricalMesh&);  // Not implemented.
};


#endif


