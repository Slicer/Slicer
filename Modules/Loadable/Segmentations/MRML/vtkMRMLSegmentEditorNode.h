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
#include <vtkMRMLSegmentationNode.h>

// Segmentations includes
#include "vtkSlicerSegmentationsModuleMRMLExport.h"

#include "vtkOrientedImageData.h"

class vtkMRMLScene;
class vtkMRMLScalarVolumeNode;

/// \ingroup Segmentations
/// \brief Parameter set node for the segment editor widget
///
/// Stores parameters for a segment editor widget (selected segmentation, segment, source volume),
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
    /// Areas added to selected segment will be removed from all other segments. (no overlap)
    OverwriteAllSegments=0,
    /// Areas added to selected segment will be removed from all visible segments. (no overlap with visible, overlap possible with hidden)
    OverwriteVisibleSegments,
    /// Areas added to selected segment will not be removed from any segments. (overlap with all other segments)
    OverwriteNone,
    // Insert valid types above this line
    Overwrite_Last
    };

public:
  static vtkMRMLSegmentEditorNode *New();
  vtkTypeMacro(vtkMRMLSegmentEditorNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create a new node of this type.
  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes from name/value pairs
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  /// Get unique node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "SegmentEditor"; }

  //@{
  /// Convert between constants IDs to/from string
  static int ConvertOverwriteModeFromString(const char* modeStr);
  static const char* ConvertOverwriteModeToString(int mode);
  //@}

public:

  //@{
  /// Get/set source volume node.
  /// Source volume node is used when editing requires an underlying image.
  vtkMRMLScalarVolumeNode* GetSourceVolumeNode();
  void SetAndObserveSourceVolumeNode(vtkMRMLScalarVolumeNode* node);
  //@}

  //@{
  /// Deprecated. Use GetSourceVolumeNode/SetAndObserveSourceVolumeNode methods instead.
  vtkMRMLScalarVolumeNode* GetMasterVolumeNode();
  void SetAndObserveMasterVolumeNode(vtkMRMLScalarVolumeNode* node);
  //@}

  //@{
  /// Get/set segmentation node
  vtkMRMLSegmentationNode* GetSegmentationNode();
  void SetAndObserveSegmentationNode(vtkMRMLSegmentationNode* node);
  //@}

  //@{
  /// Get/set selected segment ID.
  /// This is the segment that is currently being edited.
  vtkGetStringMacro(SelectedSegmentID);
  vtkSetStringMacro(SelectedSegmentID);
  //@}

  //@{
  /// Get/set name of the active effect.
  vtkGetStringMacro(ActiveEffectName);
  vtkSetStringMacro(ActiveEffectName);
  //@}

  //@{
  /// Defines which areas in the segmentation are editable.
  /// Uses vtkMRMLSegmentationNode::EditAllowed_... constants.
  /// \sa vtkMRMLSegmentationNode::EditAllowedEverywhere, vtkMRMLSegmentationNode::EditAllowedInsideAllSegments,
  /// vtkMRMLSegmentationNode::EditAllowedInsideVisibleSegments, vtkMRMLSegmentationNode::EditAllowedOutsideAllSegments,
  /// vtkMRMLSegmentationNode::EditAllowedOutsideVisibleSegments, vtkMRMLSegmentationNode::EditAllowedInsideSingleSegment
  vtkSetMacro(MaskMode, int);
  vtkGetMacro(MaskMode, int);
  //@}

  //@{
  /// Get/set mask segment ID.
  /// Painting is only allowed within the area of the mask segment if mask mode is EditAllowedInsideSingleSegment.
  /// \sa vtkMRMLSegmentationNode::EditAllowedInsideSingleSegment, SetMaskMode
  vtkGetStringMacro(MaskSegmentID);
  vtkSetStringMacro(MaskSegmentID);
  //@}

  //@{
  /// Restrict editable area to regions where source volume intensity is in the specified range.
  vtkBooleanMacro(SourceVolumeIntensityMask, bool);
  vtkGetMacro(SourceVolumeIntensityMask, bool);
  vtkSetMacro(SourceVolumeIntensityMask, bool);
  //@}

  //@{
  /// Deprecated. Use Get/SetSourceVolumeIntensityMask method instead.
  virtual void MasterVolumeIntensityMaskOn() { this->SourceVolumeIntensityMaskOn(); }
  virtual void MasterVolumeIntensityMaskOff() { this->SourceVolumeIntensityMaskOff(); }
  virtual void SetMasterVolumeIntensityMask(bool enable) { this->SetSourceVolumeIntensityMask(enable); }
  virtual bool GetMasterVolumeIntensityMask() { return this->GetSourceVolumeIntensityMask(); }
  //@}

  //@{
  /// Get/set source volume intensity range for masking.
  /// If SourceVolumeIntensityMask is enabled then only those areas are editable where
  /// source volume voxels are in this intensity range.
  /// \sa SetSourceVolumeIntensityMask()
  vtkSetVector2Macro(SourceVolumeIntensityMaskRange, double);
  vtkGetVector2Macro(SourceVolumeIntensityMaskRange, double);
  //@}

  //@{
  /// Deprecated. Use Get/SetSourceVolumeIntensityMaskRange method instead.
  virtual void SetMasterVolumeIntensityMaskRange(double _arg1, double _arg2)
    {
    vtkWarningMacro("vtkMRMLSegmentEditorNode::SetMasterVolumeIntensityMaskRange() method is deprecated, use SetSourceVolumeIntensityMaskRange method instead");
    this->SetSourceVolumeIntensityMaskRange(_arg1, _arg2);
    }
  void SetMasterVolumeIntensityMaskRange(const double _arg[2])
    {
    vtkWarningMacro("vtkMRMLSegmentEditorNode::SetMasterVolumeIntensityMaskRange() method is deprecated, use SetSourceVolumeIntensityMaskRange method instead");
    this->SetSourceVolumeIntensityMaskRange(_arg);
    }

  virtual double* GetMasterVolumeIntensityMaskRange() VTK_SIZEHINT(2)
    {
    vtkWarningMacro("vtkMRMLSegmentEditorNode::GetMasterVolumeIntensityMaskRange() method is deprecated, use GetSourceVolumeIntensityMaskRange method instead");
    return this->GetSourceVolumeIntensityMaskRange();
    }

  VTK_WRAPEXCLUDE
  virtual void GetMasterVolumeIntensityMaskRange(double& _arg1, double& _arg2)
    {
    vtkWarningMacro("vtkMRMLSegmentEditorNode::GetMasterVolumeIntensityMaskRange() method is deprecated, use GetSourceVolumeIntensityMaskRange method instead");
    this->GetSourceVolumeIntensityMaskRange(_arg2, _arg2);
    }

  VTK_WRAPEXCLUDE
  virtual void GetMasterVolumeIntensityMaskRange(double _arg[2])
    {
    vtkWarningMacro("vtkMRMLSegmentEditorNode::GetMasterVolumeIntensityMaskRange() method is deprecated, use GetSourceVolumeIntensityMaskRange method instead");
    this->GetSourceVolumeIntensityMaskRange(_arg);
    }
  //@}

  //@{
  /// Defines which areas are overwritten in other segments.
  /// Uses Overwrite... constants.
  vtkSetMacro(OverwriteMode, int);
  vtkGetMacro(OverwriteMode, int);
  //@}

protected:
  vtkMRMLSegmentEditorNode();
  ~vtkMRMLSegmentEditorNode() override;
  vtkMRMLSegmentEditorNode(const vtkMRMLSegmentEditorNode&);
  void operator=(const vtkMRMLSegmentEditorNode&);

  /// Selected segment ID
  char* SelectedSegmentID{nullptr};

  /// Active effect name
  char* ActiveEffectName{nullptr};

  int MaskMode{vtkMRMLSegmentationNode::EditAllowedEverywhere};
  char* MaskSegmentID{nullptr};

  int OverwriteMode{OverwriteAllSegments};

  bool SourceVolumeIntensityMask{false};
  double SourceVolumeIntensityMaskRange[2];
};

#endif // __vtkMRMLSegmentEditorNode_h
