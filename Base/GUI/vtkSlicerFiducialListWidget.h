/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerFiducialListWidget.h,v $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/

// .NAME vtkSlicerFiducialListWidget - 
// .SECTION Description
// Inherits most behavior from kw widget, but is specialized to observe
// the current mrml scene and update the fiducial 3d graphics to correspond
// to the currently available lists. 
//

#ifndef __vtkSlicerFiducialListWidget_h
#define __vtkSlicerFiducialListWidget_h

#include "vtkSlicerWidget.h"
#include "vtkKWRenderWidget.h"
#include <vector>
#include "vtkTransformPolyDataFilter.h"
#include "vtkMapper.h"
#include "vtkGlyph3D.h"
#include "vtkSphereSource.h"

class vtkMRMLFiducialListNode;
class vtkMRMLFiducial;
class vtkPolyData;
class vtkCellArray;
class vtkActor;
class vtkFollower;
class vtkImplicitBoolean;
class vtkKWRenderWidget;
class vtkTransform;
class vtkCollection;
class vtkSlicerViewerWidget;
class vtkSlicerViewerInteractorStyle;
class vtkPointWidget;
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerFiducialListWidget : public vtkSlicerWidget
{
public:
  static vtkSlicerFiducialListWidget* New();
  vtkTypeRevisionMacro(vtkSlicerFiducialListWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );

  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // removes observers on widgets in the class
  // virtual void RemoveWidgetObservers ( );

  // Description:
  // removes observers on widgets in the class
  virtual void RemoveMRMLObservers ( );

  // Description:
  // Get the poly data representing the 3d diamond glyph
  vtkGetObjectMacro(DiamondGlyphPolyData, vtkPolyData);
  vtkSetObjectMacro(DiamondGlyphPolyData, vtkPolyData);

  // Description:
  // Get/Set the sphere source for the 3d sphere glyph
  vtkGetObjectMacro(SphereSource, vtkSphereSource);
  vtkSetObjectMacro(SphereSource, vtkSphereSource);
  
  // Description:
  // Set/Get the main viewer, called by vtkSlicerApplicationGUI
  vtkSetObjectMacro(MainViewer, vtkKWRenderWidget);
  vtkGetObjectMacro(MainViewer, vtkKWRenderWidget);
  
  // Description:
  // Updates Actors based on fiducials in the scene
  // Calls RemoveFiducialProps then UpdateFiducialsFromMRML
  void UpdateFromMRML();

  // Description: 
  // Post a request for a render -- won't be done until the system is
  // idle, and then only once....
  void RequestRender();

  // Description: 
  // Actually do a render (don't wait for idle)
  void Render();

  // Description: 
  // Used to track the fact that there is a idle task pending requesting a render
  vtkSetMacro (RenderPending, int);
  vtkGetMacro (RenderPending, int);

  // Description:
  // return the current actor corresponding to a given MRML ID
  vtkActor * GetFiducialActorByID (const char *id);

  // Description:
  // return the current point widget corresponding to a given MRML ID
  vtkPointWidget * GetPointWidgetByID (const char *id);

  // Description:
  // Get/Set the main slicer viewer widget, for picking
  vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
  virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

  // Description:
  // Get/Set the slicer interactorstyle, for picking
  vtkGetObjectMacro(InteractorStyle, vtkSlicerViewerInteractorStyle);
  virtual void SetInteractorStyle(vtkSlicerViewerInteractorStyle *interactorStyle);

protected:
  vtkSlicerFiducialListWidget();
  virtual ~vtkSlicerFiducialListWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the properties of the text actor
  void UpdateTextActor(vtkMRMLFiducialListNode *flist, int f);
  
  // Description:
  // Update the properties of the point widget
  void UpdatePointWidget(vtkMRMLFiducialListNode *flist, int f);

  // Description:
  // Remove fiducial properties from the main viewer
  void RemoveFiducialProps();

  // Description:
  // Removes observers that this widget placed on the fiducial lists in the
  // mrml tree
  void RemoveFiducialObservers();

  // Description:
  // Goes through the MRML scene and adds observers to fiducial lists, then
  // updates the display properties from the mrml node
  void UpdateFiducialsFromMRML();

  // Description:
  // Sets actor properties for this list, position, scale, transforms, colour
  void SetFiducialDisplayProperty(vtkMRMLFiducialListNode *flist, int n,
                                  vtkActor *actor, vtkFollower *textActor);

  // Description:
  // set up all the data structures needed to display glyphs for this list
  void AddList(vtkMRMLFiducialListNode *flist);
  
  // Description:
  // clear up all the data structures used to display glyphs for this list
  void RemoveList(vtkMRMLFiducialListNode * flist);
  void RemoveFiducial(const char *id);

  // Description:
  // Keep track of the actors that represent each fiducial point
  //BTX
  std::map<std::string, vtkActor *> DisplayedFiducials;
  std::map<std::string, vtkFollower *> DisplayedTextFiducials;
 
  std::string GetFiducialActorID (const char *id, int index);
  // Description:
  // returns the fiducial list's id from the fiducial point's id
  std::string GetFiducialNodeID (const char *actorid, int &index);

  // Description:
  // encapsulates the vtk 3d widgets for each fiducial, indexed by fiducial id
  std::map<std::string, vtkPointWidget*> DisplayedPointWidgets; 
  //ETX
  
  // Description:
  // Flag set to 1 when processing mrml events
  int ProcessingMRMLEvent;
  
  // Description:
  // the poly data representing the 3d diamond glyph
  vtkPolyData * DiamondGlyphPolyData;
  
  // Description:
  // represents the sphere 3d glyph
  vtkSphereSource * SphereSource;
  
  //BTX
  // Description:
  // the transforms applied to the 3d diamond glyphs, indexed by fiducial list id
  std::map< std::string, vtkTransform * > DiamondTransformMap;
  // Description:
  // encapsulates the vtkPoints for each list, indexed by fiducial list id
  std::map< std::string, vtkPoints * > GlyphPointsMap;
  // Description:
  // encapsulates the scalars for each list, indexed by fiducial list id
  std::map< std::string, vtkFloatArray * > GlyphScalarsMap;
  // Description:
  // encapsulates the glyph points and the glyph scalars, indexed by fiducial list id
  std::map< std::string, vtkPolyData * > GlyphPolyDataMap;

  // Description:
  // encapsulates the transforms applied to the text for each list, indexed by fiducial list id
  std::map< std::string, vtkTransform * > TextTransformMap;
  // Description:
  // encapsulates the transforms applied to the symbols for each list, indexed by fiducial list id
  std::map< std::string, vtkTransform * > SymbolTransformMap;

  // Description:
  // encapsulates the filter that takes in the shape for each glyph, for each
  // list, indexed by fiducial list id
  std::map< std::string, vtkTransformPolyDataFilter * > TransformFilterMap;

  // Description:
  // encapsulates the glyphs that are used to display each list, indexed by
  // fiducial list id
  std::map< std::string, vtkGlyph3D * > Glyph3DMap;
  //std::vector< vtkGlyph3D * > Glyph3DVector;
  //std::vector< int > GlyphSymbolVector;
  

  // Description:
  // encapsulates the data mappers for each list, indexed by fiducial list id
  std::map< std::string, vtkMapper * > GlyphMapperMap;

  // Description:
  // keeps track of the kind of glyph was used for each fiducial list (2d or
  // 3d), indexed by fiducial list id. If the type changes, may need to do some
  // clean up.
  std::map< std::string, bool> Use3DSymbolsMap;
  //ETX

  // Description:
  // A collection of the vtkGlyph3Ds that are used for display
  //vtkCollection * Glyph3DList;

  // Description:
  // A pointer back to the main viewer, so that can render when update
  // fiducial display characteristics
  vtkKWRenderWidget *MainViewer;
  // Description:
  // A flag to avoid thread collisions when rendering
  int RenderPending;

  // Description:
  // A pointer back to the viewer widget, useful for picking
  vtkSlicerViewerWidget *ViewerWidget;

  // Description:
  // A poitner to the interactor style, useful for picking
  vtkSlicerViewerInteractorStyle *InteractorStyle;
  
private:
  
  vtkSlicerFiducialListWidget(const vtkSlicerFiducialListWidget&); // Not implemented
  void operator=(const vtkSlicerFiducialListWidget&); // Not Implemented
};

#endif
