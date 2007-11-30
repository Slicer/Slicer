/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkPreciseHyperStreamline.h,v $
  Date:      $Date: 2006/03/06 21:07:33 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkPreciseHyperStreamline - generate hyperstreamline in arbitrary dataset
// .SECTION Description
// vtkPreciseHyperStreamline is a filter that integrates through a tensor field to 
// generate a hyperstreamline. The integration is along the maximum eigenvector
// and the cross section of the hyperstreamline is defined by the two other
// eigenvectors. Thus the shape of the hyperstreamline is "tube-like", with 
// the cross section being elliptical. PreciseHyperstreamlines are used to visualize
// tensor fields.
//
// The starting point of a hyperstreamline can be defined in one of two ways. 
// First, you may specify an initial position. This is a x-y-z global 
// coordinate. The second option is to specify a starting location. This is 
// cellId, subId, and  cell parametric coordinates.
//
// The integration of the hyperstreamline occurs through the major eigenvector 
// field. IntegrationStepLength controls the step length within each cell 
// (i.e., this is the fraction of the cell length). The length of the 
// hyperstreamline is controlled by MaximumPropagationDistance. This parameter
// is the length of the hyperstreamline in units of distance. The tube itself 
// is composed of many small sub-tubes - NumberOfSides controls the number of 
// sides in the tube, and StepLength controls the length of the sub-tubes.
//
// Because hyperstreamlines are often created near regions of singularities, it
// is possible to control the scaling of the tube cross section by using a 
// logarithmic scale. Use LogScalingOn to turn this capability on. The Radius 
// value controls the initial radius of the tube.

// .SECTION See Also
// vtkTensorGlyph vtkStreamer

#ifndef __vtkPreciseHyperStreamline_h
#define __vtkPreciseHyperStreamline_h

#include "vtkTeemConfigure.h"

#include "vtkDataSetToPolyDataFilter.h"
#include "vtkInitialValueProblemSolver.h" // to get the function set

#define VTK_INTEGRATE_FORWARD 0
#define VTK_INTEGRATE_BACKWARD 1
#define VTK_INTEGRATE_BOTH_DIRECTIONS 2

#define VTK_INTEGRATE_MAJOR_EIGENVECTOR 0
#define VTK_INTEGRATE_MEDIUM_EIGENVECTOR 1
#define VTK_INTEGRATE_MINOR_EIGENVECTOR 2


class vtkPreciseHyperArray;

class VTK_TEEM_EXPORT vtkPreciseHyperStreamline : public vtkDataSetToPolyDataFilter
{
 public:
  vtkTypeRevisionMacro(vtkPreciseHyperStreamline,vtkDataSetToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with initial starting position (0,0,0); integration
  // step length 0.2; step length 0.01; forward integration; terminal
  // eigenvalue 0.0; number of sides 6; radius 0.5; and logarithmic scaling
  // off.
  static vtkPreciseHyperStreamline *New();

  // Description:
  // Specify the start of the hyperstreamline in the cell coordinate system. 
  // That is, cellId and subId (if composite cell), and parametric coordinates.
  void SetStartLocation(vtkIdType cellId, int subId, vtkFloatingPointType pcoords[3]);

  // Description:
  // Specify the start of the hyperstreamline in the cell coordinate system. 
  // That is, cellId and subId (if composite cell), and parametric coordinates.
  void SetStartLocation(vtkIdType cellId, int subId, vtkFloatingPointType r, vtkFloatingPointType s,
                        vtkFloatingPointType t);

  // Description:
  // Get the starting location of the hyperstreamline in the cell coordinate
  // system. Returns the cell that the starting point is in.
  vtkIdType GetStartLocation(int& subId, vtkFloatingPointType pcoords[3]);

  // Description:
  // Specify the start of the hyperstreamline in the global coordinate system. 
  // Starting from position implies that a search must be performed to find 
  // initial cell to start integration from.
  void SetStartPosition(vtkFloatingPointType x[3]);

  // Description:
  // Specify the start of the hyperstreamline in the global coordinate system. 
  // Starting from position implies that a search must be performed to find 
  // initial cell to start integration from.
  void SetStartPosition(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z);

  // Description:
  // Get the start position of the hyperstreamline in global x-y-z coordinates.
  vtkFloatingPointType *GetStartPosition();

  // Description:
  // Set / get the maximum length of the hyperstreamline expressed as absolute
  // distance (i.e., arc length) value.
  vtkSetClampMacro(MaximumPropagationDistance,vtkFloatingPointType,0.0,VTK_LARGE_FLOAT);
  vtkGetMacro(MaximumPropagationDistance,vtkFloatingPointType);

  // Description:
  // Set / get the maximum length of the hyperstreamline expressed as absolute
  // distance (i.e., arc length) value.
  vtkSetClampMacro(MinimumPropagationDistance,vtkFloatingPointType,0.0,VTK_LARGE_FLOAT);
  vtkGetMacro(MinimumPropagationDistance,vtkFloatingPointType);

  // Description:
  // Set / get the eigenvector field through which to ingrate. It is
  // possible to integrate using the major, medium or minor
  // eigenvector field.  The major eigenvector is the eigenvector
  // whose corresponding eigenvalue is closest to positive infinity.
  // The minor eigenvector is the eigenvector whose corresponding
  // eigenvalue is closest to negative infinity.  The medium
  // eigenvector is the eigenvector whose corresponding eigenvalue is
  // between the major and minor eigenvalues.
  vtkSetClampMacro(IntegrationEigenvector,int,
                   VTK_INTEGRATE_MAJOR_EIGENVECTOR,
                   VTK_INTEGRATE_MINOR_EIGENVECTOR);
  vtkGetMacro(IntegrationEigenvector,int);
  void SetIntegrationEigenvectorToMajor()
    {this->SetIntegrationEigenvector(VTK_INTEGRATE_MAJOR_EIGENVECTOR);};
  void SetIntegrationEigenvectorToMedium()
    {this->SetIntegrationEigenvector(VTK_INTEGRATE_MEDIUM_EIGENVECTOR);};
  void SetIntegrationEigenvectorToMinor()
    {this->SetIntegrationEigenvector(VTK_INTEGRATE_MINOR_EIGENVECTOR);};

  // Description:
  // Use the major eigenvector field as the vector field through which
  // to integrate.  The major eigenvector is the eigenvector whose
  // corresponding eigenvalue is closest to positive infinity.  
  void IntegrateMajorEigenvector()
    {this->SetIntegrationEigenvectorToMajor();};

  // Description:
  // Use the medium eigenvector field as the vector field through which
  // to integrate. The medium eigenvector is the eigenvector whose
  // corresponding eigenvalue is between the major and minor
  // eigenvalues.
  void IntegrateMediumEigenvector()
    {this->SetIntegrationEigenvectorToMedium();};

  // Description:
  // Use the minor eigenvector field as the vector field through which
  // to integrate. The minor eigenvector is the eigenvector whose
  // corresponding eigenvalue is closest to negative infinity.
  void IntegrateMinorEigenvector()
    {this->SetIntegrationEigenvectorToMinor();};

  // Description:
  // Set / get a nominal integration step size (expressed as a fraction of
  // the size of each cell).
  vtkSetClampMacro(IntegrationStepLength,vtkFloatingPointType,0.001,0.5);
  vtkGetMacro(IntegrationStepLength,vtkFloatingPointType);

  // Description:
  // Set / get the length of a tube segment composing the
  // hyperstreamline. The length is specified as a fraction of the
  // diagonal length of the input bounding box.
  vtkSetClampMacro(StepLength,vtkFloatingPointType,0.000001,1.0);
  vtkGetMacro(StepLength,vtkFloatingPointType);


  // Description:
  // Set / get the method used to solve the initial value problem of the
  // tensor field

  void SetMethod(vtkInitialValueProblemSolver * meth)
    { method = meth; }

  vtkInitialValueProblemSolver * GetMethod(void) 
    { return method; }
  // Description:
  // Specify the direction in which to integrate the hyperstreamline.
  vtkSetClampMacro(IntegrationDirection,int,
           VTK_INTEGRATE_FORWARD,VTK_INTEGRATE_BOTH_DIRECTIONS);
  vtkGetMacro(IntegrationDirection,int);
  void SetIntegrationDirectionToForward()
    {this->SetIntegrationDirection(VTK_INTEGRATE_FORWARD);};
  void SetIntegrationDirectionToBackward()
    {this->SetIntegrationDirection(VTK_INTEGRATE_BACKWARD);};
  void SetIntegrationDirectionToIntegrateBothDirections()
    {this->SetIntegrationDirection(VTK_INTEGRATE_BOTH_DIRECTIONS);};

  // Description:
  // Set/get terminal eigenvalue.  If major eigenvalue falls below this
  // value, hyperstreamline terminates propagation.
  vtkSetClampMacro(TerminalEigenvalue,vtkFloatingPointType,0.0,VTK_LARGE_FLOAT);
  vtkGetMacro(TerminalEigenvalue,vtkFloatingPointType);

  // Description:
  // Set/get terminal fractional anisotropy.  If fractional anisotropy falls below this
  // value, hyperstreamline terminates propagation.
  vtkSetClampMacro(TerminalFractionalAnisotropy,vtkFloatingPointType,0.0,1.0);
  vtkGetMacro(TerminalFractionalAnisotropy,vtkFloatingPointType);

  // Description:
  // Set/Get Minimum step size the method must take.
  vtkSetClampMacro(MinStep,vtkFloatingPointType,0,VTK_LARGE_FLOAT);
  vtkGetMacro(MinStep,vtkFloatingPointType);

  // Description:
  // Set/Get Maximum step size the method can take.
  vtkSetClampMacro(MaxStep,vtkFloatingPointType,0,VTK_LARGE_FLOAT);
  vtkGetMacro(MaxStep,vtkFloatingPointType);

  // Description:
  // Set/Get Maximum Angle Fibers can have.
  vtkSetClampMacro(MaxAngle,vtkFloatingPointType,0,1);
  vtkGetMacro(MaxAngle,vtkFloatingPointType);

  // Description:
  // Set/Get The length of the fibers when considering the max angle
  vtkSetClampMacro(LengthOfMaxAngle,vtkFloatingPointType,0,VTK_LARGE_FLOAT);
  vtkGetMacro(LengthOfMaxAngle,vtkFloatingPointType);

  // Description:
  // Set/Get Maximum error each step may have.
  vtkSetClampMacro(MaxError,vtkFloatingPointType,0.0000000001,VTK_LARGE_FLOAT);
  vtkGetMacro(MaxError,vtkFloatingPointType);

  // Description:
  // Set / get the number of sides for the hyperstreamlines. At a minimum,
  // number of sides is 3.
  vtkSetClampMacro(NumberOfSides,int,3,VTK_LARGE_INTEGER);
  vtkGetMacro(NumberOfSides,int);

  // Description:
  // Set / get the initial tube radius. This is the maximum "elliptical"
  // radius at the beginning of the tube. Radius varies based on ratio of
  // eigenvalues.  Note that tube section is actually elliptical and may
  // become a point or line in cross section in some cases.
  vtkSetClampMacro(Radius,vtkFloatingPointType,0.0001,VTK_LARGE_FLOAT);
  vtkGetMacro(Radius,vtkFloatingPointType);

  // Description:
  // Turn on/off logarithmic scaling. If scaling is on, the log base 10
  // of the computed eigenvalues are used to scale the cross section radii.
  vtkSetMacro(LogScaling,int);
  vtkGetMacro(LogScaling,int);
  vtkBooleanMacro(LogScaling,int);
  
 protected:
  vtkPreciseHyperStreamline();
  ~vtkPreciseHyperStreamline();

  // Integrate data
  void Execute();
  void BuildTube();

  // Flag indicates where streamlines start from (either position or location)
  int StartFrom;

  // Starting from cell location
  vtkIdType StartCell;
  int StartSubId;
  vtkFloatingPointType StartPCoords[3];

  // starting from global x-y-z position
  vtkFloatingPointType StartPosition[3];

  // initial value problem solver
  vtkInitialValueProblemSolver * method;

  // Minimum step of integration
  vtkFloatingPointType MinStep;

  // Maximum step of integration
  vtkFloatingPointType MaxStep;

  // Maximum error of each step
  vtkFloatingPointType MaxError;
  
  //array of hyperstreamlines
  vtkPreciseHyperArray *Streamers;
  int NumberOfStreamers;

  // length of hyperstreamline in absolute distance
  vtkFloatingPointType MaximumPropagationDistance;

  // minimum length of hyperstreamline in absolute distance
  vtkFloatingPointType MinimumPropagationDistance;

  // integration direction
  int IntegrationDirection;

  // the length (fraction of cell size) of integration steps
  vtkFloatingPointType IntegrationStepLength;

  // the length of the tube segments composing the hyperstreamline
  vtkFloatingPointType StepLength;

  // the maximum angle before fiber is rejected
  vtkFloatingPointType MaxAngle;

  // the length of the fibers when considering max angle 
  vtkFloatingPointType LengthOfMaxAngle;

  // terminal propagation speed
  vtkFloatingPointType TerminalEigenvalue;

  // terminal fractional anisotropy
  vtkFloatingPointType TerminalFractionalAnisotropy;

  // Regularization Bias added to the tracking method
  vtkFloatingPointType RegularizationBias;

  // number of sides of tube
  int NumberOfSides;

  // maximum radius of tube
  vtkFloatingPointType Radius;

  // boolean controls whether scaling is clamped
  int LogScaling;

  // which eigenvector to use as integration vector field
  int IntegrationEigenvector;
 private:
  vtkPreciseHyperStreamline(const vtkPreciseHyperStreamline&);  // Not implemented.
  void operator=(const vtkPreciseHyperStreamline&);  // Not implemented.
};

#endif


