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
// the current mrml scene and update the entries of the pop up menu to correspond
// to the currently available volumes.  This widget also has a notion of the current selection
// that can be observed or set externally
//


#ifndef __vtkSlicerWidget_h
#define __vtkSlicerWidget_h

#include "vtkCallbackCommand.h"

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
    { this->SetMRML ( vtkObjectPointer( &this->MRMLScene), mrml ); };
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData ) { };
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData ) { };
  

protected:
  vtkSlicerWidget();
  ~vtkSlicerWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget() = 0;

  vtkMRMLScene       *MRMLScene;
  
  //BTX
  // a shared set of functions that call the
  // virtual ProcessMRMLEvents, ProcessLogicEvents,
  // and ProcessGUIEvents methods in the
  // subclasses, if they are defined.
  static void MRMLCallback( vtkObject *__caller,
                            unsigned long eid, void *__clientData, void *callData );
  static void GUICallback( vtkObject *__caller,
                           unsigned long eid, void *__clientData, void *callData );    
  
  // functions that set MRML and Logic pointers for the GUI class,
  // either with or without adding/removing observers on them.
  void SetMRML ( vtkObject **nodePtr, vtkObject *node );
  void SetAndObserveMRML ( vtkObject **nodePtr, vtkObject *node );
  //ETX
  
  // Description::
  // Holders for MRML, GUI and Logic callbacks
  vtkCallbackCommand *MRMLCallbackCommand;
  vtkCallbackCommand *GUICallbackCommand;
  
  vtkSlicerWidget(const vtkSlicerWidget&); // Not implemented
  void operator=(const vtkSlicerWidget&); // Not Implemented
};

#endif

