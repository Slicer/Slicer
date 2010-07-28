/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLThreeDViewDisplayableManagerFactory.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

/// Factory where displayable manager classe should be registered with
/// 
/// A displayable manager class is responsible to represente a 
/// MRMLDisplayable node in a renderer.
/// 

#ifndef __vtkMRMLThreeDViewDisplayableManagerFactory_h
#define __vtkMRMLThreeDViewDisplayableManagerFactory_h

// VTK includes
#include <vtkObject.h>

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkRenderer;
class vtkMRMLThreeDViewDisplayableManagerGroup;
class vtkMRMLThreeDViewDisplayableManagerFactoryInitialize;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLThreeDViewDisplayableManagerFactory : public vtkObject 
{
public:

  vtkTypeRevisionMacro(vtkMRMLThreeDViewDisplayableManagerFactory,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //BTX
  enum
  {
    DisplayableManagerFactoryRegisteredEvent     = 30100,
    DisplayableManagerFactoryUnRegisteredEvent   = 30101
  };
  //ETX

  ///
  /// This is a singleton pattern New.  There will only be ONE
  /// reference to a vtkMRMLThreeDViewDisplayableManagerFactory object per process. Clients that
  /// call this must call Delete on the object so that the reference counting will work.
  /// The single instance will be unreferenced when the program exits.
  static vtkMRMLThreeDViewDisplayableManagerFactory *New();

  ///
  /// Return the singleton instance with no reference counting.
  static vtkMRMLThreeDViewDisplayableManagerFactory* GetInstance();

  ///
  /// Return True if Displayable Manager identified by \a vtkClassName
  /// is already registered.
  bool IsDisplayableManagerRegistered(const char* vtkClassName);

  ///
  /// Register Displayable Manager identified by \a vtkClassName
  /// \a vtkClassName should be a VTK class registered using the CMake macro VTK_MAKE_INSTANTIATOR3
  void RegisterDisplayableManager(const char* vtkClassName);

  ///
  /// UnRegister Displayable Manager identified by \a vtkClassName
  /// \a vtkClassName should be a VTK class registered using the CMake macro VTK_MAKE_INSTANTIATOR3
  void UnRegisterDisplayableManager(const char* vtkClassName);

  ///
  /// Return number of registered displayable managers
  int GetRegisteredDisplayableManagerCount();
  
  /// Instantiate registrered DisplayableManagers
  /// It returns a vtkMRMLThreeDViewDisplayableManagerGroup representing a list of DisplayableManager
  /// Internally, the factory keep track of all the Group and will invoke the ModifiedEvent
  /// of each group.
  vtkMRMLThreeDViewDisplayableManagerGroup* InstantiateDisplayableManagers(vtkRenderer * newRenderer);

protected:

  vtkMRMLThreeDViewDisplayableManagerFactory();
  virtual ~vtkMRMLThreeDViewDisplayableManagerFactory();

  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX

  //BTX
  typedef vtkMRMLThreeDViewDisplayableManagerFactory Self;
  friend class vtkMRMLThreeDViewDisplayableManagerFactoryInitialize;
  //ETX

  static vtkMRMLThreeDViewDisplayableManagerFactory* Instance;
  static void classInitialize();
  static void classFinalize();

private:

  vtkMRMLThreeDViewDisplayableManagerFactory(const vtkMRMLThreeDViewDisplayableManagerFactory&);
  void operator=(const vtkMRMLThreeDViewDisplayableManagerFactory&);

};

//----------------------------------------------------------------------------
//BTX
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLThreeDViewDisplayableManagerFactoryInitialize
{
public:
  typedef vtkMRMLThreeDViewDisplayableManagerFactoryInitialize Self;

  vtkMRMLThreeDViewDisplayableManagerFactoryInitialize();
  ~vtkMRMLThreeDViewDisplayableManagerFactoryInitialize();
private:
  static unsigned int Count;
};

/// The instance (vtkMRMLThreeDViewDisplayableManagerFactoryInitializer) will show up in any
/// translation unit that uses vtkMRMLThreeDViewDisplayableManagerFactory.
/// It will make sure vtkMRMLThreeDViewDisplayableManagerFactory is initialized before it is used.
static vtkMRMLThreeDViewDisplayableManagerFactoryInitialize vtkMRMLThreeDViewDisplayableManagerFactoryInitializer;
//ETX

#endif


