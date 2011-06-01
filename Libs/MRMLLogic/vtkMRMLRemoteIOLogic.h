/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLRemoteIOLogic.h,v $
  Date:      $Date: 2011-01-07 10:39:33 -0500 (Fri, 07 Jan 2011) $
  Version:   $Revision: 15750 $

=========================================================================auto=*/

///  vtkMRMLRemoteIOLogic - MRML logic class for color manipulation
///
/// This class manages the logic associated with instrumenting
/// a MRML scene instance with RemoteIO functionality (so vtkMRMLStorableNodes
/// can access data by URL)

#ifndef __vtkMRMLRemoteIOLogic_h
#define __vtkMRMLRemoteIOLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLLogicWin32Header.h"

// STD includes
#include <stdlib.h>

class vtkCacheManager;
class vtkDataIOManager;

class VTK_MRML_LOGIC_EXPORT vtkMRMLRemoteIOLogic : public vtkMRMLAbstractLogic
{
public:
  /// The Usual vtk class functions
  static vtkMRMLRemoteIOLogic *New();
  vtkTypeRevisionMacro(vtkMRMLRemoteIOLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  void AddDataIOToScene();
  void RemoveDataIOFromScene();

  /// 
  /// Accessors for the comonents of the remote IO infrascucture
  /// Note that the internal instances are created in the constructor
  /// and used when calling AddDataIOToScene
  /// and RemoveDataIOFromScene
  /// The Get methods can be used elsewhere, but the set methods
  /// should only be used for debuggin
  vtkGetObjectMacro (CacheManager, vtkCacheManager);
  vtkSetObjectMacro (CacheManager, vtkCacheManager);
  vtkGetObjectMacro (DataIOManager, vtkDataIOManager);
  vtkSetObjectMacro (DataIOManager, vtkDataIOManager);

protected:
  vtkMRMLRemoteIOLogic();
  virtual ~vtkMRMLRemoteIOLogic();
  // disable copy constructor and operator
  vtkMRMLRemoteIOLogic(const vtkMRMLRemoteIOLogic&);
  void operator=(const vtkMRMLRemoteIOLogic&);

  vtkCacheManager *          CacheManager;
  vtkDataIOManager *         DataIOManager;


};

#endif

