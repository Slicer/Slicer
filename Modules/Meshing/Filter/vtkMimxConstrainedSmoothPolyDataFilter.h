/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxConstrainedSmoothPolyDataFilter.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.6 $

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

// .NAME vtkMimxConstrainedSmoothPolyDataFilter - adjust point positions using Laplacian smoothing
// .SECTION Description
// vtkMimxConstrainedSmoothPolyDataFilter is vtkSmoothPolyDataFilter
// with additional of feature of user specifying which nodes to be constrained
// during smoothing operation. The constraint can be input through a scalar field
// variable for every point in the mesh. 0 represents that the point is fixed and
// 1 point can be moved during smoothing. All other constraints of the 
// vtkSmoothPolyDataFilter hold true.

//
// .SECTION Caveats
// 
// The Laplacian operation reduces high frequency information in the geometry
// of the mesh. With excessive smoothing important details may be lost, and
// the surface may shrink towards the centroid. Enabling FeatureEdgeSmoothing
// helps reduce this effect, but cannot entirely eliminate it. You may also
// wish to try vtkWindowedSincPolyDataFilter. It does a better job of 
// minimizing shrinkage.


#ifndef __vtkMimxConstrainedSmoothPolyDataFilter_h
#define __vtkMimxConstrainedSmoothPolyDataFilter_h

#include "vtkFilter.h"

#include "vtkPolyDataAlgorithm.h"

class vtkMimxSmoothPoints;

class VTK_MIMXFILTER_EXPORT vtkMimxConstrainedSmoothPolyDataFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkMimxConstrainedSmoothPolyDataFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with number of iterations 20; relaxation factor .01;
  // feature edge smoothing turned off; feature 
  // angle 45 degrees; edge angle 15 degrees; and boundary smoothing turned 
  // on. Error scalars and vectors are not generated (by default). The 
  // convergence criterion is 0.0 of the bounding box diagonal.
  static vtkMimxConstrainedSmoothPolyDataFilter *New();

  // Description:
  // Specify a convergence criterion for the iteration
  // process. Smaller numbers result in more smoothing iterations.
  vtkSetClampMacro(Convergence,double,0.0,1.0);
  vtkGetMacro(Convergence,double);

  // Description:
  // Specify the number of iterations for Laplacian smoothing,
  vtkSetClampMacro(NumberOfIterations,int,0,VTK_LARGE_INTEGER);
  vtkGetMacro(NumberOfIterations,int);

  // Description:
  // Specify the relaxation factor for Laplacian smoothing. As in all
  // iterative methods, the stability of the process is sensitive to
  // this parameter. In general, small relaxation factors and large
  // numbers of iterations are more stable than larger relaxation
  // factors and smaller numbers of iterations.
  vtkSetMacro(RelaxationFactor,double);
  vtkGetMacro(RelaxationFactor,double);

  // Description:
  // Turn on/off smoothing along sharp interior edges.
  vtkSetMacro(FeatureEdgeSmoothing,int);
  vtkGetMacro(FeatureEdgeSmoothing,int);
  vtkBooleanMacro(FeatureEdgeSmoothing,int);

  // Description:
  // Specify the feature angle for sharp edge identification.
  vtkSetClampMacro(FeatureAngle,double,0.0,180.0);
  vtkGetMacro(FeatureAngle,double);

  // Description:
  // Specify the edge angle to control smoothing along edges (either interior
  // or boundary).
  vtkSetClampMacro(EdgeAngle,double,0.0,180.0);
  vtkGetMacro(EdgeAngle,double);

  // Description:
  // Turn on/off the smoothing of vertices on the boundary of the mesh.
  vtkSetMacro(BoundarySmoothing,int);
  vtkGetMacro(BoundarySmoothing,int);
  vtkBooleanMacro(BoundarySmoothing,int);

  // Description:
  // Turn on/off the generation of scalar distance values.
  vtkSetMacro(GenerateErrorScalars,int);
  vtkGetMacro(GenerateErrorScalars,int);
  vtkBooleanMacro(GenerateErrorScalars,int);

  // Description:
  // Turn on/off the generation of error vectors.
  vtkSetMacro(GenerateErrorVectors,int);
  vtkGetMacro(GenerateErrorVectors,int);
  vtkBooleanMacro(GenerateErrorVectors,int);

  // Description:
  // Specify the source object which is used to constrain smoothing. The 
  // source defines a surface that the input (as it is smoothed) is 
  // constrained to lie upon.
  void SetSource(vtkPolyData *source);
  vtkPolyData *GetSource();
  
protected:
  vtkMimxConstrainedSmoothPolyDataFilter();
  ~vtkMimxConstrainedSmoothPolyDataFilter() {};

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

  double Convergence;
  int NumberOfIterations;
  double RelaxationFactor;
  int FeatureEdgeSmoothing;
  double FeatureAngle;
  double EdgeAngle;
  int BoundarySmoothing;
  int GenerateErrorScalars;
  int GenerateErrorVectors;

  vtkMimxSmoothPoints *SmoothPoints;
private:
  vtkMimxConstrainedSmoothPolyDataFilter(const vtkMimxConstrainedSmoothPolyDataFilter&);  // Not implemented.
  void operator=(const vtkMimxConstrainedSmoothPolyDataFilter&);  // Not implemented.
};

#endif
