/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLModelDisplayNode_h
#define __vtkMRMLModelDisplayNode_h

// MRML includes
#include "vtkMRMLDisplayNode.h"

// VTK includes
class vtkAlgorithm;
class vtkAlgorithmOutput;
class vtkAssignAttribute;
class vtkGeometryFilter;
class vtkThreshold;
class vtkPassThrough;
class vtkPointSet;
class vtkPolyData;
class vtkUnstructuredGrid;

/// \brief MRML node to represent a display property of 3D surface model.
///
/// vtkMRMLModelDisplayNode nodes stores display property of a 3D surface model
/// including reference to ColorNode, texture, opacity, etc.
class VTK_MRML_EXPORT vtkMRMLModelDisplayNode : public vtkMRMLDisplayNode
{
public:
  static vtkMRMLModelDisplayNode *New();
  vtkTypeMacro(vtkMRMLModelDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  enum SliceDisplayModeType
  {
    SliceDisplayIntersection, ///< Show model in slice view as intersection with slice
    SliceDisplayProjection, ///< Show full model projected on the slice (similar to MIP view of images)
    SliceDisplayDistanceEncodedProjection, ///< Show full model projected on the slice, colored by distance from slice plane
    SliceDisplayMode_Last // placeholder after the last valid value, this must be the last in the list of modes
  };

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLModelDisplayNode);

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "ModelDisplay";}

  /// Set and observe mesh for this model. It should be the output
  /// mesh connection of the model node.
  /// \sa GetInputMeshConnection(), GetInputMesh()
  virtual void SetInputMeshConnection(vtkAlgorithmOutput* meshConnection);
  virtual void SetInputPolyDataConnection(vtkAlgorithmOutput* polyDataConnection);

  /// Get the input mesh connection. It should be the output
  /// mesh connection of the model node
  /// \sa SetInputMeshConnection(), GetInputMesh()
  virtual vtkAlgorithmOutput* GetInputMeshConnection();
  virtual vtkAlgorithmOutput* GetInputPolyDataConnection();

  /// Return the mesh that was set by SetInputMeshConnection()
  /// \sa SetInputMeshConnection(), GetInputMeshConnection()
  virtual vtkPointSet* GetInputMesh();
  virtual vtkPolyData* GetInputPolyData();
  virtual vtkUnstructuredGrid* GetInputUnstructuredGrid();

  /// Return the mesh that is processed by the display node.
  /// This is the mesh that needs to be connected with the mappers.
  /// Return 0 if there is no input mesh but it is required.
  /// GetOutputMesh() should be reimplemented only if the model display
  /// node doesn't take a mesh as input but produce an output mesh.
  /// In all other cases, GetOutputMeshConnection() should be reimplemented.
  /// \sa GetOutputMeshConnection()
  virtual vtkPointSet* GetOutputMesh();
  virtual vtkPolyData* GetOutputPolyData();
  virtual vtkUnstructuredGrid* GetOutputUnstructuredGrid();

  /// Return the mesh that is processed by the display node.
  /// This is the mesh that needs to be connected with the mappers.
  /// \sa GetOutputMesh()
  virtual vtkAlgorithmOutput* GetOutputMeshConnection();
  virtual vtkAlgorithmOutput* GetOutputPolyDataConnection(); ///\deprecated

  /// Reimplemented to update pipeline with new value
  /// Note: If model is generated by a filter pipeline then any modification in the pipeline
  /// resets the output mesh and then recomputes the mesh. During reset the scalars in the mesh
  /// are removed, therefore if a GUI or other component observes the mesh, then it will detect that
  /// the scalar is deleted and so it may deactivate the selected scalar.
  /// \sa SetActiveAttributeLocation()
  void SetActiveScalarName(const char *scalarName) override;

  /// Reimplemented to update pipeline with new value
  /// \sa SetActiveScalarName()
  void SetActiveAttributeLocation(int location) override;

  /// Set whether to threshold the model display node.
  /// \sa ThresholdEnabled, GetThresholdEnabled()
  void SetThresholdEnabled(bool enabled);
  vtkBooleanMacro(ThresholdEnabled,bool);

  /// Get whether to threshold the model display node.
  /// \sa ThresholdEnabled, SetThresholdEnabled(), ThresholdEnabledOn(),
  /// ThresholdEnabledOff()
  vtkGetMacro(ThresholdEnabled,bool);

  /// Set the threshold range of the model display node.
  /// \sa GetThresholdMin(), GetThresholdMax()
  void SetThresholdRange(double min, double max);
  void SetThresholdRange(double range[2]);

  /// Get the threshold range of the model display node.
  /// \sa SetThresholdRange()
  void GetThresholdRange(double range[2]);
  double* GetThresholdRange() VTK_SIZEHINT(2);
  double GetThresholdMin();
  double GetThresholdMax();

  /// Specifies how to represent the 3D model in a 2D slice.
  /// By default intersection is showed.
  /// \sa SetAndObserveDistanceEncodedProjectionColorNodeID
  vtkGetMacro(SliceDisplayMode, int);
  vtkSetMacro(SliceDisplayMode, int);
  void SetSliceDisplayModeToIntersection();
  void SetSliceDisplayModeToProjection();
  void SetSliceDisplayModeToDistanceEncodedProjection();

  /// Convert between slice display mode ID and name
  static const char* GetSliceDisplayModeAsString(int id);
  static int GetSliceDisplayModeFromString(const char* name);

  /// Set and observe color node for distance encoded slice projection.
  /// Model that is projected to the slice will be colored based on the
  /// projected point distance from the slice.
  /// \sa GetDistanceEncodedProjectionColorNodeID(), GetDistanceEncodedProjectionColorNode()
  virtual void SetAndObserveDistanceEncodedProjectionColorNodeID(const char *colorNodeID);
  /// Get color node for distance encoded slice projection.
  /// \sa SetDistanceEncodedProjectionColorNodeID(), SetDistanceEncodedProjectionColorNode()
  virtual const char* GetDistanceEncodedProjectionColorNodeID();
  virtual vtkMRMLColorNode* GetDistanceEncodedProjectionColorNode();

  /// Get data set containing the scalar arrays for this node type.
  /// For models it is the input mesh
  virtual vtkDataSet* GetScalarDataSet() override;
  /// Return the current active scalar array (based on active scalar name and location)
  virtual vtkDataArray* GetActiveScalarArray() override;

  /// Set color of backface surface as HSV (hue, saturation, brightness) offset compared to node color.
  /// Values are in [-1, 1] range, 0 value means same as node color.
  vtkSetVector3Macro(BackfaceColorHSVOffset, double);
  vtkGetVector3Macro(BackfaceColorHSVOffset, double);

protected:
  vtkMRMLModelDisplayNode();
  ~vtkMRMLModelDisplayNode() override;
  vtkMRMLModelDisplayNode(const vtkMRMLModelDisplayNode&);
  void operator=(const vtkMRMLModelDisplayNode&);

  void ProcessMRMLEvents(vtkObject *caller,
                                 unsigned long event,
                                 void *callData) override;

  /// To be reimplemented in subclasses if the input of the pipeline changes
  virtual void SetInputToMeshPipeline(vtkAlgorithmOutput* meshConnection);

  /// Update the AssignAttribute filter based on
  /// its ActiveScalarName and its ActiveAttributeLocation
  virtual void UpdateAssignedAttribute() override;

  /// Filter that changes the active scalar of the input mesh
  /// using the ActiveScalarName and ActiveAttributeLocation properties.
  /// This can be useful to specify what field array is the color array that
  /// needs to be used by the VTK mappers.
  vtkAssignAttribute* AssignAttribute;

  /// Default filter when assign attribute is not used, e.g ActiveScalarName is
  /// null.
  /// \sa AssignAttribute
  vtkPassThrough* PassThrough;

  /// Filters that thresholds the output mesh from
  /// the AssignAttribute filter when the Threshold
  /// option is set to true based on its scalar values.
  /// \sa AssignAttribute, ThresholdEnabled
  vtkThreshold* ThresholdFilter;

  /// When ThresholdFilter is applied to a surface mesh, the output is
  /// an unstructured grid. This filter can convert it back to a surface mesh
  /// (vtkPolyData).
  vtkGeometryFilter* ConvertToPolyDataFilter;

  /// Indicated whether to threshold the output mesh
  /// of the display model node based on its active
  /// scalar values.
  /// \sa ThresholdFilter
  bool ThresholdEnabled;

  int SliceDisplayMode;

  /// Temporary variable to allow GetThresholdRange() method to return threshold range as a pointer
  double ThresholdRangeTemp[2];

  double BackfaceColorHSVOffset[3];
};

#endif
