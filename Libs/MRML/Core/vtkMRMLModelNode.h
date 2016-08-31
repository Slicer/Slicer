/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLModelNode_h
#define __vtkMRMLModelNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"
class vtkMRMLModelDisplayNode;
class vtkMRMLStorageNode;

// VTK includes
class vtkAlgorithmOutput;
class vtkAssignAttributes;
class vtkEventForwarderCommand;
class vtkDataArray;
class vtkPolyData;
class vtkMRMLDisplayNode;

/// \brief MRML node to represent a 3D surface model.
///
/// Model nodes describe polygonal data.
/// When a model display node (vtkMRMLModelDisplayNode) is observed by the
/// model, the output polydata is automatically set to the input of the model
/// display node: You don't have to manually set the polydata yourself.
/// Models are assumed to have been constructed with the orientation and voxel
/// dimensions of the original segmented volume.
class VTK_MRML_EXPORT vtkMRMLModelNode : public vtkMRMLDisplayableNode
{
public:
  static vtkMRMLModelNode *New();
  vtkTypeMacro(vtkMRMLModelNode,vtkMRMLDisplayableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Model";};

 /// Description:
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );

  /// Get associated model display MRML node
  vtkMRMLModelDisplayNode* GetModelDisplayNode();

  /// Set and observe poly data for this model.
  /// \deprecated
  /// \sa GetPolyData()
  virtual void SetAndObservePolyData(vtkPolyData *PolyData);

  /// Return the input poly data
  /// \sa SetAndObservePolyData()
  virtual vtkPolyData* GetPolyData();
  /// Set and observe poly data pipeline.
  /// It is propagated to the display nodes.
  /// \sa GetPolyDataConnection()
  virtual void SetPolyDataConnection(vtkAlgorithmOutput *inputPort);
  /// Return the input polydata pipeline.
  vtkGetObjectMacro(PolyDataConnection, vtkAlgorithmOutput);

  /// PolyDataModifiedEvent is fired when PolyData is changed.
  /// While it is possible for the subclasses to fire PolyDataModifiedEvent
  /// without modifying the polydata, it is not recommended to do so as it
  /// doesn't mark the polydata as modified, which my result in an incorrect
  /// return value for GetModifiedSinceRead()
  /// \sa GetModifiedSinceRead()
  enum
    {
    PolyDataModifiedEvent = 17001
    };

  /// Utility function that adds an array to the polydata's point data.
  //// \sa AddCellScalars, AddScalars
  void AddPointScalars(vtkDataArray *array);

  /// Add an array to the polydata's cell data.
  /// \sa AddPointScalars, AddScalars
  void AddCellScalars(vtkDataArray *array);

  /// Add an array to the polydata's point or cell data
  /// location is either vtkAssignAttribute::POINT_DATA or
  /// vtkAssignAttribute::CELL_DATA
  void AddScalars(vtkDataArray *array, int location);

  /// Remove an array from the polydata's point/cell data.
  void RemoveScalars(const char *scalarName);

  /// Return true if the polydata point data has an array with a
  /// \a scalarName name.
  /// \sa HasPointScalarName
  bool HasPointScalarName(const char* scalarName);

  /// Return true if the polydata cell data has an array with a
  /// \a scalarName name.
  /// \sa HasCellScalarName
  bool HasCellScalarName(const char* scalarName);

  /// Set the active poly data point scalar array, checks for the
  /// scalarName as being a valid point array, and then will set it to be the active
  /// attribute type as designated by typeName (scalars if null or
  /// empty). typeName is one of the valid strings as returned from
  /// vtkDataSetAttributes::GetAttributeTypeAsString, SetActiveScalars converts
  /// it to an integer type to pass onto the Point/Cell methods
  /// Also updates the display node's active scalars
  int SetActivePointScalars(const char *scalarName, int attributeType);

  /// Get the currently active point array name, type =
  /// vtkDataSetAttributes::AttributeTypes for an active array.
  /// Returns an empty string if it can't find one or if no input polydata
  /// is set.
  /// \sa GetActiveCellScalarName
  const char *GetActivePointScalarName(int type);

  /// Set the active poly data point scalar array, checks for the
  /// scalarName as being a valid point array, and then will set it to be the active
  /// attribute type as designated by typeName (scalars if null or
  /// empty). typeName is one of the valid strings as returned from
  /// vtkDataSetAttributes::GetAttributeTypeAsString, SetActiveScalars converts
  /// it to an integer type to pass onto the Point/Cell methods
  /// Also updates the display node's active scalars
  int SetActiveCellScalars(const char *scalarName, int attributeType);

  /// Get the currently active Point/Cell array name, type =
  /// vtkDataSetAttributes::AttributeTypes for an active array.
  /// Returns an empty string if it can't find one or if no input polydata
  /// is set.
  const char *GetActiveCellScalarName(int type);

  /// Utility function that returns the attribute type from its name.
  /// It is the opposite of vtkDataSetAttributes::GetAttributeTypeAsString(int)
  /// \sa vtkDataSetAttributes::GetAttributeTypeAsString()
  static int GetAttributeTypeFromString(const char* typeName);

  /// Take scalar fields and composite them into a new one.
  /// New array will have values from the background array where the overlay is
  /// +/- if showOverlayPositive/Negative are 0.
  /// overlayMin and Max are used to adjust the color transfer function points,
  /// both should be positive, as they are mirrored around 0. -Min to Min gives the gap
  /// where the curvature will show through.
  /// New array name is backgroundName+overlayName
  /// Returns 1 on success, 0 on failure.
  /// Based on code from K. Teich, MGH
  /// Warning: Not demand driven pipeline compliant
  int CompositeScalars(const char* backgroundName, const char* overlayName,
                       float overlayMin, float overlayMax,
                       int showOverlayPositive, int showOverlayNegative,
                       int reverseOverlay);

  /// Get bounding box in global RAS the form (xmin,xmax, ymin,ymax, zmin,zmax).
  virtual void GetRASBounds(double bounds[6]);

  /// Transforms bounds from the local coordinate system to the RAS (world)
  /// coordinate system. Only the corner points are used for determining the
  /// new bounds, therefore in case of non-linear transforms the transformed
  /// bounds may not fully contain the transformed model points.
  virtual void TransformBoundsToRAS(double inputBounds_Local[6], double outputBounds_RAS[6]);

  virtual bool CanApplyNonLinearTransforms()const;
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  virtual std::string GetDefaultStorageNodeClassName(const char* filename /* =NULL */);

  /// Create and observe default display node
  virtual void CreateDefaultDisplayNodes();

  /// Reimplemented to take into account the modified time of the polydata.
  /// Returns true if the node (default behavior) or the polydata are modified
  /// since read/written.
  /// Note: The MTime of the polydata is used to know if it has been modified.
  /// So if you invoke PolyDataModifiedEvent without calling Modified() on the
  /// polydata, GetModifiedSinceRead() won't return true.
  /// \sa vtkMRMLStorableNode::GetModifiedSinceRead()
  virtual bool GetModifiedSinceRead();

protected:
  vtkMRMLModelNode();
  ~vtkMRMLModelNode();
  vtkMRMLModelNode(const vtkMRMLModelNode&);
  void operator=(const vtkMRMLModelNode&);

  /// Called when a display node is added/removed/modified. Propagate the polydata
  /// to the new display node.
  virtual void UpdateDisplayNodePolyData(vtkMRMLDisplayNode *dnode);

  ///
  /// Called when a node reference ID is added (list size increased).
  virtual void OnNodeReferenceAdded(vtkMRMLNodeReference *reference);

  ///
  /// Called when a node reference ID is modified.
  virtual void OnNodeReferenceModified(vtkMRMLNodeReference *reference);


  /// Internal function that sets the polydata to all the display nodes.
  /// Can be called if the polydata is changed.
  void SetPolyDataToDisplayNodes();

  /// Internal function that sets the polydata to a display node.
  /// Can be reimplemented if you want to set a different polydata
  virtual void SetPolyDataToDisplayNode(vtkMRMLModelDisplayNode* modelDisplayNode);

  /// Data
  vtkAlgorithmOutput* PolyDataConnection;
  vtkEventForwarderCommand* DataEventForwarder;
};

#endif
