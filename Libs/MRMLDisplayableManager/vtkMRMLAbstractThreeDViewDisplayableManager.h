/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractThreeDViewDisplayableManager.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

/// Superclass for displayable manager classes.
/// 
/// A displayable manager class is responsible to represent a
/// MRMLDisplayable node in a renderer.
/// 


#ifndef __vtkMRMLAbstractThreeDViewDisplayableManager_h
#define __vtkMRMLAbstractThreeDViewDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLViewNode;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLAbstractThreeDViewDisplayableManager :
    public vtkMRMLAbstractDisplayableManager
{
public:
  
  static vtkMRMLAbstractThreeDViewDisplayableManager *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMRMLAbstractThreeDViewDisplayableManager,
                       vtkMRMLAbstractDisplayableManager);

  ///
  /// Get MRML ViewNode
  vtkMRMLViewNode * GetMRMLViewNode();

protected:

  vtkMRMLAbstractThreeDViewDisplayableManager();
  virtual ~vtkMRMLAbstractThreeDViewDisplayableManager();
  
private:

  vtkMRMLAbstractThreeDViewDisplayableManager(const vtkMRMLAbstractThreeDViewDisplayableManager&); // Not implemented
  void operator=(const vtkMRMLAbstractThreeDViewDisplayableManager&);                    // Not implemented
};

#endif

