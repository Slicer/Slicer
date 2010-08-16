/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractSliceViewDisplayableManager.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

/// Superclass for displayable manager classes.
/// 
/// A displayable manager class is responsible to represent a
/// MRMLDisplayable node in a renderer.
/// 


#ifndef __vtkMRMLAbstractSliceViewDisplayableManager_h
#define __vtkMRMLAbstractSliceViewDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLSliceNode;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLAbstractSliceViewDisplayableManager :
    public vtkMRMLAbstractDisplayableManager
{
public:
  
  static vtkMRMLAbstractSliceViewDisplayableManager *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMRMLAbstractSliceViewDisplayableManager,
                       vtkMRMLAbstractDisplayableManager);

  ///
  /// Get MRML SliceNode
  vtkMRMLSliceNode * GetMRMLSliceNode();

protected:

  vtkMRMLAbstractSliceViewDisplayableManager();
  virtual ~vtkMRMLAbstractSliceViewDisplayableManager();

  virtual void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller);

  /// Could be overloaded if DisplayableManager subclass
  virtual void OnMRMLSliceNodeModifiedEvent(){}
  
private:

  vtkMRMLAbstractSliceViewDisplayableManager(const vtkMRMLAbstractSliceViewDisplayableManager&); // Not implemented
  void operator=(const vtkMRMLAbstractSliceViewDisplayableManager&);                    // Not implemented
};

#endif

