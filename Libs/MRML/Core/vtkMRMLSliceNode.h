/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLSliceNode_h
#define __vtkMRMLSliceNode_h

// MRML includes
#include "vtkMRMLAbstractViewNode.h"
class vtkMRMLVolumeNode;

// VTK includes
class vtkMatrix3x3;
class vtkMatrix4x4;

/// \brief MRML node for storing a slice through RAS space.
///
/// This node stores the information about how to map from RAS space to
/// the desired slice plane.
/// \li SliceToRAS is the matrix that rotates and translates the slice plane
/// \li FieldOfView tells the size of slice plane
class VTK_MRML_EXPORT vtkMRMLSliceNode : public vtkMRMLAbstractViewNode
{
  public:
  /// \brief Instantiate a new Slice node without any orientation presets.
  ///
  /// \note To instantiate a vtkMRMLSliceNode with preconfigured
  /// orientation preset matrices (the default presets are: Axial,
  /// Sagittal and Coronal in default), it is necessary to use
  /// vtkMRMLScene::CreateNodeByClass(const char*)
  static vtkMRMLSliceNode *New();
  vtkTypeMacro(vtkMRMLSliceNode,vtkMRMLAbstractViewNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLSliceNode);

  /// \brief Reimplemented to preserve orientation and layout color when reset.
  /// \sa GetOrientationString()
  /// \sa GetLayoutColor()
  void Reset(vtkMRMLNode* defaultNode) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Slice";};

  ///
  /// Mapping from RAS space onto the slice plane
  /// This matrix is allowed to be modified from outside, for example
  /// by calling sliceNode->GetSliceToRAS->DeepCopy(...)
  /// followed by sliceNode->UpdateMatrices().
  /// TODO: maybe this should be a quaternion and a translate to avoid shears/scales
  virtual vtkMatrix4x4 *GetSliceToRAS();

  ///
  /// The visibility of the slice in the 3DViewer.
  vtkGetMacro(SliceVisible, int);
  vtkSetMacro(SliceVisible, int);

  ///
  /// The visibility of the slice plane widget in the 3DViewer.
  vtkGetMacro(WidgetVisible, int);
  vtkSetMacro(WidgetVisible, int);

  /// The visibility of the slice plane widget outline in the 3DViewer.
  vtkGetMacro(WidgetOutlineVisible, int);
  vtkSetMacro(WidgetOutlineVisible, int);

  /// Add View Node ID for the view to display this node in.
  /// \sa ViewNodeIDs, RemoveViewNodeID(), RemoveAllViewNodeIDs()
  void AddThreeDViewID(const char* viewNodeID);
  /// Remove View Node ID for the view to display this node in.
  /// \sa ViewNodeIDs, AddViewNodeID(), RemoveAllViewNodeIDs()
  void RemoveThreeDViewID(char* viewNodeID);
  /// Remove All View Node IDs for the views to display this node in.
  /// \sa ViewNodeIDs, AddThreeDViewID(), RemoveThreeDViewID()
  void RemoveAllThreeDViewIDs();
  /// Get number of View Node ID's for the view to display this node in.
  /// If 0, display in all views
  /// \sa ThreeDViewIDs, GetThreeDViewIDs(), AddThreeDViewID()
  int GetNumberOfThreeDViewIDs()const;
  /// Get View Node ID's for the view to display this node in.
  /// If nullptr, display in all views
  /// \sa ThreeDViewIDs, GetThreeDViewIDs(), AddThreeDViewID()
  const char* GetNthThreeDViewID(unsigned int index);
  /// Get all View Node ID's for the view to display this node in.
  /// If empty, display in all views
  /// \sa ThreeDViewIDs, GetNthThreeDViewID(), AddThreeDViewID()
  inline std::vector< std::string > GetThreeDViewIDs()const;
  /// True if the view node id is present in the ThreeDViewid list
  /// false if not found
  /// \sa ThreeDViewIDs, IsDisplayableInView(), AddThreeDViewID()
  bool IsThreeDViewIDPresent(const char* ThreeDViewID)const;
  /// Returns true if the ThreeDViewID is present in the ThreeDViewId list
  /// or there is no ThreeDViewId in the list (meaning all the views display the
  /// node)
  /// \sa ThreeDViewIDs, IsThreeDViewIDPresent(), AddThreeDViewID()
  bool IsDisplayableInThreeDView(const char* viewNodeID)const;

  /// The ImpplicitePlane widget mode
  /// this lock the normal of the plane to the camera's one
  vtkGetMacro(WidgetNormalLockedToCamera, int);
  vtkSetMacro(WidgetNormalLockedToCamera, int);

  ///
  /// Use the label outline filter on this slice?
  vtkGetMacro(UseLabelOutline, int);
  vtkSetMacro(UseLabelOutline, int);
  vtkBooleanMacro(UseLabelOutline, int);

  /// \brief Set 'standard' radiological convention views of patient space.
  ///
  /// If the associated orientation preset has been renamed or removed, calling
  /// these function returns \a False.
  ///
  /// \sa SetOrientation(const char*)
  bool SetOrientationToAxial();
  bool SetOrientationToSagittal();
  bool SetOrientationToCoronal();

  /// Set slice orientation to that is defined in DefaultOrientation.
  /// Returns true on success.
  bool SetOrientationToDefault();

  /// \brief Get orientation.
  ///
  /// It returns a string with a description of the slice orientation
  ///
  /// \sa GetOrientation(vtkMatrix4x4* sliceToRAS)
  /// \sa SetOrientation(const char* orientation)
  std::string GetOrientation();

  /// \brief Return the orientation name associated with \a sliceToRAS.
  std::string GetOrientation(vtkMatrix4x4* sliceToRAS);

  /// \brief Set orientation.
  ///
  /// It adjusts the SliceToRAS matrix to position the slice
  /// cutting plane.
  ///
  /// Valid \a orientations are known as presets and are easily added,
  /// removed or renamed.
  ///
  /// \sa AddSliceOrientationPreset(const std::string& name, vtkMatrix4x4* sliceToRAS)
  /// \sa UpdateMatrices()
  bool SetOrientation(const char* orientation);

  /// \brief Set default orientation preset name.
  ///
  /// If slice view orientation is reset is requested then
  /// orientation is set to this preset.
  vtkSetStringMacro(DefaultOrientation);
  vtkGetStringMacro(DefaultOrientation);

  /// \brief Get orientation.
  ///
  /// \deprecated Prefer GetOrientation()
  virtual const char* GetOrientationString();

  /// Interactive slice intersection parameters
  vtkSetMacro(InteractionVisibility, bool);
  vtkGetMacro(InteractionVisibility, bool);
  vtkBooleanMacro(InteractionVisibility, bool);
  vtkSetMacro(InteractionSize, double);
  vtkGetMacro(InteractionSize, double);
  vtkSetMacro(InteractionScale, double);
  vtkGetMacro(InteractionScale, double);
  vtkSetMacro(InteractionSizeAbsolute, bool);
  vtkGetMacro(InteractionSizeAbsolute, bool);
  vtkBooleanMacro(InteractionSizeAbsolute, bool);
  vtkSetMacro(ActiveInteractionType, int);
  vtkGetMacro(ActiveInteractionType, int);
  vtkSetMacro(ActiveInteractionIndex, int);
  vtkGetMacro(ActiveInteractionIndex, int);

protected:

  /// The OrientationReference is a place to store the last orientation
  /// that was explicitly selected.
  ///
  /// \deprecated
  vtkGetStringMacro(OrientationReference);
  vtkSetStringMacro(OrientationReference);

public:

  /// \brief Return the sliceToRAS matrix associated with \a name.
  vtkMatrix3x3 *GetSliceOrientationPreset(const std::string& name);

  /// \brief Return the preset name corresponding to \a orientationMatrix.
  ///
  /// Returns an empty string if \a orientationMatrix is not an existing
  /// preset.
  std::string GetSliceOrientationPresetName(vtkMatrix3x3* orientationMatrix);

  /// \brief Return all the orientation preset names.
  void GetSliceOrientationPresetNames(vtkStringArray* presetOrientationNames);

  /// \brief Return number of orientation presets.
  /// \sa AddSliceOrientationPreset(const std::string& name, vtkMatrix3x3* orientationMatrix)
  int GetNumberOfSliceOrientationPresets() const;

  /// \brief Add or update an orientation preset.
  ///
  /// \sa RenameSliceOrientationPreset(const std::string& name, const std::string& updatedName)
  /// \sa RemoveSliceOrientationPreset(const std::string& name)
  bool AddSliceOrientationPreset(const std::string& name, vtkMatrix3x3* orientationMatrix);

  /// \brief Remove an orientation preset.
  ///
  /// \sa AddSliceOrientationPreset(const std::string& name, vtkMatrix4x4* sliceToRAS)
  bool RemoveSliceOrientationPreset(const std::string& name);

  /// \brief Rename an orientation preset.
  ///
  /// \sa AddSliceOrientationPreset(const std::string& name, vtkMatrix4x4* sliceToRAS)
  bool RenameSliceOrientationPreset(const std::string& name, const std::string& updatedName);

  /// \brief Return True if an orientation preset is stored.
  ///
  /// \sa AddSliceOrientationPreset(const std::string& name, vtkMatrix4x4* sliceToRAS)
  bool HasSliceOrientationPreset(const std::string& name);

  /// Returns the name of "Reformat" orientation, which means that it is none of the
  /// known orientations.
  static const char* GetReformatOrientationName() { return "Reformat"; }

  /// \brief Initialize \a orientationMatrix as an `Axial` orientation matrix.
  /// \param patientRightIsScreenLeft chooses between radiology (default, patient right is left side on the screen)
  /// and neurology (patient right is right side on the screen) view orientation conventions.
  static void GetAxialSliceToRASMatrix(vtkMatrix3x3* orientationMatrix, bool patientRightIsScreenLeft=true);

  /// \brief Initialize \a orientationMatrix as a `Sagittal` orientation matrix.
  /// \param patientRightIsScreenLeft chooses between radiology (default, patient right is left side on the screen)
  /// and neurology (patient right is right side on the screen) view orientation conventions.
  static void GetSagittalSliceToRASMatrix(vtkMatrix3x3* orientationMatrix, bool patientRightIsScreenLeft=true);

  /// \brief Initialize \a orientationMatrix as a `Coronal` orientation matrix.
  /// \param patientRightIsScreenLeft chooses between radiology (default, patient right is left side on the screen)
  /// and neurology (patient right is right side on the screen) view orientation conventions.
  static void GetCoronalSliceToRASMatrix(vtkMatrix3x3* orientationMatrix, bool patientRightIsScreenLeft=true);

  /// \brief Add default slice orientation presets to \a scene.
  /// \param patientRightIsScreenLeft chooses between radiology (default, patient right is left side on the screen)
  /// and neurology (patient right is right side on the screen) view orientation conventions.
  /// \sa vtkMRMLScene::AddDefaultNode(vtkMRMLNode* node)
  /// \sa GetAxialSliceToRASMatrix, GetSagittalSliceToRASMatrix, GetCoronalSliceToRASMatrix
  static void AddDefaultSliceOrientationPresets(vtkMRMLScene* scene, bool patientRightIsScreenLeft=true);

  ///
  /// Size of the slice plane in millimeters
  vtkGetVector3Macro(FieldOfView, double);
  void SetFieldOfView(double x, double y, double z);

  ///
  /// Origin of XYZ window
  vtkGetVector3Macro(XYZOrigin, double);
  void SetXYZOrigin(double x, double y, double z);

  ///
  /// Number of samples in each direction
  /// -- note that the spacing is implicitly FieldOfView / Dimensions
  vtkGetVectorMacro(Dimensions,int,3)
  void SetDimensions(int x, int y, int z);

  ///
  /// Number of samples in each direction for the reslice operation
  /// -- this is the resolution that each slice layer is resliced into
  /// -- the outputs of the slice layers are then composited and upsampled
  ///    to the full Dimensions
  /// -- note that z, the number of slices, should be the same for both
  ///    Dimensions and UVWDimensions
  vtkGetVectorMacro(UVWDimensions,int,3)
  void SetUVWDimensions(int x, int y, int z);
  void SetUVWDimensions(int xyz[3]);

  ///
  ///    maximum limit for  UVWDimensions
  vtkGetVectorMacro(UVWMaximumDimensions,int,3)
  void SetUVWMaximumDimensions(int x, int y, int z);
  void SetUVWMaximumDimensions(int xyz[3]);

  ///
  /// Get/Set maximum extent in any direction occupied by slice
  vtkGetVector3Macro(UVWExtents, double);
  void SetUVWExtents(double x, double y, double z);
  void SetUVWExtents(double xyz[3]);

  ///
  /// Origin of UVW window
  vtkGetVector3Macro(UVWOrigin, double);
  void SetUVWOrigin(double x, double y, double z);
  void SetUVWOrigin(double xyz[3]);

  ///
  /// Origin of slice in XYZ or UVW space depending on SliceResolutionMode
  void SetSliceOrigin(double xyz[3]);
  void SetSliceOrigin(double x, double y, double z);

  ///
  /// Set UVW extents and dimensions,
  /// produces less updates then calling both separately
  void SetUVWExtentsAndDimensions (double extents[3], int dimensions[3]);


  ///
  /// Matrix mapping from XY pixel coordinates on an image window
  /// into slice coordinates in mm
  vtkMatrix4x4 *GetXYToSlice();

  ///
  /// Matrix mapping from XY pixel coordinates on an image window
  /// into RAS world coordinates
  vtkMatrix4x4 *GetXYToRAS();

  ///
  /// Matrix mapping from UVW texture coordinates
  /// into slice coordinates in mm
  vtkMatrix4x4 *GetUVWToSlice();

  ///
  /// Matrix mapping from UVW texture coordinates
  /// into RAS world coordinates
  vtkMatrix4x4 *GetUVWToRAS();

  ///
  /// helper for comparing to matrices
  bool MatrixAreEqual(const vtkMatrix4x4 *m1, const vtkMatrix4x4 *m2);

  bool MatrixAreEqual(const vtkMatrix4x4 *matrix,
                      const vtkMatrix3x3 *orientationMatrix);
  ///
  /// Recalculate XYToSlice and XYToRAS in terms or fov, dim, SliceToRAS
  /// - called when any of the inputs change
  void UpdateMatrices();

  ///
  /// Set the number of rows and columns to use in a LightBox display
  /// of the node
  void SetLayoutGrid( int rows, int columns );

  ///
  /// Set/Get the number of rows to use ina LightBox display
  vtkGetMacro(LayoutGridRows, int);
  virtual void SetLayoutGridRows(int rows);

  ///
  /// Set/Get the number of columns to use ina LightBox display
  vtkGetMacro(LayoutGridColumns, int);
  virtual void SetLayoutGridColumns(int cols);

  ///
  /// Set the SliceToRAS matrix according to the position and orientation of the locator:
  /// N(x, y, z) - the direction vector of the locator
  /// T(x, y, z) - the transverse direction vector of the locator
  /// P(x, y, z) - the tip location of the locator
  /// All the above values are in RAS space.
  void SetSliceToRASByNTP(double Nx, double Ny, double Nz,
                          double Tx, double Ty, double Tz,
                          double Px, double Py, double Pz,
                          int Orientation);

  ///
  /// Set the RAS offset of the Slice to the passed values. JumpSlice
  /// and JumpAllSlices use the JumpMode to determine how to jump.
  void JumpSlice(double r, double a, double s);
  void JumpAllSlices(double r, double a, double s);
  /// Jump all slices in the scene.
  /// viewGroup can be used to jump only slice views that are in a specific group.
  /// By default viewGroup is set to -1, which means that all slice views are jumped.
  /// If a non-nullptr exclude pointer is specified then position of that slice node will not be changed.
  /// If jumpMode is set to vtkMRMLSliceNode::DefaultJumpSlice then jump mode set in the slice node will be used.
  /// specified in the slice node will be used.
  static void JumpAllSlices(vtkMRMLScene* scene, double r, double a, double s,
    int jumpMode = vtkMRMLSliceNode::DefaultJumpSlice, int viewGroup = -1, vtkMRMLSliceNode* exclude = nullptr);
  void JumpSliceByOffsetting(double r, double a, double s);
  void JumpSliceByOffsetting(int k, double r, double a, double s);
  void JumpSliceByCentering(double r, double a, double s);

  /// Enum to specify the method of jumping slices
  enum {DefaultJumpSlice=-1, CenteredJumpSlice=0, OffsetJumpSlice};

  ///
  /// Control how JumpSlice operates. CenteredJumpMode puts the
  /// specified RAS position in the center of the slice. OffsetJumpMode
  /// does not change the slice position, merely adjusts the slice
  /// offset to get the RAS position on the slice.
  vtkSetMacro(JumpMode, int);
  vtkGetMacro(JumpMode, int);
  void SetJumpModeToCentered();
  void SetJumpModeToOffset();

  /// Enum to specify whether the slice spacing is automatically
  /// determined or prescribed
  enum {AutomaticSliceSpacingMode=0, PrescribedSliceSpacingMode};

  ///
  /// Get/Set the slice spacing mode. Slice spacing can be
  /// automatically calculated using GetLowestVolumeSliceSpacing() or prescribed
  vtkGetMacro(SliceSpacingMode, int);
  vtkSetMacro(SliceSpacingMode, int);
  void SetSliceSpacingModeToAutomatic();
  void SetSliceSpacingModeToPrescribed();

  ///
  /// Set/get the slice spacing to use when the SliceSpacingMode is
  /// "Prescribed"
  vtkSetVector3Macro(PrescribedSliceSpacing, double);
  vtkGetVector3Macro(PrescribedSliceSpacing, double);

  ///
  /// Get/Set the current distance from the origin to the slice plane
  double GetSliceOffset();
  void SetSliceOffset(double offset);

  /// Set/get the active slice in the lightbox. The active slice is
  /// shown in the 3D scene
  vtkSetMacro(ActiveSlice, int);
  vtkGetMacro(ActiveSlice, int);

  /// Adjusts the slice node to align with the
  /// native space of the image data so that no oblique resampling
  /// occurs when rendering (helps to see original acquisition data
  /// and for oblique volumes with few slices).
  /// \param forceSlicePlaneToSingleSlice If the volume is single-slice and forceSlicePlaneToSingleSlice
  /// is enabled then slice view will be aligned with the volume's slice plane. If the flag is disabled
  /// or the volume has more than one slice then the slice view will be rotated to the closest orthogonal axis.
  void RotateToVolumePlane(vtkMRMLVolumeNode *volumeNode, bool forceSlicePlaneToSingleSlice=true);

  /// Adjusts the slice node to align with the
  /// axes of the provided reference coordinate system
  /// so that no oblique resampling
  /// occurs when rendering (helps to see original acquisition data
  /// and for obluique volumes with few slices).
  /// If sliceNormalAxisIndex is >=0 then slice plane normal will
  /// be aligned with that axis.
  void RotateToAxes(vtkMatrix4x4 *referenceToRAS, int sliceNormalAxisIndex=-1);

  /// Get/Set a flag indicating whether this node is actively being
  /// manipulated (usually) by a user interface. This flag is used by
  /// logic classes to determine whether state changes should be
  /// propagated to other nodes to implement linked controls. Does not
  /// cause a Modified().
  void SetInteracting(int);
  vtkGetMacro(Interacting, int);
  vtkBooleanMacro(Interacting, int);

  /// Enum identifying the parameters being manipulated with calls to
  /// InteractionOn() and InteractionOff(). Identifiers are powers of
  /// two so they can be combined into a bitmask to manipulate
  /// multiple parameters.
  /// The meanings for the flags are:
  ///    SliceToRASFlag - broadcast the SliceToRAS matrix to all linked viewers
  ///    FieldOfViewFlag - broadcast a specific field of view setting
  ///    OrientationFlag - broadcast the orientation
  ///    ResetFieldOfViewFlag - broadcast a message to reset the field of view
  ///    MultiplanarReformatFlag - broadcast reformat widget transformation
  ///    XYZOriginFlag - broadcast the XYZOrigin to all linked viewers
  ///    LabelOutlineFlag - broadcast outlining the labelmaps
  ///    SliceVisibleFlag = broadcast display of slice in 3D
  ///    ResetOrientationFlag = broadcast a reset to default orientation to all linked viewers
  enum InteractionFlagType
  {
    None = 0,
    SliceToRASFlag = 1,
    FieldOfViewFlag = 2, // broadcast a specific field of view setting
    OrientationFlag = 4,
    ResetFieldOfViewFlag = 8, // broadcast a reset to all viewers
    MultiplanarReformatFlag = 16, // broadcast reformat widget transformation
    XYZOriginFlag = 32,
    LabelOutlineFlag = 64,
    SliceVisibleFlag = 128,
    SliceSpacingFlag = 256,
    ResetOrientationFlag = 512,
    RotateToBackgroundVolumePlaneFlag = 1024
  };

  /// Get/Set a flag indicating what parameters are being manipulated
  /// within calls to InteractingOn() and InteractingOff(). These
  /// fields are used to propagate linked behaviors. This flag is a
  /// bitfield, with multiple parameters OR'd to compose the
  /// flag. Does not cause a Modified().
  void SetInteractionFlags(unsigned int);
  vtkGetMacro(InteractionFlags, unsigned int);

  /// Get/Set a flag indicating how the linking behavior should be modified.
  /// InteractionFlags modifier uses bits defined by InteractionFlagType enum
  /// that by default are all set and result in the default behavior. If a
  /// bit is not set, this will result in slice node interactions
  /// not broadcast.
  void SetInteractionFlagsModifier(unsigned int);
  vtkGetMacro(InteractionFlagsModifier, unsigned int);

  /// Set all flags of the modifier to 1, resulting in the default linking
  /// behavior (broadcast of the updates to the parameters defined by the
  /// InteractionFlagType enum).
  void ResetInteractionFlagsModifier();



  /// Enum to specify the method for setting UVW extents

  enum SliceResolutionModeType
  {
    SliceResolutionMatchVolumes=0,
    SliceResolutionMatch2DView,
    SliceFOVMatch2DViewSpacingMatchVolumes,
    SliceFOVMatchVolumesSpacingMatch2DView,
    SliceResolutionCustom
  };

  ///
  /// method for setting UVW space (extents, dimensions and spacing)
  virtual void SetSliceResolutionMode(int mode);
  vtkGetMacro(SliceResolutionMode, int);

  /// Interactive slice intersection parameters
  bool InteractionVisibility{ false };
  int ActiveInteractionType{ -1 };
  int ActiveInteractionIndex{ -1 };
  bool InteractionSizeAbsolute{ false };
  double InteractionSize{ 5.0 };
  double InteractionScale{ 15.0 };

protected:
  vtkMRMLSliceNode();
  ~vtkMRMLSliceNode() override;
  vtkMRMLSliceNode(const vtkMRMLSliceNode&);
  void operator=(const vtkMRMLSliceNode&);

  vtkSmartPointer<vtkMatrix4x4> SliceToRAS;

  vtkSmartPointer<vtkMatrix4x4> XYToSlice;
  vtkSmartPointer<vtkMatrix4x4> XYToRAS;

  vtkSmartPointer<vtkMatrix4x4> UVWToSlice;
  vtkSmartPointer<vtkMatrix4x4> UVWToRAS;

  typedef std::pair <std::string, vtkSmartPointer<vtkMatrix3x3> > OrientationPresetType;
  std::vector< OrientationPresetType > OrientationMatrices;

  int JumpMode;

  int SliceVisible;
  int WidgetVisible;
  int WidgetOutlineVisible;
  int WidgetNormalLockedToCamera;
  int UseLabelOutline;

  double FieldOfView[3];
  double XYZOrigin[3];
  double UVWOrigin[3];
  int Dimensions[3];

  int SliceResolutionMode;
  double UVWExtents[3];
  int UVWDimensions[3];
  int UVWMaximumDimensions[3];

  // Hold the string returned by GetOrientationString
  std::string OrientationString;

  char* DefaultOrientation;

  char* OrientationReference;

  int LayoutGridRows;
  int LayoutGridColumns;

  int SliceSpacingMode;
  double PrescribedSliceSpacing[3];

  int ActiveSlice;

  int Interacting;
  unsigned int InteractionFlags;
  unsigned int InteractionFlagsModifier;

  int IsUpdatingMatrices;

  std::vector< std::string > ThreeDViewIDs;
};

//----------------------------------------------------------------------------
std::vector< std::string > vtkMRMLSliceNode::GetThreeDViewIDs()const
{
  return this->ThreeDViewIDs;
}

#endif
