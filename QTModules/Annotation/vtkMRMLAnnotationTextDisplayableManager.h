/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
 Date:      $Date: 2006/01/08 04:48:05 $
 Version:   $Revision: 1.45 $

 =========================================================================auto=*/

#ifndef __vtkSlicerAnnotationTextManager_h
#define __vtkSlicerAnnotationTextManager_h

#include "vtkMRMLAnnotationDisplayableManager.h"
#include "qSlicerAnnotationModuleExport.h"

class vtkMRMLAnnotationTextNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationTextDisplayableManager : public vtkMRMLAnnotationDisplayableManager
{

public:




  virtual
  ~vtkMRMLAnnotationTextDisplayableManager();

  //vtkTypeRevisionMacro(vtkSlicerAnnotationTextManager, vtkSlicerWidget);
  //void PrintSelf(ostream& os, vtkIndent indent);

  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void
  ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  /// add observers on node
  virtual void
  AddMRMLObservers();

  // remove observers on node, from AbstractDisplayableManager
  virtual void
  RemoveMRMLObservers();

  // update triggered from MRML node, from AbstractDisplayableManager
  virtual void
  UpdateFromMRML();

  // Called after the corresponding MRML event is triggered, from AbstractDisplayableManager
  // \sa ProcessMRMLEvents
  virtual void
  OnMRMLSceneClosingEvent(vtkMRMLScene* /*scene*/);
  virtual void
  OnMRMLSceneCloseEvent(vtkMRMLScene* /*scene*/);
  virtual void
  OnMRMLSceneLoadStartEvent(vtkMRMLScene* /*scene*/);
  virtual void
  OnMRMLSceneLoadEndEvent(vtkMRMLScene* /*scene*/);
  virtual void
  OnMRMLSceneRestoredEvent(vtkMRMLScene* /*scene*/);
  virtual void
  OnMRMLSceneNodeAddedEvent(vtkMRMLScene* /*scene*/, vtkMRMLNode* /*node*/);
  virtual void
  OnMRMLSceneNodeRemovedEvent(vtkMRMLScene* /*scene*/, vtkMRMLNode* /*node*/);

  /// check scene to make sure that have a widget for each ruler node, and no extra widgets...
  void
  Update3DWidget(vtkMRMLAnnotationTextNode *activeNode);

  /// encapsulated 3d widgets for each ruler node
  //std::map<std::string, vtkTextWidget *> TextWidgets;
  vtkTextWidget *
  GetTextWidget(const char * nodeID);
  void
  AddTextWidget(vtkMRMLAnnotationTextNode *node);
  void
  RemoveTextWidget(vtkMRMLAnnotationTextNode *node);
  void
  RemoveTextWidgets();

  /// Get/set the viewer widget so can add a the ruler widget to it
  //vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
  //virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

  void
  UpdateWidget(vtkMRMLAnnotationTextNode *activeNode);

  /// Invoke vtkCommand::UpdateEvent and then call vtkMRMLDisplayableManagerFactory::RequestRender()
  /// which will also invoke vtkCommand::UpdateEvent
  /// An observer can then listen for that event and "compress" the different Render requests
  /// to efficiently call RenderWindow->Render()
  void RequestRender();

  void
  Render();

protected:

  void
  UpdateWidgetInteractors();
  void
  UpdateLockUnlock(vtkMRMLAnnotationTextNode* textNode);

  /// the id of the mrml node currently displayed in the widget
  char* NodeID;

  /// flag set to 1 when updating a 3d widget (todo: set it to the index of the
  /// ruler node / 3d widget once have more than one)
  int Updating3DWidget;

  /// pointer to the viewer widget so can add props, request renders
  vtkSlicerViewerWidget *ViewerWidget;
  int ProcessingMRMLEvent;

private:


  // private constructor, access by getinstance
  vtkMRMLAnnotationTextDisplayableManager();



  //vtkMRMLAnnotationTextDisplayableManager(const vtkMRMLAnnotationTextDisplayableManager&); /// Not implemented
  void
  operator=(const vtkMRMLAnnotationTextDisplayableManager&); /// Not Implemented

};

#endif

