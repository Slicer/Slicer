/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
/// \deprecated Used for backward compatibility for Slicer3 fiducial lists, please use the
///  Annotation Module Logic
///
///  vtkSlicerFiducialsLogic - slicer logic class for volumes manipulation
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the volumes


#ifndef __vtkSlicerFiducialsLogic_h
#define __vtkSlicerFiducialsLogic_h

// SlicerLogic includes
#include "vtkSlicerBaseLogic.h"

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

// MRML includes
class vtkMRMLFiducial;
class vtkMRMLFiducialListNode;

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerFiducialsLogic : public vtkMRMLAbstractLogic
{
  public:

  /// The Usual vtk class functions
  static vtkSlicerFiducialsLogic *New();
  vtkTypeMacro(vtkSlicerFiducialsLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Get the currently selected list from the scene. Returns nullptr if no
  /// selection node or no active fiducial list id is set on the selection
  /// node.
  vtkMRMLFiducialListNode *GetSelectedList();

  ///
  /// Create new mrml node for a full list, make it the selected list, and clear up local pointers
  void AddFiducialListSelected();

  ///
  /// Create new mrml node and associated display node for a full list,
  /// return the node
  vtkMRMLFiducialListNode * AddFiducialList();

  ///
  /// Add a fiducial to the currently selected list, as kept in the
  /// vtkMRMLSelectionNode
  /// Returns the index of the new fiducial in the list, -1 on failure
  /// AddFiducialSelected includes a selected flag option, AddFiducial calls
  /// AddFiducialSelected with selected set to false.
  int AddFiducial(float x, float y, float z);
  int AddFiducialSelected (float x, float y, float z, int selected);

  /// Add a fiducial, but transform it first by the inverse of any
  /// transformation node on the list. Called by Pick methods. Calls
  /// AddFiducialSelected with the transformed x,y,z and same selected flag
  /// (defaults to 0).
  int AddFiducialPicked(float x, float y, float z, int selected = 0);

  ///
  /// Load a fiducial list from file, returns nullptr on failure
  vtkMRMLFiducialListNode *LoadFiducialList(const char *path);

protected:
  vtkSlicerFiducialsLogic();
  ~vtkSlicerFiducialsLogic() override;
  vtkSlicerFiducialsLogic(const vtkSlicerFiducialsLogic&);
  void operator=(const vtkSlicerFiducialsLogic&);
};

#endif

