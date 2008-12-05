/*
 *  vtkMeshQualityClass.h
 *  
 *
 *  Created by C Lisle on 1/4/07.
 *  Copyright 2007 KnowleldgeVis,LLC..
 *
 */

#ifndef __vtkMeshQualityClass_h
#define __vtkMeshQualityClass_h
 

// declarations for VTK classes
#include "vtkObject.h"

//#include "vtkKWWindow.h"
#include "vtkShrinkFilter.h"
#include "vtkActor.h"
#include "vtkExtractGeometry.h"
#include "vtkPlane.h"
#include "vtkActor2D.h"
#include "vtkKWListBoxWithScrollbars.h"
#include "vtkMimxCommonWin32Header.h"

// OO separation violation by passing an object to invoke methods on to display
// the values of bad elements.  This is ignored if no object is connected via "AddCanvas" call
#include "vtkKWListBox.h"

// use custom version of Sandia Mesh quality filter extended for our purposes 
#include "vtkMeshQualityExtended.h"


class vtkKWRenderWidget;
class vtkMeshQuality;
class vtkLookupTable;
class vtkLogLookupTable;
class vtkDataSetMapper;
class vtkThreshold;
class vtkFaceHedgeHog;
class vtkExtractCells;
class vtkIdList;
class vtkPassThroughFilter;



// the VTK_HYBRID_EXPORT label facilitates dynamic linking in windows.  Export declarations
// must match for all classes that are dynamically linked together under windows.
// 
class  VTK_MIMXCOMMON_EXPORT vtkMeshQualityClass : public vtkObject {

private: 
  double thresholdValue;
  char MeshFileName[256];
  double ElementOpacity;
  int OutlineIsEnabled;
  int IsInitialized;            /* set when the pipeline is setup */
  int SavedQualityMetric;   /* place for enumeration of which metric to run */
  int ShowFilledElements;
  int ShowOutline;
  int ShowClippedOutline;
  int ShowInteriorOutlines;
  int ShowSurfaceHedgehog;
  int ShowSelectedElement;  /* enable outline and value pop-up for selected element */
  double SavedCuttingPlaneOpacity;
  int CuttingPlaneEnabled;
  int OverrideMeshColorRange;
  int ElementsHighlighted;
  double minQualityFound;
  double maxQualityFound;

  // pointer to the Sandia Labs filter that runs the quality algorithms
  vtkMeshQualityExtended* VTKQualityFilterPtr;
  vtkExtractGeometry* ExtractSelectedGeometryFilter;
  
  // save pointers to the mappers and actors we create for rendering. These pointers
  // are required to provide local memory for vtk objects that have to be retained between 
  // method calls
  
  vtkDataSetMapper* SavedElementMapper;
  vtkActor *SavedElementActor;
  vtkActor *SavedOutlineActor;
  vtkActor *SavedClippedOutlineActor;
  vtkActor *SavedInteriorOutlinesActor;
  vtkActor *SavedSurfaceHedgehogActor;    
  vtkActor2D *SavedColorLegendActor;
  vtkPlane *SavedCuttingPlaneImplicitFunction;
  vtkFaceHedgeHog *SavedHedgehogFilter;
  vtkExtractCells *CellSelectionFilter;
  vtkIdList       *SavedCellList;
  
  // handle to element thresholding filter
  //vtkThreshold *thresholdFilter;
  vtkPassThroughFilter  *thresholdFilter;
  
  // how much to shrink elements before display
  double ElementShrinkFactor;
  
  // how many digits to use when displaying numbers - used by mesh quality display
  int  SavedDisplayPrecision;
  
  // handle to the LUT so we can modify it and cause re-evaluation
  vtkLookupTable *lutFilter;
  
  //handle to the shrink filter so we can modify it
  vtkShrinkFilter *shrinkFilter;
  
  vtkUnstructuredGrid *InitialMesh;     // before quality attributes added
  vtkUnstructuredGrid *SavedMesh;       // after quality attributes added
  
  void SetupVTKPipeline(void); 
  void InvokeQualityMeasure(void);
  void FindMinimumAndMaximumQualityForMesh(vtkUnstructuredGrid *mesh, double *minQualityFound, double *maxQualityFound);
  
  // initialize pipelines for each type of actor we create
  void SetupOutlineVTKPipeline(void);
  void SetupClippedOutlineVTKPipeline(void);
  void SetupInteriorVTKPipeline(void);
  void SetupMeshFilterPipeline(void);
  void SetupHedgeHogVTKPipeline(void);
  void SetupColorLegendActor(void);
  
  // internal processing to do when we are given a new mesh the first time
  void  CreateProcessedMesh(void);
  
  vtkKWListBoxWithScrollbars *ReportingCanvas;
  
  // internal state memory, not for user use
  vtkSetMacro(ElementsHighlighted,int);
 
public:
  static vtkMeshQualityClass* New();
  vtkMeshQualityClass();
  ~vtkMeshQualityClass();
  vtkTypeRevisionMacro(vtkMeshQualityClass,vtkObject);
  
  void InitVariables(void);

  // pass either an unstructrured grid object pointer or a filename to initialize the 
  // rendering datastructures from.  The dataset
  // is assumed to be an unstructured grid.  
  
  void InitializeFromFile(char* filestr); 
  void InitializeFromExternalMesh(vtkUnstructuredGrid *grid);
  
  // specify a threshold, under which the elements will be considered un-interesting
  // this normalized value ranges from 1.0 (show all) down toward 0.0 (to show only the 
  // lowest quality elements 
  
  void SetThresholdValue (double thresh);
  double GetThresholdValue(void) { return this->thresholdValue;}
  
  vtkGetMacro(ElementsHighlighted,int);
  vtkGetObjectMacro(InitialMesh, vtkUnstructuredGrid);
  
  void SetMeshColorRange(double low, double high);
  
  // highlight a particular element by making all elements 
  // transparent accept for the selected element.  There is a separate
  // API call for enabling and disabling 
  
  void HighlightFocusElements(void);
  void CancelElementHighlight(void);
  
  // This class allows the user to pass an implicit function that is used as a "cutting plane".
  // The plane defeats the drawing of elements on the negative side of the plane.  Thus, the 
  // user "sweep" through a dataset and look at interior elements.   Currently, separate enable
  // and disable events are used because this allows the VTK pipeline to be changed on demand.  It 
  // would force a pipeline re-execution each time if we used the vtkSetMacro approach, so this method
  // is currently chosen.   If the invertSelection is enabled, then the "negative" side of 
  // the plane is used to select elements instead of the "positive" side. 
  
  void EnableCuttingPlane(void);
  void DisableCuttingPlane(void);
  void SetCuttingPlaneFunction(vtkPlane *cuttingPlane);
  void SetInvertCuttingPlaneSelection(int invert);
  void SetDisplayPrecision(int precision);
  
  // Multiple actors are created to show the mesh in several different 
  // ways.  Set these options to determine whether these actors will be 
  // enabled or disabled in the current rendering.  The actors are always
  // created and these state variables control whether the actors are displayed
  // or not.  This may cause extra re-calculation, but it is at the end of the VTK 
  // pipeline execution, so render times should not be too adversely affected. 
  
  vtkSetMacro(ShowFilledElements,int);
  vtkGetMacro(ShowFilledElements,int);
  vtkSetMacro(ShowOutline,int);
  vtkGetMacro(ShowOutline,int);
  vtkSetMacro(ShowClippedOutline,int);
  vtkGetMacro(ShowClippedOutline,int);
  vtkSetMacro(ShowInteriorOutlines,int);
  vtkGetMacro(ShowInteriorOutlines,int);
  vtkSetMacro(ShowSurfaceHedgehog,int);
  vtkGetMacro(ShowSurfaceHedgehog,int);
  vtkSetMacro(OverrideMeshColorRange,int);
  vtkGetMacro(OverrideMeshColorRange,int);

 
  // set the scale of the vectors drawn on the surface
  void SetHedgehogScale(double value);
 
  // assign a value for all visible elements to have.  0->1 goes from invisible to 
  // opaque.  This works separately from the threshold.  All displayed elements will
  // have at most this opacity. 
  
  void SetElementOpacity(double opacity); 
  double GetElementOpacity(void) { return this->ElementOpacity;}

  // run the algorithm and update the actor representations
  void CalculateMeshQuality (void);
  
  // return actors *** had to do this because invoking methods on renderer failed
  vtkActor* ReturnFilledElementsActor(void);
  vtkActor* ReturnInternalOutlinesActor(void);
  vtkActor* ReturnOutlineActor(void);  
  vtkActor* ReturnClippedOutlineActor(void);  
  vtkActor* ReturnHedgehogActor(void);
  vtkActor2D* ReturnColorLegendActor(void);
  
  // re-evaluate the display options part of the pipeline because the user
  // has changed parameters, such as the colors, opacity, or threshold to use
  // for display. 

  void UpdatePipeline(void);
  
  // the filter picks a default test to run, if the user wants to change the test
  // then an enumeration is supported to allow the selection of different tests.  The 
  // enumeration is defined in the VTK/Graphics/vtkMeshQualityFilter.h header file. 
  
  // The default is VTK_QUALITY_VOLUME and valid values also include
  // VTK_QUALITY_SHAPE, VTK_QUALITY_JACOBIAN, VTK_QUALITY_MED_ASPECT_FROBENIUS,
  // and VTK_QUALITY_EDGE_RATIO.  Others could be added in the future.  
  // The implementation of these algorithms is included in the VERDICT library written
  // by Sandia and contributed to VTK. The enumeration argument values are 
  // zero-based and correspond to the order listed above.  
  // For example, 0=Volume, 4=Edge-Ratio, etc. 
  
  void SetQualityMeasure(int qualityTestEnumeration);
  void SetQualityMeasureToVolume(void);
  void SetQualityMeasureToShape(void);
  void SetQualityMeasureToJacobian(void);
  void SetQualityMeasureToFrobenius(void);  
  void SetQualityMeasureToEdgeRatio(void);  
  
  // User control over the normalized amount of shrinking of the elements 0.0=completely 
  // invisible, 1.0 indicates no gap between the elements.  This allows the user
  // to visually change the element size and examine internal boundaries. 
  
  void SetElementShrinkFactor(double shrink); 
  
  // allow the user to save a portion of the current mesh to a new file.  The user would 
  // enable the selection plane, position it to select the proper subset, and then call this
  // method with a valid filename string. 
  
  void SaveSelectionAsNewMeshFile(char *filename);
  
  void AddErrorReportingCanvas(vtkKWListBoxWithScrollbars* canvas) {this->ReportingCanvas = canvas;}

};

#endif

