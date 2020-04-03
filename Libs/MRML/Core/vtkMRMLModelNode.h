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
class vtkPointSet;
class vtkPolyData;
class vtkTransformFilter;
class vtkUnstructuredGrid;
class vtkMRMLDisplayNode;

/// \brief MRML node to represent a 3D surface model.
///
/// Model nodes describe polygonal data.
/// When a model display node (vtkMRMLModelDisplayNode) is observed by the
/// model, the output mesh is automatically set to the input of the model
/// display node: You don't have to manually set the mesh yourself.
/// Models are assumed to have been constructed with the orientation and voxel
/// dimensions of the original segmented volume.
class VTK_MRML_EXPORT vtkMRMLModelNode : public vtkMRMLDisplayableNode
{
public:
  static vtkMRMLModelNode *New();
  vtkTypeMacro(vtkMRMLModelNode,vtkMRMLDisplayableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Model";};

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLModelNode);

  /// alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  /// Get associated model display MRML node
  vtkMRMLModelDisplayNode* GetModelDisplayNode();

  /// Set and observe mesh for this model.
  /// \sa GetMesh()
  virtual void SetAndObserveMesh(vtkPointSet *Mesh);

  /// Set and observe mesh for this model.
  /// \deprecated Use SetAndObserveMesh instead.
  virtual void SetAndObservePolyData(vtkPolyData *polyData);

  /// Return the input mesh.
  /// \sa SetAndObserveMesh(), GetPolyData(), GetUnstructuredGrid(), GetMeshConnection()
  virtual vtkPointSet* GetMesh();

  /// Return the input mesh if it is a polydata.
  /// \sa GetMesh(), SetAndObserveMesh()
  virtual vtkPolyData* GetPolyData();

  /// Return the input mesh if it is an
  /// unstructured grid.
  /// \sa GetMesh(), SetAndObserveMesh()
  virtual vtkUnstructuredGrid* GetUnstructuredGrid();

  /// Set and observe polydata pipeline.
  /// It is propagated to the display nodes.
  /// \sa GetMeshConnection(), SetUnstructuredGridConnection()
  virtual void SetPolyDataConnection(vtkAlgorithmOutput *inputPort);

  /// Set and observe unstructured grid pipeline.
  /// It is propagated to the display nodes.
  /// \sa GetMeshConnection(), SetPolyDataConnection()
  virtual void SetUnstructuredGridConnection(vtkAlgorithmOutput *inputPort);

  /// Return the input mesh pipeline.
  /// \sa GetPolyDataConnection(), GetUnstructuredGridConnection()
  vtkGetObjectMacro(MeshConnection,vtkAlgorithmOutput)

  /// Return the input mesh pipeline if the mesh
  /// is a polydata.
  /// \sa GetMeshConnection(), SetPolyDataConnection()
  virtual vtkAlgorithmOutput* GetPolyDataConnection();

  /// Return the input mesh pipeline if the mesh
  /// is an unstructured grid.
  /// \sa GetMeshConnection(), SetUnstructuredGridConnection()
  virtual vtkAlgorithmOutput* GetUnstructuredGridConnection();

  /// Mesh Type hint
  /// \sa GetMeshType()
  typedef enum {
    PolyDataMeshType = 0,
    UnstructuredGridMeshType
  } MeshTypeHint;

  /// Get the mesh type of that model. The safest way
  /// to know if the mesh is unstructuredGrid is to check
  /// if GetUnstructuredGrid() is not nullptr, but it requires
  /// to update the pipeline.
  /// \sa MeshType, GetUnstructuredGrid()
  vtkGetMacro(MeshType, MeshTypeHint);

  /// MeshModifiedEvent is fired when Mesh is changed.
  /// While it is possible for the subclasses to fire MeshModifiedEvent
  /// without modifying the mesh, it is not recommended to do so as it
  /// doesn't mark the mesh as modified, which my result in an incorrect
  /// return value for GetModifiedSinceRead()
  /// \sa GetModifiedSinceRead()
  enum
    {
    MeshModifiedEvent = 17001,
    PolyDataModifiedEvent = 17001 ///< \deprecated Use MeshModifiedEvent
    };

  /// Utility function that adds an array to the mesh's point data.
  //// \sa AddCellScalars, AddScalars
  void AddPointScalars(vtkDataArray *array);

  /// Add an array to the mesh's cell data.
  /// \sa AddPointScalars, AddScalars
  void AddCellScalars(vtkDataArray *array);

  /// Add an array to the mesh's point or cell data
  /// location is either vtkAssignAttribute::POINT_DATA or
  /// vtkAssignAttribute::CELL_DATA
  void AddScalars(vtkDataArray *array, int location);

  /// Remove an array from the mesh's point/cell data.
  void RemoveScalars(const char *scalarName);

  /// Return true if the mesh point data has an array with a
  /// \a scalarName name.
  /// \sa HasPointScalarName
  bool HasPointScalarName(const char* scalarName);

  /// Return true if the mesh cell data has an array with a
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
  /// Returns an empty string if it can't find one or if no input mesh
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
  /// Returns an empty string if it can't find one or if no input mesh
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

  /// Get bounding box in global RAS form (xmin,xmax, ymin,ymax, zmin,zmax).
  /// This method returns the bounds of the object with any transforms that may
  /// be applied to it.
  /// \sa GetBounds()
  void GetRASBounds(double bounds[6]) override;

  /// Get bounding box in global RAS form (xmin,xmax, ymin,ymax, zmin,zmax).
  /// This method always returns the bounds of the untransformed object.
  /// \sa GetRASBounds()
  void GetBounds(double bounds[6]) override;

  /// Transforms bounds from the local coordinate system to the RAS (world)
  /// coordinate system. Only the corner points are used for determining the
  /// new bounds, therefore in case of non-linear transforms the transformed
  /// bounds may not fully contain the transformed model points.
  virtual void TransformBoundsToRAS(double inputBounds_Local[6], double outputBounds_RAS[6]);

  bool CanApplyNonLinearTransforms()const override;
  void ApplyTransform(vtkAbstractTransform* transform) override;

  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  std::string GetDefaultStorageNodeClassName(const char* filename /* =nullptr */) override;

  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

  /// Reimplemented to take into account the modified time of the mesh.
  /// Returns true if the node (default behavior) or the mesh are modified
  /// since read/written.
  /// Note: The MTime of the mesh is used to know if it has been modified.
  /// So if you invoke MeshModifiedEvent without calling Modified() on the
  /// mesh, GetModifiedSinceRead() won't return true.
  /// \sa vtkMRMLStorableNode::GetModifiedSinceRead()
  bool GetModifiedSinceRead() override;

protected:
  vtkMRMLModelNode();
  ~vtkMRMLModelNode() override;
  vtkMRMLModelNode(const vtkMRMLModelNode&);
  void operator=(const vtkMRMLModelNode&);

  /// Called by SetPolyDataConnection and SetUnstructuredGridConnection
  virtual void SetMeshConnection(vtkAlgorithmOutput *inputPort);

  /// Called when a display node is added/removed/modified. Propagate the mesh
  /// to the new display node.
  virtual void UpdateDisplayNodeMesh(vtkMRMLDisplayNode *dnode);

  ///
  /// Called when a node reference ID is added (list size increased).
  void OnNodeReferenceAdded(vtkMRMLNodeReference *reference) override;

  ///
  /// Called when a node reference ID is modified.
  void OnNodeReferenceModified(vtkMRMLNodeReference *reference) override;


  /// Internal function that sets the mesh to all the display nodes.
  /// Can be called if the mesh is changed.
  void SetMeshToDisplayNodes();

  /// Internal function that sets the mesh to a display node.
  /// Can be reimplemented if you want to set a different mesh
  virtual void SetMeshToDisplayNode(vtkMRMLModelDisplayNode* modelDisplayNode);

  /// Data
  vtkAlgorithmOutput* MeshConnection;
  vtkEventForwarderCommand* DataEventForwarder;
  MeshTypeHint MeshType;
};

#endif
