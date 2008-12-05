/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxBoundaryConditionActor.h,v $
Language:  C++
Date:      $Date: 2008/07/22 17:38:39 $
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

// .NAME vtkMimxBoundaryConditionActor - 
// .SECTION Description
// vtkMimxBoundaryConditionActor is the class to handle display and manipulation
// of mesh boundary conditions. These are defined via Node Sets. The user can
// use this class to turn on / off the Boundary Conditions based on the
// Node set, type, and step number that they are applied.

#ifndef __vtkMimxBoundaryConditionActor_h
#define __vtkMimxBoundaryConditionActor_h

#include <string>
#include <list>

#include "vtkMimxActorBase.h"
#include "vtkMimxCommonWin32Header.h"

class vtkActor;
class vtkDataSetMapper;
class vtkIdList;
class vtkUnstructuredGrid;
class vtkShrinkFilter;
class vtkPolyDataMapper;
class vtkTubeFilter;
class vtkRenderer;
class vtkGeometryFilter;
class vtkExtractGeometry;
class vtkScalarBarActor;
class vtkLookupTable;
class vtkPlaneWidget;
class vtkPlane;
class vtkRenderWindowInteractor;
class vtkExtractCells;
class vtkPointSet;
class vtkDoubleArray;
class vtkLookupTable;
class vtkGlyph3D;

class VTK_MIMXCOMMON_EXPORT BoundaryConditionProperty
{
public:
  char name[128];
  bool IsVisible;
  char bcType[16];
  char Axis[2];
  char StepNumber[16];
  double bcValue;
  char nodeSet[64];
  vtkActor *GlyphActor;
  vtkGlyph3D *Glyph;
  double AxisFactor;
};

class VTK_MIMXCOMMON_EXPORT vtkMimxBoundaryConditionActor : public vtkMimxActorBase
{
public:

  enum ConditionType { 
    UnknownCondition = 0,
    Force            = 1,
    Displacement     = 2,
    Rotation         = 3,
    Moment           = 4
  };
  
  enum AxisType { 
    Xaxis            = 0,
    Yaxis            = 1,
    Zaxis            = 2,
    UnknownAxis      = 3
  };
  
  static vtkMimxBoundaryConditionActor *New();
  vtkTypeRevisionMacro(vtkMimxBoundaryConditionActor,vtkMimxActorBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkUnstructuredGrid* GetDataSet();
  void SetDataSet( vtkUnstructuredGrid *ugrid);

  // Set the Scale factor and update the list
  void SetScaleFactor(double sFactor);

  // Get/Set the Actor Renderer
  void SetRenderer(vtkRenderer *renderer);
  vtkRenderer* GetRenderer();
  
  // Set the scale of the Actors
  double GetGlyphScale( );
  
  // Set the base size of the Actors
  void SetGlyphSize( double size );
  double GetGlyphSize( );
  
  // Get/Set the Step for the Boundary Condition
  void SetStep( int bcStep );
  int GetStep( );
  
  // Show all Step Boundary Conditions
  void ShowAllStepActors( int bcStep );
  
  // Hide all Step Boundary Conditions
  void HideAllStepActors( int bcStep );
  
  // Show the specified Boundary Conditions
  void ShowBoundaryCondition(int step, const char *setName, int type);
  
  // Hide the specified Boundary Conditions
  void HideBoundaryCondition(int step, const char *setName, int type);
  
  // Get the visibility of the specified Boundary Conditions
  int GetBoundaryConditionVisibility(int step, const char *setName, int type);
  
  // Get Number of Step Boundary Conditions
  //int GetNumberOfStepBoundaryConditions();
  //void GetBoundaryConditionType(int step, int index, char *setName, int &type);
  int HasBoundaryConditionActor( std::string bcName );
  void DeleteBoundaryConditionActor( std::string bcName );
  void AddBoundaryConditionActor( const char *StepNum, const char *NodeSetName, 
          const char *BCType, const char *BCName);
  void DeleteAllBoundaryConditionActors( );
  
  BoundaryConditionProperty* GetBoundaryCondition(const char *bcLabel);
  
protected:
  vtkMimxBoundaryConditionActor();
  ~vtkMimxBoundaryConditionActor();
  
  void CreateBoundaryConditionActor( const char *StepNum, const char *NodeSetName, const char *BCType);
  void CreateIndividualBoundaryConditionActor( const char *StepNum, const char *NodeSetName, 
          const char *BCType, const char *Direction, double scaleFactor, double bcValue, const char *BCName);
  vtkPointSet* GetPointSetOfNodeSet(const char* NodeSetName);
  void CreateStepActors( );
  void UpdateGlyphSize( );
  double GetValue(const char *);
private:
  vtkUnstructuredGrid *UnstructuredGrid;
  vtkRenderer *Renderer;
  std::list<BoundaryConditionProperty*> BoundaryConditionDisplayList;
  double GlyphScale;
  int Step;
  double GlyphSize;
  double ScaleFactor;
  vtkMimxBoundaryConditionActor(const vtkMimxBoundaryConditionActor&);  // Not implemented.
  void operator=(const vtkMimxBoundaryConditionActor&);  // Not implemented.
};

#endif

