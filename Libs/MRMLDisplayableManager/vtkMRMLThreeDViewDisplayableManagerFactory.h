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

// MRMLDisplayableManager includes
#include "vtkMRMLDisplayableManagerFactory.h"

// VTK includes
#include <vtkSingleton.h>

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkRenderer;
class vtkMRMLThreeDViewDisplayableManagerGroup;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLThreeDViewDisplayableManagerFactory
  : public vtkMRMLDisplayableManagerFactory
{
public:

  vtkTypeRevisionMacro(vtkMRMLThreeDViewDisplayableManagerFactory,
                       vtkMRMLDisplayableManagerFactory);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// This is a singleton pattern New.  There will only be ONE
  /// reference to a vtkMRMLThreeDViewDisplayableManagerFactory object per process. Clients that
  /// call this must call Delete on the object so that the reference counting will work.
  /// The single instance will be unreferenced when the program exits.
  static vtkMRMLThreeDViewDisplayableManagerFactory *New();

  ///
  /// Return the singleton instance with no reference counting.
  static vtkMRMLThreeDViewDisplayableManagerFactory* GetInstance();

protected:

  vtkMRMLThreeDViewDisplayableManagerFactory();
  virtual ~vtkMRMLThreeDViewDisplayableManagerFactory();

  //BTX
  VTK_SINGLETON_DECLARE(vtkMRMLThreeDViewDisplayableManagerFactory);
  //ETX

private:

  vtkMRMLThreeDViewDisplayableManagerFactory(const vtkMRMLThreeDViewDisplayableManagerFactory&);
  void operator=(const vtkMRMLThreeDViewDisplayableManagerFactory&);

};

//BTX
VTK_SINGLETON_DECLARE_INITIALIZER(VTK_MRML_DISPLAYABLEMANAGER_EXPORT,
                                  vtkMRMLThreeDViewDisplayableManagerFactory);
//ETX

#endif


