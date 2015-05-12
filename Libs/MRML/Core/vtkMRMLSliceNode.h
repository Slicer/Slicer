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
class vtkMatrix4x4;

/// \brief MRML node for storing a slice through RAS space.
///
/// This node stores the information about how to map from RAS space to
/// the desired slice plane.
/// \li SliceToRAS is the matrix that rotates and translates the slice plane
/// \li FieldOfView tells the size of  slice plane
class VTK_MRML_EXPORT vtkMRMLSliceNode : public vtkMRMLAbstractViewNode
{
  public:
  static vtkMRMLSliceNode *New();
  vtkTypeMacro(vtkMRMLSliceNode,vtkMRMLAbstractViewNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// \brief Reimplemented to preserve orientation and layout color when reset.
  /// \sa GetOrientationString()
  /// \sa GetLayoutColor()
  virtual void Reset();

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Slice";};

  ///
  /// Mapping from RAS space onto the slice plane
  /// TODO: maybe this should be a quaternion and a translate to avoid shears/scales
  vtkGetObjectMacro (SliceToRAS, vtkMatrix4x4);
  virtual void SetSliceToRAS(vtkMatrix4x4* sliceToRAS);

  ///
  /// The visibility of the slice in the 3DViewer.
  vtkGetMacro ( SliceVisible, int );
  vtkSetMacro ( SliceVisible, int );

  ///
  /// The visibility of the slice plane widget in the 3DViewer.
  vtkGetMacro ( WidgetVisible, int );
  vtkSetMacro ( WidgetVisible, int );

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
  /// If NULL, display in all views
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
  vtkGetMacro ( WidgetNormalLockedToCamera, int );
  vtkSetMacro ( WidgetNormalLockedToCamera, int );

  ///
  /// Use the label outline filter on this slice?
  vtkGetMacro ( UseLabelOutline, int );
  vtkSetMacro ( UseLabelOutline, int );
  vtkBooleanMacro ( UseLabelOutline, int );

  ///
  /// 'standard' radiological convention views of patient space
  /// these calls adjust the SliceToRAS matrix to position the slice
  /// cutting plane
  void SetOrientationToAxial();
  void SetOrientationToSagittal();
  void SetOrientationToCoronal();

  ///
  /// General 'reformat' view that allows for multiplanar reformat
  void SetOrientationToReformat();

  /// Convenient function that calls SetOrientationToAxial(),
  /// SetOrientationToSagittal(), SetOrientationToCoronal() or
  /// SetOrientationToReformat() depending on the value of the string
  void SetOrientation(const char* orientation);

  /// Description
  /// A description of the current orientation
  /// Warning, OrientationString doesn't change the matrices, use
  /// SetOrientation() instead.
  vtkGetStringMacro (OrientationString);
  vtkSetStringMacro (OrientationString);

  /// Description
  /// The OrientationReference is a place to store the last orientation
  /// that was explicitly selected.  This way if they RotateToVolumePlane
  /// is called repeatedly it will always return the same plane
  /// (without the hint, it would first try to match, say, Coronal, and then
  /// try to match 'Reformat' but would not know what overall orientation to pick).
  vtkGetStringMacro (OrientationReference);
  vtkSetStringMacro (OrientationReference);

  ///
  /// Size of the slice plane in millimeters
  vtkGetVector3Macro (FieldOfView, double);
  void SetFieldOfView (double x, double y, double z);

  ///
  /// Origin of XYZ window
  vtkGetVector3Macro (XYZOrigin, double);
  void SetXYZOrigin (double x, double y, double z);

  ///
  /// Number of samples in each direction
  /// -- note that the spacing is implicitly FieldOfView / Dimensions
  vtkGetVectorMacro(Dimensions,int,3)
  void SetDimensions (int x, int y, int z);

  ///
  /// Number of samples in each direction for the reslice operation
  /// -- this is the resolution that each slice layer is resliced into
  /// -- the outputs of the slice layers are then composited and upsampled
  ///    to the full Dimensions
  /// -- note that z, the number of slices, should be the same for both
  ///    Dimensions and UVWDimensions
  vtkGetVectorMacro(UVWDimensions,int,3)
  void SetUVWDimensions (int x, int y, int z);
  void SetUVWDimensions (int xyz[3]);

  ///
  ///    maximum limit for  UVWDimensions
  vtkGetVectorMacro(UVWMaximumDimensions,int,3)
  void SetUVWMaximumDimensions (int x, int y, int z);
  void SetUVWMaximumDimensions (int xyz[3]);

  ///
  /// Get/Set maximum extent in any direction occupied by slice
  vtkGetVector3Macro (UVWExtents, double);
  void SetUVWExtents (double x, double y, double z);
  void SetUVWExtents (double xyz[3]);

  ///
  /// Origin of UVW window
  vtkGetVector3Macro (UVWOrigin, double);
  void SetUVWOrigin (double x, double y, double z);
  void SetUVWOrigin (double xyz[3]);

  ///
  /// Origin of slice in XYZ or UVW space depending on SliceResolutionMode
  void SetSliceOrigin (double xyz[3]);
  void SetSliceOrigin (double x, double y, double z);

  ///
  /// Set UVW extents and dimensions,
  /// produces less upadtes then calling both separately
  void SetUVWExtentsAndDimensions (double extents[3], int dimensions[3]);


  ///
  /// Matrix mapping from XY pixel coordinates on an image window
  /// into slice coordinates in mm
  vtkGetObjectMacro (XYToSlice, vtkMatrix4x4);

  ///
  /// Matrix mapping from XY pixel coordinates on an image window
  /// into RAS world coordinates
  vtkGetObjectMacro (XYToRAS, vtkMatrix4x4);

  ///
  /// Matrix mapping from UVW texture coordinates
  /// into slice coordinates in mm
  vtkGetObjectMacro (UVWToSlice, vtkMatrix4x4);

  ///
  /// Matrix mapping from UVW texture coordinates
  /// into RAS world coordinates
  vtkGetObjectMacro (UVWToRAS, vtkMatrix4x4);

  ///
  /// helper for comparing to matrices
  /// TODO: is there a standard VTK method?
  int Matrix4x4AreEqual(vtkMatrix4x4 *m1, vtkMatrix4x4 *m2);

  ///
  /// Recalculate XYToSlice and XYToRAS in terms or fov, dim, SliceToRAS
  /// - called when any of the inputs change
  void UpdateMatrices();

  /// Color for layout as rgb
  vtkSetVector3Macro(LayoutColor, double);
  vtkGetVector3Macro(LayoutColor, double);

  static double* redColor();
  static double* yellowColor();
  static double* greenColor();
  static double* compareColor();
  static double* grayColor();

  ///
  /// Set the number of rows and columns to use in a LightBox display
  /// of the node
  void SetLayoutGrid( int rows, int columns );

  ///
  /// Set/Get the number of rows to use ina LightBox display
  vtkGetMacro (LayoutGridRows, int);
  virtual void SetLayoutGridRows(int rows);

  ///
  /// Set/Get the number of columns to use ina LightBox display
  vtkGetMacro (LayoutGridColumns, int);
  virtual void SetLayoutGridColumns(int cols);

  ///
  /// Set the SliceToRAS matrix according to the position and orientation of the locator:
  /// N(x, y, z) - the direction vector of the locator
  /// T(x, y, z) - the transverse direction vector of the locator
  /// P(x, y, z) - the tip location of the locator
  /// All the above values are in RAS space.
  void SetSliceToRASByNTP (double Nx, double Ny, double Nz,
                           double Tx, double Ty, double Tz,
                           double Px, double Py, double Pz,
                           int Orientation);

  ///
  /// Set the RAS offset of the Slice to the passed values. JumpSlice
  /// and JumpAllSlices use the JumpMode to determine how to jump.
  void JumpSlice(double r, double a, double s);
  void JumpAllSlices(double r, double a, double s);
  void JumpSliceByOffsetting(double r, double a, double s);
  void JumpSliceByOffsetting(int k, double r, double a, double s);
  void JumpSliceByCentering(double r, double a, double s);

  /// Enum to specify the method of jumping slices
  enum {CenteredJumpSlice=0, OffsetJumpSlice};

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

  /// adjusts the slice node to align with the
  /// native space of the image data so that no oblique resampling
  /// occurs when rendering (helps to see original acquisition data
  /// and for obluique volumes with few slices).
  void RotateToVolumePlane(vtkMRMLVolumeNode *volumeNode);

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
    SliceSpacingFlag = 256
    // Next one needs to be 512
  };

  /// Get/Set a flag indicating what parameters are being manipulated
  /// within calls to InteractingOn() and InteractingOff(). These
  /// fields are used to propagate linked behaviors. This flag is a
  /// bitfield, with multiple parameters OR'd to composte the
  /// flag. Does not cause a Modifed().
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

protected:


  vtkMRMLSliceNode();
  ~vtkMRMLSliceNode();
  vtkMRMLSliceNode(const vtkMRMLSliceNode&);
  void operator=(const vtkMRMLSliceNode&);

  vtkMatrix4x4 *SliceToRAS;

  vtkMatrix4x4 *XYToSlice;
  vtkMatrix4x4 *XYToRAS;

  vtkMatrix4x4 *UVWToSlice;
  vtkMatrix4x4 *UVWToRAS;


  int JumpMode;

  int SliceVisible;
  int WidgetVisible;
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

  char *OrientationString;
  char *OrientationReference;

  double LayoutColor[3];

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
