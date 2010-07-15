/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayableManagerFactory.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

/// Factory where displayable manager classe should be registered with
/// 
/// A displayable manager class is responsible to represente a 
/// MRMLDisplayable node in a renderer.
/// 

#ifndef __vtkMRMLDisplayableManagerFactory_h
#define __vtkMRMLDisplayableManagerFactory_h

// VTK includes
#include <vtkObject.h>

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkRenderer;
class vtkMRMLDisplayableManagerGroup;
class vtkMRMLDisplayableManagerFactoryInitialize;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLDisplayableManagerFactory : public vtkObject 
{
public:

  vtkTypeRevisionMacro(vtkMRMLDisplayableManagerFactory,vtkObject);
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
  /// reference to a vtkMRMLDisplayableManagerFactory object per process. Clients that
  /// call this must call Delete on the object so that the reference counting will work.
  /// The single instance will be unreferenced when the program exits.
  static vtkMRMLDisplayableManagerFactory *New();

  ///
  /// Return the singleton instance with no reference counting.
  static vtkMRMLDisplayableManagerFactory* GetInstance();

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
  /// It returns a vtkMRMLDisplayableManagerGroup representing a list of DisplayableManager
  /// Internally, the factory keep track of all the Group and will invoke the ModifiedEvent
  /// of each group.
  vtkMRMLDisplayableManagerGroup* InstantiateDisplayableManagers(vtkRenderer * newRenderer);

protected:

  vtkMRMLDisplayableManagerFactory();
  virtual ~vtkMRMLDisplayableManagerFactory();

  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX

  //BTX
  typedef vtkMRMLDisplayableManagerFactory Self;
  friend class vtkMRMLDisplayableManagerFactoryInitialize;
  //ETX

  static vtkMRMLDisplayableManagerFactory* Instance;
  static void classInitialize();
  static void classFinalize();

private:

  vtkMRMLDisplayableManagerFactory(const vtkMRMLDisplayableManagerFactory&);
  void operator=(const vtkMRMLDisplayableManagerFactory&);

};

//----------------------------------------------------------------------------
//BTX
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLDisplayableManagerFactoryInitialize
{
public:
  typedef vtkMRMLDisplayableManagerFactoryInitialize Self;

  vtkMRMLDisplayableManagerFactoryInitialize();
  ~vtkMRMLDisplayableManagerFactoryInitialize();
private:
  static unsigned int Count;
};

/// The instance (vtkMRMLDisplayableManagerFactoryInitializer) will show up in any
/// translation unit that uses vtkMRMLDisplayableManagerFactory.
/// It will make sure vtkMRMLDisplayableManagerFactory is initialized before it is used.
static vtkMRMLDisplayableManagerFactoryInitialize vtkMRMLDisplayableManagerFactoryInitializer;
//ETX

#endif


