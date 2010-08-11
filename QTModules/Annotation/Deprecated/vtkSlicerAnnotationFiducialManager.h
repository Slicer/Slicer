/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/


#ifndef __vtkSlicerAnnotationFiducialManager_h
#define __vtkSlicerAnnotationFiducialManager_h


#include "qSlicerWidget.h"
#include "qSlicerAnnotationModuleExport.h"

class vtkMRMLAnnotationFiducialNode;
class vtkSlicerViewerWidget;
class vtkCamera;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
//class vtkSlicerSeedWidgetClass;
class vtkMRMLFiducialListNode;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkSlicerAnnotationFiducialManager : public qSlicerWidget
{
  
public:
  //static vtkSlicerAnnotationFiducialManager* New();
  //vtkTypeRevisionMacro(vtkSlicerAnnotationFiducialManager, vtkSlicerWidget);
  //void PrintSelf(ostream& os, vtkIndent indent);

  /// Getting the mrml Fiducial node id
  //vtkGetStringMacro(FiducialNodeID);

  /// Set the fid list id, and update the widgets
  //void SetFiducialNodeID(char *id);
  
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
  /// alternative method to propagate events generated in GUI to logic / mrml
  //virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// add observers on node
  virtual void AddMRMLObservers ( );

  /// remove observers on node
  virtual void RemoveMRMLObservers ( );

  /// Update the interactors on all the widgets
  void UpdateFiducialWidgetInteractors();

  /// encapsulated 3d widgets for each Fiducial node
  //std::map<std::string, vtkSlicerSeedWidgetClass*> SeedWidgets;

  //vtkSlicerSeedWidgetClass *GetSeedWidget(const char * nodeID);
  void AddSeedWidget(vtkMRMLAnnotationFiducialNode *FiducialNode);
  void AddSeedWidget(vtkMRMLFiducialListNode *FiducialListNode);

  void RemoveSeedWidget(vtkMRMLAnnotationFiducialNode *FiducialNode);
  void RemoveSeedWidgets();

  void AddSeed(vtkMRMLFiducialListNode *fiducialListNode, const char *pointID);
  void RemoveSeed(vtkMRMLFiducialListNode *fiducialListNode, const char *pointID);
  void UpdateSeed(vtkMRMLFiducialListNode *flist, const char *fidID); 

  /// Get/set the viewer widget so can add a the Fiducial widget to it
  //vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
  //virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

  void UpdateCamera();
  vtkCamera *GetActiveCamera();

  /// encapsulated text display node for each Fiducial node
  //std::map<std::string, std::string> TextDisplayNodes;
  std::string GetTextDisplayNodeID(const char *nodeID);
  std::string GetFiducialNodeIDFromTextDisplayNode(const char *nodeID);
  void AddTextDiaplayNodeID(vtkMRMLAnnotationFiducialNode *FiducialNode);
  void RemoveTextDisplayNodeID(vtkMRMLAnnotationFiducialNode *FiducialNode);

  /// encapsulated point display node for each Fiducial node
  //std::map<std::string, std::string> PointDisplayNodes;
  std::string GetPointDisplayNodeID(const char *nodeID);
  std::string GetFiducialNodeIDFromPointDisplayNode(const char *nodeID);
  void AddPointDiaplayNodeID(vtkMRMLAnnotationFiducialNode *FiducialNode);
  void RemovePointDisplayNodeID(vtkMRMLAnnotationFiducialNode *FiducialNode);

  void UpdateFiducialMeasurement(vtkMRMLAnnotationFiducialNode *FiducialNode);

  enum
  {
    AddFiducialCompletedEvent = 0,
  };


  vtkSlicerAnnotationFiducialManager(){

  }
  ~vtkSlicerAnnotationFiducialManager(){

  }
protected:
  /// update the widget GUI from the settings in the passed in FiducialNode
  void UpdateWidget(vtkMRMLAnnotationFiducialNode *FiducialNode);

  /// update the 3d distance widget from the settings in the passed in
  /// FiducialNode
  void Update3DWidget(vtkMRMLFiducialListNode *activeFiducialListNode);

  /// update the display nodes for FiducialNode
  void UpdateDisplayNode(vtkMRMLAnnotationFiducialNode *FiducialNode);

  /// update the text property passed by the text display node
  void UpdateTextDisplayProperty(vtkMRMLAnnotationTextDisplayNode *node);

  /// update the point property passed by the point display node
  void UpdatePointDisplayProperty(vtkMRMLAnnotationPointDisplayNode *node);

  void UpdateLockUnlock(vtkMRMLFiducialListNode* node);

  /// the id of the mrml node currently displayed in the widget
  char* FiducialNodeID;

  /// flag set to 1 when updating a 3d widget (todo: set it to the index of the
  /// Fiducial node / 3d widget once have more than one)
  int Updating3DFiducial;

  /// pointer to the viewer widget so can add props, request renders 
  vtkSlicerViewerWidget *ViewerWidget;

  /// Sets Slicer's interaction mode to "pick and manipulate"
  /// when a widget is being manipulated, and
  void UpdateInteractionModeAtStartInteraction();
  ///
  /// Resets Slicer's interaction mode back to original
  /// interaction mode when manipulation is finished.
  void UpdateInteractionModeAtEndInteraction();

  ///  update the visibility of the 3d widget associated with the passed node
  void Update3DWidgetVisibility(vtkMRMLFiducialListNode *activeFiducialListNode);


private:

  vtkSlicerAnnotationFiducialManager(const vtkSlicerAnnotationFiducialManager&); /// Not implemented
  void operator=(const vtkSlicerAnnotationFiducialManager&); /// Not Implemented

};

#endif

