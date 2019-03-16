/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
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

// STD includes
#include <cstdlib>

class vtkCacheManager;
class vtkDataIOManager;

class VTK_MRML_LOGIC_EXPORT vtkMRMLRemoteIOLogic : public vtkMRMLAbstractLogic
{
public:
  /// The Usual vtk class functions
  static vtkMRMLRemoteIOLogic *New();
  vtkTypeMacro(vtkMRMLRemoteIOLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

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
  virtual void SetCacheManager(vtkCacheManager*);
  vtkGetObjectMacro (DataIOManager, vtkDataIOManager);
  virtual void SetDataIOManager(vtkDataIOManager*);

protected:
  vtkMRMLRemoteIOLogic();
  ~vtkMRMLRemoteIOLogic() override;
  // disable copy constructor and operator
  vtkMRMLRemoteIOLogic(const vtkMRMLRemoteIOLogic&);
  void operator=(const vtkMRMLRemoteIOLogic&);

  vtkCacheManager *          CacheManager;
  vtkDataIOManager *         DataIOManager;
};

#endif
