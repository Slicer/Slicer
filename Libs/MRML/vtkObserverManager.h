/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkObserverManager.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkObserverManager - superclass for slicer logic classes
// .SECTION Description
// Superclass for all slicer logic classes (application, views, slices).
// There must be a corresponding vtkSlicerGUI subclass corresponding 
// to each logic class that handles all GUI interaction (no GUI code
// goes in the logic class).

#ifndef __vtkObserverManager_h
#define __vtkObserverManager_h

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkUnsignedLongArray.h"
#include "vtkIntArray.h"

#include "vtkMRML.h"

#include <map>
#include <string>

class vtkCallbackCommand;

#ifndef vtkObjectPointer
#define vtkObjectPointer(xx) (reinterpret_cast <vtkObject **>( (xx) ))
#endif

class VTK_MRML_EXPORT vtkObserverManager : public vtkObject 
{
  public:
  
  // The Usual vtk class functions
  static vtkObserverManager *New();
  vtkTypeRevisionMacro(vtkObserverManager,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // functions that set vtkObjects
  // either with or without adding/removirg observers on them.
  void SetObject(vtkObject **nodePtr, vtkObject *node);
  void SetAndObserveObject(vtkObject **nodePtr, vtkObject *node);
  void SetAndObserveObjectEvents(vtkObject **nodePtr, vtkObject *node, vtkIntArray *events);

  void RemoveObjectEvents(vtkObject *nodePtr);
  void AddObjectEvents(vtkObject *nodePtr, vtkIntArray *events);
  
  vtkGetObjectMacro (CallbackCommand, vtkCallbackCommand);

protected:
  vtkObserverManager();
  virtual ~vtkObserverManager();
  vtkObserverManager(const vtkObserverManager&);
  void operator=(const vtkObserverManager&);

  //ETX

  // Description:
  // Holder for callback
  vtkCallbackCommand *CallbackCommand;

  //BTX
  std::map< vtkObject*, vtkUnsignedLongArray* > ObserverTags;
  //ETX

};

#endif

