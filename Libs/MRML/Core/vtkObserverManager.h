/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkObserverManager.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

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
class vtkFloatArray;
class vtkUnsignedLongArray;

#ifndef vtkObjectPointer
#define vtkObjectPointer(xx) (reinterpret_cast <vtkObject **>( (xx) ))
#endif

/// \brief Manages adding and deleting of obserevers with events.
///
/// Class that manages adding and deleting of obserevers with events
/// This class keeps track of obserevers and events added to each vtk object
/// it caches tags returned by AddObserver method so that obserevers can be removed properly.
class VTK_MRML_EXPORT vtkObserverManager : public vtkObject
{
  public:

  /// The Usual vtk class functions
  static vtkObserverManager *New();
  vtkTypeMacro(vtkObserverManager,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// set vtkObject to a specified pointer and remove all observers for all events
  void SetObject(vtkObject **nodePtr, vtkObject *node);

  /// set vtkObject to a specified pointer, remove all observers for all events, add observer for Modify event
  void SetAndObserveObject(vtkObject **nodePtr, vtkObject *node, float priority=0.0, bool logWarningIfSameObservationExists=true);

  /// set vtkObject to a specified pointer, remove all observers for all events, add observers for specified events
  void SetAndObserveObjectEvents(vtkObject **nodePtr, vtkObject *node, vtkIntArray *events, vtkFloatArray *priorities=nullptr, bool logWarningIfSameObservationExists=true);

  /// remove all observers for all events
  void RemoveObjectEvents(vtkObject *nodePtr);

  /// get a list of all observed events and priorities for the selected node
  void GetObjectEvents(vtkObject *nodePtr, vtkIntArray *events, vtkFloatArray *priorities);

  /// Observe ModifiedEvent on the object
  void ObserveObject(vtkObject *node, float priority=0.0);

  /// add observers for specified events
  void AddObjectEvents(vtkObject *nodePtr, vtkIntArray *events, vtkFloatArray *priorities=nullptr);

  /// accessors for the owner class
  /// - note we do not hold a registered pointer to the owner
  ///   to avoid reference loops
  /// - the owner must be careful to always clean up the
  ///   ObserverManager in the destructor (this is the standard use case)
  vtkGetObjectMacro (Owner, vtkObject);
  void AssignOwner (vtkObject *owner) { this->Owner = owner; };
  /// Returns the owner if any, otherwise returns the manager itself
  vtkObject* GetObserver();

  vtkGetObjectMacro (CallbackCommand, vtkCallbackCommand);

  /// Return the number of observations by the manager on the node.
  /// If event is != 0 , only observations matching the events are counted
  int GetObservationsCount(vtkObject* nodePtr, unsigned long event = 0);
protected:
  vtkObserverManager();
  ~vtkObserverManager() override;
  vtkObserverManager(const vtkObserverManager&);
  void operator=(const vtkObserverManager&);

  ///
  /// The owner of the observer manager (e.g. the vtkMRMLNode)
  vtkObject *Owner;

  ///
  /// Holder for callback
  vtkCallbackCommand *CallbackCommand;

  std::map< vtkObject*, vtkUnsignedLongArray* > ObserverTags;

};

#endif
