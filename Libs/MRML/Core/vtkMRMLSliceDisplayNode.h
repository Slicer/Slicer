/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkMRMLSliceDisplayNode_h
#define __vtkMRMLSliceDisplayNode_h

// MRML includes
#include "vtkMRMLModelDisplayNode.h"

/// \brief MRML node to store display properties of slice nodes.
///
/// This node controls appearance of slice intersections in slice views
/// and slices in 3D views.
class VTK_MRML_EXPORT vtkMRMLSliceDisplayNode : public vtkMRMLModelDisplayNode
{
public:
  static vtkMRMLSliceDisplayNode *New();
  vtkTypeMacro(vtkMRMLSliceDisplayNode,vtkMRMLModelDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLSliceDisplayNode);

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "SliceDisplay";}

  /// toggles visibility of intersections of other slices in the slice viewer
  bool GetSliceIntersectionVisibility() { return this->GetVisibility2D(); };
  void SetSliceIntersectionVisibility(bool visible) { this->SetVisibility2D(visible); };
  vtkBooleanMacro(SliceIntersectionVisibility, bool);

  /// toggles interaction with slice intersections
  vtkGetMacro(SliceIntersectionInteractive, bool);
  vtkSetMacro(SliceIntersectionInteractive, bool);
  vtkBooleanMacro(SliceIntersectionInteractive, bool);

  // Interaction handles
  enum SliceIntersectionInteractiveMode
  {
    ModeRotation,
    ModeTranslation
  };
  vtkGetMacro(SliceIntersectionTranslationEnabled, bool);
  vtkSetMacro(SliceIntersectionTranslationEnabled, bool);
  vtkBooleanMacro(SliceIntersectionTranslationEnabled, bool);
  vtkGetMacro(SliceIntersectionRotationEnabled, bool);
  vtkSetMacro(SliceIntersectionRotationEnabled, bool);
  vtkBooleanMacro(SliceIntersectionRotationEnabled, bool);
  void SetSliceIntersectionInteractiveModeEnabled(SliceIntersectionInteractiveMode mode, bool enabled);
  bool GetSliceIntersectionInteractiveModeEnabled(SliceIntersectionInteractiveMode mode);

  /// Get name of the default interaction context (typically the mouse)
  static const std::string GetDefaultContextName() { return ""; };

  /// Active component (that the mouse or other interaction context is hovered over).
  /// This property is computed on-the-fly and saved to file.
  /// \param context Name of the interaction context. By default it is empty string, meaning mouse.
  ///   Additional devices, such as virtual reality controllers can specify additional context names.
  ///   This mechanism allows interacting with multiple markups at the same time (user can grab
  ///   different markup points with each controller at the same time).
  int GetActiveComponentType(std::string context = vtkMRMLSliceDisplayNode::GetDefaultContextName());
  enum ComponentType
    {
    ComponentNone = 0, ///< no component of the slice or slice intersection widget is active
    ComponentTranslateIntersectingSlicesHandle, ///< mouse is near the intersection point of slice intersections
    ComponentRotateIntersectingSlicesHandle, ///< mouse is near the end of the slice intersection (rotation section)
    ComponentTranslateSingleIntersectingSliceHandle, ///< mouse is near the middle of the slice intersection (translation section)
    ComponentSliceIntersection, ///< slice intersection is active (not any handle), e.g., because user is interacting with the widget
    Component_Last
    };
  struct ComponentInfo
    {
    ComponentInfo()
      {
      this->Type = ComponentNone;
      this->Index = -1;
      }
    int Type;
    int Index;
    };

  /// Index of active component (that the mouse or other interaction context is hovered over).
  /// This property is computed on-the-fly and saved to file.
  /// \param context Name of the interaction context. By default it is empty string, meaning mouse
  int GetActiveComponentIndex(std::string context= vtkMRMLSliceDisplayNode::GetDefaultContextName());

  /// Set active component type and index for interaction context (empty by default, meaning mouse)
  void SetActiveComponent(int componentType, int componentIndex,
                          std::string context= vtkMRMLSliceDisplayNode::GetDefaultContextName());

  /// Query if there is an active component for any interaction context
  bool HasActiveComponent();

  /// Get list of interaction context names that have active components
  /// \return List of interaction context names that have active components
  std::vector<std::string> GetActiveComponentInteractionContexts();

protected:
  vtkMRMLSliceDisplayNode();
  ~vtkMRMLSliceDisplayNode() override;
  vtkMRMLSliceDisplayNode(const vtkMRMLSliceDisplayNode&);
  void operator=(const vtkMRMLSliceDisplayNode&);

  bool SliceIntersectionInteractive{ false };
  bool SliceIntersectionTranslationEnabled{ true };
  bool SliceIntersectionRotationEnabled{ true };

  /// Current active point or widget component type and index (hovered by the mouse or other interaction context)
  /// Map interaction context identifier (empty string for mouse) to component type enum
  std::map<std::string, ComponentInfo> ActiveComponents;
};

#endif
