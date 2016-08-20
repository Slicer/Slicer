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
#include "qSlicerSegmentEditorPaintEffect.h"
#include "qSlicerSegmentEditorPaintEffect_p.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentEditorNode.h"
#include "vtkOrientedImageData.h"

// Qt includes
#include <QDebug>
#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

// VTK includes
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkBoundingBox.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellPicker.h>
#include <vtkCollection.h>
#include <vtkCommand.h>
#include <vtkGlyph2D.h>
#include <vtkGlyph3D.h>
#include <vtkIdList.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageStencil.h>
#include <vtkImageStencilData.h>
#include <vtkImageStencilToImage.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>
#include <vtkPropPicker.h>
#include <vtkRegularPolygonSource.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWorldPointPicker.h>
// CTK includes
#include "ctkDoubleSlider.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>

// Slicer includes
#include "qMRMLSliceView.h"
#include "qMRMLSliceWidget.h"
#include "qMRMLSpinBox.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerApplication.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceLayerLogic.h"
#include "vtkOrientedImageDataResample.h"

//-----------------------------------------------------------------------------
/// Visualization objects and pipeline for each slice view for the paint brush
class BrushPipeline
{
public:
  BrushPipeline()
    {
    this->WorldToSliceTransform = vtkSmartPointer<vtkTransform>::New();
    this->SlicePlane = vtkSmartPointer<vtkPlane>::New();
    };
  virtual ~BrushPipeline()
    {
    };
  virtual void SetBrushVisibility(bool visibility) = 0;
  virtual void SetFeedbackVisibility(bool visibility) = 0;

  vtkSmartPointer<vtkTransform> WorldToSliceTransform;
  vtkSmartPointer<vtkPlane> SlicePlane;
};

class BrushPipeline2D : public BrushPipeline
{
public:
  BrushPipeline2D()
    {
    this->BrushCutter = vtkSmartPointer<vtkCutter>::New();
    this->BrushCutter->SetCutFunction(this->SlicePlane);
    this->BrushCutter->SetGenerateCutScalars(0);

    this->BrushWorldToSliceTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    this->BrushWorldToSliceTransformer->SetTransform(this->WorldToSliceTransform);
    this->BrushWorldToSliceTransformer->SetInputConnection(this->BrushCutter->GetOutputPort());

    this->BrushMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->BrushMapper->SetInputConnection(this->BrushWorldToSliceTransformer->GetOutputPort());

    this->BrushActor = vtkSmartPointer<vtkActor2D>::New();
    this->BrushActor->SetMapper(this->BrushMapper);
    this->BrushActor->VisibilityOff();

    this->FeedbackCutter = vtkSmartPointer<vtkCutter>::New();
    this->FeedbackCutter->SetCutFunction(this->SlicePlane);
    this->FeedbackCutter->SetGenerateCutScalars(0);

    this->FeedbackWorldToSliceTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    this->FeedbackWorldToSliceTransformer->SetTransform(this->WorldToSliceTransform);
    this->FeedbackWorldToSliceTransformer->SetInputConnection(this->FeedbackCutter->GetOutputPort());

    this->FeedbackMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->FeedbackMapper->SetInputConnection(this->FeedbackWorldToSliceTransformer->GetOutputPort());
    this->FeedbackActor = vtkSmartPointer<vtkActor2D>::New();
    vtkProperty2D* feedbackActorProperty = this->FeedbackActor->GetProperty();
    feedbackActorProperty->SetColor(0.7, 0.7, 0.0);
    feedbackActorProperty->SetOpacity(0.5);
    this->FeedbackActor->SetMapper(this->FeedbackMapper);
    this->FeedbackActor->VisibilityOff();
    };
  ~BrushPipeline2D()
    {
    };

  void SetBrushVisibility(bool visibility)
    {
    this->BrushActor->SetVisibility(visibility);
    };
  void SetFeedbackVisibility(bool visibility)
    {
    this->FeedbackActor->SetVisibility(visibility);
    };

  vtkSmartPointer<vtkActor2D> BrushActor;
  vtkSmartPointer<vtkPolyDataMapper2D> BrushMapper;
  vtkSmartPointer<vtkActor2D> FeedbackActor;
  vtkSmartPointer<vtkPolyDataMapper2D> FeedbackMapper;
  vtkSmartPointer<vtkCutter> BrushCutter;
  vtkSmartPointer<vtkTransformPolyDataFilter> BrushWorldToSliceTransformer;
  vtkSmartPointer<vtkCutter> FeedbackCutter;
  vtkSmartPointer<vtkTransformPolyDataFilter> FeedbackWorldToSliceTransformer;
};

class BrushPipeline3D : public BrushPipeline
{
public:
  BrushPipeline3D()
    {
    this->BrushMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    this->BrushActor = vtkSmartPointer<vtkActor>::New();
    this->BrushActor->SetMapper(this->BrushMapper);
    this->BrushActor->VisibilityOff();
    this->BrushActor->PickableOff(); // otherwise picking in 3D view would not work

    this->FeedbackMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    this->FeedbackActor = vtkSmartPointer<vtkActor>::New();
    this->FeedbackActor->VisibilityOff();
    this->FeedbackActor->PickableOff(); // otherwise picking in 3D view would not work
    vtkProperty* feedbackActorProperty = this->FeedbackActor->GetProperty();
    feedbackActorProperty->SetColor(0.7, 0.7, 0.0);
    this->FeedbackActor->SetMapper(this->FeedbackMapper);
    this->FeedbackActor->VisibilityOff();
    };
  ~BrushPipeline3D()
    {
    };
  void SetBrushVisibility(bool visibility)
    {
    this->BrushActor->SetVisibility(visibility);
    };
  void SetFeedbackVisibility(bool visibility)
    {
    this->FeedbackActor->SetVisibility(visibility);
    };

  vtkSmartPointer<vtkPolyDataMapper> BrushMapper;
  vtkSmartPointer<vtkActor> BrushActor;
  vtkSmartPointer<vtkActor> FeedbackActor;
  vtkSmartPointer<vtkPolyDataMapper> FeedbackMapper;
};


//-----------------------------------------------------------------------------
// qSlicerSegmentEditorPaintEffectPrivate methods

//-----------------------------------------------------------------------------
qSlicerSegmentEditorPaintEffectPrivate::qSlicerSegmentEditorPaintEffectPrivate(qSlicerSegmentEditorPaintEffect& object)
  : q_ptr(&object)
  , DelayedPaint(true)
  , IsPainting(false)
  , BrushRadiusFrame(NULL)
  , BrushRadiusSpinBox(NULL)
  , BrushRadiusSlider(NULL)
  , BrushRadiusUnitsToggle(NULL)
  , BrushSphereCheckbox(NULL)
  , ColorSmudgeCheckbox(NULL)
  , BrushPixelModeCheckbox(NULL)
{
  this->PaintCoordinates_World = vtkSmartPointer<vtkPoints>::New();
  this->FeedbackPointsPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->FeedbackPointsPolyData->SetPoints(this->PaintCoordinates_World);

  this->PaintIcon = QIcon(":Icons/Paint.png");

  this->BrushCylinderSource = vtkSmartPointer<vtkCylinderSource>::New();
  this->BrushSphereSource = vtkSmartPointer<vtkSphereSource>::New();
  this->BrushToWorldOriginTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->BrushToWorldOriginTransform = vtkSmartPointer<vtkTransform>::New();
  this->BrushToWorldOriginTransformer->SetTransform(this->BrushToWorldOriginTransform);
  this->BrushPolyDataNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
  this->BrushPolyDataNormals->SetInputConnection(this->BrushToWorldOriginTransformer->GetOutputPort());
  this->BrushPolyDataNormals->AutoOrientNormalsOn();

  this->WorldOriginToWorldTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->WorldOriginToWorldTransform = vtkSmartPointer<vtkTransform>::New();
  this->WorldOriginToWorldTransformer->SetTransform(this->WorldOriginToWorldTransform);
  this->WorldOriginToWorldTransformer->SetInputConnection(this->BrushPolyDataNormals->GetOutputPort());

  this->WorldOriginToModifierLabelmapIjkTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->WorldOriginToModifierLabelmapIjkTransform = vtkSmartPointer<vtkTransform>::New();
  this->WorldOriginToModifierLabelmapIjkTransformer->SetTransform(this->WorldOriginToModifierLabelmapIjkTransform);
  this->WorldOriginToModifierLabelmapIjkTransformer->SetInputConnection(this->BrushPolyDataNormals->GetOutputPort());
  this->BrushPolyDataToStencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
  this->BrushPolyDataToStencil->SetOutputSpacing(1.0,1.0,1.0);
  this->BrushPolyDataToStencil->SetInputConnection(this->WorldOriginToModifierLabelmapIjkTransformer->GetOutputPort());

  this->FeedbackGlyphFilter = vtkSmartPointer<vtkGlyph3D>::New();
  this->FeedbackGlyphFilter->SetInputData(this->FeedbackPointsPolyData);
  this->FeedbackGlyphFilter->SetSourceConnection(this->BrushPolyDataNormals->GetOutputPort());

}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorPaintEffectPrivate::~qSlicerSegmentEditorPaintEffectPrivate()
{
  this->clearBrushPipelines();
}

//-----------------------------------------------------------------------------
BrushPipeline* qSlicerSegmentEditorPaintEffectPrivate::brushForWidget(qMRMLWidget* viewWidget)
{
  Q_Q(qSlicerSegmentEditorPaintEffect);

  if (this->BrushPipelines.contains(viewWidget))
    {
    return this->BrushPipelines[viewWidget];
    }

  // Create brushPipeline if does not yet exist
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget)
    {
    BrushPipeline2D* pipeline = new BrushPipeline2D();
    pipeline->BrushCutter->SetInputConnection(this->WorldOriginToWorldTransformer->GetOutputPort());
    pipeline->FeedbackCutter->SetInputConnection(this->FeedbackGlyphFilter->GetOutputPort());
    this->updateBrush(viewWidget, pipeline);
    q->addActor2D(viewWidget, pipeline->BrushActor);
    q->addActor2D(viewWidget, pipeline->FeedbackActor);
    this->BrushPipelines[viewWidget] = pipeline;
    return pipeline;
    }
  else if (threeDWidget)
    {
    BrushPipeline3D* pipeline = new BrushPipeline3D();
    pipeline->BrushMapper->SetInputConnection(this->WorldOriginToWorldTransformer->GetOutputPort());
    pipeline->FeedbackMapper->SetInputConnection(this->FeedbackGlyphFilter->GetOutputPort());
    this->updateBrush(viewWidget, pipeline);
    q->addActor3D(viewWidget, pipeline->BrushActor);
    q->addActor3D(viewWidget, pipeline->FeedbackActor);
    this->BrushPipelines[viewWidget] = pipeline;
    return pipeline;
    }

  return NULL;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::forceRender(qMRMLWidget* viewWidget)
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
void qSlicerSegmentEditorPaintEffectPrivate::scheduleRender(qMRMLWidget* viewWidget)
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

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::paintAddPoint(qMRMLWidget* viewWidget, double brushPosition_World[3])
{
  Q_Q(qSlicerSegmentEditorPaintEffect);

  this->PaintCoordinates_World->InsertNextPoint(brushPosition_World);
  this->PaintCoordinates_World->Modified();

  if (q->integerParameter("BrushPixelMode") || !this->DelayedPaint)
    {
    this->paintApply(viewWidget);
    this->forceRender(viewWidget); // TODO: repaint all?
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::paintApply(qMRMLWidget* viewWidget)
{
  Q_Q(qSlicerSegmentEditorPaintEffect);

  vtkOrientedImageData* modifierLabelmap = q->defaultModifierLabelmap();
  if (!modifierLabelmap)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentationNode";
    return;
    }
  if (!q->parameterSetNode())
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node!";
    return;
    }
  vtkMRMLSegmentationNode* segmentationNode = q->parameterSetNode()->GetSegmentationNode();
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentationNode";
    return;
    }

  q->saveStateForUndo();

  if (q->integerParameter("BrushPixelMode"))
    {
    this->paintPixels(viewWidget, this->PaintCoordinates_World);
    }
  else
    {
    this->updateBrushStencil(viewWidget);

    this->BrushPolyDataToStencil->Update();
    vtkImageStencilData* stencilData = this->BrushPolyDataToStencil->GetOutput();
    int stencilExtent[6]={0,-1,0,-1,0,-1};
    stencilData->GetExtent(stencilExtent);

    vtkNew<vtkTransform> worldToModifierLabelmapIjkTransform;

    vtkNew<vtkMatrix4x4> segmentationToSegmentationIjkTransformMatrix;
    modifierLabelmap->GetImageToWorldMatrix(segmentationToSegmentationIjkTransformMatrix.GetPointer());
    segmentationToSegmentationIjkTransformMatrix->Invert();
    worldToModifierLabelmapIjkTransform->Concatenate(segmentationToSegmentationIjkTransformMatrix.GetPointer());

    vtkNew<vtkMatrix4x4> worldToSegmentationTransformMatrix;
    // We don't support painting in non-linearly transformed node (it could be implemented, but would probably slow down things too much)
    // TODO: show a meaningful error message to the user if attempted
    vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(NULL, segmentationNode->GetParentTransformNode(), worldToSegmentationTransformMatrix.GetPointer());
    worldToModifierLabelmapIjkTransform->Concatenate(worldToSegmentationTransformMatrix.GetPointer());

    vtkNew<vtkPoints> paintCoordinates_Ijk;
    worldToModifierLabelmapIjkTransform->TransformPoints(this->PaintCoordinates_World, paintCoordinates_Ijk.GetPointer());

    vtkNew<vtkImageStencilToImage> stencilToImage;
    stencilToImage->SetInputConnection(this->BrushPolyDataToStencil->GetOutputPort());
    stencilToImage->SetInsideValue(q->m_FillValue);
    stencilToImage->SetOutsideValue(q->m_EraseValue);
    stencilToImage->SetOutputScalarType(modifierLabelmap->GetScalarType());

    vtkNew<vtkImageChangeInformation> brushPositioner;
    brushPositioner->SetInputConnection(stencilToImage->GetOutputPort());
    brushPositioner->SetOutputSpacing(modifierLabelmap->GetSpacing());
    brushPositioner->SetOutputOrigin(modifierLabelmap->GetOrigin());

    vtkIdType numberOfPoints = this->PaintCoordinates_World->GetNumberOfPoints();
    for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++)
      {
      double* shiftDouble = paintCoordinates_Ijk->GetPoint(pointIndex);
      int shift[3] = {int(shiftDouble[0]+0.5), int(shiftDouble[1]+0.5), int(shiftDouble[2]+0.5)};
      brushPositioner->SetExtentTranslation(shift);
      brushPositioner->Update();
      vtkNew<vtkOrientedImageData> orientedBrushPositionerOutput;
      orientedBrushPositionerOutput->ShallowCopy(brushPositioner->GetOutput());
      orientedBrushPositionerOutput->CopyDirections(modifierLabelmap);
      vtkOrientedImageDataResample::ModifyImage(modifierLabelmap, orientedBrushPositionerOutput.GetPointer(), vtkOrientedImageDataResample::OPERATION_MAXIMUM);
      }
    modifierLabelmap->Modified();
    }
  this->PaintCoordinates_World->Reset();

  // Notify editor about changes
  qSlicerSegmentEditorAbstractEffect::ModificationMode modificationMode = (q->m_Erase ? qSlicerSegmentEditorAbstractEffect::ModificationModeRemove : qSlicerSegmentEditorAbstractEffect::ModificationModeAdd);
  //TODO: it would be nice to compute paint extent and pass it to make the painting operation faster
  q->modifySelectedSegmentByLabelmap(modifierLabelmap, modificationMode);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::updateBrushStencil(qMRMLWidget* viewWidget)
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  Q_UNUSED(viewWidget);

  if (!q->parameterSetNode())
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node!";
    return;
    }
  vtkMRMLSegmentationNode* segmentationNode = q->parameterSetNode()->GetSegmentationNode();
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentationNode";
    return;
    }
  vtkOrientedImageData* modifierLabelmap = q->modifierLabelmap();
  if (!modifierLabelmap)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid modifierLabelmap";
    return;
    }

  // Brush stencil transform

  this->WorldOriginToModifierLabelmapIjkTransform->Identity();

  vtkNew<vtkMatrix4x4> segmentationToSegmentationIjkTransformMatrix;
  modifierLabelmap->GetImageToWorldMatrix(segmentationToSegmentationIjkTransformMatrix.GetPointer());
  segmentationToSegmentationIjkTransformMatrix->Invert();
  segmentationToSegmentationIjkTransformMatrix->SetElement(0,3, 0);
  segmentationToSegmentationIjkTransformMatrix->SetElement(1,3, 0);
  segmentationToSegmentationIjkTransformMatrix->SetElement(2,3, 0);
  this->WorldOriginToModifierLabelmapIjkTransform->Concatenate(segmentationToSegmentationIjkTransformMatrix.GetPointer());

  vtkNew<vtkMatrix4x4> worldToSegmentationTransformMatrix;
  // We don't support painting in non-linearly transformed node (it could be implemented, but would probably slow down things too much)
  // TODO: show a meaningful error message to the user if attempted
  vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(NULL, segmentationNode->GetParentTransformNode(), worldToSegmentationTransformMatrix.GetPointer());
  worldToSegmentationTransformMatrix->SetElement(0,3, 0);
  worldToSegmentationTransformMatrix->SetElement(1,3, 0);
  worldToSegmentationTransformMatrix->SetElement(2,3, 0);
  this->WorldOriginToModifierLabelmapIjkTransform->Concatenate(worldToSegmentationTransformMatrix.GetPointer());

  this->WorldOriginToModifierLabelmapIjkTransformer->Update();
  vtkPolyData* brushModel_ModifierLabelmapIjk = this->WorldOriginToModifierLabelmapIjkTransformer->GetOutput();
  double* boundsIjk = brushModel_ModifierLabelmapIjk->GetBounds();
  this->BrushPolyDataToStencil->SetOutputWholeExtent(floor(boundsIjk[0])-1, ceil(boundsIjk[1])+1,
    floor(boundsIjk[2])-1, ceil(boundsIjk[3])+1, floor(boundsIjk[4])-1, ceil(boundsIjk[5])+1);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::paintPixel(qMRMLWidget* viewWidget, double pixelPosition_World[3])
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->InsertNextPoint(pixelPosition_World);
  this->paintPixels(viewWidget, points);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::paintPixels(
    qMRMLWidget* viewWidget,
    vtkPoints* pixelPositions)
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  Q_UNUSED(viewWidget);

  if (!pixelPositions)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid pixelPositions";
    return;
    }
  /*
  TODO: implement
  vtkOrientedImageData* modifierLabelmap = q->modifierLabelmap();
  if (!modifierLabelmap)
    {
    return;
    }

  int dims[3] = { 0, 0, 0 };
  modifierLabelmap->GetDimensions(dims);

  double valueToSet = (q->m_Erase ? q->m_EraseValue : q->m_FillValue);

  vtkIdType numberOfPoints = pixelPositions->GetNumberOfPoints();
  for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++)
    {
    int ijk[3] = { 0, 0, 0 };
    q->xyzToIjk(pixelPositions->GetPoint(pointIndex), ijk, sliceWidget, modifierLabelmap);

    // Clamp to image extent
    if (ijk[0] < 0 || ijk[0] >= dims[0]) { continue; }
    if (ijk[1] < 0 || ijk[1] >= dims[1]) { continue; }
    if (ijk[2] < 0 || ijk[2] >= dims[2]) { continue; }

    modifierLabelmap->SetScalarComponentFromDouble(ijk[0], ijk[1], ijk[2], 0, valueToSet);
    }
  */
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::scaleRadius(double scaleFactor)
{
  Q_Q(qSlicerSegmentEditorPaintEffect);

  q->setCommonParameter("BrushRadius", q->doubleParameter("BrushRadius") * scaleFactor);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::onRadiusUnitsClicked()
{
  if (this->BrushRadiusUnitsToggle->text().compare("mm:"))
    {
    this->BrushRadiusUnitsToggle->setText("mm:");
    }
  else
    {
    this->BrushRadiusUnitsToggle->setText("px:");
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::onQuickRadiusButtonClicked()
{
  Q_Q(qSlicerSegmentEditorPaintEffect);

  vtkOrientedImageData* modifierLabelmap = q->modifierLabelmap();
  QPushButton* senderButton = dynamic_cast<QPushButton*>(sender());
  int radius = senderButton->property("BrushRadius").toInt();

  double radiusMm = 0.0;
  if (!this->BrushRadiusUnitsToggle->text().compare("px:"))
    {
    if (modifierLabelmap)
      {
      double spacing[3] = {0.0, 0.0, 0.0};
      modifierLabelmap->GetSpacing(spacing);
      double minimumSpacing = qMin(spacing[0], qMin(spacing[1], spacing[2]));
      radiusMm = minimumSpacing * radius;
      }
    }
  else
    {
    radiusMm = radius;
    }

  this->onRadiusValueChanged(radiusMm);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::onRadiusValueChanged(double value)
{
  Q_Q(qSlicerSegmentEditorPaintEffect);

  q->setCommonParameter("BrushRadius", value);
}

//----------------------------------------------------------------------------
double qSlicerSegmentEditorPaintEffectPrivate::GetSliceSpacing(qMRMLSliceWidget* sliceWidget)
{
  // Implementation copied from vtkSliceViewInteractorStyle::GetSliceSpacing()
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

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::updateBrushModel(qMRMLWidget* viewWidget, double brushPosition_World[3])
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  double radiusMm = q->doubleParameter("BrushRadius");
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  if (!sliceWidget || q->integerParameter("BrushSphere"))
    {
    this->BrushSphereSource->SetRadius(radiusMm);
    this->BrushSphereSource->SetPhiResolution(16);
    this->BrushSphereSource->SetThetaResolution(16);
    this->BrushToWorldOriginTransformer->SetInputConnection(this->BrushSphereSource->GetOutputPort());
    }
  else
    {
    this->BrushCylinderSource->SetRadius(radiusMm);
    this->BrushCylinderSource->SetResolution(16);
    double sliceSpacingMm = this->GetSliceSpacing(sliceWidget);
    this->BrushCylinderSource->SetHeight(sliceSpacingMm);
    this->BrushCylinderSource->SetCenter(0, 0, sliceSpacingMm/2.0);
    this->BrushToWorldOriginTransformer->SetInputConnection(this->BrushCylinderSource->GetOutputPort());
    }

  vtkNew<vtkMatrix4x4> brushToWorldOriginTransformMatrix;
  if (sliceWidget)
    {
    // brush is rotated to the slice widget plane
    brushToWorldOriginTransformMatrix->DeepCopy(sliceWidget->sliceLogic()->GetSliceNode()->GetSliceToRAS());
    brushToWorldOriginTransformMatrix->SetElement(0,3, 0);
    brushToWorldOriginTransformMatrix->SetElement(1,3, 0);
    brushToWorldOriginTransformMatrix->SetElement(2,3, 0);
    }
  this->BrushToWorldOriginTransform->Identity();
  this->BrushToWorldOriginTransform->Concatenate(brushToWorldOriginTransformMatrix.GetPointer());
  this->BrushToWorldOriginTransform->RotateX(90); // cylinder's long axis is the Y axis, we need to rotate it to Z axis

  this->WorldOriginToWorldTransform->Identity();
  this->WorldOriginToWorldTransform->Translate(brushPosition_World);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::updateBrush(qMRMLWidget* viewWidget, BrushPipeline* pipeline)
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  if (q->integerParameter("BrushPixelMode"))
    {
    pipeline->SetBrushVisibility(false);
    return;
    }
  pipeline->SetBrushVisibility(true);

  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  if (sliceWidget)
    {
    // Update slice cutting plane position and orientation
    vtkMatrix4x4* sliceXyToRas = sliceWidget->sliceLogic()->GetSliceNode()->GetXYToRAS();
    pipeline->SlicePlane->SetNormal(sliceXyToRas->GetElement(0,2),sliceXyToRas->GetElement(1,2),sliceXyToRas->GetElement(2,2));
    pipeline->SlicePlane->SetOrigin(sliceXyToRas->GetElement(0,3),sliceXyToRas->GetElement(1,3),sliceXyToRas->GetElement(2,3));

    vtkNew<vtkMatrix4x4> rasToSliceXy;
    vtkMatrix4x4::Invert(sliceXyToRas, rasToSliceXy.GetPointer());
    pipeline->WorldToSliceTransform->SetMatrix(rasToSliceXy.GetPointer());
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::updateBrushes()
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  // unusedWidgetPipelines will contain those widget pointers that are not in the layout anymore
  QList<qMRMLWidget*> unusedWidgetPipelines = this->BrushPipelines.keys();

  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  foreach(QString sliceViewName, layoutManager->sliceViewNames())
    {
    qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget(sliceViewName);
    unusedWidgetPipelines.removeOne(sliceWidget); // not an orphan

    BrushPipeline* brushPipeline = this->brushForWidget(sliceWidget);
    this->updateBrush(sliceWidget, brushPipeline);
    this->scheduleRender(sliceWidget);
    }
  for (int threeDViewId = 0; threeDViewId < layoutManager->threeDViewCount(); ++threeDViewId)
    {
    qMRMLThreeDWidget* threeDWidget = layoutManager->threeDWidget(threeDViewId);
    unusedWidgetPipelines.removeOne(threeDWidget); // not an orphan

    BrushPipeline* brushPipeline = this->brushForWidget(threeDWidget);
    this->updateBrush(threeDWidget, brushPipeline);
    this->scheduleRender(threeDWidget);
    }

  foreach (qMRMLWidget* viewWidget, unusedWidgetPipelines)
    {
    BrushPipeline* pipeline = this->BrushPipelines[viewWidget];
    BrushPipeline2D* pipeline2D = dynamic_cast<BrushPipeline2D*>(pipeline);
    BrushPipeline3D* pipeline3D = dynamic_cast<BrushPipeline3D*>(pipeline);
    if (pipeline2D)
      {
      q->removeActor2D(viewWidget, pipeline2D->BrushActor);
      q->removeActor2D(viewWidget, pipeline2D->FeedbackActor);
      }
    else if (pipeline3D)
      {
      q->removeActor3D(viewWidget, pipeline3D->BrushActor);
      q->removeActor3D(viewWidget, pipeline3D->FeedbackActor);
      }
    delete pipeline;
    this->BrushPipelines.remove(viewWidget);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::clearBrushPipelines()
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  QMapIterator<qMRMLWidget*, BrushPipeline*> it(this->BrushPipelines);
  while (it.hasNext())
    {
    it.next();
    qMRMLWidget* viewWidget = it.key();
    BrushPipeline* brushPipeline = it.value();
    BrushPipeline2D* pipeline2D = dynamic_cast<BrushPipeline2D*>(brushPipeline);
    BrushPipeline3D* pipeline3D = dynamic_cast<BrushPipeline3D*>(brushPipeline);
    if (pipeline2D)
      {
      q->removeActor2D(viewWidget, pipeline2D->BrushActor);
      q->removeActor2D(viewWidget, pipeline2D->FeedbackActor);
      }
    else if (pipeline3D)
      {
      q->removeActor3D(viewWidget, pipeline3D->BrushActor);
      q->removeActor3D(viewWidget, pipeline3D->FeedbackActor);
      }
    delete brushPipeline;
    }
  this->BrushPipelines.clear();
}


//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorPaintEffect methods

//----------------------------------------------------------------------------
qSlicerSegmentEditorPaintEffect::qSlicerSegmentEditorPaintEffect(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSegmentEditorPaintEffectPrivate(*this) )
{
  this->m_Name = QString("Paint");
  this->m_Erase = false;
}

//----------------------------------------------------------------------------
qSlicerSegmentEditorPaintEffect::~qSlicerSegmentEditorPaintEffect()
{
}

//---------------------------------------------------------------------------
QIcon qSlicerSegmentEditorPaintEffect::icon()
{
  Q_D(qSlicerSegmentEditorPaintEffect);

  return d->PaintIcon;
}

//---------------------------------------------------------------------------
QString const qSlicerSegmentEditorPaintEffect::helpText()const
{
  return QString("Left-click and drag in slice or 3D viewers to paint on selected segment with a round brush.");
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect* qSlicerSegmentEditorPaintEffect::clone()
{
  return new qSlicerSegmentEditorPaintEffect();
}

//---------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffect::deactivate()
{
  Q_D(qSlicerSegmentEditorPaintEffect);
  Superclass::deactivate();
  d->clearBrushPipelines();
}

//---------------------------------------------------------------------------
bool qSlicerSegmentEditorPaintEffect::processInteractionEvents(
  vtkRenderWindowInteractor* callerInteractor,
  unsigned long eid,
  qMRMLWidget* viewWidget )
{
  Q_D(qSlicerSegmentEditorPaintEffect);
  bool abortEvent = false;

  // This effect only supports interactions in the 2D slice views currently
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  BrushPipeline* brushPipeline = NULL;
  if (sliceWidget)
    {
    brushPipeline = d->brushForWidget(sliceWidget);
    }
  else if (threeDWidget)
    {
    brushPipeline = d->brushForWidget(threeDWidget);
    }
  if (!brushPipeline)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to create brushPipeline";
    return abortEvent;
    }

  int eventPosition[2] = { 0, 0 };
  callerInteractor->GetEventPosition(eventPosition);

  double brushPosition_World[4] = {0.0, 0.0, 0.0, 1.0};
  if (sliceWidget)
    {
    double eventPositionXY[4] = {
      static_cast<double>(eventPosition[0]),
      static_cast<double>(eventPosition[1]),
      0.0,
      1.0};
    sliceWidget->sliceLogic()->GetSliceNode()->GetXYToRAS()->MultiplyPoint(eventPositionXY, brushPosition_World);
    }
  else if (threeDWidget)
    {
    vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(viewWidget);
    if (!renderer)
      {
      return abortEvent;
      }
    static bool useCellPicker = true;
    if (useCellPicker)
      {
      vtkNew<vtkCellPicker> picker;
      picker->SetTolerance( .005 );
      if (!picker->Pick(eventPosition[0], eventPosition[1], 0, renderer))
        {
        return abortEvent;
        }

      vtkPoints* pickPositions = picker->GetPickedPositions();
      int numberOfPickedPositions = pickPositions->GetNumberOfPoints();
      if (numberOfPickedPositions<1)
        {
        return abortEvent;
        }
      double cameraPosition[3]={0,0,0};
      renderer->GetActiveCamera()->GetPosition(cameraPosition);
      pickPositions->GetPoint(0, brushPosition_World);
      double minDist2 = vtkMath::Distance2BetweenPoints(brushPosition_World, cameraPosition);
      for (int i=1; i<numberOfPickedPositions; i++)
        {
        double currentMinDist2 = vtkMath::Distance2BetweenPoints(pickPositions->GetPoint(i), cameraPosition);
        if (currentMinDist2<minDist2)
          {
          pickPositions->GetPoint(i, brushPosition_World);
          minDist2 = currentMinDist2;
          }
        }
      }
    else
      {
      vtkNew<vtkPropPicker> picker;
      //vtkNew<vtkWorldPointPicker> picker;
      if (!picker->Pick(eventPosition[0], eventPosition[1], 0, renderer))
        {
        return abortEvent;
        }
      picker->GetPickPosition(brushPosition_World);
      }
    }

  if (eid == vtkCommand::LeftButtonPressEvent)
    {
    d->IsPainting = true;
    if (!this->integerParameter("BrushPixelMode"))
      {
      //this->cursorOff(sliceWidget);
      }
    QList<qMRMLWidget*> viewWidgets = d->BrushPipelines.keys();
    foreach (qMRMLWidget* viewWidget, viewWidgets)
      {
      d->BrushPipelines[viewWidget]->SetFeedbackVisibility(d->DelayedPaint);
      }
    if (this->integerParameter("ColorSmudge"))
      {
      //TODO:
      //EditUtil.setLabel(self.getLabelPixel(xy))
      }
    d->paintAddPoint(viewWidget, brushPosition_World);
    abortEvent = true;
    }
  else if (eid == vtkCommand::LeftButtonReleaseEvent)
    {
    d->paintApply(viewWidget);
    d->IsPainting = false;

    QList<qMRMLWidget*> viewWidgets = d->BrushPipelines.keys();
    foreach (qMRMLWidget* viewWidget, viewWidgets)
      {
      d->BrushPipelines[viewWidget]->SetFeedbackVisibility(false);
      }

    //this->cursorOn(sliceWidget);
    }
  else if (eid == vtkCommand::MouseMoveEvent)
    {
    if (d->IsPainting)
      {
      d->paintAddPoint(viewWidget, brushPosition_World);
      abortEvent = false;
      }
    }
  else if (eid == vtkCommand::EnterEvent)
    {
    brushPipeline->SetBrushVisibility(!this->integerParameter("BrushPixelMode"));
    }
  else if (eid == vtkCommand::LeaveEvent)
    {
    brushPipeline->SetBrushVisibility(false);
    }
  else if (eid == vtkCommand::KeyPressEvent)
    {
    const char* key = callerInteractor->GetKeySym();
    if (!strcmp(key, "plus") || !strcmp(key, "equal"))
      {
      d->scaleRadius(1.2);
      }
    if (!strcmp(key, "minus") || !strcmp(key, "underscore"))
      {
      d->scaleRadius(0.8);
      }
    }

  // Update paint feedback glyph to follow mouse
  d->updateBrushModel(viewWidget, brushPosition_World);
  d->updateBrushes();
  d->forceRender(viewWidget);
  return abortEvent;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffect::processViewNodeEvents(
    vtkMRMLAbstractViewNode* callerViewNode,
    unsigned long eid,
    qMRMLWidget* viewWidget)
{
  Q_D(qSlicerSegmentEditorPaintEffect);
  Q_UNUSED(callerViewNode);
  Q_UNUSED(eid);

  // This effect only supports interactions in the 2D slice views currently
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (!sliceWidget && !threeDWidget)
    {
    return;
    }
  BrushPipeline* brushPipeline = d->brushForWidget(viewWidget);
  if (!brushPipeline)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to create brushPipeline!";
    return;
    }

  d->updateBrush(viewWidget, brushPipeline);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffect::setupOptionsFrame()
{
  // Setup widgets corresponding to the parent class of this effect
  Superclass::setupOptionsFrame();

  Q_D(qSlicerSegmentEditorPaintEffect);

  // Create options frame for this effect
  d->BrushRadiusFrame = new QFrame();
  d->BrushRadiusFrame->setLayout(new QHBoxLayout());
  this->addOptionsWidget(d->BrushRadiusFrame);

  QLabel* radiusLabel = new QLabel("Radius:", d->BrushRadiusFrame);
  radiusLabel->setToolTip("Set the radius of the paint brush in millimeters");
  d->BrushRadiusFrame->layout()->addWidget(radiusLabel);

  d->BrushRadiusSpinBox = new qMRMLSpinBox(d->BrushRadiusFrame);
  d->BrushRadiusSpinBox->setToolTip("Set the radius of the paint brush in millimeters");
  d->BrushRadiusSpinBox->setQuantity("length");
  d->BrushRadiusSpinBox->setUnitAwareProperties(qMRMLSpinBox::Prefix | qMRMLSpinBox::Suffix);
  d->BrushRadiusFrame->layout()->addWidget(d->BrushRadiusSpinBox);

  d->BrushRadiusUnitsToggle = new QPushButton("px:");
  d->BrushRadiusUnitsToggle->setToolTip("Toggle radius quick set buttons between mm and label volume pixel size units");
  d->BrushRadiusUnitsToggle->setFixedWidth(35);
  d->BrushRadiusFrame->layout()->addWidget(d->BrushRadiusUnitsToggle);

  QList<int> quickRadii;
  quickRadii << 2 << 3 << 4 << 5 << 10 << 20;
  foreach (int radius, quickRadii)
    {
    QPushButton* quickRadiusButton = new QPushButton(QString::number(radius));
    quickRadiusButton->setProperty("BrushRadius", QVariant(radius));
    quickRadiusButton->setFixedWidth(25);
    quickRadiusButton->setToolTip("Set radius based on mm or label voxel size units depending on toggle value");
    d->BrushRadiusFrame->layout()->addWidget(quickRadiusButton);
    QObject::connect(quickRadiusButton, SIGNAL(clicked()), d, SLOT(onQuickRadiusButtonClicked()));
    }

  d->BrushRadiusSlider = new ctkDoubleSlider();
  d->BrushRadiusSlider->setOrientation(Qt::Horizontal);
  this->addOptionsWidget(d->BrushRadiusSlider);

  d->BrushSphereCheckbox = new QCheckBox("Sphere brush");
  d->BrushSphereCheckbox->setToolTip("Use a 3D spherical brush rather than a 2D circular brush.");
  this->addOptionsWidget(d->BrushSphereCheckbox);

  d->ColorSmudgeCheckbox = new QCheckBox("Color smudge");
  //TODO: Smudge is not yet implemented. It is now a more complex function,
  //  as it involves switching segment instead of simply changing label color.
  //d->ColorSmudgeCheckbox->setToolTip("Set the label number automatically by sampling the pixel location where the brush stroke starts.");
  d->ColorSmudgeCheckbox->setToolTip("Smudge function is not yet implemented!");
  d->ColorSmudgeCheckbox->setEnabled(false);
  this->addOptionsWidget(d->ColorSmudgeCheckbox);

  d->BrushPixelModeCheckbox = new QCheckBox("Pixel mode");
  d->BrushPixelModeCheckbox->setToolTip("Paint exactly the pixel under the cursor, ignoring the radius, threshold, and paint over.");
  this->addOptionsWidget(d->BrushPixelModeCheckbox);

  QObject::connect(d->BrushRadiusUnitsToggle, SIGNAL(clicked()), d, SLOT(onRadiusUnitsClicked()));
  QObject::connect(d->BrushSphereCheckbox, SIGNAL(clicked()), this, SLOT(updateMRMLFromGUI()));
  QObject::connect(d->ColorSmudgeCheckbox, SIGNAL(clicked()), this, SLOT(updateMRMLFromGUI()));
  QObject::connect(d->BrushPixelModeCheckbox, SIGNAL(clicked()), this, SLOT(updateMRMLFromGUI()));
  QObject::connect(d->BrushRadiusSlider, SIGNAL(valueChanged(double)), d, SLOT(onRadiusValueChanged(double)));
  QObject::connect(d->BrushRadiusSpinBox, SIGNAL(valueChanged(double)), d, SLOT(onRadiusValueChanged(double)));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffect::setMRMLDefaults()
{
  Superclass::setMRMLDefaults();

  this->setCommonParameterDefault("BrushMinimumRadius", 0.01);
  this->setCommonParameterDefault("BrushMaximumRadius", 100.0);
  this->setCommonParameterDefault("BrushRadius", 5.0);
  this->setCommonParameterDefault("BrushSphere", 0);
  this->setCommonParameterDefault("ColorSmudge", 0);
  this->setCommonParameterDefault("BrushPixelMode", 0);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffect::updateGUIFromMRML()
{
  Q_D(qSlicerSegmentEditorPaintEffect);
  if (!this->active())
    {
    // updateGUIFromMRML is called when the effect is activated
    return;
    }

  if (!this->scene())
    {
    return;
    }

  d->BrushSphereCheckbox->blockSignals(true);
  d->BrushSphereCheckbox->setChecked(this->integerParameter("BrushSphere"));
  d->BrushSphereCheckbox->blockSignals(false);

  d->ColorSmudgeCheckbox->blockSignals(true);
  d->ColorSmudgeCheckbox->setChecked(this->integerParameter("ColorSmudge"));
  d->ColorSmudgeCheckbox->blockSignals(false);

  bool pixelMode = this->integerParameter("BrushPixelMode");
  d->BrushPixelModeCheckbox->blockSignals(true);
  d->BrushPixelModeCheckbox->setChecked(pixelMode);
  d->BrushPixelModeCheckbox->blockSignals(false);

  Superclass::updateGUIFromMRML();

  // Radius is also disabled if pixel mode is on
  d->BrushRadiusFrame->setEnabled(!pixelMode);

  d->BrushRadiusSlider->blockSignals(true);
  d->BrushRadiusSlider->setMinimum(this->doubleParameter("BrushMinimumRadius"));
  d->BrushRadiusSlider->setMaximum(this->doubleParameter("BrushMaximumRadius"));
  d->BrushRadiusSlider->setValue(this->doubleParameter("BrushRadius"));
  d->BrushRadiusSlider->setSingleStep(this->doubleParameter("BrushMinimumRadius"));
  d->BrushRadiusSlider->blockSignals(false);

  d->BrushRadiusSpinBox->blockSignals(true);
  d->BrushRadiusSpinBox->setMRMLScene(this->scene());
  d->BrushRadiusSpinBox->setMinimum(this->doubleParameter("BrushMinimumRadius"));
  d->BrushRadiusSpinBox->setMaximum(this->doubleParameter("BrushMaximumRadius"));
  d->BrushRadiusSpinBox->setValue(this->doubleParameter("BrushRadius"));
  int decimals = (int)(log10(this->doubleParameter("BrushMinimumRadius")));
  if (decimals < 0)
    {
    d->BrushRadiusSpinBox->setDecimals(-decimals * 2);
    }
  d->BrushRadiusSpinBox->blockSignals(false);

  // Update brushes
  d->updateBrushes();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffect::updateMRMLFromGUI()
{
  Q_D(qSlicerSegmentEditorPaintEffect);

  Superclass::updateMRMLFromGUI();

  this->setCommonParameter("BrushSphere", (int)d->BrushSphereCheckbox->isChecked());
  this->setCommonParameter("ColorSmudge", (int)d->ColorSmudgeCheckbox->isChecked());
  bool pixelMode = d->BrushPixelModeCheckbox->isChecked();
  bool pixelModeChanged = (pixelMode != (bool)this->integerParameter("BrushPixelMode"));
  this->setCommonParameter("BrushPixelMode", (int)pixelMode);
  this->setCommonParameter("BrushRadius", d->BrushRadiusSlider->value());

  // If pixel mode changed, then other GUI changes are due
  if (pixelModeChanged)
    {
    // Update label options based on constraints set by pixel mode
    Superclass::updateGUIFromMRML();

    d->BrushRadiusFrame->setEnabled(!pixelMode);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffect::referenceGeometryChanged()
{
  Superclass::referenceGeometryChanged();

  vtkOrientedImageData* referenceGeometryImage = this->referenceGeometryImage();
  if (referenceGeometryImage == NULL)
    {
    return;
    }
  double spacing[3] = {0.0, 0.0, 0.0};
  referenceGeometryImage->GetSpacing(spacing);
  double minimumSpacing = qMin(spacing[0], qMin(spacing[1], spacing[2]));
  double minimumRadius = 0.5 * minimumSpacing;

  int dimensions[3] = {0, 0, 0};
  referenceGeometryImage->GetDimensions(dimensions);
  double bounds[3] = {spacing[0]*dimensions[0], spacing[1]*dimensions[1], spacing[2]*dimensions[2]};
  double maximumBounds = qMax(bounds[0], qMax(bounds[1], bounds[2]));
  double maximumRadius = 0.5 * maximumBounds;

  this->setCommonParameter("BrushMinimumRadius", minimumRadius);
  this->setCommonParameter("BrushMaximumRadius", maximumRadius);
  this->setCommonParameter("BrushRadius", qMin(50.0 * minimumRadius, 0.5 * maximumRadius));

  this->updateGUIFromMRML();
}
