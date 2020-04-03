/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLViewNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLViewNode_h
#define __vtkMRMLViewNode_h

// VTK includes
#include "vtkMRMLAbstractViewNode.h"

/// \brief MRML node to represent a 3D view.
///
/// View node contains view parameters.
class VTK_MRML_EXPORT vtkMRMLViewNode
  : public vtkMRMLAbstractViewNode
{
public:
  static vtkMRMLViewNode *New();
  vtkTypeMacro(vtkMRMLViewNode,vtkMRMLAbstractViewNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLViewNode);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override;

  /// Return the color the view nodes have for the background by default.
  static double* defaultBackgroundColor();
  static double* defaultBackgroundColor2();

  /// Indicates if the box is visible
  vtkGetMacro(BoxVisible, int);
  vtkSetMacro(BoxVisible, int);

  /// Indicates if the axis labels are visible
  vtkGetMacro(AxisLabelsVisible, int);
  vtkSetMacro(AxisLabelsVisible, int);

  /// Indicates if the axis labels visibility controlled by camera orientation
  vtkGetMacro(AxisLabelsCameraDependent, int);
  vtkSetMacro(AxisLabelsCameraDependent, int);

  /// Toggles visibility of fiducial points in 3D viewer
  vtkGetMacro(FiducialsVisible, int);
  vtkSetMacro(FiducialsVisible, int);

  vtkGetMacro(FiducialLabelsVisible, int);
  vtkSetMacro(FiducialLabelsVisible, int);

  /// Field of view size
  vtkGetMacro(FieldOfView, double);
  vtkSetMacro(FieldOfView, double);

  /// Axis label size
  vtkGetMacro(LetterSize, double);
  vtkSetMacro(LetterSize, double);

  /// Turn on and off animated spinning or rocking.
  vtkGetMacro(AnimationMode, int);
  vtkSetMacro(AnimationMode, int);
  static const char* GetAnimationModeAsString(int id);
  static int GetAnimationModeFromString(const char* name);

  vtkGetMacro(ViewAxisMode, int);
  vtkSetMacro(ViewAxisMode, int);
  static const char* GetViewAxisModeAsString(int id);
  static int GetViewAxisModeFromString(const char* name);

  /// Direction of animated spinning
  vtkGetMacro(SpinDirection, int);
  vtkSetMacro(SpinDirection, int);
  static const char* GetSpinDirectionAsString(int id);
  static int GetSpinDirectionFromString(const char* name);

  /// Number of degrees in spin increment.
  vtkGetMacro(SpinDegrees, double);
  vtkSetMacro(SpinDegrees, double);

  vtkGetMacro(RotateDegrees, double);
  vtkSetMacro(RotateDegrees, double);

  /// Amount of wait time between spin increments
  vtkGetMacro(AnimationMs, int);
  vtkSetMacro(AnimationMs, int);

  /// Length of animated rocking
  vtkGetMacro(RockLength, int);
  vtkSetMacro(RockLength, int);

  /// Increment of animated rock
  vtkGetMacro(RockCount, int);
  vtkSetMacro(RockCount, int);

  /// Stereo mode (including NoStereo)
  vtkGetMacro(StereoType, int);
  vtkSetMacro(StereoType, int);
  static const char* GetStereoTypeAsString(int id);
  static int GetStereoTypeFromString(const char* name);

  /// Specifies orthographic or perspective rendering
  vtkGetMacro(RenderMode, int);
  vtkSetMacro(RenderMode, int);
  static const char* GetRenderModeAsString(int id);
  static int GetRenderModeFromString(const char* name);

  /// Use depth peeling or not.
  /// 0 by default.
  vtkGetMacro(UseDepthPeeling, int);
  vtkSetMacro(UseDepthPeeling, int);

  /// Show FPS in the lower right side of the screen.
  /// 0 by default.
  vtkGetMacro(FPSVisible, int);
  vtkSetMacro(FPSVisible, int);

  /// GPU memory size in MB
  /// 0 by default (application default)
  vtkGetMacro(GPUMemorySize, int);
  vtkSetMacro(GPUMemorySize, int);

  /// Expected FPS
  vtkSetMacro(ExpectedFPS, double);
  vtkGetMacro(ExpectedFPS, double);

  vtkSetMacro(VolumeRenderingQuality, int);
  vtkGetMacro(VolumeRenderingQuality, int);
  static const char* GetVolumeRenderingQualityAsString(int id);
  static int GetVolumeRenderingQualityFromString(const char* name);

  /// Rycasting technique for volume rendering
  vtkGetMacro(RaycastTechnique, int);
  vtkSetMacro(RaycastTechnique, int);
  static const char* GetRaycastTechniqueAsString(int id);
  static int GetRaycastTechniqueFromString(const char* name);

  /// Reduce wood grain artifact to make surfaces appear smoother.
  /// For example, by applying jittering on casted rays.
  /// Note: Only applies to GPU-based techniques
  vtkGetMacro(VolumeRenderingSurfaceSmoothing, bool);
  vtkSetMacro(VolumeRenderingSurfaceSmoothing, bool);

  /// Oversampling factor for sample distance. The sample distance is calculated by \sa
  /// GetSampleDistance to be the volume's minimum spacing divided by the oversampling
  /// factor.
  /// If \sa VolumeRenderingQuality is set to maximum quality, then a fix oversampling factor of 10 is used.
  vtkSetMacro(VolumeRenderingOversamplingFactor, double);
  vtkGetMacro(VolumeRenderingOversamplingFactor, double);

  /// Modes for automatically controlling camera
  enum
    {
    RotateAround = 0,
    LookFrom,
    ViewAxisMode_Last
    };

  /// Rotate camera directions
  enum
    {
    PitchUp = 0,
    PitchDown,
    RollLeft,
    RollRight,
    YawLeft,
    YawRight,
    SpinDirection_Last
    };

  /// Stereo modes
  enum
    {
    NoStereo = 0,
    RedBlue,
    Anaglyph,
    QuadBuffer,
    Interlaced,
    UserDefined_1,
    UserDefined_2,
    UserDefined_3,
    StereoType_Last
    };

  /// Render modes
  enum
    {
    Perspective = 0,
    Orthographic,
    RenderMode_Last
    };

  /// Animation mode
  enum
    {
    Off = 0,
    Spin,
    Rock,
    AnimationMode_Last
    };

  /// Quality setting used for \sa VolumeRenderingQuality
  enum
    {
    Adaptive = 0, ///< quality determined from desired update rate
    Normal,       ///< good image quality at reasonable speed
    Maximum,      ///< high image quality, rendering time is not considered
    VolumeRenderingQuality_Last
    };

  /// Ray casting technique for volume rendering
  enum
    {
    Composite = 0, // Composite with directional lighting (default)
    CompositeEdgeColoring, // Composite with fake lighting (edge coloring, faster) - Not used
    MaximumIntensityProjection,
    MinimumIntensityProjection,
    GradiantMagnitudeOpacityModulation, // Not used
    IllustrativeContextPreservingExploration, // Not used
    RaycastTechnique_Last
    };

  /// Events
  enum
    {
    GraphicalResourcesCreatedEvent = 19001,
    ResetFocalPointRequestedEvent,
    };

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
  /// RenderModeFlag - broadcast the RenderMode to all linked viewers
  enum InteractionFlagType
  {
    None = 0,
    AnimationModeFlag,
    RenderModeFlag,
    BoxVisibleFlag,
    BoxLabelVisibileFlag,
    BackgroundColorFlag,
    StereoTypeFlag,
    OrientationMarkerTypeFlag,
    OrientationMarkerSizeFlag,
    RulerTypeFlag,
    RulerColorFlag,
    UseDepthPeelingFlag,
    FPSVisibleFlag,
  };

  ///
  /// toggle the view linking
  vtkGetMacro (LinkedControl, int );
  vtkSetMacro (LinkedControl, int );
  vtkBooleanMacro(LinkedControl, int);

  /// Get/Set a flag indicating what parameters are being manipulated
  /// within calls to InteractingOn() and InteractingOff(). These
  /// fields are used to propagate linked behaviors. This flag is a
  /// bitfield, with multiple parameters OR'd to compose the
  /// flag. Does not cause a Modified().
  void SetInteractionFlags(unsigned int);
  vtkGetMacro(InteractionFlags, unsigned int);

protected:
  vtkMRMLViewNode();
  ~vtkMRMLViewNode() override;
  vtkMRMLViewNode(const vtkMRMLViewNode&);
  void operator=(const vtkMRMLViewNode&);

  int FiducialsVisible;
  int FiducialLabelsVisible;
  int BoxVisible;
  int AxisLabelsVisible;
  int AxisLabelsCameraDependent;
  double FieldOfView;
  double LetterSize;

  /// Parameters of automatic spin
  int AnimationMode;
  int SpinDirection;
  double SpinDegrees;
  int AnimationMs;

  /// Parameters of automatic rock
  int RockLength;
  int RockCount;

  /// Increment used to rotate the view
  /// once about an axis.
  double RotateDegrees;

  /// Parameters for stereo viewing
  int StereoType;

  /// Specifies orthographic or perspective rendering
  int RenderMode;

  /// Parameters for look-from or rotate-around
  /// automatic view control
  int ViewAxisMode;

  /// Use the depth peeling rendering mode.
  int UseDepthPeeling;

  /// Show the Frame per second as text on the lower right part of the view
  int FPSVisible;

  /// Tracking GPU memory size in MB.
  /// Not saved into scene file because different machines may have different GPU memory values.
  /// A value of 0 indicates to use the default value in the settings
  int GPUMemorySize;

  /// Expected frame per second rendered
  double ExpectedFPS;

  /// Volume rendering quality control mode
  /// 0: Adaptive
  /// 1: Normal Quality
  /// 2: Maximum Quality
  int VolumeRenderingQuality;

  /// Techniques for volume rendering ray cast
  /// 0: Composite with directional lighting (default)
  /// 1: Composite with fake lighting (edge coloring, faster) - Not used
  /// 2: MIP
  /// 3: MINIP
  /// 4: Gradient Magnitude Opacity Modulation - Not used
  /// 5: Illustrative Context Preserving Exploration - Not used
  int RaycastTechnique;

  /// Make surface appearance smoother in volume rendering. Off by default
  bool VolumeRenderingSurfaceSmoothing;

  /// Oversampling factor for sample distance. The sample distance is calculated by \sa
  /// GetSampleDistance to be the volume's minimum spacing divided by the oversampling
  /// factor.
  /// If \sa VolumeRenderingQuality is set to maximum quality, then a fix oversampling factor of 10 is used.
  double VolumeRenderingOversamplingFactor;

  int LinkedControl;
  int Interacting;
  unsigned int InteractionFlags;

};

#endif
