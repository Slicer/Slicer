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
 , d_ptr( new qSlicerSegmentEditorAbstractLabelEffectPrivate(*this) )
{
}

//----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractLabelEffect::~qSlicerSegmentEditorAbstractLabelEffect() = default;

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::referenceGeometryChanged()
{
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::masterVolumeNodeChanged()
{
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::setupOptionsFrame()
{
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::setMRMLDefaults()
{
}

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
void qSlicerSegmentEditorAbstractLabelEffect::updateMRMLFromGUI()
{
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::appendPolyMask(vtkOrientedImageData* input, vtkPolyData* polyData, qMRMLSliceWidget* sliceWidget, vtkMRMLSegmentationNode* segmentationNode/*=nullptr*/)
{
  // Rasterize a poly data onto the input image into the slice view
  // - Points are specified in current XY space
  vtkSmartPointer<vtkOrientedImageData> polyMaskImage = vtkSmartPointer<vtkOrientedImageData>::New();
  qSlicerSegmentEditorAbstractLabelEffect::createMaskImageFromPolyData(polyData, polyMaskImage, sliceWidget);

  if (segmentationNode && segmentationNode->GetParentTransformNode())
    {
    if (segmentationNode->GetParentTransformNode()->IsTransformToWorldLinear())
      {
      vtkNew<vtkMatrix4x4> worldToSegmentation;
      segmentationNode->GetParentTransformNode()->GetMatrixTransformFromWorld(worldToSegmentation);
      vtkNew<vtkMatrix4x4> imageToWorldMatrix;
      polyMaskImage->GetImageToWorldMatrix(imageToWorldMatrix);
      vtkNew<vtkMatrix4x4> imageToSegmentation;
      vtkMatrix4x4::Multiply4x4(worldToSegmentation, imageToWorldMatrix, imageToSegmentation);
      polyMaskImage->SetImageToWorldMatrix(imageToSegmentation);
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Parent transform is non-linear, which cannot be handled! Skipping.";
      }
    }

  // Append poly mask onto input image
  qSlicerSegmentEditorAbstractLabelEffect::appendImage(input, polyMaskImage);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::appendImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* appendedImage)
{
  if (!inputImage || !appendedImage)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid inputs!";
    return;
    }

  // Make sure appended image has the same lattice as the input image
  vtkSmartPointer<vtkOrientedImageData> resampledAppendedImage = vtkSmartPointer<vtkOrientedImageData>::New();
  vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(
    appendedImage, inputImage, resampledAppendedImage);

  // Add image created from poly data to input image
  vtkSmartPointer<vtkImageMathematics> imageMath = vtkSmartPointer<vtkImageMathematics>::New();
  imageMath->SetInput1Data(inputImage);
  imageMath->SetInput2Data(resampledAppendedImage);
  imageMath->SetOperationToMax();
  imageMath->Update();
  inputImage->DeepCopy(imageMath->GetOutput());
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::createMaskImageFromPolyData(vtkPolyData* polyData, vtkOrientedImageData* outputMask, qMRMLSliceWidget* sliceWidget)
{
  if (!polyData || !outputMask)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid inputs!";
    return;
    }
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(
    qSlicerSegmentEditorAbstractEffect::viewNode(sliceWidget) );
  if (!sliceNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get slice node!";
    return;
    }

  // Need to know the mapping from RAS into polygon space
  // so the painter can use this as a mask
  // - Need the bounds in RAS space
  // - Need to get an IJKToRAS for just the mask area
  // - Directions are the XYToRAS for this slice
  // - Origin is the lower left of the polygon bounds
  // - TODO: need to account for the boundary pixels
  //
  // Note: uses the slicer2-based vtkImageFillROI filter
  vtkSmartPointer<vtkMatrix4x4> maskIjkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  maskIjkToRasMatrix->DeepCopy(sliceNode->GetXYToRAS());

  polyData->GetPoints()->Modified();
  double bounds[6] = {0,0,0,0,0,0};
  polyData->GetBounds(bounds);

  double xlo = bounds[0] - 1.0;
  double xhi = bounds[1];
  double ylo = bounds[2] - 1.0;
  double yhi = bounds[3];

  double originXYZ[3] = {xlo, ylo, 0.0};
  double originRAS[3] = {0.0,0.0,0.0};
  qSlicerSegmentEditorAbstractEffect::xyzToRas(originXYZ, originRAS, sliceWidget);

  maskIjkToRasMatrix->SetElement(0, 3, originRAS[0]);
  maskIjkToRasMatrix->SetElement(1, 3, originRAS[1]);
  maskIjkToRasMatrix->SetElement(2, 3, originRAS[2]);

  // Get a good size for the draw buffer
  // - Needs to include the full region of the polygon
  // - Plus a little extra
  //
  // Round to int and add extra pixel for both sides
  // TODO: figure out why we need to add buffer pixels on each
  //   side for the width in order to end up with a single extra
  //   pixel in the rasterized image map.  Probably has to
  //   do with how boundary conditions are handled in the filler
  int w = (int)(xhi - xlo) + 32;
  int h = (int)(yhi - ylo) + 32;

  vtkSmartPointer<vtkOrientedImageData> imageData = vtkSmartPointer<vtkOrientedImageData>::New();
  imageData->SetDimensions(w, h, 1);
  imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

  // Move the points so the lower left corner of the bounding box is at 1, 1 (to avoid clipping)
  vtkSmartPointer<vtkTransform> translate = vtkSmartPointer<vtkTransform>::New();
  translate->Translate(-xlo, -ylo, 0.0);

  vtkSmartPointer<vtkPoints> drawPoints = vtkSmartPointer<vtkPoints>::New();
  drawPoints->Reset();
  translate->TransformPoints(polyData->GetPoints(), drawPoints);
  drawPoints->Modified();

  vtkSmartPointer<vtkImageFillROI> fill = vtkSmartPointer<vtkImageFillROI>::New();
  fill->SetInputData(imageData);
  fill->SetValue(1);
  fill->SetPoints(drawPoints);
  fill->Update();

  outputMask->DeepCopy(fill->GetOutput());
  outputMask->SetGeometryFromImageToWorldMatrix(maskIjkToRasMatrix);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::imageToWorldMatrix(vtkMRMLVolumeNode* node, vtkMatrix4x4* ijkToRas)
{
  if (!node || !ijkToRas)
    {
    return;
    }

  node->GetIJKToRASMatrix(ijkToRas);

  vtkMRMLTransformNode* transformNode = node->GetParentTransformNode();
  if (transformNode)
    {
    if (transformNode->IsTransformToWorldLinear())
      {
      vtkSmartPointer<vtkMatrix4x4> volumeRasToWorldRas = vtkSmartPointer<vtkMatrix4x4>::New();
      transformNode->GetMatrixTransformToWorld(volumeRasToWorldRas);
      vtkMatrix4x4::Multiply4x4(volumeRasToWorldRas, ijkToRas, ijkToRas);
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Parent transform is non-linear, which cannot be handled! Skipping.";
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractLabelEffect::imageToWorldMatrix(vtkOrientedImageData* image, vtkMRMLSegmentationNode* node, vtkMatrix4x4* ijkToRas)
{
  if (!image || !node || !ijkToRas)
    {
    return;
    }

  image->GetImageToWorldMatrix(ijkToRas);

  vtkMRMLTransformNode* transformNode = node->GetParentTransformNode();
  if (transformNode)
    {
    if (transformNode->IsTransformToWorldLinear())
      {
      vtkSmartPointer<vtkMatrix4x4> segmentationRasToWorldRas = vtkSmartPointer<vtkMatrix4x4>::New();
      transformNode->GetMatrixTransformToWorld(segmentationRasToWorldRas);
      vtkMatrix4x4::Multiply4x4(segmentationRasToWorldRas, ijkToRas, ijkToRas);
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Parent transform is non-linear, which cannot be handled! Skipping.";
      }
    }
}
