/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkSlicerLogic - superclass for slicer logic classes
// .SECTION Description
// Superclass for all slicer logic classes (application, views, slices).
// There must be a corresponding vtkSlicerGUI subclass corresponding 
// to each logic class that handles all GUI interaction (no GUI code
// goes in the logic class).

#ifndef __vtkSlicerLogic_h
#define __vtkSlicerLogic_h

#include "vtkSlicerBaseLogic.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkMRMLScene.h"

class vtkCallbackCommand;

#ifndef vtkObjectPointer
#define vtkObjectPointer(xx) (reinterpret_cast <vtkObject **>( (xx) ))
#endif

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerLogic : public vtkObject 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerLogic *New();
  vtkTypeRevisionMacro(vtkSlicerLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description
  // All logic classes need to know about the current mrml scene
  vtkGetObjectMacro (MRMLScene, vtkMRMLScene);

  // Description:
  // API for setting or setting and observing MRMLScene
  void SetMRMLScene ( vtkMRMLScene *mrml )
      {
          this->SetMRML ( vtkObjectPointer( &this->MRMLScene), mrml );
      }
  void SetAndObserveMRMLScene ( vtkMRMLScene *mrml )
      {
          this->SetAndObserveMRML ( vtkObjectPointer( &this->MRMLScene), mrml );
      }


  virtual void ProcessMRMLEvents() {};
  virtual void ProcessLogicEvents() {};

  // Description:
  // Flags to avoid event loops
  // NOTE: don't use the SetMacro or it call modified itself and generate even more events!
  void SetInLogicCallbackFlag (int flag) {
    this->InLogicCallbackFlag = flag;
  }
  vtkGetMacro(InLogicCallbackFlag, int);
  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }
  vtkGetMacro(InMRMLCallbackFlag, int);

  // Additional functionality:
    
protected:
  vtkSlicerLogic();
  ~vtkSlicerLogic();
  vtkSlicerLogic(const vtkSlicerLogic&);
  void operator=(const vtkSlicerLogic&);

  vtkMRMLScene *MRMLScene;

  //BTX
  // a shared set of functions that call the 
  // virtual ProcessMRMLEvents and ProcessLogicEvents methods in the
  // subclasses (if they are defined)
  static void MRMLCallback(vtkObject *caller, 
                unsigned long eid, void *clientData, void *callData);
  static void LogicCallback(vtkObject *caller, 
                unsigned long eid, void *clientData, void *callData);

  // functions that set MRML for the Logic class,
  // either with or without adding/removirg observers on them.
  void SetMRML(vtkObject **nodePtr, vtkObject *node);
  void SetAndObserveMRML(vtkObject **nodePtr, vtkObject *node);
  //ETX

  // Description:
  // Holder for MRML and Logic callbacks
  vtkCallbackCommand *MRMLCallbackCommand;
  vtkCallbackCommand *LogicCallbackCommand;

  // Description:
  // Flag to avoid event loops
  int InLogicCallbackFlag;
  int InMRMLCallbackFlag;
 
};

#endif

