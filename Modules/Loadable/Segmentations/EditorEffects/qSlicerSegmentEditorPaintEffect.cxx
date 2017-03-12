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
#include <QToolButton>

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
  , ActiveViewWidget(NULL)
  , BrushDiameterFrame(NULL)
  , BrushDiameterSpinBox(NULL)
  , BrushDiameterSlider(NULL)
  , BrushDiameterRelativeToggle(NULL)
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

  this->ActiveViewLastInteractionPosition[0] = 0;
  this->ActiveViewLastInteractionPosition[1] = 0;
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
void qSlicerSegmentEditorPaintEffectPrivate::paintAddPoint(qMRMLWidget* viewWidget, double brushPosition_World[3])
{
  Q_Q(qSlicerSegmentEditorPaintEffect);

  this->PaintCoordinates_World->InsertNextPoint(brushPosition_World);
  this->PaintCoordinates_World->Modified();

  if (q->integerParameter("BrushPixelMode") || !this->DelayedPaint)
    {
    this->paintApply(viewWidget);
    qSlicerSegmentEditorAbstractEffect::forceRender(viewWidget); // TODO: repaint all?
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

  QList<int> updateExtentList;

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
    modifierLabelmap->GetWorldToImageMatrix(segmentationToSegmentationIjkTransformMatrix.GetPointer());
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
    int updateExtent[6] = { 0, -1, 0, -1, 0, -1 };
    for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++)
      {
      double* shiftDouble = paintCoordinates_Ijk->GetPoint(pointIndex);
      int shift[3] = {int(shiftDouble[0]+0.5), int(shiftDouble[1]+0.5), int(shiftDouble[2]+0.5)};
      brushPositioner->SetExtentTranslation(shift);
      brushPositioner->Update();
      vtkNew<vtkOrientedImageData> orientedBrushPositionerOutput;
      orientedBrushPositionerOutput->ShallowCopy(brushPositioner->GetOutput());
      orientedBrushPositionerOutput->CopyDirections(modifierLabelmap);
      if (pointIndex == 0)
        {
        orientedBrushPositionerOutput->GetExtent(updateExtent);
        }
      else
        {
        int* brushExtent = orientedBrushPositionerOutput->GetExtent();
        for (int i = 0; i < 3; i++)
          {
          if (brushExtent[i * 2] < updateExtent[i * 2])
            {
            updateExtent[i * 2] = brushExtent[i * 2];
            }
          if (brushExtent[i * 2 + 1] > updateExtent[i * 2 + 1])
            {
            updateExtent[i * 2 + 1] = brushExtent[i * 2 + 1];
            }
          }
        }
      vtkOrientedImageDataResample::ModifyImage(modifierLabelmap, orientedBrushPositionerOutput.GetPointer(), vtkOrientedImageDataResample::OPERATION_MAXIMUM);
      }
    modifierLabelmap->Modified();
    for (int i = 0; i < 6; i++)
      {
      updateExtentList << updateExtent[i];
      }
    }
  this->PaintCoordinates_World->Reset();

  // Notify editor about changes
  qSlicerSegmentEditorAbstractEffect::ModificationMode modificationMode = (q->m_Erase ? qSlicerSegmentEditorAbstractEffect::ModificationModeRemove : qSlicerSegmentEditorAbstractEffect::ModificationModeAdd);
  q->modifySelectedSegmentByLabelmap(modifierLabelmap, modificationMode, updateExtentList);
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
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->InsertNextPoint(pixelPosition_World);
  this->paintPixels(viewWidget, points);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::paintPixels(
    qMRMLWidget* viewWidget,
    vtkPoints* pixelPositions)
{
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
void qSlicerSegmentEditorPaintEffectPrivate::scaleDiameter(double scaleFactor)
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  if (q->integerParameter("BrushDiameterIsRelative"))
    {
    q->setCommonParameter("BrushRelativeDiameter", q->doubleParameter("BrushRelativeDiameter") * scaleFactor);
    }
  else
    {
    q->setCommonParameter("BrushAbsoluteDiameter", q->doubleParameter("BrushAbsoluteDiameter") * scaleFactor);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::onDiameterUnitsClicked()
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  if (q->integerParameter("BrushDiameterIsRelative") == 0)
    {
    q->setCommonParameter("BrushDiameterIsRelative", 1);
    }
  else
    {
    q->setCommonParameter("BrushDiameterIsRelative", 0);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::onQuickDiameterButtonClicked()
{
  QToolButton* senderButton = dynamic_cast<QToolButton*>(sender());
  int diameter = senderButton->property("BrushDiameter").toInt();

  this->onDiameterValueChanged(diameter);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::onDiameterValueChanged(double value)
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  if (q->integerParameter("BrushDiameterIsRelative") == 0)
    {
    q->setCommonParameter("BrushAbsoluteDiameter", value);
    }
  else
    {
    q->setCommonParameter("BrushRelativeDiameter", value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::updateAbsoluteBrushDiameter()
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  if (!q->integerParameter("BrushDiameterIsRelative"))
    {
    // user specified absolute brush diameter
    return;
    }
  if (this->ActiveViewWidget == NULL)
    {
    return;
    }

  double mmPerPixel = 1.0;
  int screenSizePixel = 1000;
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(this->ActiveViewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(this->ActiveViewWidget);
  if (sliceWidget)
    {
    vtkMatrix4x4* xyToSlice = sliceWidget->sliceLogic()->GetSliceNode()->GetXYToSlice();
    mmPerPixel = sqrt(xyToSlice->GetElement(0, 1)*xyToSlice->GetElement(0, 1)
      + xyToSlice->GetElement(1, 1)*xyToSlice->GetElement(1, 1)
      + xyToSlice->GetElement(2, 1)*xyToSlice->GetElement(2, 1));
    screenSizePixel = sliceWidget->sliceView()->renderWindow()->GetScreenSize()[1];
    }
  else if (threeDWidget && threeDWidget->threeDView() && threeDWidget->threeDView()->renderer()
    && threeDWidget->threeDView()->renderer()->GetActiveCamera())
    {
    screenSizePixel = threeDWidget->threeDView()->renderWindow()->GetScreenSize()[1];
    vtkRenderer* renderer = threeDWidget->threeDView()->renderer();
    // Viewport: xmin, ymin, xmax, ymax; range: 0.0-1.0; origin is bottom left
    // Determine the available renderer size in pixels
    double minX = 0;
    double minY = 0;
    renderer->NormalizedDisplayToDisplay(minX, minY);
    double maxX = 1;
    double maxY = 1;
    renderer->NormalizedDisplayToDisplay(maxX, maxY);
    int rendererSizeInPixels[2] = { static_cast<int>(maxX - minX), static_cast<int>(maxY - minY) };
    vtkCamera *cam = renderer->GetActiveCamera();
    if (cam->GetParallelProjection())
      {
      // Parallel scale: height of the viewport in world-coordinate distances.
      // Larger numbers produce smaller images.
      mmPerPixel = (cam->GetParallelScale() * 2.0) / double(rendererSizeInPixels[1]);
      }
    else
      {
      double cameraFP[4] = { 0 };
      cam->GetFocalPoint(cameraFP);
      cameraFP[3] = 1.0;

      double cameraViewUp[3] = { 0 };
      cam->GetViewUp(cameraViewUp);
      vtkMath::Normalize(cameraViewUp);

      // Get distance in pixels between two points at unit distance above and below the focal point
      renderer->SetWorldPoint(cameraFP[0] + cameraViewUp[0], cameraFP[1] + cameraViewUp[1], cameraFP[2] + cameraViewUp[2], cameraFP[3]);
      renderer->WorldToDisplay();
      double topCenter[3] = { 0 };
      renderer->GetDisplayPoint(topCenter);
      renderer->SetWorldPoint(cameraFP[0] - cameraViewUp[0], cameraFP[1] - cameraViewUp[1], cameraFP[2] - cameraViewUp[2], cameraFP[3]);
      renderer->WorldToDisplay();
      double bottomCenter[3] = { 0 };
      renderer->GetDisplayPoint(bottomCenter);
      double distInPixels = sqrt(vtkMath::Distance2BetweenPoints(topCenter, bottomCenter));

      // 2.0 = 2x length of viewUp vector in mm (because viewUp is unit vector)
      mmPerPixel = 2.0 / distInPixels;
      }
    }
  double brushRelativeDiameter = q->doubleParameter("BrushRelativeDiameter");
  double newBrushAbsoluteDiameter = screenSizePixel * (brushRelativeDiameter / 100.0) * mmPerPixel;

  double brushAbsoluteDiameter = q->doubleParameter("BrushAbsoluteDiameter");
  if (brushAbsoluteDiameter > 0 && fabs(newBrushAbsoluteDiameter - brushAbsoluteDiameter) / brushAbsoluteDiameter < 0.01)
    {
    // no brush size change
    return;
    }
  q->setCommonParameter("BrushAbsoluteDiameter", newBrushAbsoluteDiameter);

  if (this->ActiveViewWidget)
    {
    qSlicerSegmentEditorAbstractEffect::scheduleRender(this->ActiveViewWidget);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::updateBrushModel(qMRMLWidget* viewWidget, double brushPosition_World[3])
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  this->updateAbsoluteBrushDiameter();

  double diameterMm = q->doubleParameter("BrushAbsoluteDiameter");

  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  if (!sliceWidget || q->integerParameter("BrushSphere"))
    {
    this->BrushSphereSource->SetRadius(diameterMm/2.0);
    this->BrushSphereSource->SetPhiResolution(32);
    this->BrushSphereSource->SetThetaResolution(32);
    this->BrushToWorldOriginTransformer->SetInputConnection(this->BrushSphereSource->GetOutputPort());
    }
  else
    {
    this->BrushCylinderSource->SetRadius(diameterMm/2.0);
    this->BrushCylinderSource->SetResolution(32);
    double sliceSpacingMm = qSlicerSegmentEditorAbstractEffect::sliceSpacing(sliceWidget);
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

  if (brushPosition_World)
    {
    this->WorldOriginToWorldTransform->Identity();
    this->WorldOriginToWorldTransform->Translate(brushPosition_World);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffectPrivate::updateBrush(qMRMLWidget* viewWidget, BrushPipeline* pipeline)
{
  Q_Q(qSlicerSegmentEditorPaintEffect);
  if (this->BrushToWorldOriginTransformer->GetNumberOfInputConnections(0) == 0
      || q->integerParameter("BrushPixelMode"))
    {
    pipeline->SetBrushVisibility(false);
    return;
    }
  pipeline->SetBrushVisibility(this->ActiveViewWidget != NULL);

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
    qSlicerSegmentEditorAbstractEffect::scheduleRender(sliceWidget);
    }
  for (int threeDViewId = 0; threeDViewId < layoutManager->threeDViewCount(); ++threeDViewId)
    {
    qMRMLThreeDWidget* threeDWidget = layoutManager->threeDWidget(threeDViewId);
    unusedWidgetPipelines.removeOne(threeDWidget); // not an orphan

    BrushPipeline* brushPipeline = this->brushForWidget(threeDWidget);
    this->updateBrush(threeDWidget, brushPipeline);
    qSlicerSegmentEditorAbstractEffect::scheduleRender(threeDWidget);
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

  // On exiting Slicer, this method (triggered by deactivation of the effect when exiting the
  // module containing the segment editor widget) may be called after the layout was destroyed
  bool removeActors = true;
  if (!qSlicerApplication::application() || !qSlicerApplication::application()->layoutManager())
    {
    removeActors = false;
    }

  QMapIterator<qMRMLWidget*, BrushPipeline*> it(this->BrushPipelines);
  while (it.hasNext())
    {
    it.next();
    BrushPipeline* brushPipeline = it.value();
    if (removeActors)
      {
      qMRMLWidget* viewWidget = it.key();
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
  this->m_ShowEffectCursorInThreeDView = true;
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
  d->ActiveViewWidget = NULL;
}


//---------------------------------------------------------------------------
bool qSlicerSegmentEditorPaintEffectPrivate::brushPositionInWorld(qMRMLWidget* viewWidget, int brushPositionInView[2], double brushPosition_World[3])
{
  // This effect only supports interactions in the 2D slice views currently
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);

  if (sliceWidget)
    {
    double eventPositionXY[4] = {
      static_cast<double>(brushPositionInView[0]),
      static_cast<double>(brushPositionInView[1]),
      0.0,
      1.0};
    sliceWidget->sliceLogic()->GetSliceNode()->GetXYToRAS()->MultiplyPoint(eventPositionXY, brushPosition_World);
    }
  else if (threeDWidget)
    {
    vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(viewWidget);
    if (!renderer)
      {
      return false;
      }
    static bool useCellPicker = true;
    if (useCellPicker)
      {
      vtkNew<vtkCellPicker> picker;
      picker->SetTolerance( .005 );
      if (!picker->Pick(brushPositionInView[0], brushPositionInView[1], 0, renderer))
        {
        return false;
        }

      vtkPoints* pickPositions = picker->GetPickedPositions();
      int numberOfPickedPositions = pickPositions->GetNumberOfPoints();
      if (numberOfPickedPositions<1)
        {
        return false;
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
      if (!picker->Pick(brushPositionInView[0], brushPositionInView[1], 0, renderer))
        {
        return false;
        }
      picker->GetPickPosition(brushPosition_World);
      }
    }

  return true;
}

//---------------------------------------------------------------------------
bool qSlicerSegmentEditorPaintEffect::processInteractionEvents(
  vtkRenderWindowInteractor* callerInteractor,
  unsigned long eid,
  qMRMLWidget* viewWidget )
{
  Q_D(qSlicerSegmentEditorPaintEffect);

  bool shiftKeyPressed = callerInteractor->GetShiftKey();

  // Process events that do not provide event position (or we don't need event position)
  double scaleDiameterRequested = -1.0; // <0 means no scale change is requested
  const double zoomFactor = 0.2;
  if (eid == vtkCommand::KeyPressEvent)
    {
    const char* key = callerInteractor->GetKeySym();
    if (!strcmp(key, "plus") || !strcmp(key, "equal"))
      {
      scaleDiameterRequested = (1.0 + zoomFactor);
      }
    else if (!strcmp(key, "minus") || !strcmp(key, "underscore"))
      {
      scaleDiameterRequested = (1.0 - zoomFactor);
      }
    else
      {
      return false;
      }
    }
  else if (eid == vtkCommand::KeyReleaseEvent)
    {
    return false;
    }
  else if (eid == vtkCommand::MouseWheelForwardEvent)
    {
    if (shiftKeyPressed)
      {
      scaleDiameterRequested = (1.0 + zoomFactor);
      }
    else
      {
      return false;
      }
    }
  else if (eid == vtkCommand::MouseWheelBackwardEvent && shiftKeyPressed)
    {
    if (shiftKeyPressed)
      {
      scaleDiameterRequested = (1.0 - zoomFactor);
      }
    else
      {
      return false;
      }
    }
  if (scaleDiameterRequested > 0)
    {
    d->scaleDiameter(scaleDiameterRequested);
    d->updateBrushModel(d->ActiveViewWidget, NULL);
    // Only schedule render done force immediate update.
    // When zooming slice in/out then immediate rendering would produce jitter, because
    // brush size would be updated immediately and then slice position would be updated shortly after.
    qSlicerSegmentEditorAbstractEffect::scheduleRender(viewWidget);
    return true; // abortEvent
    }

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
    return false;
    }

  if (d->ActiveViewWidget == 0 && viewWidget != NULL)
    {
    // Mouse pointer entered the widget
    // It is more reliable to check for viewWidget pointer change than vtkCommand::EnterEvent, because
    // the effect may be activated using keyboard shortcut while the mouse pointer is already inside the widget.
    brushPipeline->SetBrushVisibility(!this->integerParameter("BrushPixelMode"));
    }
  d->ActiveViewWidget = viewWidget;

  int eventPosition[2] = { 0, 0 };
  callerInteractor->GetEventPosition(eventPosition);
  d->ActiveViewLastInteractionPosition[0] = eventPosition[0];
  d->ActiveViewLastInteractionPosition[1] = eventPosition[1];

  double brushPosition_World[4] = {0.0, 0.0, 0.0, 1.0};
  if (!d->brushPositionInWorld(viewWidget, eventPosition, brushPosition_World))
    {
    return false;
    }

  bool abortEvent = false;

  if (eid == vtkCommand::LeftButtonPressEvent && !shiftKeyPressed)
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
  else if (eid == vtkCommand::LeaveEvent)
    {
    brushPipeline->SetBrushVisibility(false);
    d->ActiveViewWidget = NULL;
    }

  // Update paint feedback glyph to follow mouse
  d->updateBrushModel(viewWidget, brushPosition_World);
  d->updateBrushes();

  qSlicerSegmentEditorAbstractEffect::forceRender(viewWidget);

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

  if (viewWidget == d->ActiveViewWidget)
    {
    double brushPosition_World[4] = { 0.0, 0.0, 0.0, 1.0 };
    if (d->brushPositionInWorld(viewWidget, d->ActiveViewLastInteractionPosition, brushPosition_World))
      {
      d->updateBrushModel(viewWidget, brushPosition_World);
      }
    else
      {
      d->updateBrushModel(viewWidget, NULL);
      }
    d->updateBrushes();
    qSlicerSegmentEditorAbstractEffect::scheduleRender(d->ActiveViewWidget);
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
  d->BrushDiameterFrame = new QFrame();
  d->BrushDiameterFrame->setLayout(new QHBoxLayout());
  this->addOptionsWidget(d->BrushDiameterFrame);

  QLabel* diameterLabel = new QLabel("Diameter:", d->BrushDiameterFrame);
  diameterLabel->setToolTip("Set the paint brush size as percentage of screen size or as fixed length");
  d->BrushDiameterFrame->layout()->addWidget(diameterLabel);

  d->BrushDiameterSpinBox = new qMRMLSpinBox(d->BrushDiameterFrame);
  d->BrushDiameterSpinBox->setToolTip("Set the paint brush size as percentage of screen size or as fixed length");
  d->BrushDiameterFrame->layout()->addWidget(d->BrushDiameterSpinBox);

  QList<int> quickDiameters;
  quickDiameters << 1 << 3 << 5 << 10 << 20 << 40;
  foreach (int diameter, quickDiameters)
    {
    // QToolbutton is used instead of a regular push button to make the button smaller
    QToolButton* quickDiameterButton = new QToolButton();
    quickDiameterButton->setText(QString::number(diameter));
    quickDiameterButton->setProperty("BrushDiameter", QVariant(diameter));
    quickDiameterButton->setToolTip("Set the paint brush size as percentage of screen size or as fixed length");

    d->BrushDiameterFrame->layout()->addWidget(quickDiameterButton);
    QObject::connect(quickDiameterButton, SIGNAL(clicked()), d, SLOT(onQuickDiameterButtonClicked()));
    }

  d->BrushDiameterRelativeToggle = new QToolButton();
  d->BrushDiameterRelativeToggle->setText("%");
  d->BrushDiameterRelativeToggle->setToolTip("Toggle diameter quick set buttons between percentage of window size / absolute size in millimeters");
  d->BrushDiameterFrame->layout()->addWidget(d->BrushDiameterRelativeToggle);

  d->BrushDiameterSlider = new ctkDoubleSlider();
  d->BrushDiameterSlider->setOrientation(Qt::Horizontal);
  this->addOptionsWidget(d->BrushDiameterSlider);

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
  d->BrushPixelModeCheckbox->setToolTip("Paint exactly the pixel under the cursor, ignoring the diameter, threshold, and paint over.");
  this->addOptionsWidget(d->BrushPixelModeCheckbox);

  QObject::connect(d->BrushDiameterRelativeToggle, SIGNAL(clicked()), d, SLOT(onDiameterUnitsClicked()));
  QObject::connect(d->BrushSphereCheckbox, SIGNAL(clicked()), this, SLOT(updateMRMLFromGUI()));
  QObject::connect(d->ColorSmudgeCheckbox, SIGNAL(clicked()), this, SLOT(updateMRMLFromGUI()));
  QObject::connect(d->BrushPixelModeCheckbox, SIGNAL(clicked()), this, SLOT(updateMRMLFromGUI()));
  QObject::connect(d->BrushDiameterSlider, SIGNAL(valueChanged(double)), d, SLOT(onDiameterValueChanged(double)));
  QObject::connect(d->BrushDiameterSpinBox, SIGNAL(valueChanged(double)), d, SLOT(onDiameterValueChanged(double)));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorPaintEffect::setMRMLDefaults()
{
  Superclass::setMRMLDefaults();

  this->setCommonParameterDefault("BrushMinimumAbsoluteDiameter", 0.01);
  this->setCommonParameterDefault("BrushMaximumAbsoluteDiameter", 100.0);
  this->setCommonParameterDefault("BrushAbsoluteDiameter", 5.0);
  this->setCommonParameterDefault("BrushRelativeDiameter", 3.0);
  this->setCommonParameterDefault("BrushDiameterIsRelative", 1);
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

  // Diameter is also disabled if pixel mode is on
  d->BrushDiameterFrame->setEnabled(!pixelMode);

  bool brushDiameterIsRelative = this->integerParameter("BrushDiameterIsRelative");

  d->BrushDiameterRelativeToggle->blockSignals(true);
  d->BrushDiameterRelativeToggle->setText(brushDiameterIsRelative ? "%" : "mm");
  d->BrushDiameterRelativeToggle->blockSignals(false);

  d->BrushDiameterSlider->blockSignals(true);
  if (brushDiameterIsRelative)
    {
    d->BrushDiameterSlider->setMinimum(1);
    d->BrushDiameterSlider->setMaximum(25);
    d->BrushDiameterSlider->setValue(this->doubleParameter("BrushRelativeDiameter"));
    d->BrushDiameterSlider->setSingleStep(1);
    }
  else
    {
    d->BrushDiameterSlider->setMinimum(this->doubleParameter("BrushMinimumAbsoluteDiameter"));
    d->BrushDiameterSlider->setMaximum(this->doubleParameter("BrushMaximumAbsoluteDiameter"));
    d->BrushDiameterSlider->setValue(this->doubleParameter("BrushAbsoluteDiameter"));
    d->BrushDiameterSlider->setSingleStep(this->doubleParameter("BrushMinimumAbsoluteDiameter"));
    }
  d->BrushDiameterSlider->blockSignals(false);


  d->BrushDiameterSpinBox->blockSignals(true);
  d->BrushDiameterSpinBox->setMRMLScene(this->scene());
  d->BrushDiameterSpinBox->setMinimum(d->BrushDiameterSlider->minimum());
  d->BrushDiameterSpinBox->setMaximum(d->BrushDiameterSlider->maximum());
  d->BrushDiameterSpinBox->setValue(d->BrushDiameterSlider->value());
  if (brushDiameterIsRelative)
    {
    d->BrushDiameterSpinBox->setQuantity("");
    d->BrushDiameterSpinBox->setSuffix("%");
    d->BrushDiameterSpinBox->setDecimals(0);
    }
  else
    {
    d->BrushDiameterSpinBox->setQuantity("length");
    d->BrushDiameterSpinBox->setUnitAwareProperties(qMRMLSpinBox::Prefix | qMRMLSpinBox::Suffix);
    int decimals = (int)(log10(d->BrushDiameterSlider->minimum()));
    if (decimals < 0)
      {
      d->BrushDiameterSpinBox->setDecimals(-decimals * 2);
      }
    }
  d->BrushDiameterSpinBox->blockSignals(false);

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

  bool isBrushDiameterRelative = (d->BrushDiameterRelativeToggle->text() == "%");
  this->setCommonParameter("BrushDiameterIsRelative", isBrushDiameterRelative ? 1 : 0);
  if (isBrushDiameterRelative)
    {
    this->setCommonParameter("BrushRelativeDiameter", d->BrushDiameterSlider->value());
    }
  else
    {
    this->setCommonParameter("BrushAbsoluteDiameter", d->BrushDiameterSlider->value());
    }

  // If pixel mode changed, then other GUI changes are due
  if (pixelModeChanged)
    {
    // Update label options based on constraints set by pixel mode
    Superclass::updateGUIFromMRML();

    d->BrushDiameterFrame->setEnabled(!pixelMode);
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
  double minimumDiameter = 0.5 * minimumSpacing;

  int dimensions[3] = {0, 0, 0};
  referenceGeometryImage->GetDimensions(dimensions);
  double bounds[3] = {spacing[0]*dimensions[0], spacing[1]*dimensions[1], spacing[2]*dimensions[2]};
  double maximumBounds = qMax(bounds[0], qMax(bounds[1], bounds[2]));
  double maximumDiameter = 0.5 * maximumBounds;

  this->setCommonParameter("BrushAbsoluteMinimumDiameter", minimumDiameter);
  this->setCommonParameter("BrushAbsoluteMaximumDiameter", maximumDiameter);
  this->setCommonParameter("BrushAbsoluteDiameter", qMin(50.0 * minimumDiameter, 0.5 * maximumDiameter));

  this->updateGUIFromMRML();
}
