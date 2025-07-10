/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Ebatinca S.L., Las Palmas de Gran Canaria, Spain

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

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
  static vtkMRMLSliceDisplayNode* New();
  vtkTypeMacro(vtkMRMLSliceDisplayNode, vtkMRMLModelDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLSliceDisplayNode);

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "SliceDisplay"; }

  //@{
  /// Toggles visibility of intersections of other slices in the slice viewer
  bool GetIntersectingSlicesVisibility() { return this->GetVisibility2D(); };
  void SetIntersectingSlicesVisibility(bool visible) { this->SetVisibility2D(visible); };
  vtkBooleanMacro(IntersectingSlicesVisibility, bool);
  //@}

  //@{
  /// Toggles visibility of thick slabs of other slices in the slice viewer
  vtkGetMacro(IntersectingThickSlabVisibility, bool);
  vtkSetMacro(IntersectingThickSlabVisibility, bool);
  vtkBooleanMacro(IntersectingThickSlabVisibility, bool);
  //@}

  //@{
  /// Toggles interaction with slice intersections
  vtkGetMacro(IntersectingSlicesInteractive, bool);
  vtkSetMacro(IntersectingSlicesInteractive, bool);
  vtkBooleanMacro(IntersectingSlicesInteractive, bool);
  //@}

  //@{
  /// Toggles interaction with thick slab intersections
  vtkGetMacro(IntersectingThickSlabInteractive, bool);
  vtkSetMacro(IntersectingThickSlabInteractive, bool);
  vtkBooleanMacro(IntersectingThickSlabInteractive, bool);
  //@}

  //@{
  /// Enable/disable interaction handles
  vtkGetMacro(IntersectingSlicesTranslationEnabled, bool);
  vtkSetMacro(IntersectingSlicesTranslationEnabled, bool);
  vtkBooleanMacro(IntersectingSlicesTranslationEnabled, bool);
  vtkGetMacro(IntersectingSlicesRotationEnabled, bool);
  vtkSetMacro(IntersectingSlicesRotationEnabled, bool);
  vtkBooleanMacro(IntersectingSlicesRotationEnabled, bool);
  //@}

  enum IntersectingSlicesInteractiveMode
  {
    ModeRotation,
    ModeTranslation
  };

  //@{
  /// Enable/disable interaction handles.
  /// \sa IntersectingSlicesInteractiveMode, SetIntersectingSlicesTranslationEnabled,
  /// SetIntersectingSlicesRotationEnabled
  void SetIntersectingSlicesInteractiveModeEnabled(IntersectingSlicesInteractiveMode mode, bool enabled);
  bool GetIntersectingSlicesInteractiveModeEnabled(IntersectingSlicesInteractiveMode mode);
  //@}

  enum HandlesVisibilityMode
  {
    NeverVisible = 0, ///< Handles are never displayed
    NearbyVisible,    ///< Handles are displayed when mouse pointer gets close
    AlwaysVisible,    ///< Handles are always visible
    // FadingVisible, // Handles' opacity increases as the mouse gets closer to them
    HandlesVisibilityMode_Last // insert new types above this line
  };

  //@{
  /// Get/Set interaction handles visibility mode.
  /// \sa HandlesVisibilityMode
  vtkGetMacro(IntersectingSlicesInteractiveHandlesVisibilityMode, int);
  vtkSetMacro(IntersectingSlicesInteractiveHandlesVisibilityMode, int);
  //@}

  //@{
  /// Convert between intersecting slices interactive handles visibility mode ID and string.
  static const char* GetIntersectingSlicesInteractiveHandlesVisibilityModeAsString(int id);
  static int GetIntersectingSlicesInteractiveHandlesVisibilityModeFromString(const char*);
  //@}

  enum IntersectionMode
  {
    SkipLineCrossings = 0, ///< Lines do not intersect and there is a gap around the intersection point
    FullLines,             ///< Lines intersect
    IntersectionMode_Last  // insert new types above this line
  };

  //@{
  /// Get/set intersection mode.
  /// \sa IntersectionMode
  vtkGetMacro(IntersectingSlicesIntersectionMode, int);
  vtkSetMacro(IntersectingSlicesIntersectionMode, int);
  //@}

  //@{
  /// Convert between intersecting slices intersection mode ID and string.
  static const char* GetIntersectingSlicesIntersectionModeAsString(int id);
  static int GetIntersectingSlicesIntersectionModeFromString(const char*);
  //@}

  enum LineThicknessMode
  {
    FineLines = 0,         ///< Thin lines
    MediumLines,           ///< Medium lines
    ThickLines,            ///< Thick lines
    LineThicknessMode_Last // insert new types above this line
  };

  //@{
  /// Get/set line thickness mode.
  /// \sa LineThicknessMode
  vtkGetMacro(IntersectingSlicesLineThicknessMode, int);
  vtkSetMacro(IntersectingSlicesLineThicknessMode, int);
  //@}

  //@{
  /// Convert between intersecting slices line thickness mode ID and string
  /// \sa LineThicknessMode
  static const char* GetIntersectingSlicesLineThicknessModeAsString(int id);
  static int GetIntersectingSlicesLineThicknessModeFromString(const char*);
  //@}

  /// Get name of the default interaction context (typically the mouse)
  static const std::string GetDefaultContextName() { return ""; };

  enum ComponentType
  {
    ComponentNone = 0,                          ///< no component of the slice or slice intersection widget is active
    ComponentTranslateIntersectingSlicesHandle, ///< mouse is near the intersection point of slice intersections
    ComponentRotateIntersectingSlicesHandle,    ///< mouse is near the end of the slice intersection (rotation section)
    ComponentTranslateSingleIntersectingSliceHandle, ///< mouse is near the middle of the slice intersection
                                                     ///< (translation section)
    ComponentSliceIntersection, ///< slice intersection is active (not any handle), e.g., because user is interacting
                                ///< with the widget
    ComponentTranslateIntersectingThickSlabHandle, ///< mouse is near the handle for adjusting a thick slab
    Component_Last
  };

  /// Active component (that the mouse or other interaction context is hovered over).
  /// This property is computed on-the-fly and saved to file.
  /// \param context Name of the interaction context. By default it is empty string, meaning mouse.
  ///   Additional devices, such as virtual reality controllers can specify additional context names.
  ///   This mechanism allows interacting with multiple markups at the same time (user can grab
  ///   different markup points with each controller at the same time).
  /// \sa ComponentType
  int GetActiveComponentType(std::string context = vtkMRMLSliceDisplayNode::GetDefaultContextName());

  /// Index of active component (that the mouse or other interaction context is hovered over).
  /// This property is computed on-the-fly and saved to file.
  /// \param context Name of the interaction context. By default it is empty string, meaning mouse
  int GetActiveComponentIndex(std::string context = vtkMRMLSliceDisplayNode::GetDefaultContextName());

  /// Set active component type and index for interaction context (empty by default, meaning mouse)
  void SetActiveComponent(int componentType,
                          int componentIndex,
                          std::string context = vtkMRMLSliceDisplayNode::GetDefaultContextName());

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

  bool IntersectingSlicesInteractive{ false };
  bool IntersectingSlicesTranslationEnabled{ true };
  bool IntersectingSlicesRotationEnabled{ true };
  bool IntersectingThickSlabInteractive{ false };
  bool IntersectingThickSlabVisibility{ false };

  int IntersectingSlicesInteractiveHandlesVisibilityMode{ NeverVisible };

  int IntersectingSlicesIntersectionMode{ FullLines };

  int IntersectingSlicesLineThicknessMode{ FineLines };

  /// Current active point or widget component type and index (hovered by the mouse or other interaction context)
  /// Map interaction context identifier (empty string for mouse) to component type enum
  std::map<std::string, ComponentInfo> ActiveComponents;
};

#endif
