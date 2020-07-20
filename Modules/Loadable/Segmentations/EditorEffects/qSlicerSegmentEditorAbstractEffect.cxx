/*==============================================================================

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

// Segmentations includes
#include "qSlicerSegmentEditorAbstractEffect.h"
#include "qSlicerSegmentEditorAbstractEffect_p.h"

#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSegmentEditorNode.h"

#include "vtkSlicerSegmentationsModuleLogic.h"

// SegmentationCore includes
#include <vtkOrientedImageData.h>
#include <vtkOrientedImageDataResample.h>

// Qt includes
#include <QDebug>
#include <QFormLayout>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QPaintDevice>
#include <QFrame>
#include <QColor>

// Slicer includes
#include "qMRMLSliceWidget.h"
#include "qMRMLSliceView.h"
#include "qMRMLThreeDWidget.h"
#include "qMRMLThreeDView.h"
#include "vtkMRMLSliceLogic.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkImageConstantPad.h>
#include <vtkImageShiftScale.h>
#include <vtkImageThreshold.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkProp.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorAbstractEffectPrivate methods

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffectPrivate::qSlicerSegmentEditorAbstractEffectPrivate(qSlicerSegmentEditorAbstractEffect& object)
  : q_ptr(&object)
  , Scene(nullptr)
  , SavedCursor(QCursor(Qt::ArrowCursor))
  , OptionsFrame(nullptr)
{
  this->OptionsFrame = new QFrame();
  this->OptionsFrame->setFrameShape(QFrame::NoFrame);
  this->OptionsFrame->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
  QFormLayout* layout = new QFormLayout(this->OptionsFrame);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffectPrivate::~qSlicerSegmentEditorAbstractEffectPrivate()
{
  if (this->OptionsFrame)
    {
    delete this->OptionsFrame;
    this->OptionsFrame = nullptr;
    }
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// qSlicerSegmentEditorAbstractEffect methods

//----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect::qSlicerSegmentEditorAbstractEffect(QObject* parent)
 : Superclass(parent)
 , m_Name(QString())
 , d_ptr(new qSlicerSegmentEditorAbstractEffectPrivate(*this))
{
}

//----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect::~qSlicerSegmentEditorAbstractEffect() = default;

//-----------------------------------------------------------------------------
QString qSlicerSegmentEditorAbstractEffect::name()const
{
  if (m_Name.isEmpty())
    {
    qCritical() << Q_FUNC_INFO << ": Empty effect name!";
    }
  return this->m_Name;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setName(QString name)
{
  Q_UNUSED(name);
  qCritical() << Q_FUNC_INFO << ": Cannot set effect name by method, only in constructor!";
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::perSegment()const
{
  return this->m_PerSegment;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setPerSegment(bool perSegment)
{
  Q_UNUSED(perSegment);
  qCritical() << Q_FUNC_INFO << ": Cannot set per-segment flag by method, only in constructor!";
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::requireSegments()const
{
  return this->m_RequireSegments;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setRequireSegments(bool requireSegments)
{
  this->m_RequireSegments = requireSegments;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::activate()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  // Show options frame
  d->OptionsFrame->setVisible(true);

  this->m_Active = true;

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::deactivate()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  // Hide options frame
  d->OptionsFrame->setVisible(false);

  this->m_Active = false;
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::active()
{
  return m_Active;
}


//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCallbackSlots(QObject* receiver, const char* selectEffectSlot,
  const char* updateVolumeSlot, const char* saveStateForUndoSlot)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  QObject::connect(d, SIGNAL(selectEffectSignal(QString)), receiver, selectEffectSlot);
  QObject::connect(d, SIGNAL(updateVolumeSignal(void*,bool&)), receiver, updateVolumeSlot);
  QObject::connect(d, SIGNAL(saveStateForUndoSignal()), receiver, saveStateForUndoSlot);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::applyImageMask(vtkOrientedImageData* input, vtkOrientedImageData* mask, double fillValue,
  bool notMask/*=false*/)
{
  // The method is now moved to vtkOrientedImageDataResample::ApplyImageMask but kept here
  // for a while for backward compatibility.
  vtkOrientedImageDataResample::ApplyImageMask(input, mask, fillValue, notMask);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::modifySelectedSegmentByLabelmap(vtkOrientedImageData* modifierLabelmap,
  ModificationMode modificationMode, bool bypassMasking/*=false*/)
{
  int modificationExtent[6] = { 0, -1, 0, -1, 0, -1 };
  this->modifySelectedSegmentByLabelmap(modifierLabelmap, modificationMode, modificationExtent, bypassMasking);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::modifySelectedSegmentByLabelmap(vtkOrientedImageData* modifierLabelmap,
  ModificationMode modificationMode, QList<int> extent, bool bypassMasking/*=false*/)
{
  if (extent.size() != 6)
    {
    qCritical() << Q_FUNC_INFO << " failed: extent must have 6 int values";
    return;
    }
  int modificationExtent[6] = { extent[0], extent[1], extent[2], extent[3], extent[4], extent[5] };
  this->modifySelectedSegmentByLabelmap(modifierLabelmap, modificationMode, modificationExtent, bypassMasking);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::modifySelectedSegmentByLabelmap(vtkOrientedImageData* modifierLabelmap,
  ModificationMode modificationMode, const int modificationExtent[6], bool bypassMasking/*=false*/)
{
  this->modifySegmentByLabelmap(this->parameterSetNode()->GetSegmentationNode(),
    this->parameterSetNode()->GetSelectedSegmentID() ? this->parameterSetNode()->GetSelectedSegmentID() : "",
    modifierLabelmap, modificationMode, modificationExtent, bypassMasking);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::modifySegmentByLabelmap(vtkMRMLSegmentationNode* segmentationNode, const char* segmentID,
  vtkOrientedImageData* modifierLabelmap, ModificationMode modificationMode, bool bypassMasking/*=false*/)
{
  int modificationExtent[6] = { 0, -1, 0, -1, 0, -1 };
  this->modifySegmentByLabelmap(segmentationNode, segmentID, modifierLabelmap, modificationMode, modificationExtent, bypassMasking);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::modifySegmentByLabelmap(vtkMRMLSegmentationNode* segmentationNode, const char* segmentID,
  vtkOrientedImageData* modifierLabelmapInput, ModificationMode modificationMode, const int modificationExtent[6], bool bypassMasking/*=false*/)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  vtkMRMLSegmentEditorNode* parameterSetNode = this->parameterSetNode();
  if (!parameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    this->defaultModifierLabelmap();
    return;
    }

  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentation";
    this->defaultModifierLabelmap();
    return;
    }

  vtkSegment* segment = nullptr;
  if (segmentID)
    {
    segment = segmentationNode->GetSegmentation()->GetSegment(segmentID);
    }
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment";
    this->defaultModifierLabelmap();
    return;
    }

  if (!modifierLabelmapInput)
    {
    // If per-segment flag is off, then it is not an error (the effect itself has written it back to segmentation)
    if (this->perSegment())
      {
      qCritical() << Q_FUNC_INFO << ": Cannot apply edit operation because modifier labelmap cannot be accessed";
      }
    this->defaultModifierLabelmap();
    return;
    }

  vtkSmartPointer<vtkOrientedImageData> modifierLabelmap = modifierLabelmapInput;
  if ((!bypassMasking && parameterSetNode->GetMaskMode() != vtkMRMLSegmentEditorNode::PaintAllowedEverywhere) ||
    parameterSetNode->GetMasterVolumeIntensityMask())
    {
    vtkNew<vtkOrientedImageData> maskImage;
    maskImage->DeepCopy(modifierLabelmap);
    vtkOrientedImageDataResample::FillImage(maskImage, m_EraseValue);

    // Apply mask to modifier labelmap if masking is enabled
    if (!bypassMasking && parameterSetNode->GetMaskMode() != vtkMRMLSegmentEditorNode::PaintAllowedEverywhere)
      {
      vtkOrientedImageDataResample::ModifyImage(maskImage, this->maskLabelmap(), vtkOrientedImageDataResample::OPERATION_MAXIMUM);
      }

    // Apply threshold mask if paint threshold is turned on
    if (parameterSetNode->GetMasterVolumeIntensityMask())
      {
      vtkOrientedImageData* masterVolumeOrientedImageData = this->masterVolumeImageData();
      if (!masterVolumeOrientedImageData)
        {
        qCritical() << Q_FUNC_INFO << ": Unable to get master volume image";
        this->defaultModifierLabelmap();
        return;
        }
      // Make sure the modifier labelmap has the same geometry as the master volume
      if (!vtkOrientedImageDataResample::DoGeometriesMatch(modifierLabelmap, masterVolumeOrientedImageData))
        {
        qCritical() << Q_FUNC_INFO << ": Modifier labelmap should have the same geometry as the master volume";
        this->defaultModifierLabelmap();
        return;
        }

      // Create threshold image
      vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
      threshold->SetInputData(masterVolumeOrientedImageData);
      threshold->ThresholdBetween(parameterSetNode->GetMasterVolumeIntensityMaskRange()[0], parameterSetNode->GetMasterVolumeIntensityMaskRange()[1]);
      threshold->SetInValue(m_EraseValue);
      threshold->SetOutValue(m_FillValue);
      threshold->SetOutputScalarTypeToUnsignedChar();
      threshold->Update();

      vtkSmartPointer<vtkOrientedImageData> thresholdMask = vtkSmartPointer<vtkOrientedImageData>::New();
      thresholdMask->ShallowCopy(threshold->GetOutput());
      vtkSmartPointer<vtkMatrix4x4> modifierLabelmapToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
      modifierLabelmap->GetImageToWorldMatrix(modifierLabelmapToWorldMatrix);
      thresholdMask->SetGeometryFromImageToWorldMatrix(modifierLabelmapToWorldMatrix);
      vtkOrientedImageDataResample::ModifyImage(maskImage, thresholdMask, vtkOrientedImageDataResample::OPERATION_MAXIMUM);
      }

    vtkNew<vtkOrientedImageData> maskWithCurrentSegment;
    maskWithCurrentSegment->DeepCopy(maskImage);

    bool paintInsideSegments = this->parameterSetNode()->GetMaskMode() == vtkMRMLSegmentationNode::EditAllowedInsideAllSegments ||
      this->parameterSetNode()->GetMaskMode() == vtkMRMLSegmentationNode::EditAllowedInsideVisibleSegments ||
      this->parameterSetNode()->GetMaskMode() == vtkMRMLSegmentationNode::EditAllowedInsideSingleSegment;
    vtkSmartPointer<vtkOrientedImageData> segmentLayerLabelmap =
      vtkOrientedImageData::SafeDownCast(segment->GetRepresentation(segmentationNode->GetSegmentation()->GetMasterRepresentationName()));
    if (segmentLayerLabelmap && paintInsideSegments)
      {
      // If we are painting inside a segment, some effects can modify the current segment outside the masking region (ex. erase effect can add-back regions)
      // Add the current segment to the editable area
      vtkNew<vtkImageThreshold> segmentInverter;
      segmentInverter->SetInputData(segmentLayerLabelmap);
      segmentInverter->SetInValue(m_EraseValue);
      segmentInverter->SetOutValue(m_FillValue);
      segmentInverter->ReplaceInOn();
      segmentInverter->ThresholdBetween(segment->GetLabelValue(), segment->GetLabelValue());
      segmentInverter->SetOutputScalarTypeToUnsignedChar();
      segmentInverter->Update();

      vtkNew<vtkOrientedImageData> invertedSegment;
      invertedSegment->ShallowCopy(segmentInverter->GetOutput());
      invertedSegment->CopyDirections(segmentLayerLabelmap);
      vtkOrientedImageDataResample::ModifyImage(maskWithCurrentSegment, invertedSegment, vtkOrientedImageDataResample::OPERATION_MINIMUM);
      }

    // If we need to the modifier labelmap, make a copy to not modify the input
    vtkOrientedImageDataResample::ApplyImageMask(modifierLabelmap, maskWithCurrentSegment, m_EraseValue, true);

    if (segmentLayerLabelmap && modificationMode == qSlicerSegmentEditorAbstractEffect::ModificationModeSet)
      {
      // If modification mode is "set", we don't want to erase the existing labelmap outside of the mask region,
      // so we need to add it to the modifier labelmap
      vtkNew<vtkImageThreshold> segmentThreshold;
      segmentThreshold->SetInputData(segmentLayerLabelmap);
      segmentThreshold->SetInValue(m_FillValue);
      segmentThreshold->SetOutValue(m_EraseValue);
      segmentThreshold->ReplaceInOn();
      segmentThreshold->ThresholdBetween(segment->GetLabelValue(), segment->GetLabelValue());
      segmentThreshold->SetOutputScalarTypeToUnsignedChar();
      segmentThreshold->Update();

      vtkNew<vtkOrientedImageData> segmentOutsideMask;
      segmentOutsideMask->ShallowCopy(segmentThreshold->GetOutput());
      segmentOutsideMask->CopyDirections(segmentLayerLabelmap);
      vtkOrientedImageDataResample::ModifyImage(segmentOutsideMask, maskImage, vtkOrientedImageDataResample::OPERATION_MINIMUM);
      vtkOrientedImageDataResample::ModifyImage(modifierLabelmap, segmentOutsideMask, vtkOrientedImageDataResample::OPERATION_MAXIMUM);
      }
    }

  // Copy the temporary padded modifier labelmap to the segment.
  // Mask and threshold was already applied on modifier labelmap at this point if requested.
  const int* extent = modificationExtent;
  if (extent[0]>extent[1] || extent[2]>extent[3] || extent[4]>extent[5])
    {
    // invalid extent, it means we have to work with the entire modifier labelmap
    extent = nullptr;
    }

  std::vector<std::string> allSegmentIDs;
  segmentationNode->GetSegmentation()->GetSegmentIDs(allSegmentIDs);
  // remove selected segment, that is already handled
  allSegmentIDs.erase(std::remove(allSegmentIDs.begin(), allSegmentIDs.end(), segmentID), allSegmentIDs.end());

  std::vector<std::string> visibleSegmentIDs;
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  if (displayNode)
    {
    for (std::vector<std::string>::iterator segmentIDIt = allSegmentIDs.begin(); segmentIDIt != allSegmentIDs.end(); ++segmentIDIt)
      {
      if (displayNode->GetSegmentVisibility(*segmentIDIt))
        {
        visibleSegmentIDs.push_back(*segmentIDIt);
        }
      }
    }

  std::vector<std::string> segmentIDsToOverwrite;
  switch (this->parameterSetNode()->GetOverwriteMode())
    {
    case vtkMRMLSegmentEditorNode::OverwriteNone:
      // nothing to overwrite
      break;
    case vtkMRMLSegmentEditorNode::OverwriteVisibleSegments:
      segmentIDsToOverwrite = visibleSegmentIDs;
      break;
    case vtkMRMLSegmentEditorNode::OverwriteAllSegments:
      segmentIDsToOverwrite = allSegmentIDs;
      break;
    }

  if (bypassMasking)
    {
    segmentIDsToOverwrite.clear();
    }

  std::vector<std::string> sharedSegmentIDs;
  segmentationNode->GetSegmentation()->GetSegmentIDsSharingBinaryLabelmapRepresentation(segmentID, sharedSegmentIDs, false);

  std::vector<std::string> segmentsToErase;
  for (std::string segmentIDToOverwrite : segmentIDsToOverwrite)
    {
    std::vector<std::string>::iterator foundSegmentIDIt = std::find(sharedSegmentIDs.begin(), sharedSegmentIDs.end(), segmentIDToOverwrite);
    if (foundSegmentIDIt == sharedSegmentIDs.end())
      {
      segmentsToErase.push_back(segmentIDToOverwrite);
      }
    }

  // Create inverted binary labelmap
  vtkSmartPointer<vtkImageThreshold> inverter = vtkSmartPointer<vtkImageThreshold>::New();
  inverter->SetInputData(modifierLabelmap);
  inverter->SetInValue(VTK_UNSIGNED_CHAR_MAX);
  inverter->SetOutValue(m_EraseValue);
  inverter->ThresholdByLower(0);
  inverter->SetOutputScalarTypeToUnsignedChar();

  if (modificationMode == qSlicerSegmentEditorAbstractEffect::ModificationModeSet)
    {
    vtkSmartPointer<vtkImageThreshold> segmentInverter = vtkSmartPointer<vtkImageThreshold>::New();
    segmentInverter->SetInputData(segment->GetRepresentation(segmentationNode->GetSegmentation()->GetMasterRepresentationName()));
    segmentInverter->SetInValue(m_EraseValue);
    segmentInverter->SetOutValue(VTK_UNSIGNED_CHAR_MAX);
    segmentInverter->ReplaceInOn();
    segmentInverter->ThresholdBetween(segment->GetLabelValue(), segment->GetLabelValue());
    segmentInverter->SetOutputScalarTypeToUnsignedChar();
    segmentInverter->Update();
    vtkNew<vtkOrientedImageData> invertedModifierLabelmap;
    invertedModifierLabelmap->ShallowCopy(segmentInverter->GetOutput());
    vtkNew<vtkMatrix4x4> imageToWorldMatrix;
    modifierLabelmap->GetImageToWorldMatrix(imageToWorldMatrix.GetPointer());
    invertedModifierLabelmap->SetGeometryFromImageToWorldMatrix(imageToWorldMatrix.GetPointer());
    if (!vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(
      invertedModifierLabelmap.GetPointer(), segmentationNode, segmentID, vtkSlicerSegmentationsModuleLogic::MODE_MERGE_MIN,
      nullptr, false, segmentIDsToOverwrite))
      {
      qCritical() << Q_FUNC_INFO << ": Failed to remove modifier labelmap from selected segment";
      }
    if (!vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(
      modifierLabelmap, segmentationNode, segmentID, vtkSlicerSegmentationsModuleLogic::MODE_MERGE_MASK, extent, false, segmentIDsToOverwrite))
      {
      qCritical() << Q_FUNC_INFO << ": Failed to add modifier labelmap to selected segment";
      }
    }
  else if (modificationMode == qSlicerSegmentEditorAbstractEffect::ModificationModeAdd)
    {
    if (!vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(
      modifierLabelmap, segmentationNode, segmentID, vtkSlicerSegmentationsModuleLogic::MODE_MERGE_MASK, extent, false, segmentIDsToOverwrite))
      {
      qCritical() << Q_FUNC_INFO << ": Failed to add modifier labelmap to selected segment";
      }
    }
  else if (modificationMode == qSlicerSegmentEditorAbstractEffect::ModificationModeRemove
    || modificationMode == qSlicerSegmentEditorAbstractEffect::ModificationModeRemoveAll)
    {
    inverter->Update();
    vtkNew<vtkOrientedImageData> invertedModifierLabelmap;
    invertedModifierLabelmap->ShallowCopy(inverter->GetOutput());
    vtkNew<vtkMatrix4x4> imageToWorldMatrix;
    modifierLabelmap->GetImageToWorldMatrix(imageToWorldMatrix.GetPointer());
    invertedModifierLabelmap->SetGeometryFromImageToWorldMatrix(imageToWorldMatrix.GetPointer());
    if (!vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(
      invertedModifierLabelmap.GetPointer(), segmentationNode, segmentID, vtkSlicerSegmentationsModuleLogic::MODE_MERGE_MIN,
      extent, false, segmentIDsToOverwrite))
      {
      qCritical() << Q_FUNC_INFO << ": Failed to remove modifier labelmap from selected segment";
      }
    }

  if (segment)
    {
    if (vtkSlicerSegmentationsModuleLogic::GetSegmentStatus(segment) == vtkSlicerSegmentationsModuleLogic::NotStarted)
      {
      vtkSlicerSegmentationsModuleLogic::SetSegmentStatus(segment, vtkSlicerSegmentationsModuleLogic::InProgress);
      }
    }

  if (!segmentsToErase.empty() &&
     ( modificationMode == qSlicerSegmentEditorAbstractEffect::ModificationModeSet
    || modificationMode == qSlicerSegmentEditorAbstractEffect::ModificationModeAdd
    || modificationMode == qSlicerSegmentEditorAbstractEffect::ModificationModeRemoveAll))
    {
    inverter->Update();
    vtkNew<vtkOrientedImageData> invertedModifierLabelmap;
    invertedModifierLabelmap->ShallowCopy(inverter->GetOutput());
    vtkNew<vtkMatrix4x4> imageToWorldMatrix;
    modifierLabelmap->GetImageToWorldMatrix(imageToWorldMatrix.GetPointer());
    invertedModifierLabelmap->SetGeometryFromImageToWorldMatrix(imageToWorldMatrix.GetPointer());

    std::map<vtkDataObject*, bool> erased;
    for (std::string eraseSegmentID : segmentsToErase)
      {
      vtkSegment* currentSegment = segmentationNode->GetSegmentation()->GetSegment(eraseSegmentID);
      vtkDataObject* dataObject = currentSegment->GetRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
      if (erased[dataObject])
        {
        continue;
        }
      erased[dataObject] = true;

      vtkOrientedImageData* currentLabelmap = vtkOrientedImageData::SafeDownCast(dataObject);

      std::vector<std::string> dontOverwriteIDs;
      std::vector<std::string> currentSharedIDs;
      segmentationNode->GetSegmentation()->GetSegmentIDsSharingBinaryLabelmapRepresentation(eraseSegmentID, currentSharedIDs, true);
      for (std::string sharedSegmentID : currentSharedIDs)
        {
        if (std::find(segmentsToErase.begin(), segmentsToErase.end(), sharedSegmentID) == segmentsToErase.end())
          {
          dontOverwriteIDs.push_back(sharedSegmentID);
          }
        }

      vtkSmartPointer<vtkOrientedImageData> invertedModifierLabelmap2 = invertedModifierLabelmap;
      if (dontOverwriteIDs.size() > 0)
        {
        invertedModifierLabelmap2 = vtkSmartPointer<vtkOrientedImageData>::New();
        invertedModifierLabelmap2->DeepCopy(invertedModifierLabelmap);

        vtkNew<vtkOrientedImageData> maskImage;
        maskImage->CopyDirections(currentLabelmap);
        for (std::string dontOverwriteID : dontOverwriteIDs)
          {
          vtkSegment* dontOverwriteSegment = segmentationNode->GetSegmentation()->GetSegment(dontOverwriteID);
          vtkNew<vtkImageThreshold> threshold;
          threshold->SetInputData(currentLabelmap);
          threshold->ThresholdBetween(dontOverwriteSegment->GetLabelValue(), dontOverwriteSegment->GetLabelValue());
          threshold->SetInValue(1);
          threshold->SetOutValue(0);
          threshold->SetOutputScalarTypeToUnsignedChar();
          threshold->Update();
          maskImage->ShallowCopy(threshold->GetOutput());
          vtkOrientedImageDataResample::ApplyImageMask(invertedModifierLabelmap2, maskImage, VTK_UNSIGNED_CHAR_MAX, true);
          }
        }

      if (!vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(
        invertedModifierLabelmap2, segmentationNode, eraseSegmentID, vtkSlicerSegmentationsModuleLogic::MODE_MERGE_MIN, extent, true, segmentIDsToOverwrite))
        {
        qCritical() << Q_FUNC_INFO << ": Failed to set modifier labelmap to segment " << (eraseSegmentID.c_str());
        }
      }
    }
  else if (modificationMode == qSlicerSegmentEditorAbstractEffect::ModificationModeRemove)
    {
    // In general, we don't try to "add back" areas to other segments when an area is removed from the selected segment.
    // The only exception is when we draw inside one specific segment. In that case erasing adds to the mask segment. It is useful
    // for splitting a segment into two by painting.
    if (this->parameterSetNode()->GetMaskMode() == vtkMRMLSegmentEditorNode::PaintAllowedInsideSingleSegment
      && this->parameterSetNode()->GetMaskSegmentID())
      {
      if (!vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(
        modifierLabelmap, segmentationNode, this->parameterSetNode()->GetMaskSegmentID(), vtkSlicerSegmentationsModuleLogic::MODE_MERGE_MASK,
        extent, false, segmentIDsToOverwrite))
        {
        qCritical() << Q_FUNC_INFO << ": Failed to remove modifier labelmap from segment " << this->parameterSetNode()->GetMaskSegmentID();
        }
      }
    }

  // Make sure the segmentation node is under the same parent as the master volume
  vtkMRMLScalarVolumeNode* masterVolumeNode = d->ParameterSetNode->GetMasterVolumeNode();
  if (masterVolumeNode)
    {
    vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(d->ParameterSetNode->GetScene());
    if (shNode)
      {
      vtkIdType segmentationId = shNode->GetItemByDataNode(segmentationNode);
      vtkIdType masterVolumeShId = shNode->GetItemByDataNode(masterVolumeNode);
      if (segmentationId && masterVolumeShId)
        {
        shNode->SetItemParent(segmentationId, shNode->GetItemParent(masterVolumeShId));
        }
      else
        {
        qCritical() << Q_FUNC_INFO << ": Subject hierarchy items not found for segmentation or master volume";
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::selectEffect(QString effectName)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  emit d->selectEffectSignal(effectName);
}

//-----------------------------------------------------------------------------
QCursor qSlicerSegmentEditorAbstractEffect::createCursor(qMRMLWidget* viewWidget)
{
  Q_UNUSED(viewWidget); // The default cursor is not view-specific, but this method can be overridden

  QImage baseImage(":Icons/CursorBaseArrow.png");
  QIcon effectIcon(this->icon());
  if (effectIcon.isNull())
    {
    QPixmap cursorPixmap = QPixmap::fromImage(baseImage);
    return QCursor(cursorPixmap, baseImage.width()/2, 0);
    }

  QImage effectImage(effectIcon.pixmap(effectIcon.availableSizes()[0]).toImage());
  int width = qMax(baseImage.width(), effectImage.width());
  int pad = -9;
  int height = pad + baseImage.height() + effectImage.height();
  width = height = qMax(width,height);
  int center = width/2;
  QImage cursorImage(width, height, QImage::Format_ARGB32);
  QPainter painter;
  cursorImage.fill(0);
  painter.begin(&cursorImage);
  QPoint point(center - (baseImage.width()/2), 0);
  painter.drawImage(point, baseImage);
  point.setX(center - (effectImage.width()/2));
  point.setY(cursorImage.height() - effectImage.height());
  painter.drawImage(point, effectImage);
  painter.end();

  QPixmap cursorPixmap = QPixmap::fromImage(cursorImage);
  return QCursor(cursorPixmap, center, 0);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::cursorOff(qMRMLWidget* viewWidget)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  d->SavedCursor = QCursor(viewWidget->cursor());
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget && sliceWidget->sliceView())
    {
    sliceWidget->sliceView()->setDefaultViewCursor(QCursor(Qt::BlankCursor));
    }
  else if (threeDWidget && threeDWidget->threeDView())
    {
    threeDWidget->threeDView()->setDefaultViewCursor(QCursor(Qt::BlankCursor));
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::cursorOn(qMRMLWidget* viewWidget)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget && sliceWidget->sliceView())
    {
    sliceWidget->sliceView()->setDefaultViewCursor(d->SavedCursor);
    }
  else if (threeDWidget && threeDWidget->threeDView())
    {
    threeDWidget->threeDView()->setDefaultViewCursor(d->SavedCursor);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::addActor3D(qMRMLWidget* viewWidget, vtkProp3D* actor)
{
  vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(viewWidget);
  if (renderer)
    {
    renderer->AddViewProp(actor);
    this->scheduleRender(viewWidget);
    }
  else
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get renderer for view widget";
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::addActor2D(qMRMLWidget* viewWidget, vtkActor2D* actor)
{
  vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(viewWidget);
  if (renderer)
    {
    renderer->AddActor2D(actor);
    this->scheduleRender(viewWidget);
    }
  else
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get renderer for view widget";
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::removeActor3D(qMRMLWidget* viewWidget, vtkProp3D* actor)
{
  vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(viewWidget);
  if (renderer)
    {
    renderer->RemoveActor(actor);
    this->scheduleRender(viewWidget);
    }
  else
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get renderer for view widget";
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::removeActor2D(qMRMLWidget* viewWidget, vtkActor2D* actor)
{
  vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(viewWidget);
  if (renderer)
    {
    renderer->RemoveActor2D(actor);
    this->scheduleRender(viewWidget);
    }
  else
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get renderer for view widget";
    }
}

//-----------------------------------------------------------------------------
QFrame* qSlicerSegmentEditorAbstractEffect::optionsFrame()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  return d->OptionsFrame;
}

//-----------------------------------------------------------------------------
QFormLayout* qSlicerSegmentEditorAbstractEffect::optionsLayout()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  QFormLayout* formLayout = qobject_cast<QFormLayout*>(d->OptionsFrame->layout());
  return formLayout;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::addOptionsWidget(QWidget* newOptionsWidget)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  newOptionsWidget->setParent(d->OptionsFrame);
  newOptionsWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
  this->optionsLayout()->addRow(newOptionsWidget);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::addOptionsWidget(QLayout* newOptionsWidget)
{
  this->optionsLayout()->addRow(newOptionsWidget);
}

//-----------------------------------------------------------------------------
QWidget* qSlicerSegmentEditorAbstractEffect::addLabeledOptionsWidget(QString label, QWidget* newOptionsWidget)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  QLabel* labelWidget = new QLabel(label);
  newOptionsWidget->setParent(d->OptionsFrame);
  newOptionsWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
  this->optionsLayout()->addRow(labelWidget, newOptionsWidget);
  return labelWidget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerSegmentEditorAbstractEffect::addLabeledOptionsWidget(QString label, QLayout* newOptionsWidget)
{
  QLabel* labelWidget = new QLabel(label);
  if (dynamic_cast<QHBoxLayout*>(newOptionsWidget) == nullptr)
    {
    // for multiline layouts, align label to the top
    labelWidget->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    }
  this->optionsLayout()->addRow(labelWidget, newOptionsWidget);
  return labelWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerSegmentEditorAbstractEffect::scene()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  if (!d->ParameterSetNode)
    {
    return nullptr;
    }

  return d->ParameterSetNode->GetScene();
}

//-----------------------------------------------------------------------------
vtkMRMLSegmentEditorNode* qSlicerSegmentEditorAbstractEffect::parameterSetNode()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  return d->ParameterSetNode.GetPointer();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameterSetNode(vtkMRMLSegmentEditorNode* node)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  d->ParameterSetNode = node;
}

//-----------------------------------------------------------------------------
QString qSlicerSegmentEditorAbstractEffect::parameter(QString name)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
    {
    return QString();
    }

  // Get effect-specific prefixed parameter first
  QString attributeName = QString("%1.%2").arg(this->name()).arg(name);
  const char* value = d->ParameterSetNode->GetAttribute(attributeName.toUtf8().constData());
  // Look for common parameter if effect-specific one is not found
  if (!value)
    {
    value = d->ParameterSetNode->GetAttribute(name.toUtf8().constData());
    }
  if (!value)
    {
    qCritical() << Q_FUNC_INFO << ": Parameter named " << name << " cannot be found for effect " << this->name();
    return QString();
    }

  return QString(value);
}

//-----------------------------------------------------------------------------
int qSlicerSegmentEditorAbstractEffect::integerParameter(QString name)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
    {
    return 0;
    }

  QString parameterStr = this->parameter(name);
  bool ok = false;
  int parameterInt = parameterStr.toInt(&ok);
  if (!ok)
    {
    qCritical() << Q_FUNC_INFO << ": Parameter named " << name << " cannot be converted to integer!";
    return 0;
    }

  return parameterInt;
}

//-----------------------------------------------------------------------------
double qSlicerSegmentEditorAbstractEffect::doubleParameter(QString name)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
    {
    return 0.0;
    }

  QString parameterStr = this->parameter(name);
  bool ok = false;
  double parameterDouble = parameterStr.toDouble(&ok);
  if (!ok)
    {
    qCritical() << Q_FUNC_INFO << ": Parameter named " << name << " cannot be converted to floating point number!";
    return 0.0;
    }

  return parameterDouble;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameter(QString name, QString value)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node set to effect " << this->name();
    return;
    }

  // Set parameter as attribute
  QString attributeName = QString("%1.%2").arg(this->name()).arg(name);
  this->setCommonParameter(attributeName, value);
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::parameterDefined(QString name)
{
  QString attributeName = QString("%1.%2").arg(this->name()).arg(name);
  return this->commonParameterDefined(attributeName);
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::commonParameterDefined(QString name)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
    {
    return false;
    }
  const char* existingValue = d->ParameterSetNode->GetAttribute(name.toUtf8().constData());
  return (existingValue != nullptr && strlen(existingValue) > 0);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameterDefault(QString name, QString value)
{
  if (this->parameterDefined(name))
    {
    return;
    }
  this->setParameter(name, value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonParameter(QString name, QString value)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node set to effect " << this->name();
    return;
    }

  const char* oldValue = d->ParameterSetNode->GetAttribute(name.toUtf8().constData());
  if (oldValue == nullptr && value.isEmpty())
    {
    // no change
    return;
    }
  if (value == QString(oldValue))
    {
    // no change
    return;
    }

  // Disable full modified events in all cases (observe EffectParameterModified instead if necessary)
  int disableState = d->ParameterSetNode->GetDisableModifiedEvent();
  d->ParameterSetNode->SetDisableModifiedEvent(1);

  // Set parameter as attribute
  d->ParameterSetNode->SetAttribute(name.toUtf8().constData(), value.toUtf8().constData());

  // Re-enable full modified events for parameter node
  d->ParameterSetNode->SetDisableModifiedEvent(disableState);

  // Emit parameter modified event if requested
  // Don't pass parameter name as char pointer, as custom modified events may be compressed and invoked after EndModify()
  // and by that time the pointer may not be valid anymore.
  d->ParameterSetNode->InvokeCustomModifiedEvent(vtkMRMLSegmentEditorNode::EffectParameterModified);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonParameterDefault(QString name, QString value)
{
  if (this->commonParameterDefined(name))
    {
    return;
    }
  this->setCommonParameter(name, value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameter(QString name, int value)
{
  this->setParameter(name, QString::number(value));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameterDefault(QString name, int value)
{
  if (this->parameterDefined(name))
    {
    return;
    }
  this->setParameter(name, value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonParameter(QString name, int value)
{
  this->setCommonParameter(name, QString::number(value));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonParameterDefault(QString name, int value)
{
  if (this->commonParameterDefined(name))
    {
    return;
    }
  this->setCommonParameter(name, value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameter(QString name, double value)
{
  this->setParameter(name, QString::number(value));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameterDefault(QString name, double value)
{
  if (this->parameterDefined(name))
    {
    return;
    }
  this->setParameter(name, value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonParameter(QString name, double value)
{
  this->setCommonParameter(name, QString::number(value));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonParameterDefault(QString name, double value)
{
  if (this->commonParameterDefined(name))
    {
    return;
    }
  this->setCommonParameter(name, value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setVolumes(vtkOrientedImageData* alignedMasterVolume,
  vtkOrientedImageData* modifierLabelmap, vtkOrientedImageData* maskLabelmap,
  vtkOrientedImageData* selectedSegmentLabelmap, vtkOrientedImageData* referenceGeometryImage)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  d->ModifierLabelmap = modifierLabelmap;
  d->MaskLabelmap = maskLabelmap;
  d->AlignedMasterVolume = alignedMasterVolume;
  d->SelectedSegmentLabelmap = selectedSegmentLabelmap;
  d->ReferenceGeometryImage = referenceGeometryImage;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::defaultModifierLabelmap()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  bool success = false;
  emit d->updateVolumeSignal(d->ModifierLabelmap.GetPointer(), success); // this resets the labelmap and cleares it
  if (!success)
    {
    return nullptr;
    }
  return d->ModifierLabelmap;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::modifierLabelmap()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  return d->ModifierLabelmap;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::maskLabelmap()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  bool success = false;
  emit d->updateVolumeSignal(d->MaskLabelmap.GetPointer(), success);
  if (!success)
    {
    return nullptr;
    }
  return d->MaskLabelmap;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::masterVolumeImageData()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  bool success = false;
  emit d->updateVolumeSignal(d->AlignedMasterVolume.GetPointer(), success);
  if (!success)
    {
    return nullptr;
    }
  return d->AlignedMasterVolume;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::selectedSegmentLabelmap()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  bool success = false;
  emit d->updateVolumeSignal(d->SelectedSegmentLabelmap.GetPointer(), success);
  if (!success)
    {
    return nullptr;
    }
  return d->SelectedSegmentLabelmap;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::referenceGeometryImage()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  bool success = false;
  emit d->updateVolumeSignal(d->ReferenceGeometryImage.GetPointer(), success); // this resets the labelmap and clears it
  if (!success)
    {
    return nullptr;
    }
  return d->ReferenceGeometryImage;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::saveStateForUndo()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  emit d->saveStateForUndoSignal();
}

//-----------------------------------------------------------------------------
vtkRenderWindow* qSlicerSegmentEditorAbstractEffect::renderWindow(qMRMLWidget* viewWidget)
{
  if (!viewWidget)
    {
    return nullptr;
    }

  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget)
    {
    if (!sliceWidget->sliceView())
      {
      // probably the application is closing
      return nullptr;
      }
    return sliceWidget->sliceView()->renderWindow();
    }
  else if (threeDWidget)
    {
    if (!threeDWidget->threeDView())
      {
      // probably the application is closing
      return nullptr;
      }
      return threeDWidget->threeDView()->renderWindow();
    }

  qCritical() << Q_FUNC_INFO << ": Unsupported view widget type!";
  return nullptr;
}

//-----------------------------------------------------------------------------
vtkRenderer* qSlicerSegmentEditorAbstractEffect::renderer(qMRMLWidget* viewWidget)
{
  vtkRenderWindow* renderWindow = qSlicerSegmentEditorAbstractEffect::renderWindow(viewWidget);
  if (!renderWindow)
    {
    return nullptr;
    }

  return vtkRenderer::SafeDownCast(renderWindow->GetRenderers()->GetItemAsObject(0));
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractViewNode* qSlicerSegmentEditorAbstractEffect::viewNode(qMRMLWidget* viewWidget)
{
  if (!viewWidget)
    {
    return nullptr;
    }

  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget)
    {
    return sliceWidget->sliceLogic()->GetSliceNode();
    }
  else if (threeDWidget)
    {
    return threeDWidget->mrmlViewNode();
    }

  qCritical() << Q_FUNC_INFO << ": Unsupported view widget type!";
  return nullptr;
}

//-----------------------------------------------------------------------------
QPoint qSlicerSegmentEditorAbstractEffect::rasToXy(double ras[3], qMRMLSliceWidget* sliceWidget)
{
  QPoint xy(0,0);

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(
    qSlicerSegmentEditorAbstractEffect::viewNode(sliceWidget) );
  if (!sliceNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get slice node!";
    return xy;
    }

  double rast[4] = {ras[0], ras[1], ras[2], 1.0};
  double xyzw[4] = {0.0, 0.0, 0.0, 1.0};
  vtkSmartPointer<vtkMatrix4x4> rasToXyMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  rasToXyMatrix->DeepCopy(sliceNode->GetXYToRAS());
  rasToXyMatrix->Invert();
  rasToXyMatrix->MultiplyPoint(rast, xyzw);

  xy.setX(xyzw[0]);
  xy.setY(xyzw[1]);
  return xy;
}

//-----------------------------------------------------------------------------
QPoint qSlicerSegmentEditorAbstractEffect::rasToXy(QVector3D rasVector, qMRMLSliceWidget* sliceWidget)
{
  double ras[3] = {rasVector.x(), rasVector.y(), rasVector.z()};
  return qSlicerSegmentEditorAbstractEffect::rasToXy(ras, sliceWidget);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::xyzToRas(double inputXyz[3], double outputRas[3], qMRMLSliceWidget* sliceWidget)
{
  outputRas[0] = outputRas[1] = outputRas[2] = 0.0;

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(
    qSlicerSegmentEditorAbstractEffect::viewNode(sliceWidget) );
  if (!sliceNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get slice node!";
    return;
    }

  // x,y uses slice (canvas) coordinate system and actually has a 3rd z component (index into the
  // slice you're looking at), hence xyToRAS is really performing xyzToRAS. RAS is patient world
  // coordinate system. Note the 1 is because the transform uses homogeneous coordinates.
  double xyzw[4] = {inputXyz[0], inputXyz[1], inputXyz[2], 1.0};
  double rast[4] = {0.0, 0.0, 0.0, 1.0};
  sliceNode->GetXYToRAS()->MultiplyPoint(xyzw, rast);
  outputRas[0] = rast[0];
  outputRas[1] = rast[1];
  outputRas[2] = rast[2];
}

//-----------------------------------------------------------------------------
QVector3D qSlicerSegmentEditorAbstractEffect::xyzToRas(QVector3D inputXyzVector, qMRMLSliceWidget* sliceWidget)
{
  double inputXyz[3] = {inputXyzVector.x(), inputXyzVector.y(), inputXyzVector.z()};
  double outputRas[3] = {0.0, 0.0, 0.0};
  qSlicerSegmentEditorAbstractEffect::xyzToRas(inputXyz, outputRas, sliceWidget);
  QVector3D outputVector(outputRas[0], outputRas[1], outputRas[2]);
  return outputVector;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::xyToRas(QPoint xy, double outputRas[3], qMRMLSliceWidget* sliceWidget)
{
  double xyz[3] = {
    static_cast<double>(xy.x()),
    static_cast<double>(xy.y()),
    0.0};

  qSlicerSegmentEditorAbstractEffect::xyzToRas(xyz, outputRas, sliceWidget);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::xyToRas(double xy[2], double outputRas[3], qMRMLSliceWidget* sliceWidget)
{
  double xyz[3] = {xy[0], xy[1], 0.0};
  qSlicerSegmentEditorAbstractEffect::xyzToRas(xyz, outputRas, sliceWidget);
}

//-----------------------------------------------------------------------------
QVector3D qSlicerSegmentEditorAbstractEffect::xyToRas(QPoint xy, qMRMLSliceWidget* sliceWidget)
{
  double outputRas[3] = {0.0, 0.0, 0.0};
  qSlicerSegmentEditorAbstractEffect::xyToRas(xy, outputRas, sliceWidget);
  QVector3D outputVector(outputRas[0], outputRas[1], outputRas[2]);
  return outputVector;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::xyzToIjk(double inputXyz[3], int outputIjk[3], qMRMLSliceWidget* sliceWidget, vtkOrientedImageData* image, vtkMRMLTransformNode* parentTransformNode/*=nullptr*/)
{
  outputIjk[0] = outputIjk[1] = outputIjk[2] = 0;

  if (!sliceWidget || !image)
    {
    return;
    }

  // Convert from XY to RAS first
  double ras[3] = {0.0, 0.0, 0.0};
  qSlicerSegmentEditorAbstractEffect::xyzToRas(inputXyz, ras, sliceWidget);

  // Move point from world to same transform as image
  if (parentTransformNode)
    {
    if (parentTransformNode->IsTransformToWorldLinear())
      {
      vtkNew<vtkMatrix4x4> worldToParentTransform;
      parentTransformNode->GetMatrixTransformFromWorld(worldToParentTransform);
      double worldPos[4] = { ras[0], ras[1], ras[2], 1.0 };
      double parentPos[4] = { 0.0 };
      worldToParentTransform->MultiplyPoint(worldPos, parentPos);
      ras[0] = parentPos[0];
      ras[1] = parentPos[1];
      ras[2] = parentPos[2];
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Parent transform is non-linear, which cannot be handled! Skipping.";
      }
    }

  // Convert RAS to image IJK
  double rast[4] = {ras[0], ras[1], ras[2], 1.0};
  double ijkl[4] = {0.0, 0.0, 0.0, 1.0};
  vtkSmartPointer<vtkMatrix4x4> rasToIjkMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  image->GetImageToWorldMatrix(rasToIjkMatrix);
  rasToIjkMatrix->Invert();
  rasToIjkMatrix->MultiplyPoint(rast, ijkl);

  outputIjk[0] = (int)(ijkl[0] + 0.5);
  outputIjk[1] = (int)(ijkl[1] + 0.5);
  outputIjk[2] = (int)(ijkl[2] + 0.5);
}

//-----------------------------------------------------------------------------
QVector3D qSlicerSegmentEditorAbstractEffect::xyzToIjk(QVector3D inputXyzVector, qMRMLSliceWidget* sliceWidget, vtkOrientedImageData* image, vtkMRMLTransformNode* parentTransformNode/*=nullptr*/)
{
  double inputXyz[3] = {inputXyzVector.x(), inputXyzVector.y(), inputXyzVector.z()};
  int outputIjk[3] = {0, 0, 0};
  qSlicerSegmentEditorAbstractEffect::xyzToIjk(inputXyz, outputIjk, sliceWidget, image, parentTransformNode);

  QVector3D outputVector(outputIjk[0], outputIjk[1], outputIjk[2]);
  return outputVector;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::xyToIjk(QPoint xy, int outputIjk[3], qMRMLSliceWidget* sliceWidget, vtkOrientedImageData* image, vtkMRMLTransformNode* parentTransformNode/*=nullptr*/)
{
  double xyz[3] = {
    static_cast<double>(xy.x()),
    static_cast<double>(xy.y()),
    0.0};
  qSlicerSegmentEditorAbstractEffect::xyzToIjk(xyz, outputIjk, sliceWidget, image, parentTransformNode);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::xyToIjk(double xy[2], int outputIjk[3], qMRMLSliceWidget* sliceWidget, vtkOrientedImageData* image, vtkMRMLTransformNode* parentTransformNode/*=nullptr*/)
{
  double xyz[3] = {xy[0], xy[0], 0.0};
  qSlicerSegmentEditorAbstractEffect::xyzToIjk(xyz, outputIjk, sliceWidget, image, parentTransformNode);
}

//-----------------------------------------------------------------------------
QVector3D qSlicerSegmentEditorAbstractEffect::xyToIjk(QPoint xy, qMRMLSliceWidget* sliceWidget, vtkOrientedImageData* image, vtkMRMLTransformNode* parentTransformNode/*=nullptr*/)
{
  int outputIjk[3] = {0, 0, 0};
  qSlicerSegmentEditorAbstractEffect::xyToIjk(xy, outputIjk, sliceWidget, image, parentTransformNode);
  QVector3D outputVector(outputIjk[0], outputIjk[1], outputIjk[2]);
  return outputVector;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::forceRender(qMRMLWidget* viewWidget)
{
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget)
    {
    sliceWidget->sliceView()->forceRender();
    }
  if (threeDWidget)
    {
    threeDWidget->threeDView()->forceRender();
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::scheduleRender(qMRMLWidget* viewWidget)
{
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget)
    {
    sliceWidget->sliceView()->scheduleRender();
    }
  if (threeDWidget)
    {
    threeDWidget->threeDView()->scheduleRender();
    }
}

//----------------------------------------------------------------------------
double qSlicerSegmentEditorAbstractEffect::sliceSpacing(qMRMLSliceWidget* sliceWidget)
{
  // Implementation copied from vtkMRMLSliceViewInteractorStyle::GetSliceSpacing()
  vtkMRMLSliceNode *sliceNode = sliceWidget->sliceLogic()->GetSliceNode();
  double spacing = 1.0;
  if (sliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::PrescribedSliceSpacingMode)
    {
    spacing = sliceNode->GetPrescribedSliceSpacing()[2];
    }
  else
    {
    spacing = sliceWidget->sliceLogic()->GetLowestVolumeSliceSpacing()[2];
    }
  return spacing;
}

//----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::showEffectCursorInSliceView()
{
  return m_ShowEffectCursorInSliceView;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setShowEffectCursorInSliceView(bool show)
{
  this->m_ShowEffectCursorInSliceView = show;
}

//----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::showEffectCursorInThreeDView()
{
  return m_ShowEffectCursorInThreeDView;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setShowEffectCursorInThreeDView(bool show)
{
  this->m_ShowEffectCursorInThreeDView = show;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::interactionNodeModified(vtkMRMLInteractionNode* interactionNode)
{
  if (interactionNode == nullptr)
    {
    return;
    }
  // Deactivate the effect if user switched to markup placement mode
  // to avoid double effect (e.g., paint & place fiducial at the same time)
  if (interactionNode->GetCurrentInteractionMode() != vtkMRMLInteractionNode::ViewTransform)
    {
    this->selectEffect("");
    }
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::segmentationDisplayableInView(vtkMRMLAbstractViewNode* viewNode)
{
  if (!viewNode)
    {
    qWarning() << Q_FUNC_INFO << ": failed. Invalid viewNode.";
    return false;
    }

  vtkMRMLSegmentEditorNode* parameterSetNode = this->parameterSetNode();
  if (!parameterSetNode)
    {
    return false;
    }

  vtkMRMLSegmentationNode* segmentationNode = parameterSetNode->GetSegmentationNode();
  if (!segmentationNode)
    {
    return false;
    }
  const char* viewNodeID = viewNode->GetID();
  int numberOfDisplayNodes = segmentationNode->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
    {
    vtkMRMLDisplayNode* segmentationDisplayNode = segmentationNode->GetNthDisplayNode(displayNodeIndex);
    if (segmentationDisplayNode->IsDisplayableInView(viewNodeID))
      {
      return true;
      }
    }
  return false;
}
