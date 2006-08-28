/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerNodeSelectorWidget - menu to select volumes from current mrml scene
// .SECTION Description
// Inherits most behavior from kw widget, but is specialized to observe
// the current mrml scene 
//


#ifndef __vtkSlicerWidget_h
#define __vtkSlicerWidget_h

#include "vtkCallbackCommand.h"
#include "vtkUnsignedLongArray.h"
#include "vtkKWObject.h"
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWCompositeWidget.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

class vtkCallbackCommmand;

#ifndef vtkObjectPointer
#define vtkObjectPointer( xx ) (reinterpret_cast <vtkObject **>( (xx) ))
#endif
       
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerWidget : public vtkKWCompositeWidget
{
  
public:
  static vtkSlicerWidget* New(){return NULL;};
  vtkTypeRevisionMacro(vtkSlicerWidget,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Getting setting and observing MRMLScene.
  vtkGetObjectMacro ( MRMLScene, vtkMRMLScene );
  
  void SetMRMLScene ( vtkMRMLScene *mrml )
    { this->SetMRML ( vtkObjectPointer( &this->MRMLScene), mrml ); };
  
  
  void SetAndObserveMRMLScene ( vtkMRMLScene *mrml )
    { this->SetAndObserveMRML ( vtkObjectPointer( &this->MRMLScene), mrml ); };

  void AddMRMLObserver ( vtkObject *obj, unsigned long event ) 
    { obj->AddObserver ( event, this->MRMLCallbackCommand ); };

  void RemoveMRMLObserver ( vtkObject *obj, unsigned long event ) 
    { obj->RemoveObservers ( event, this->MRMLCallbackCommand ); };
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData ) { };
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData ) { };

  // Description:
  // Flags to avoid event loops
  // NOTE: don't use the SetMacro or it call modified itself and generate even more events!
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

  // Description:
  // Create the widget.
  virtual void CreateWidget() {this->Superclass::CreateWidget();};

  vtkMRMLScene       *MRMLScene;
  
  //BTX
  // a shared set of functions that call the
  // virtual ProcessMRMLEvents
  // and ProcessGUIEvents methods in the
  // subclasses, if they are defined.
  static void MRMLCallback( vtkObject *caller,
                            unsigned long eid, void *clientData, void *callData );
  static void WidgetCallback( vtkObject *caller,
                           unsigned long eid, void *clientData, void *callData );    
  
  // functions that set MRML pointers for the GUI class,
  // either with or without adding/removing observers on them.
  void SetMRML ( vtkObject **nodePtr, vtkObject *node );
  void SetAndObserveMRML ( vtkObject **nodePtr, vtkObject *node );

  //ETX
  
  // Description::
  // Holders for MRML, GUI and Logic callbacks
  vtkCallbackCommand *MRMLCallbackCommand;
  vtkCallbackCommand *GUICallbackCommand;

  // Description:
  // Flag to avoid event loops
  int InWidgetCallbackFlag;
  int InMRMLCallbackFlag;
  
  vtkSlicerWidget(const vtkSlicerWidget&); // Not implemented
  void operator=(const vtkSlicerWidget&); // Not Implemented
};

#endif

