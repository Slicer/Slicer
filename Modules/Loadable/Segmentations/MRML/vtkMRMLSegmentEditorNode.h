/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __vtkMRMLSegmentEditorNode_h
#define __vtkMRMLSegmentEditorNode_h

// MRML includes
#include <vtkMRMLNode.h>

// Segmentations includes
#include "vtkSlicerSegmentationsModuleMRMLExport.h"

#include "vtkOrientedImageData.h"

class vtkMRMLScene;
class vtkMRMLScalarVolumeNode;
class vtkMRMLSegmentationNode;

/// \ingroup Segmentations
/// \brief Parameter set node for the segment editor widget
///
/// Stores parameters for a segment editor widget (selected segmentation, segment, master volume),
/// and all the editor effects. The effect parameters are stored as attributes with names
/// EffectName.ParameterName. If a parameter is changed, the node Modified event is not emitted,
/// but the custom EffectParameterModified event that triggers update of the effect options widget only.
///
class VTK_SLICER_SEGMENTATIONS_MODULE_MRML_EXPORT vtkMRMLSegmentEditorNode : public vtkMRMLNode
{
public:
  enum
    {
    /// Fired when an effect parameter is modified. As this node handles not only the effect parameters,
    /// but also the segment editor state, a full Modified event is an overkill, because it would trigger
    /// editor widget UI update, instead of simple update of the effect option widgets only.
    EffectParameterModified = 62200
    };

  enum
    {
    /// Modification is allowed everywhere.
    PaintAllowedEverywhere=0,
    /// Modification is allowed inside all segments.
    PaintAllowedInsideAllSegments,
    /// Modification is allowed inside all visible segments.
    PaintAllowedInsideVisibleSegments,
    /// Modification is allowed outside all segments.
    PaintAllowedOutsideAllSegments,
    /// Modification is allowed outside all visible segments.
    PaintAllowedOutsideVisibleSegments,
    /// Modification is allowed only over the area covered by segment specified in MaskSegmentID.
    PaintAllowedInsideSingleSegment,

    PaintAllowed_Last /// Insert valid types above this line
    };

  enum
    {
    /// Areas added to selected segment will be removed from all other segments. (no overlap)
    OverwriteAllSegments=0,
    /// Areas added to selected segment will be removed from all visible segments. (no overlap with visible, overlap possible with hidden)
    OverwriteVisibleSegments,
    /// Areas added to selected segment will be removed from all other segments. (overlap with all other segments)
    OverwriteNone,

    Overwrite_Last /// Insert valid types above this line
    };

public:
  static vtkMRMLSegmentEditorNode *New();
  vtkTypeMacro(vtkMRMLSegmentEditorNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Create instance of a GAD node.
  virtual vtkMRMLNode* CreateNodeInstance();

  /// Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() { return "SegmentEditor"; };

  static int ConvertOverwriteModeFromString(const char* modeStr);
  static const char* ConvertOverwriteModeToString(int mode);
  static const char* ConvertMaskModeToString(int mode);
  static int ConvertMaskModeFromString(const char* modeStr);

public:
  /// Get master volume node
  vtkMRMLScalarVolumeNode* GetMasterVolumeNode();
  /// Set and observe master volume node
  void SetAndObserveMasterVolumeNode(vtkMRMLScalarVolumeNode* node);

  /// Get segmentation node
  vtkMRMLSegmentationNode* GetSegmentationNode();
  /// Set and observe segmentation node
  void SetAndObserveSegmentationNode(vtkMRMLSegmentationNode* node);

  /// Get selected segment ID
  vtkGetStringMacro(SelectedSegmentID);
  /// Set selected segment ID
  vtkSetStringMacro(SelectedSegmentID);

  /// Get active effect name
  vtkGetStringMacro(ActiveEffectName);
  /// Set active effect name
  vtkSetStringMacro(ActiveEffectName);

  /// Defines which areas are editable.
  /// Uses PAINT_ALLOWED_... constants.
  vtkSetMacro(MaskMode, int);
  vtkGetMacro(MaskMode, int);

  /// Set mask segment ID.
  /// Painting is only allowed within the area of the mask segment if mask mode is PAINT_ALLOWED_INSIDE_SINGLE_SEGMENT.
  vtkGetStringMacro(MaskSegmentID);
  /// Get mask segment ID.
  vtkSetStringMacro(MaskSegmentID);

  /// Restrict editable area to regions where mask volume intensity is in the specified range.
  vtkBooleanMacro(MasterVolumeIntensityMask, bool);
  vtkGetMacro(MasterVolumeIntensityMask, bool);
  vtkSetMacro(MasterVolumeIntensityMask, bool);

  /// Set mask volume intensity range for masking.
  /// \sa SetMasterVolumeIntensityMask()
  vtkSetVector2Macro(MasterVolumeIntensityMaskRange, double);
  /// Get mask volume intensity range for masking.
  /// \sa SetMasterVolumeIntensityMask()
  vtkGetVector2Macro(MasterVolumeIntensityMaskRange, double);

  /// Defines which areas are overwritten in other segments.
  /// Uses OVERWRITE_... constants.
  vtkSetMacro(OverwriteMode, int);
  vtkGetMacro(OverwriteMode, int);

protected:
  vtkMRMLSegmentEditorNode();
  ~vtkMRMLSegmentEditorNode();
  vtkMRMLSegmentEditorNode(const vtkMRMLSegmentEditorNode&);
  void operator=(const vtkMRMLSegmentEditorNode&);

  /// Selected segment ID
  char* SelectedSegmentID;

  /// Active effect name
  char* ActiveEffectName;

  int MaskMode;
  char* MaskSegmentID;

  int OverwriteMode;

  bool MasterVolumeIntensityMask;
  double MasterVolumeIntensityMaskRange[2];
};

#endif // __vtkMRMLSegmentEditorNode_h
