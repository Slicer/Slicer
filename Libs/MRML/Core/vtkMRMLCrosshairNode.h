/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCrosshairNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLCrosshairNode_h
#define __vtkMRMLCrosshairNode_h

#include "vtkMRMLNode.h"
#include "vtkMRMLSliceNode.h"

/// \brief MRML node for storing a crosshair through RAS space
///
/// This node stores the information about a crosshair (position,
/// style, attributes)
class VTK_MRML_EXPORT vtkMRMLCrosshairNode : public vtkMRMLNode
{
public:
  static vtkMRMLCrosshairNode* New();
  vtkTypeMacro(vtkMRMLCrosshairNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// CursorPositionModifiedEvent is invoked when the cursor position is modified,
  /// for example a mouse pointer is moved in a slice view.
  enum
  {
    CursorPositionModifiedEvent = 22000
  };

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes(const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLCrosshairNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "Crosshair"; };

  ///@{
  /// Configure crosshair appearance.
  vtkGetMacro(CrosshairMode, int);
  vtkSetMacro(CrosshairMode, int);
  static const char* GetCrosshairModeAsString(int id);
  static int GetCrosshairModeFromString(const char* name);

  vtkSetClampMacro(CrosshairThickness, int, 1, 3);
  vtkGetMacro(CrosshairThickness, int);
  void SetCrosshairToFine() { this->SetCrosshairThickness(1); }
  void SetCrosshairToMedium() { this->SetCrosshairThickness(2); }
  void SetCrosshairToThick() { this->SetCrosshairThickness(3); }
  static const char* GetCrosshairThicknessAsString(int id);
  static int GetCrosshairThicknessFromString(const char* name);

  vtkGetVector3Macro(CrosshairColor, float);
  vtkSetVector3Macro(CrosshairColor, float);
  ///@}

  ///@{
  /// Configure crosshair behavior.
  vtkGetMacro(CrosshairBehavior, int);
  vtkSetMacro(CrosshairBehavior, int);
  static const char* GetCrosshairBehaviorAsString(int id);
  static int GetCrosshairBehaviorFromString(const char* name);
  ///@}

  ///
  /// Set cursor position in 3D.
  /// This should be called whenever the cursor is moved by using a 3D positioning device
  /// (that does not have an associated slice viewer).
  /// GetCursorPositionXYZ returns with nullptr if SetCursorPositionRAS is used for setting the
  /// cursor position, so if slice position is available then SetCursorPositionXYZ method should
  /// be used instead (it sets both XYZ and RAS position, too).
  void SetCursorPositionRAS(double ras[3]);

  ///
  /// Set cursor position in a 2D slice view.
  /// The method also computes the RAS position.
  /// This method should be called whenever the mouse moves in a slice viewer.
  void SetCursorPositionXYZ(double xyz[3], vtkMRMLSliceNode* sliceNode);

  /// Invalidates the cursor position. This should be called whenever the mouse moves out of a view.
  void SetCursorPositionInvalid();

  /// Get the position of the cursor in the current view in RAS coordinate system.
  /// Returns false if the cursor position is unknown.
  /// This is typically called by other classes to get the current cursor position after receiving
  /// CursorPositionModifiedEvent event.
  bool GetCursorPositionRAS(double ras[3]);

  /// Get the position of the cursor in the current view in XYZ coordinate system.
  /// Returns nullptr if the cursor is not in a slice viewer.
  /// This is typically called by other classes to get the current cursor position after receiving
  /// CursorPositionModifiedEvent event.
  vtkMRMLSliceNode* GetCursorPositionXYZ(double xyz[3]);

  ///
  /// Set crosshair position
  vtkSetVector3Macro(CrosshairRAS, double);
  vtkGetVector3Macro(CrosshairRAS, double);

  /// Set the crosshair position and cache which pane of lightbox that
  /// position was in
  void SetCrosshairRAS(double ras[3], int id);

  /// Get the pane of the lightbox that was last cached
  vtkGetMacro(LightBoxPane, int);

  ///
  /// Name of the layout
  void SetCrosshairName(const char* name) { this->SetSingletonTag(name); }
  char* GetCrosshairName() { return this->GetSingletonTag(); }

  /// Modes for crosshair display
  enum
  {
    NoCrosshair = 0,
    ShowBasic,
    ShowIntersection,
    ShowHashmarks,
    ShowAll,
    ShowSmallBasic,
    ShowSmallIntersection,
    CrosshairMode_Last
  };
  enum
  {
    Fine = 1,
    Medium,
    Thick,
    CrosshairThickness_Last
  };
  /// Behavior when crosshair position is changed.
  enum
  {
    NoAction = 0,
    OffsetJumpSlice = 1,
    Normal = 1, ///< \deprecated Use OffsetJumpSlice instead
    CenteredJumpSlice = 2,
    JumpSlice = 2, ///< \deprecated Use CenteredJumpSlice instead
    CrosshairBehavior_Last
  };

  ///@{
  /// Experimental feature to choose between fast or accurate position computation in 3D views.
  /// If fast option is chosen then Z-buffer in the renderer may be used for determining position
  /// along the view line in 3D views, which is very fast but may not give optimal results
  /// because semi-transparent objects, such as semi-transparent models or
  /// volume-rendered images are ignored.
  /// This property is only for evaluation of this feature and the value
  /// is not stored persistently in the scene file.
  vtkSetMacro(FastPick3D, bool);
  vtkGetMacro(FastPick3D, bool);
  vtkBooleanMacro(FastPick3D, bool);
  ///@}

protected:
  vtkMRMLCrosshairNode();
  ~vtkMRMLCrosshairNode() override;
  vtkMRMLCrosshairNode(const vtkMRMLCrosshairNode&);
  void operator=(const vtkMRMLCrosshairNode&);

  int CrosshairMode{ NoCrosshair };
  int CrosshairThickness{ Fine };
  int CrosshairBehavior{ OffsetJumpSlice };
  float CrosshairColor[3]{ 1.0f, 0.8f, 0.1f }; // Light yellow

  double CrosshairRAS[3]{ 0.0, 0.0, 0.0 };
  int LightBoxPane{ 0 };

  /// Last known cursor position in RAS coordinate system.
  /// If CursorPositionValid is false then this position is not up-to-date anymore.
  double CursorPositionRAS[3]{ 0.0, 0.0, 0.0 };
  /// Last known cursor position in XYZ coordinate system.
  /// If CursorSliceNode is nullptr then this position is not up-to-date anymore.
  double CursorPositionXYZ[3]{ 0.0, 0.0, 0.0 };
  /// CursorSliceNode points to the slice where the cursor is. It is not stored as a MRML node reference
  /// as its value is not saved with the scene and also it changes frequently therefore it is better
  /// to keep a lightweight reference.
  vtkWeakPointer<vtkMRMLSliceNode> CursorSliceNode;

  /// Set to false if the cursor is not in a view
  bool CursorPositionRASValid{ false };
  bool FastPick3D{ false };
};

#endif
