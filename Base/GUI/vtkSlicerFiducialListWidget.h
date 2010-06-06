/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerFiducialListWidget.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkSlicerFiducialListWidget - 
/// 
/// Inherits most behavior from kw widget, but is specialized to observe
/// the current mrml scene and update the fiducial 3d graphics to correspond
/// to the currently available lists. 
//

#ifndef __vtkSlicerFiducialListWidget_h
#define __vtkSlicerFiducialListWidget_h

#include "vtkSlicerWidget.h"

#include "vtkMRMLViewNode.h"

class vtkMRMLFiducialListNode;
class vtkMRMLFiducial;
class vtkPolyData;
class vtkCellArray;
class vtkActor;
class vtkCamera;
class vtkFollower;
class vtkImplicitBoolean;
class vtkTransform;
class vtkSlicerViewerWidget;
class vtkSlicerViewerInteractorStyle;
class vtkSlicerSeedWidgetClass;
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerFiducialListWidget : public vtkSlicerWidget
{
public:
  static vtkSlicerFiducialListWidget* New();
  vtkTypeRevisionMacro(vtkSlicerFiducialListWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );

  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// removes observers on widgets in the class
  /// virtual void RemoveWidgetObservers ( );

  /// 
  /// removes observers on widgets in the class
  virtual void RemoveMRMLObservers ( );
  
  /// 
  /// Updates Actors based on fiducials in the scene
  /// Calls RemoveFiducialProps then UpdateFiducialsFromMRML
  void UpdateFromMRML();

  ///  
  /// Post a request for a render -- won't be done until the system is
  /// idle, and then only once....
  void RequestRender();

  ///  
  /// Actually do a render (don't wait for idle)
  void Render();

  ///  
  /// Used to track the fact that there is a idle task pending requesting a render
  vtkSetMacro (RenderPending, int);
  vtkGetMacro (RenderPending, int);

  /// Get/Set the slicer viewer widget, for picking
  vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
  virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

  /// 
  /// Get/Set the slicer interactorstyle, for picking
  vtkGetObjectMacro(InteractorStyle, vtkSlicerViewerInteractorStyle);
  virtual void SetInteractorStyle(vtkSlicerViewerInteractorStyle *interactorStyle);

//BTX
  /// encapsulated 3d widgets for each seed node
  std::map<std::string, vtkSlicerSeedWidgetClass *> SeedWidgets;
//ETX
  /// get a seed widget by fiducial list node id, by default the createFlag is
  /// 1 so the method will create a new seed widget class if it doesn't exist
  /// for the node with this id
  vtkSlicerSeedWidgetClass *GetSeedWidget(const char * nodeID, int createFlag = 1);

  /// set up a new seed widget for the node with this id
  void AddSeedWidgetByID(const char *nodeID);

  ///
  /// set up a new seed widget for this node
  void AddSeedWidget(vtkMRMLFiducialListNode *fiducialListNode);

  ///
  /// add a point to the seed widget for this list
  void AddSeed(vtkMRMLFiducialListNode *fiducialListNode, const char *pointID);
  ///
  /// remove the nth point in the seed widget for this list
  void RemoveSeed(vtkMRMLFiducialListNode *fiducialListNode, const char *pointID);
  ///
  /// remove seed widget for this node
  void RemoveSeedWidget(vtkMRMLFiducialListNode *fiducialListNode);
  ///
  /// remove seed widgets for all nodes
  void RemoveSeedWidgets();
  ///
  /// check scene to make sure that have a widget for each list node, and no extra widgets...
  void Update3DWidgetsFromMRML();

  /// Description:
  /// Get/Set the ViewNode
  vtkGetObjectMacro(ViewNode, vtkMRMLViewNode);
  void SetAndObserveViewNode (vtkMRMLViewNode *snode)
    {
    vtkSetAndObserveMRMLNodeMacro( this->ViewNode, snode );
    };
  ///
  /// change the lock flags on all seed widgets. If lockFlag is 0, don't turn
  /// off locking if the fiducial list node associated with the list has
  /// locking turned on
  void ModifyAllWidgetLock(int lockFlag);

protected:
  vtkSlicerFiducialListWidget();
  virtual ~vtkSlicerFiducialListWidget();

  /// 
  /// Create the widget.
  virtual void CreateWidget();

  ///
  /// Update things that depend on the view node
  void UpdateViewNode();
  
  ///
  /// Update the properties of the seed for this fiducial
  void UpdateSeed(vtkMRMLFiducialListNode *flist, const char *fidID); /// int f);

  /// 
  /// Remove fiducial properties from the main viewer
  //void RemoveFiducialProps();

  ///
  /// get the mainviewer's active camera
  vtkCamera *GetActiveCamera();
  ///
  /// Update the fiducials' camera to the mainviewer's active camera
  void UpdateFiducialsCamera();

  ///
  /// Update the seed widget's camera to the main viewer's render window
  /// interactor
  void UpdateSeedWidgetInteractors();

  /// 
  /// Removes observers that this widget placed on the fiducial lists in the
  /// mrml tree
  void RemoveFiducialObservers();

  /// 
  /// Remove observers that this widget placed on the fiducial list
  void RemoveFiducialObserversForList(vtkMRMLFiducialListNode *flist);

  /// 
  /// Add observers to a fiducial list
  void AddObserversToFiducialList(vtkMRMLFiducialListNode *flist);

  /// 
  /// Goes through the MRML scene and for each fiducial
  /// list node, calls UpdateFiducialListFromMRML. Once done, requests a render.
  void UpdateFiducialsFromMRML();

  ///
  /// Adds observers to the fiducial list.
  void UpdateFiducialListFromMRML(vtkMRMLFiducialListNode *flist);
  
  ///
  /// Sets material properties for this list
  void SetSeedWidgetMaterialProperties(vtkMRMLFiducialListNode *flist);

  ///
  /// Flag set to 1 when processing mrml events
  int ProcessingMRMLEvent;
  
  ///
  /// A flag to avoid thread collisions when rendering
  int RenderPending;
  
  ///
  /// A pointer back to the viewer widget, useful for picking
  vtkSlicerViewerWidget *ViewerWidget;

  ///
  /// A poitner to the interactor style, useful for picking
  vtkSlicerViewerInteractorStyle *InteractorStyle;

  /// flag set to 1 when updating a 3d widget (todo: set it to the index of the
  /// angle node / 3d widget once have more than one)
  int Updating3DWidget;

  
  ///
  /// update the 3d seed widget from the lock settings in the passed in
  /// fiducial list node
  void Update3DWidgetLock(vtkMRMLFiducialListNode *fiducialListNode);

  ///
  /// update the 3d seed widget from the visibility settings in the passed in
  /// fiducial list node
  void Update3DWidgetVisibility(vtkMRMLFiducialListNode *fiducialListNode);

  ///
  /// update the 3d seed widget from the selected settings in the passed in
  /// fiducial list node
  void Update3DWidgetSelected(vtkMRMLFiducialListNode *fiducialListNode);


  ///
  /// update the 3d seed widget from the glyph setting in the passed in
  /// fiducial list node
  void Update3DWidgetGlyphType(vtkMRMLFiducialListNode *fiducialListNode);

  ///
  /// update the 3d seed widget from the text settings in the passed in
  /// fiducial list node
  void Update3DWidgetText(vtkMRMLFiducialListNode *fiducialListNode);


  ///
  /// update the 3d seed widget from the position settings in the passed in
  /// fiducial list node
  void Update3DWidgetPositions(vtkMRMLFiducialListNode *fiducialListNode);

  ///
  /// update the 3d seed widget from the colour settings in the passed in
  /// fiducial list node, both selected and colour
  void Update3DWidgetColour(vtkMRMLFiducialListNode *fiducialListNode);

  /// update the 3d seed widget from the scale settings in the passed in
  /// fiducial list node, both text and glyph
  void Update3DWidgetScale(vtkMRMLFiducialListNode *fiducialListNode);


  /// update the 3d seed widget from the display settings in the passed in
  /// fiducialListNode. Called by Update3DWidget
  void Update3DWidgetDisplayProperties(vtkMRMLFiducialListNode *fiducialListNode);
  
  /// update the 3d seed widget from the settings in the passed in
  /// fiducialListNode
  void Update3DWidget(vtkMRMLFiducialListNode *fiducialListNode);


  ///
  /// when a fiducial point is moved up or down in the fiducial list, need to
  /// update the seeds that represent the swapped points
  void Swap(vtkMRMLFiducialListNode *flist, int first, int second);

  ///
  /// Sets Slicer's interaction mode to "pick and manipulate"
  /// when a widget is being manipulated, and
  void UpdateInteractionModeAtStartInteraction();
  ///
  /// Resets Slicer's interaction mode back to original
  /// interaction mode when manipulation is finished.
  void UpdateInteractionModeAtEndInteraction();
  

  ///
  /// pointer to a view node
  vtkMRMLViewNode *ViewNode;
private:
  
  vtkSlicerFiducialListWidget(const vtkSlicerFiducialListWidget&); /// Not implemented
  void operator=(const vtkSlicerFiducialListWidget&); /// Not Implemented
};

#endif
