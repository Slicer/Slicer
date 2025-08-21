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
#include "qSlicerSegmentEditorAbstractLabelEffect.h"
#include "qSlicerSegmentEditorAbstractLabelEffect_p.h"

#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentEditorNode.h"
#include "vtkSlicerSegmentationsModuleLogic.h"

// Qt includes
#include <QDebug>
#include <QCheckBox>
#include <QLabel>

// CTK includes
#include "ctkRangeWidget.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkImageConstantPad.h>
#include <vtkImageMask.h>
#include <vtkImageThreshold.h>
#include <vtkPolyData.h>
#include <vtkImageMathematics.h>

// Slicer includes
#include "qMRMLSliceWidget.h"
#include "vtkImageFillROI.h"

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkSegmentEditorLogic.h"

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorAbstractLabelEffectPrivate methods

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractLabelEffectPrivate::qSlicerSegmentEditorAbstractLabelEffectPrivate(qSlicerSegmentEditorAbstractLabelEffect& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractLabelEffectPrivate::~qSlicerSegmentEditorAbstractLabelEffectPrivate() = default;

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorAbstractLabelEffect methods

//----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractLabelEffect::qSlicerSegmentEditorAbstractLabelEffect(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerSegmentEditorAbstractLabelEffectPrivate(*this))
{
}

//----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractLabelEffect::~qSlicerSegmentEditorAbstractLabelEffect() = default;

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::referenceGeometryChanged() {}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::sourceVolumeNodeChanged() {}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::setupOptionsFrame() {}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::setMRMLDefaults() {}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::updateGUIFromMRML()
{
  if (!this->active())
  {
    // updateGUIFromMRML is called when the effect is activated
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::updateMRMLFromGUI() {}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::appendPolyMask(vtkOrientedImageData* input,
                                                             vtkPolyData* polyData,
                                                             qMRMLSliceWidget* sliceWidget,
                                                             vtkMRMLSegmentationNode* segmentationNode /*=nullptr*/)
{
  vtkSegmentEditorLogic::AppendPolyMask(input, polyData, qSlicerSegmentEditorAbstractEffect::sliceNode(sliceWidget), segmentationNode);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::appendImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* appendedImage)
{
  vtkSegmentEditorLogic::AppendImage(inputImage, appendedImage);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::createMaskImageFromPolyData(vtkPolyData* polyData, vtkOrientedImageData* outputMask, qMRMLSliceWidget* sliceWidget)
{
  vtkSegmentEditorLogic::CreateMaskImageFromPolyData(polyData, outputMask, qSlicerSegmentEditorAbstractEffect::sliceNode(sliceWidget));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::imageToWorldMatrix(vtkMRMLVolumeNode* node, vtkMatrix4x4* ijkToRas)
{
  vtkSegmentEditorLogic::ImageToWorldMatrix(node, ijkToRas);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::imageToWorldMatrix(vtkOrientedImageData* image, vtkMRMLSegmentationNode* node, vtkMatrix4x4* ijkToRas)
{
  vtkSegmentEditorLogic::ImageToWorldMatrix(image, node, ijkToRas);
}
