/*
 *  mesh-quality-class.h
 *  tfm-cxx
 *
 *  Created by C Lisle on 1/4/07.
 *  Copyright 2007 KnowleldgeVis,LLC..
 *
 */

#ifndef __vtkMeshQualityClass_h
#define __vtkMeshQualityClass_h
 
#include "vtkCommon.h"


// declarations for VTK classes
#include "vtkObject.h"
#include "vtkMeshQuality.h"
//#include "vtkKWWindow.h"
#include "vtkShrinkFilter.h"
#include "vtkActor.h"

class vtkKWRenderWidget;
class vtkMeshQuality;
class vtkLookupTable;
class vtkLogLookupTable;
class vtkDataSetMapper;

// the VTK_HYBRID_EXPORT label facilitates dynamic linking in windows.  Export declarations
// must match for all classes that are dynamically linked together under windows.
// 
class  VTK_MIMXCOMMON_EXPORT vtkMeshQualityClass : public vtkObject {

private: 
  bool threshIsAbsolute;
  double thresholdValue;
  char MeshFileName[256];
  double ElementOpacity;
  int OutlineIsEnabled;
  int IsInitialized;     /* set when the pipeline is setup */
  int SavedQualityMetric;   /* place for enumeration of which metric to run */

  vtkDataSetMapper* SavedElementMapper;
  
  vtkActor *outputActor;
  vtkMeshQuality *VTKQualityFilterPtr;
  
  // how much to shrink elements before display
  double ElementShrinkFactor;
  
  // handle to the LUT so we can modify it and cause re-evaluation
  vtkLogLookupTable *lutFilter;
  
  //handle to the shrink filter so we can modify it
  vtkShrinkFilter *shrinkFilter;
  
  vtkUnstructuredGrid *InitialMesh;
  vtkUnstructuredGrid *SavedMesh;
  
  void SetupVTKPipeline(void); 
  void InvokeQualityMeasure(void);
  void FindMinimumAndMaximumQualityForMesh(vtkUnstructuredGrid *mesh, double *minQualityFound, double *maxQualityFound);

  
  
public:
  static vtkMeshQualityClass* New();
  vtkTypeRevisionMacro(vtkMeshQualityClass,vtkMeshQualityClass);
  
  // re-evaluate the display options part of the pipeline because the user
  // has changed parameters, such as the colors, opacity, or threshold to use
  // for display. 

  void UpdatePipeline(void);
  
  // specific a threshold, under which the elements will be considered un-interesting
  // this normalized value is reduced from 1.0 (show all) down toward 0.0 (to show only the 
  // lowest quality elements 
  
  void SetThresholdValue (double thresh);

  // pass either an unstructrured grid object pointer or a filename to initialize the 
  // rendering datastructures from.  The dataset
  // is assumed to be an unstructured grid.  
  
  void InitializeMeshFromFile(char* filestr);
  
  void InitializeFromExternalMesh(vtkUnstructuredGrid *grid);
  
  // assign a value for all visible elements to have.  0->1 goes from invisible to 
  // opaque.  This works separately from the threshold.  All displayed elements will
  // have at most this opacity. 
  
  void SetElementOpacity(double opacity); 

  // Should we draw an outline around the edge of the mesh object
  void SetOutlineEnabled(int outline);

  // run the algorithm and return a pointer to an actor 
  vtkActor* CalculateMeshQuality (void);
  
  // the filter picks a default test to run, if the user wants to change the test
  // then an enumeration is supported to allow the selection of different tests.  The 
  // enumeration is defined in the VTK/Graphics/vtkMeshQualityFilter.h header file. 
  
  // The default is VTK_QUALITY_MAX_ASPECT_FROBENIUS and valid values also include
  // VTK_QUALITY_EDGE_RATIO, VTK_QUALITY_MAX_ASPECT_FROBENIUS, 
  // VTK_QUALITY_MAX_EDGE_RATIOS, VTK_QUALITY_SKEW, VTK_QUALITY_TAPER, VTK_QUALITY_VOLUME,
  // VTK_QUALITY_STRETCH, VTK_QUALITY_DIAGONAL, VTK_QUALITY_DIMENSION,
  // VTK_QUALITY_ODDY, VTK_QUALITY_CONDITION, VTK_QUALITY_JACOBIAN,
  // VTK_QUALITY_SCALED_JACOBIAN, VTK_QUALITY_SHEAR, VTK_QUALITY_SHAPE,
  // VTK_QUALITY_RELATIVE_SIZE_SQUARED, VTK_QUALITY_SHAPE_AND_SIZE,
  // VTK_QUALITY_SHEAR_AND_SIZE, and VTK_QUALITY_DISTORTION.
  
  void SetQualityMeasure(int qualityTest);
  
  
  void SetElementShrinkFactor(double shrink); 
  
  vtkMeshQualityClass();
  
  ~vtkMeshQualityClass();


};

#endif

