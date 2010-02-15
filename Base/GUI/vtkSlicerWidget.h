/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkSlicerNodeSelectorWidget - menu to select volumes from current mrml scene
/// 
/// Inherits most behavior from kw widget, but is specialized to observe
/// the current mrml scene 
//


#ifndef __vtkSlicerWidget_h
#define __vtkSlicerWidget_h

#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkUnsignedLongArray.h"
#include "vtkKWObject.h"
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWCompositeWidget.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkObserverManager.h"

class vtkCallbackCommmand;

//BTX

#ifndef vtkSetMRMLNodeMacro
#define vtkSetMRMLNodeMacro(node,value)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetObject ( vtkObjectPointer( &(node)), (value) ); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

#ifndef vtkSetAndObserveMRMLNodeMacro
#define vtkSetAndObserveMRMLNodeMacro(node,value)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node) ), (value) ); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

#ifndef vtkSetAndObserveMRMLNodeEventsMacro
#define vtkSetAndObserveMRMLNodeEventsMacro(node,value,events)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &(node)), (value), (events)); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

//ETX


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerWidget : public vtkKWCompositeWidget
{
  
public:
  static vtkSlicerWidget* New(){return NULL;};
  vtkTypeRevisionMacro(vtkSlicerWidget,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  /// 
  /// Getting setting and observing MRMLScene.
  vtkGetObjectMacro ( MRMLScene, vtkMRMLScene );
  
  /// 
  /// API for setting or setting and observing MRMLScene
  void SetMRMLScene ( vtkMRMLScene *mrml )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetObject ( vtkObjectPointer( &this->MRMLScene), mrml );
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }

  void SetAndObserveMRMLScene ( vtkMRMLScene *mrml )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &this->MRMLScene), mrml );
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }

  void SetAndObserveMRMLSceneEvents ( vtkMRMLScene *mrml, vtkIntArray *events )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &this->MRMLScene), mrml, events );
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }
  
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *vtkNotUsed(caller),
                                     unsigned long vtkNotUsed(event),
                                     void *vtkNotUsed(callData) ) { };
  
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *vtkNotUsed(caller),
                                   unsigned long vtkNotUsed(event),
                                   void *vtkNotUsed(callData) ) { };

  /// 
  /// Flags to avoid event loops
  /// NOTE: don't use the SetMacro or it call modified itself and generate even more events!
  vtkGetMacro(InWidgetCallbackFlag, int);
  void SetInWidgetCallbackFlag (int flag) {
    this->InWidgetCallbackFlag = flag;
  }
  vtkGetMacro(InMRMLCallbackFlag, int);
  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }


protected:
  vtkSlicerWidget();
  virtual ~vtkSlicerWidget();

  /// 
  /// Create the widget.
  virtual void CreateWidget() {this->Superclass::CreateWidget();};

  vtkMRMLScene       *MRMLScene;

  vtkObserverManager *MRMLObserverManager;

  //BTX
  /// a shared set of functions that call the
  /// virtual ProcessMRMLEvents
  /// and ProcessGUIEvents methods in the
  /// subclasses, if they are defined.
  static void MRMLCallback( vtkObject *caller,
                            unsigned long eid, void *clientData, void *callData );
  static void WidgetCallback( vtkObject *caller,
                           unsigned long eid, void *clientData, void *callData );    
  //ETX
  
  /// :
  /// Holders for MRML, GUI and Logic callbacks
  vtkCallbackCommand *MRMLCallbackCommand;
  vtkCallbackCommand *GUICallbackCommand;

  /// 
  /// Flag to avoid event loops
  int InWidgetCallbackFlag;
  int InMRMLCallbackFlag;
  
  vtkSlicerWidget(const vtkSlicerWidget&); /// Not implemented
  void operator=(const vtkSlicerWidget&); /// Not Implemented
};

#endif

