/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkObserverManager.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
///  vtkObserverManager - class that manages adding and deleting of obserevers with events
/// 
/// Class that manages adding and deleting of obserevers with events
/// This class keeps track of obserevers and events added to each vtk object 
/// it caches tags returned by AddObserver method so that obserevers can be removed properly

#ifndef __vtkObserverManager_h
#define __vtkObserverManager_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkObject.h>

// STD includes
#include <map>

class vtkCallbackCommand;
class vtkIntArray;
class vtkUnsignedLongArray;

#ifndef vtkObjectPointer
#define vtkObjectPointer(xx) (reinterpret_cast <vtkObject **>( (xx) ))
#endif

class VTK_MRML_EXPORT vtkObserverManager : public vtkObject 
{
  public:
  
  /// The Usual vtk class functions
  static vtkObserverManager *New();
  vtkTypeRevisionMacro(vtkObserverManager,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// set vtkObject to a specified pointer and remove all observers for all events
  void SetObject(vtkObject **nodePtr, vtkObject *node);

  /// set vtkObject to a specified pointer, remove all observers for all events, add observer for Modify event
  void SetAndObserveObject(vtkObject **nodePtr, vtkObject *node);

  /// set vtkObject to a specified pointer, remove all observers for all events, add observers for specified events
  void SetAndObserveObjectEvents(vtkObject **nodePtr, vtkObject *node, vtkIntArray *events);

  /// remove all observers for all events
  void RemoveObjectEvents(vtkObject *nodePtr);

  /// Observe ModifiedEvent on the object
  void ObserveObject(vtkObject *node);

  /// add observers for specified events
  void AddObjectEvents(vtkObject *nodePtr, vtkIntArray *events);

  /// accessors for the owner class
  /// - note we do not hold a registered pointer to the owner
  ///   to avoid reference loops
  /// - the owner must be careful to always clean up the 
  ///   ObserverManager in the destructor (this is the standard use case)
  vtkGetObjectMacro (Owner, vtkObject);
  void AssignOwner (vtkObject *owner) { this->Owner = owner; };

  vtkGetObjectMacro (CallbackCommand, vtkCallbackCommand);

protected:
  vtkObserverManager();
  virtual ~vtkObserverManager();
  vtkObserverManager(const vtkObserverManager&);
  void operator=(const vtkObserverManager&);

  /// 
  /// The owner of the observer manager (e.g. the vtkMRMLNode)
  vtkObject *Owner;

  /// 
  /// Holder for callback
  vtkCallbackCommand *CallbackCommand;

  //BTX
  std::map< vtkObject*, vtkUnsignedLongArray* > ObserverTags;
  //ETX

};

#endif

