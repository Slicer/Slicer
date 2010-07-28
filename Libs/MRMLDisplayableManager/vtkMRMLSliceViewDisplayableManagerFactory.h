/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceViewDisplayableManagerFactory.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

/// Factory where displayable manager classe should be registered with
/// 
/// A displayable manager class is responsible to represente a 
/// MRMLDisplayable node in a renderer.
/// 

#ifndef __vtkMRMLSliceViewDisplayableManagerFactory_h
#define __vtkMRMLSliceViewDisplayableManagerFactory_h

// MRMLDisplayableManager includes
#include "vtkMRMLDisplayableManagerFactory.h"

// VTK includes
#include <vtkSingleton.h>

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkRenderer;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLSliceViewDisplayableManagerFactory
  : public vtkMRMLDisplayableManagerFactory
{
public:

  vtkTypeRevisionMacro(vtkMRMLSliceViewDisplayableManagerFactory,
                       vtkMRMLDisplayableManagerFactory);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// This is a singleton pattern New.  There will only be ONE
  /// reference to a vtkMRMLSliceViewDisplayableManagerFactory object per process. Clients that
  /// call this must call Delete on the object so that the reference counting will work.
  /// The single instance will be unreferenced when the program exits.
  static vtkMRMLSliceViewDisplayableManagerFactory *New();

  ///
  /// Return the singleton instance with no reference counting.
  static vtkMRMLSliceViewDisplayableManagerFactory* GetInstance();

protected:

  vtkMRMLSliceViewDisplayableManagerFactory();
  virtual ~vtkMRMLSliceViewDisplayableManagerFactory();

  //BTX
  VTK_SINGLETON_DECLARE(vtkMRMLSliceViewDisplayableManagerFactory);
  //ETX

private:

  vtkMRMLSliceViewDisplayableManagerFactory(const vtkMRMLSliceViewDisplayableManagerFactory&);
  void operator=(const vtkMRMLSliceViewDisplayableManagerFactory&);

};

//BTX
VTK_SINGLETON_DECLARE_INITIALIZER(VTK_MRML_DISPLAYABLEMANAGER_EXPORT,
                                  vtkMRMLSliceViewDisplayableManagerFactory);
//ETX

#endif


