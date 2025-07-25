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
#include "qSlicerSegmentEditorScissorsEffect.h"

#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkMRMLSegmentEditorNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"

// Qt includes
#include <QApplication>
#include <QButtonGroup>
#include <QDebug>
#include <QComboBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>

// VTK includes
#include <vtkActor2D.h>
#include <vtkAppendPolyData.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCubeSource.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageStencilData.h>
#include <vtkImageStencilToImage.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVector.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkEventBroker.h>
#include "qMRMLSliceView.h"
#include "qMRMLSpinBox.h"
#include "qMRMLThreeDView.h"

// Slicer includes
#include "qMRMLSliceWidget.h"
#include "qMRMLThreeDWidget.h"
#include "vtkMRMLSliceLayerLogic.h"
#include "vtkMRMLSliceLogic.h"

// TODO: temporary code, only for debugging
// This will be removed once investigation of https://github.com/Slicer/Slicer/issues/6705 is completed.
#include "vtkXMLPolyDataWriter.h"
QString qSlicerSegmentEditorScissorsEffect::DebugOutputFolder;
void DebugWritePolyData(vtkPolyData* poly)
{
  if (qSlicerSegmentEditorScissorsEffect::DebugOutputFolder.isEmpty())
  {
    return;
  }
  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetInputData(poly);
  QString filepath = qSlicerSegmentEditorScissorsEffect::DebugOutputFolder + "/DebugScissorBrush.vtp";
  writer->SetFileName(filepath.toStdString().c_str());
  writer->SetCompressorTypeToNone();
  writer->SetDataModeToAscii();
  writer->Write();
}

//-----------------------------------------------------------------------------
/// Visualization objects and pipeline for each slice view for drawing cutting outline
class ScissorsPipeline : public QObject
{
public:
  ScissorsPipeline()
  {
    this->IsDragging = false;
    this->PolyData = vtkSmartPointer<vtkPolyData>::New();
    this->Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->Mapper->SetInputData(this->PolyData);
    this->Actor = vtkSmartPointer<vtkActor2D>::New();
    this->Actor->SetMapper(this->Mapper);
    this->Actor->VisibilityOff();
    vtkProperty2D* outlineProperty = this->Actor->GetProperty();
    outlineProperty->SetColor(1, 1, 0);
    outlineProperty->SetLineWidth(2);

    this->PolyDataThin = vtkSmartPointer<vtkPolyData>::New();
    this->MapperThin = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->MapperThin->SetInputData(this->PolyDataThin);
    this->ActorThin = vtkSmartPointer<vtkActor2D>::New();
    this->ActorThin->SetMapper(this->MapperThin);
    this->ActorThin->VisibilityOff();
    vtkProperty2D* outlinePropertyThin = this->ActorThin->GetProperty();
    // Make the line a bit thinner and darker to distinguish from the thick line
    outlinePropertyThin->SetColor(0.7, 0.7, 0);
    outlinePropertyThin->SetLineStipplePattern(0xff00); // Note: line stipple may not be supported in VTK OpenGL2 backend
    outlinePropertyThin->SetLineWidth(1);
  };
  ~ScissorsPipeline() override = default;

public:
  bool IsDragging;
  vtkSmartPointer<vtkActor2D> Actor;
  vtkSmartPointer<vtkPolyDataMapper2D> Mapper;
  vtkSmartPointer<vtkPolyData> PolyData;
  vtkSmartPointer<vtkActor2D> ActorThin;
  vtkSmartPointer<vtkPolyDataMapper2D> MapperThin;
  vtkSmartPointer<vtkPolyData> PolyDataThin;
};

//-----------------------------------------------------------------------------
class qSlicerSegmentEditorScissorsEffectPrivate : public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSegmentEditorScissorsEffect);

protected:
  qSlicerSegmentEditorScissorsEffect* const q_ptr;

public:
  qSlicerSegmentEditorScissorsEffectPrivate(qSlicerSegmentEditorScissorsEffect& object);
  ~qSlicerSegmentEditorScissorsEffectPrivate() override;

public:
  enum
  {
    ShapeFreeForm,
    ShapeCircle,
    ShapeRectangle,
    Shape_Last // must be the last item
  };

  enum
  {
    OperationEraseInside,
    OperationEraseOutside,
    OperationFillInside,
    OperationFillOutside,
    Operation_Last // must be the last item
  };

  enum
  {
    SliceCutModeUnlimited,
    SliceCutModePositive,
    SliceCutModeNegative,
    SliceCutModeSymmetric,
    SliceCutMode_Last // must be the last item
  };

  /// Draw outline glyph
  void createGlyph(ScissorsPipeline* pipeline, const vtkVector2i& eventPosition);
  /// Update outline glyph based on positions
  void updateGlyphWithNewPosition(ScissorsPipeline* pipeline, const vtkVector2i& eventPosition, bool finalize);
  void finalizeGlyph(ScissorsPipeline* pipeline, const vtkVector2i& eventPosition);
  /// Update 3D mesh of the brush
  bool updateBrushModel(qMRMLWidget* viewWidget);
  /// Update image stencil from mesh
  bool updateBrushStencil(qMRMLWidget* viewWidget);
  /// Paint brush into segment
  void paintApply(qMRMLWidget* viewWidget);

  static int ConvertShapeFromString(const QString& shapeStr);
  static int ConvertOperationFromString(const QString& operationStr);
  static int ConvertSliceCutModeFromString(const QString& sliceCutModeStr);
  static QString ConvertShapeToString(int shape);
  static QString ConvertOperationToString(int operation);
  static QString ConvertSliceCutModeToString(int operation);

  bool operationErase();
  bool operationInside();
  int shape();
  int shapeDrawCentered();

protected:
  /// Get pipeline object for widget. Create if does not exist
  ScissorsPipeline* scissorsPipelineForWidget(qMRMLWidget* viewWidget);
  /// Delete all scissors pipelines
  void clearScissorsPipelines();

public:
  QIcon ScissorsIcon;

  vtkVector2i DragStartPosition;
  int CircleNumberOfPoints;

  vtkSmartPointer<vtkPoints> PaintCoordinates_View;

  vtkSmartPointer<vtkPolyDataNormals> BrushPolyDataNormals;
  vtkSmartPointer<vtkTransformPolyDataFilter> WorldToModifierLabelmapIjkTransformer;
  // transforms from polydata source to modifierLabelmap's IJK coordinate system (brush origin in IJK origin)
  vtkSmartPointer<vtkTransform> WorldToModifierLabelmapIjkTransform;
  vtkSmartPointer<vtkPolyDataToImageStencil> BrushPolyDataToStencil;

  QMap<qMRMLWidget*, ScissorsPipeline*> ScissorsPipelines;

  QGridLayout* gridLayout;

  QRadioButton* eraseInsideRadioButton;
  QRadioButton* eraseOutsideRadioButton;
  QRadioButton* fillInsideRadioButton;
  QRadioButton* fillOutsideRadioButton;
  QButtonGroup* operationGroup;

  QRadioButton* freeFormRadioButton;
  QRadioButton* circleRadioButton;
  QRadioButton* rectangleRadioButton;
  QButtonGroup* shapeGroup;
  QCheckBox* shapeDrawCenteredCheckBox;

  QRadioButton* unlimitedRadioButton;
  QRadioButton* positiveRadioButton;
  QRadioButton* negativeRadioButton;
  QRadioButton* symmetricRadioButton;
  QButtonGroup* sliceCutModeGroup;

  qMRMLSpinBox* sliceCutDepthSpinBox;
  QCheckBox* applyToAllVisibleSegmentsCheckBox;
};

//-----------------------------------------------------------------------------
qSlicerSegmentEditorScissorsEffectPrivate::qSlicerSegmentEditorScissorsEffectPrivate(qSlicerSegmentEditorScissorsEffect& object)
  : q_ptr(&object)
{
  this->ScissorsIcon = QIcon(":Icons/Medium/SlicerEditCut.png");
  this->CircleNumberOfPoints = 36;
  this->PaintCoordinates_View = vtkSmartPointer<vtkPoints>::New();

  this->BrushPolyDataNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
  this->BrushPolyDataNormals->AutoOrientNormalsOn();

  this->WorldToModifierLabelmapIjkTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->WorldToModifierLabelmapIjkTransform = vtkSmartPointer<vtkTransform>::New();
  this->WorldToModifierLabelmapIjkTransformer->SetTransform(this->WorldToModifierLabelmapIjkTransform);
  this->WorldToModifierLabelmapIjkTransformer->SetInputConnection(this->BrushPolyDataNormals->GetOutputPort());
  this->BrushPolyDataToStencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
  this->BrushPolyDataToStencil->SetOutputSpacing(1.0, 1.0, 1.0);
  this->BrushPolyDataToStencil->SetInputConnection(this->WorldToModifierLabelmapIjkTransformer->GetOutputPort());
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorScissorsEffectPrivate::~qSlicerSegmentEditorScissorsEffectPrivate()
{
  this->clearScissorsPipelines();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffectPrivate::clearScissorsPipelines()
{
  Q_Q(qSlicerSegmentEditorScissorsEffect);
  QMapIterator<qMRMLWidget*, ScissorsPipeline*> it(this->ScissorsPipelines);
  while (it.hasNext())
  {
    it.next();
    qMRMLWidget* viewWidget = it.key();
    ScissorsPipeline* pipeline = it.value();
    q->removeActor2D(viewWidget, pipeline->Actor);
    q->removeActor2D(viewWidget, pipeline->ActorThin);
    delete pipeline;
  }
  this->ScissorsPipelines.clear();
}

//-----------------------------------------------------------------------------
ScissorsPipeline* qSlicerSegmentEditorScissorsEffectPrivate::scissorsPipelineForWidget(qMRMLWidget* viewWidget)
{
  Q_Q(qSlicerSegmentEditorScissorsEffect);

  if (this->ScissorsPipelines.contains(viewWidget))
  {
    return this->ScissorsPipelines[viewWidget];
  }

  // Create pipeline if does not yet exist
  ScissorsPipeline* pipeline = new ScissorsPipeline();
  vtkVector2i eventPosition;
  this->createGlyph(pipeline, eventPosition);

  vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(viewWidget);
  if (!renderer)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get renderer!";
  }
  else
  {
    q->addActor2D(viewWidget, pipeline->Actor);
    q->addActor2D(viewWidget, pipeline->ActorThin);
  }

  this->ScissorsPipelines[viewWidget] = pipeline;
  return pipeline;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffectPrivate::createGlyph(ScissorsPipeline* pipeline, const vtkVector2i& eventPosition)
{
  if (pipeline->IsDragging)
  {
    int numberOfPoints = 0;
    switch (this->shape())
    {
      case ShapeRectangle: numberOfPoints = 4; break;
      case ShapeCircle: numberOfPoints = this->CircleNumberOfPoints; break;
      case ShapeFreeForm: numberOfPoints = 1; break;
    }
    this->DragStartPosition = eventPosition;

    pipeline->PolyData->Initialize();
    pipeline->PolyDataThin->Initialize();

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    pipeline->PolyData->SetPoints(points);
    pipeline->PolyData->SetLines(lines);

    int previousPointIndex = -1;
    int firstPointIndex = -1;
    int pointIndex = -1;
    for (int corner = 0; corner < numberOfPoints; ++corner)
    {
      pointIndex = points->InsertNextPoint(eventPosition[0], eventPosition[1], 0.0);
      if (previousPointIndex != -1)
      {
        vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
        idList->InsertNextId(previousPointIndex);
        idList->InsertNextId(pointIndex);
        pipeline->PolyData->InsertNextCell(VTK_LINE, idList);
      }
      previousPointIndex = pointIndex;
      if (firstPointIndex == -1)
      {
        firstPointIndex = pointIndex;
      }
    }

    // Make the last line in the polydata (connects first and last point)
    // Free-form shape uses a separate actor to show it with a thinner line
    if (this->shape() == ShapeFreeForm)
    {
      // pipeline->PolyDataThin contains the thin closing line (from the first to last point)
      vtkSmartPointer<vtkPoints> pointsThin = vtkSmartPointer<vtkPoints>::New();
      vtkSmartPointer<vtkCellArray> linesThin = vtkSmartPointer<vtkCellArray>::New();
      pipeline->PolyDataThin->SetPoints(pointsThin);
      pipeline->PolyDataThin->SetLines(linesThin);

      pointsThin->InsertNextPoint(eventPosition[0], eventPosition[1], 0.0);
      pointsThin->InsertNextPoint(eventPosition[0], eventPosition[1], 0.0);

      vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
      idList->InsertNextId(0);
      idList->InsertNextId(1);
      pipeline->PolyDataThin->InsertNextCell(VTK_LINE, idList);

      pipeline->ActorThin->VisibilityOn();
    }
    else
    {
      vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
      idList->InsertNextId(pointIndex);
      idList->InsertNextId(firstPointIndex);
      pipeline->PolyData->InsertNextCell(VTK_LINE, idList);
      pipeline->ActorThin->VisibilityOff();
    }

    pipeline->Actor->VisibilityOn();
  }
  else
  {
    pipeline->Actor->VisibilityOff();
    pipeline->ActorThin->VisibilityOff();
  }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffectPrivate::updateGlyphWithNewPosition(ScissorsPipeline* pipeline, const vtkVector2i& eventPosition, bool finalize)
{
  if (pipeline->IsDragging)
  {
    vtkPoints* points = pipeline->PolyData->GetPoints();
    int numberOfPoints = points->GetNumberOfPoints();
    switch (this->shape())
    {
      case ShapeRectangle:
      {
        if (numberOfPoints != 4)
        {
          qWarning() << Q_FUNC_INFO << "Rectangle glyph is expected to have 4 points";
          break;
        }

        if (this->shapeDrawCentered())
        {
          int halfWidth = std::abs(eventPosition[0] - this->DragStartPosition[0]);
          int halfHeight = std::abs(eventPosition[1] - this->DragStartPosition[1]);
          points->SetPoint(0, this->DragStartPosition[0] - halfWidth, this->DragStartPosition[1] - halfHeight, 0.0);
          points->SetPoint(1, this->DragStartPosition[0] + halfWidth, this->DragStartPosition[1] - halfHeight, 0.0);
          points->SetPoint(2, this->DragStartPosition[0] + halfWidth, this->DragStartPosition[1] + halfHeight, 0.0);
          points->SetPoint(3, this->DragStartPosition[0] - halfWidth, this->DragStartPosition[1] + halfHeight, 0.0);
        }
        else
        {
          points->SetPoint(1, this->DragStartPosition[0], eventPosition[1], 0.0);
          points->SetPoint(2, eventPosition[0], eventPosition[1], 0.0);
          points->SetPoint(3, eventPosition[0], this->DragStartPosition[1], 0.0);
        }
        points->Modified();
        break;
      }
      case ShapeCircle:
      {
        if (numberOfPoints < 4)
        {
          qWarning() << Q_FUNC_INFO << "Circle glyph is expected to have at least 4 points";
          break;
        }
        double radius = sqrt((eventPosition[0] - this->DragStartPosition[0]) * (eventPosition[0] - this->DragStartPosition[0])
                             + (eventPosition[1] - this->DragStartPosition[1]) * (eventPosition[1] - this->DragStartPosition[1]));
        double position[3] = { 0 };
        for (int i = 0; i < numberOfPoints; i++)
        {
          double angle = 2.0 * vtkMath::Pi() * i / double(numberOfPoints);
          if (this->shapeDrawCentered())
          {
            position[0] = this->DragStartPosition[0] + radius * sin(angle);
            position[1] = this->DragStartPosition[1] + radius * cos(angle);
          }
          else
          {
            position[0] = (eventPosition[0] + this->DragStartPosition[0]) / 2 + radius / 2 * sin(angle);
            position[1] = (eventPosition[1] + this->DragStartPosition[1]) / 2 + radius / 2 * cos(angle);
          }
          points->SetPoint(i, position);
        }
        points->Modified();
        break;
      }
      case ShapeFreeForm:
      {
        if (numberOfPoints < 1)
        {
          qWarning() << Q_FUNC_INFO << "Free-form glyph is expected to have at least 1 point";
          break;
        }
        vtkIdType newPointId = points->InsertNextPoint(eventPosition[0], eventPosition[1], 0.0);
        vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
        if (finalize)
        {
          idList->InsertNextId(newPointId);
          idList->InsertNextId(0);
        }
        else
        {
          idList->InsertNextId(newPointId - 1);
          idList->InsertNextId(newPointId);
        }
        pipeline->PolyData->InsertNextCell(VTK_LINE, idList);
        points->Modified();
        pipeline->PolyDataThin->GetPoints()->SetPoint(1, eventPosition[0], eventPosition[1], 0.0);
        pipeline->PolyDataThin->GetPoints()->Modified();
        break;
      }
    }
  }
  else
  {
    pipeline->Actor->VisibilityOff();
    pipeline->ActorThin->VisibilityOff();
  }
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorScissorsEffectPrivate::updateBrushModel(qMRMLWidget* viewWidget)
{
  Q_Q(qSlicerSegmentEditorScissorsEffect);
  ScissorsPipeline* pipeline = this->scissorsPipelineForWidget(viewWidget);
  if (!pipeline)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get pipeline";
    return false;
  }

  vtkPoints* pointsXY = pipeline->PolyData->GetPoints();
  int numberOfPoints = pointsXY ? pointsXY->GetNumberOfPoints() : 0;
  if (numberOfPoints < 3)
  {
    // at least a triangle is needed
    return false;
  }

  // Get bounds of modifier labelmap
  if (!q->parameterSetNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return false;
  }
  vtkMRMLSegmentationNode* segmentationNode = q->parameterSetNode()->GetSegmentationNode();
  if (!segmentationNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentationNode";
    return false;
  }
  vtkOrientedImageData* modifierLabelmap = q->modifierLabelmap();
  if (!modifierLabelmap)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid modifierLabelmap";
    return false;
  }
  vtkNew<vtkMatrix4x4> segmentationToWorldMatrix;
  // We don't support painting in non-linearly transformed node (it could be implemented, but would probably slow down things too much)
  // TODO: show a meaningful error message to the user if attempted
  vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(segmentationNode->GetParentTransformNode(), nullptr, segmentationToWorldMatrix.GetPointer());

  vtkNew<vtkPoints> closedSurfacePoints; // p0Top, p0Bottom, p1Top, p1Bottom, ...

  // Used for excluding one side of the slice plane from being modified when
  // filling or erasing outside in slice cut mode.
  vtkSmartPointer<vtkPolyData> additionalBrushRegion;

  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget)
  {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(qSlicerSegmentEditorAbstractEffect::viewNode(sliceWidget));
    if (!sliceNode)
    {
      qCritical() << Q_FUNC_INFO << ": Failed to get slice node";
      return false;
    }

    // Get modifier labelmap extent in slice coordinate system to know how much we
    // have to cut through
    vtkNew<vtkTransform> segmentationToSliceXYTransform;
    vtkNew<vtkMatrix4x4> worldToSliceXYMatrix;
    vtkMatrix4x4::Invert(sliceNode->GetXYToRAS(), worldToSliceXYMatrix.GetPointer());
    segmentationToSliceXYTransform->Concatenate(worldToSliceXYMatrix.GetPointer());
    segmentationToSliceXYTransform->Concatenate(segmentationToWorldMatrix.GetPointer());
    double segmentationBounds_SliceXY[6] = { 0, -1, 0, -1, 0, -1 };
    double originalSegmentationBounds_SliceXY[6] = { 0, -1, 0, -1, 0, -1 };
    vtkOrientedImageDataResample::TransformOrientedImageDataBounds(modifierLabelmap, segmentationToSliceXYTransform.GetPointer(), segmentationBounds_SliceXY);
    vtkOrientedImageDataResample::TransformOrientedImageDataBounds(modifierLabelmap, segmentationToSliceXYTransform.GetPointer(), originalSegmentationBounds_SliceXY);
    // Extend bounds by half slice to make sure the boundaries are included
    int sliceCutMode = this->ConvertSliceCutModeFromString(q->parameter("SliceCutMode"));
    switch (sliceCutMode)
    {
      case SliceCutModePositive: segmentationBounds_SliceXY[4] = 0; break;
      case SliceCutModeNegative: segmentationBounds_SliceXY[5] = 0; break;
      case SliceCutModeSymmetric:
      {
        vtkNew<vtkMatrix4x4> sliceXYToSegmentationTransform;
        vtkMatrix4x4::Invert(segmentationToSliceXYTransform->GetMatrix(), sliceXYToSegmentationTransform.GetPointer());
        double sliceNormalVector_SliceXY[4] = { 0, 0, 1, 0 };
        double sliceNormalVector_World[4] = { 0, 0, 1, 0 };
        sliceXYToSegmentationTransform->MultiplyPoint(sliceNormalVector_SliceXY, sliceNormalVector_World);
        double sliceThicknessMmPerPixel = vtkMath::Norm(sliceNormalVector_World);
        double sliceCutDepthMm = q->doubleParameter("SliceCutDepthMm");
        double halfSliceCutDepthPixel = sliceCutDepthMm / sliceThicknessMmPerPixel / 2.0;
        if (halfSliceCutDepthPixel < 0.5)
        {
          // include at least the current slice
          halfSliceCutDepthPixel = 0.5;
        }
        segmentationBounds_SliceXY[4] = -halfSliceCutDepthPixel;
        segmentationBounds_SliceXY[5] = halfSliceCutDepthPixel;
      }
      break;
      default:
        // unlimited
        break;
    }
    if (sliceCutMode != SliceCutModeSymmetric)
    {
      // Add half slice to make sure the current slice and the last slice are fully included
      double brushZEpsilon = 0.0; // small offset to make have main and additional brush planes very close but not coincident
      if (segmentationBounds_SliceXY[4] < segmentationBounds_SliceXY[5])
      {
        segmentationBounds_SliceXY[4] -= 0.5;
        segmentationBounds_SliceXY[5] += 0.5;
        brushZEpsilon = 0.001;
      }
      else
      {
        segmentationBounds_SliceXY[4] += 0.5;
        segmentationBounds_SliceXY[5] -= 0.5;
        brushZEpsilon = -0.001;
      }
      if (!this->operationInside())
      {
        // Make sure the non-selected side of the plane is unaffected
        // when working "outside"
        vtkNew<vtkCubeSource> cube;
        switch (sliceCutMode)
        {
          case SliceCutModePositive:
            cube->SetBounds(segmentationBounds_SliceXY[0],
                            segmentationBounds_SliceXY[1],
                            segmentationBounds_SliceXY[2],
                            segmentationBounds_SliceXY[3],
                            originalSegmentationBounds_SliceXY[4],
                            segmentationBounds_SliceXY[4] - brushZEpsilon);
            break;
          case SliceCutModeNegative:
            cube->SetBounds(segmentationBounds_SliceXY[0],
                            segmentationBounds_SliceXY[1],
                            segmentationBounds_SliceXY[2],
                            segmentationBounds_SliceXY[3],
                            segmentationBounds_SliceXY[5] + brushZEpsilon,
                            originalSegmentationBounds_SliceXY[5]);
            break;
        }
        vtkNew<vtkTransformPolyDataFilter> transformToRasFilter;
        vtkNew<vtkTransform> transformToRas;
        transformToRas->SetMatrix(sliceNode->GetXYToRAS());
        transformToRasFilter->SetTransform(transformToRas.GetPointer());
        transformToRasFilter->SetInputConnection(cube->GetOutputPort());
        transformToRasFilter->Update();
        additionalBrushRegion = transformToRasFilter->GetOutput();
      }
    }

    for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++)
    {
      double pointXY[4] = { 0., 0., 0., 1. };
      double pointWorld[4] = { 0., 0., 0., 1. };
      pointsXY->GetPoint(pointIndex, pointXY);

      pointXY[2] = segmentationBounds_SliceXY[4];
      sliceNode->GetXYToRAS()->MultiplyPoint(pointXY, pointWorld);
      closedSurfacePoints->InsertNextPoint(pointWorld);

      pointXY[2] = segmentationBounds_SliceXY[5];
      sliceNode->GetXYToRAS()->MultiplyPoint(pointXY, pointWorld);
      closedSurfacePoints->InsertNextPoint(pointWorld);
    }
  }
  else if (threeDWidget)
  {
    vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(threeDWidget);
    if (!renderer)
    {
      qCritical() << Q_FUNC_INFO << ": Failed to get renderer";
      return false;
    }
    vtkCamera* camera = renderer->GetActiveCamera();
    if (!camera)
    {
      qCritical() << Q_FUNC_INFO << ": Failed to get camera";
      return false;
    }

    // Camera parameters
    // Camera position
    double cameraPos[4] = { 0 };
    camera->GetPosition(cameraPos);
    cameraPos[3] = 1.0;
    // Focal point position
    double cameraFP[4] = { 0 };
    camera->GetFocalPoint(cameraFP);
    cameraFP[3] = 1.0;
    // Direction of projection
    double cameraDOP[3] = { 0 };
    for (int i = 0; i < 3; i++)
    {
      cameraDOP[i] = cameraFP[i] - cameraPos[i];
    }
    vtkMath::Normalize(cameraDOP);
    // Camera view up
    double cameraViewUp[3] = { 0 };
    camera->GetViewUp(cameraViewUp);
    vtkMath::Normalize(cameraViewUp);

    renderer->SetWorldPoint(cameraFP[0], cameraFP[1], cameraFP[2], cameraFP[3]);
    renderer->WorldToDisplay();
    double* displayCoords = renderer->GetDisplayPoint();
    double selectionZ = displayCoords[2];

    // Get modifier labelmap extent in camera coordinate system to know how much we
    // have to cut through
    vtkNew<vtkMatrix4x4> cameraToWorldMatrix;
    double cameraViewRight[3] = { 1, 0, 0 };
    vtkMath::Cross(cameraDOP, cameraViewUp, cameraViewRight);
    for (int i = 0; i < 3; i++)
    {
      cameraToWorldMatrix->SetElement(i, 3, cameraPos[i]);
      cameraToWorldMatrix->SetElement(i, 0, cameraViewUp[i]);
      cameraToWorldMatrix->SetElement(i, 1, cameraViewRight[i]);
      cameraToWorldMatrix->SetElement(i, 2, cameraDOP[i]);
    }
    vtkNew<vtkMatrix4x4> worldToCameraMatrix;
    vtkMatrix4x4::Invert(cameraToWorldMatrix.GetPointer(), worldToCameraMatrix.GetPointer());
    vtkNew<vtkTransform> segmentationToCameraTransform;
    segmentationToCameraTransform->Concatenate(worldToCameraMatrix.GetPointer());
    segmentationToCameraTransform->Concatenate(segmentationToWorldMatrix.GetPointer());
    double segmentationBounds_Camera[6] = { 0, -1, 0, -1, 0, -1 };
    vtkOrientedImageDataResample::TransformOrientedImageDataBounds(modifierLabelmap, segmentationToCameraTransform.GetPointer(), segmentationBounds_Camera);
    double clipRangeFromModifierLabelmap[2] = { std::min(segmentationBounds_Camera[4], segmentationBounds_Camera[5]),
                                                std::max(segmentationBounds_Camera[4], segmentationBounds_Camera[5]) };
    // Extend bounds by half slice to make sure the boundaries are included
    clipRangeFromModifierLabelmap[0] -= 0.5;
    clipRangeFromModifierLabelmap[1] += 0.5;

    // Clip what we see on the camera but reduce it to the modifier labelmap's range
    // to keep the stencil as small as possible
    double* clipRangeFromCamera = camera->GetClippingRange();
    double clipRange[2] = {
      std::max(clipRangeFromModifierLabelmap[0], clipRangeFromCamera[0]),
      std::min(clipRangeFromModifierLabelmap[1], clipRangeFromCamera[1]),
    };

    for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++)
    {
      // Convert the selection point into world coordinates.
      //
      double pointXY[3] = { 0., 0., 0. };
      pointsXY->GetPoint(pointIndex, pointXY);
      renderer->SetDisplayPoint(pointXY[0], pointXY[1], selectionZ);
      renderer->DisplayToWorld();
      double* worldCoords = renderer->GetWorldPoint();
      if (worldCoords[3] == 0.0)
      {
        qWarning() << Q_FUNC_INFO << ": Bad homogeneous coordinates";
        return false;
      }
      double pickPosition[3] = { 0 };
      for (int i = 0; i < 3; i++)
      {
        pickPosition[i] = worldCoords[i] / worldCoords[3];
      }

      //  Compute the ray endpoints.  The ray is along the line running from
      //  the camera position to the selection point, starting where this line
      //  intersects the front clipping plane, and terminating where this
      //  line intersects the back clipping plane.
      double ray[3] = { 0 };
      for (int i = 0; i < 3; i++)
      {
        ray[i] = pickPosition[i] - cameraPos[i];
      }

      double rayLength = vtkMath::Dot(cameraDOP, ray);
      if (rayLength == 0.0)
      {
        qWarning() << Q_FUNC_INFO << ": Cannot process points";
        return false;
      }

      double p1World[4] = { 0 };
      double p2World[4] = { 0 };
      double tF = 0;
      double tB = 0;
      if (camera->GetParallelProjection())
      {
        tF = clipRange[0] - rayLength;
        tB = clipRange[1] - rayLength;
        for (int i = 0; i < 3; i++)
        {
          p1World[i] = pickPosition[i] + tF * cameraDOP[i];
          p2World[i] = pickPosition[i] + tB * cameraDOP[i];
        }
      }
      else
      {
        tF = clipRange[0] / rayLength;
        tB = clipRange[1] / rayLength;
        for (int i = 0; i < 3; i++)
        {
          p1World[i] = cameraPos[i] + tF * ray[i];
          p2World[i] = cameraPos[i] + tB * ray[i];
        }
      }
      p1World[3] = p2World[3] = 1.0;

      closedSurfacePoints->InsertNextPoint(p1World);
      closedSurfacePoints->InsertNextPoint(p2World);
    }
  }
  else
  {
    return false;
  }

  // Skirt
  vtkNew<vtkCellArray> closedSurfacePolys;
  vtkNew<vtkCellArray> closedSurfaceStrips;
  closedSurfaceStrips->InsertNextCell(numberOfPoints * 2 + 2);
  for (int i = 0; i < numberOfPoints * 2; i++)
  {
    closedSurfaceStrips->InsertCellPoint(i);
  }
  closedSurfaceStrips->InsertCellPoint(0);
  closedSurfaceStrips->InsertCellPoint(1);
  // Front cap
  closedSurfacePolys->InsertNextCell(numberOfPoints);
  for (int i = 0; i < numberOfPoints; i++)
  {
    closedSurfacePolys->InsertCellPoint(i * 2);
  }
  // Back cap
  closedSurfacePolys->InsertNextCell(numberOfPoints);
  for (int i = 0; i < numberOfPoints; i++)
  {
    closedSurfacePolys->InsertCellPoint(i * 2 + 1);
  }

  // Construct polydata
  vtkNew<vtkPolyData> closedSurfacePolyData;
  closedSurfacePolyData->SetPoints(closedSurfacePoints.GetPointer());
  closedSurfacePolyData->SetStrips(closedSurfaceStrips.GetPointer());
  closedSurfacePolyData->SetPolys(closedSurfacePolys.GetPointer());

  if (additionalBrushRegion)
  {
    vtkNew<vtkAppendPolyData> append;
    append->AddInputData(closedSurfacePolyData.GetPointer());
    append->AddInputData(additionalBrushRegion);
    this->BrushPolyDataNormals->SetInputConnection(append->GetOutputPort());

    // TODO: temporary code, only for debugging
    // This will be removed once investigation of https://github.com/Slicer/Slicer/issues/6705 is completed.
    append->Update();
    DebugWritePolyData(append->GetOutput());
  }
  else
  {
    this->BrushPolyDataNormals->SetInputData(closedSurfacePolyData.GetPointer());

    // TODO: temporary code, only for debugging
    // This will be removed once investigation of https://github.com/Slicer/Slicer/issues/6705 is completed.
    DebugWritePolyData(closedSurfacePolyData);
  }

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorScissorsEffectPrivate::updateBrushStencil(qMRMLWidget* viewWidget)
{
  Q_Q(qSlicerSegmentEditorScissorsEffect);
  Q_UNUSED(viewWidget);

  if (!q->parameterSetNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return false;
  }
  vtkMRMLSegmentationNode* segmentationNode = q->parameterSetNode()->GetSegmentationNode();
  if (!segmentationNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentationNode";
    return false;
  }
  vtkOrientedImageData* modifierLabelmap = q->modifierLabelmap();
  if (!modifierLabelmap)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid modifierLabelmap";
    return false;
  }

  // Brush stencil transform

  this->WorldToModifierLabelmapIjkTransform->Identity();

  vtkNew<vtkMatrix4x4> segmentationToSegmentationIjkTransformMatrix;
  modifierLabelmap->GetWorldToImageMatrix(segmentationToSegmentationIjkTransformMatrix.GetPointer());
  this->WorldToModifierLabelmapIjkTransform->Concatenate(segmentationToSegmentationIjkTransformMatrix.GetPointer());

  vtkNew<vtkMatrix4x4> worldToSegmentationTransformMatrix;
  // We don't support painting in non-linearly transformed node (it could be implemented, but would probably slow down things too much)
  // TODO: show a meaningful error message to the user if attempted
  vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(nullptr, segmentationNode->GetParentTransformNode(), worldToSegmentationTransformMatrix.GetPointer());
  this->WorldToModifierLabelmapIjkTransform->Concatenate(worldToSegmentationTransformMatrix.GetPointer());

  this->WorldToModifierLabelmapIjkTransformer->Update();
  if (this->operationInside())
  {
    // Clip modifier labelmap to non-null region to make labelmap modification faster later
    vtkPolyData* brushModel_ModifierLabelmapIjk = this->WorldToModifierLabelmapIjkTransformer->GetOutput();
    double* boundsIjk = brushModel_ModifierLabelmapIjk->GetBounds();
    this->BrushPolyDataToStencil->SetOutputWholeExtent(
      floor(boundsIjk[0]) - 1, ceil(boundsIjk[1]) + 1, floor(boundsIjk[2]) - 1, ceil(boundsIjk[3]) + 1, floor(boundsIjk[4]) - 1, ceil(boundsIjk[5]) + 1);
  }
  else
  {
    this->BrushPolyDataToStencil->SetOutputWholeExtent(modifierLabelmap->GetExtent());
  }
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffectPrivate::paintApply(qMRMLWidget* viewWidget)
{
  Q_Q(qSlicerSegmentEditorScissorsEffect);

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

  if (!this->updateBrushModel(viewWidget))
  {
    return;
  }
  if (!this->updateBrushStencil(viewWidget))
  {
    return;
  }

  // Create a list of segment IDs that will be processed.
  std::string selectedSegmentID = q->parameterSetNode()->GetSelectedSegmentID() ? q->parameterSetNode()->GetSelectedSegmentID() : "";
  vtkNew<vtkStringArray> allProcessedSegmentIDs;
  bool applyToAllVisibleSegments = (q->integerParameter("ApplyToAllVisibleSegments") != 0);
  if (applyToAllVisibleSegments)
  {
    // Iterate through visible segments
    // set each as selected one by one
    // Paint on modifier labelmap
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
    if (displayNode)
    {
      displayNode->GetVisibleSegmentIDs(allProcessedSegmentIDs);
    }
  }
  else
  {
    // Paint on current segment
    allProcessedSegmentIDs->InsertNextValue(selectedSegmentID);
  }

  q->saveStateForUndo();
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  this->BrushPolyDataToStencil->Update();
  vtkImageStencilData* stencilData = this->BrushPolyDataToStencil->GetOutput();
  int stencilExtent[6] = { 0, -1, 0, -1, 0, -1 };
  stencilData->GetExtent(stencilExtent);

  vtkNew<vtkTransform> worldToModifierLabelmapIjkTransform;

  vtkNew<vtkMatrix4x4> segmentationToSegmentationIjkTransformMatrix;
  modifierLabelmap->GetImageToWorldMatrix(segmentationToSegmentationIjkTransformMatrix.GetPointer());
  segmentationToSegmentationIjkTransformMatrix->Invert();
  worldToModifierLabelmapIjkTransform->Concatenate(segmentationToSegmentationIjkTransformMatrix.GetPointer());

  vtkNew<vtkMatrix4x4> worldToSegmentationTransformMatrix;
  // We don't support painting in non-linearly transformed node (it could be implemented, but would probably slow down things too much)
  // TODO: show a meaningful error message to the user if attempted
  vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(nullptr, segmentationNode->GetParentTransformNode(), worldToSegmentationTransformMatrix.GetPointer());
  worldToModifierLabelmapIjkTransform->Concatenate(worldToSegmentationTransformMatrix.GetPointer());

  vtkNew<vtkImageStencilToImage> stencilToImage;
  stencilToImage->SetInputConnection(this->BrushPolyDataToStencil->GetOutputPort());
  if (this->operationInside())
  {
    stencilToImage->SetInsideValue(q->m_FillValue);
    stencilToImage->SetOutsideValue(q->m_EraseValue);
  }
  else
  {
    stencilToImage->SetInsideValue(q->m_EraseValue);
    stencilToImage->SetOutsideValue(q->m_FillValue);
  }

  stencilToImage->SetOutputScalarType(modifierLabelmap->GetScalarType());

  vtkNew<vtkImageChangeInformation> brushPositioner;
  brushPositioner->SetInputConnection(stencilToImage->GetOutputPort());
  brushPositioner->SetOutputSpacing(modifierLabelmap->GetSpacing());
  brushPositioner->SetOutputOrigin(modifierLabelmap->GetOrigin());

  brushPositioner->Update();
  vtkNew<vtkOrientedImageData> orientedBrushPositionerOutput;
  orientedBrushPositionerOutput->ShallowCopy(stencilToImage->GetOutput());
  vtkNew<vtkMatrix4x4> imageToWorld;
  modifierLabelmap->GetImageToWorldMatrix(imageToWorld.GetPointer());
  orientedBrushPositionerOutput->SetImageToWorldMatrix(imageToWorld.GetPointer());

  vtkOrientedImageDataResample::ModifyImage(modifierLabelmap, orientedBrushPositionerOutput.GetPointer(), vtkOrientedImageDataResample::OPERATION_MAXIMUM);

  // Notify editor about changes
  qSlicerSegmentEditorAbstractEffect::ModificationMode modificationMode = qSlicerSegmentEditorAbstractEffect::ModificationModeAdd;
  if (this->operationErase())
  {
    modificationMode = qSlicerSegmentEditorAbstractEffect::ModificationModeRemove;
  }

  // Select input segment and process one by one.
  // Probably in some cases the performance could ve optimized when when multiple segments are processed,
  // but it would make the implementation more complicated.
  for (int segmentIndex = 0; segmentIndex < allProcessedSegmentIDs->GetNumberOfValues(); segmentIndex++)
  {
    std::string segmentID = allProcessedSegmentIDs->GetValue(segmentIndex);
    q->parameterSetNode()->SetSelectedSegmentID(segmentID.c_str());
    q->modifySelectedSegmentByLabelmap(modifierLabelmap, modificationMode);
  }

  // restore original segment selection
  q->parameterSetNode()->SetSelectedSegmentID(selectedSegmentID.c_str());

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
int qSlicerSegmentEditorScissorsEffectPrivate::ConvertShapeFromString(const QString& shapeStr)
{
  for (int i = 0; i < Shape_Last; i++)
  {
    if (shapeStr == ConvertShapeToString(i))
    {
      return i;
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
QString qSlicerSegmentEditorScissorsEffectPrivate::ConvertShapeToString(int shape)
{
  switch (shape)
  {
    case ShapeFreeForm: return "FreeForm";
    case ShapeCircle: return "Circle";
    case ShapeRectangle: return "Rectangle";
    default: return "";
  }
}

//-----------------------------------------------------------------------------
int qSlicerSegmentEditorScissorsEffectPrivate::ConvertOperationFromString(const QString& operationStr)
{
  for (int i = 0; i < Operation_Last; i++)
  {
    if (operationStr == ConvertOperationToString(i))
    {
      return i;
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
QString qSlicerSegmentEditorScissorsEffectPrivate::ConvertOperationToString(int operation)
{
  switch (operation)
  {
    case OperationEraseInside: return "EraseInside";
    case OperationEraseOutside: return "EraseOutside";
    case OperationFillInside: return "FillInside";
    case OperationFillOutside: return "FillOutside";
    default: return "";
  }
}

//-----------------------------------------------------------------------------
int qSlicerSegmentEditorScissorsEffectPrivate::ConvertSliceCutModeFromString(const QString& sliceCutModeStr)
{
  for (int i = 0; i < SliceCutMode_Last; i++)
  {
    if (sliceCutModeStr == ConvertSliceCutModeToString(i))
    {
      return i;
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
QString qSlicerSegmentEditorScissorsEffectPrivate::ConvertSliceCutModeToString(int operation)
{
  switch (operation)
  {
    case SliceCutModeUnlimited: return "Unlimited";
    case SliceCutModePositive: return "Positive";
    case SliceCutModeNegative: return "Negative";
    case SliceCutModeSymmetric: return "Symmetric";
    default: return "";
  }
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorScissorsEffectPrivate::operationInside()
{
  Q_Q(qSlicerSegmentEditorScissorsEffect);
  int operation = this->ConvertOperationFromString(q->parameter("Operation"));
  return (operation == OperationEraseInside || operation == OperationFillInside);
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorScissorsEffectPrivate::operationErase()
{
  Q_Q(qSlicerSegmentEditorScissorsEffect);
  int operation = this->ConvertOperationFromString(q->parameter("Operation"));
  return (operation == OperationEraseInside || operation == OperationEraseOutside);
}

//-----------------------------------------------------------------------------
int qSlicerSegmentEditorScissorsEffectPrivate::shapeDrawCentered()
{
  Q_Q(qSlicerSegmentEditorScissorsEffect);
  return q->integerParameter("ShapeDrawCentered");
}

//-----------------------------------------------------------------------------
int qSlicerSegmentEditorScissorsEffectPrivate::shape()
{
  Q_Q(qSlicerSegmentEditorScissorsEffect);
  return this->ConvertShapeFromString(q->parameter("Shape"));
}

//----------------------------------------------------------------------------
qSlicerSegmentEditorScissorsEffect::qSlicerSegmentEditorScissorsEffect(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerSegmentEditorScissorsEffectPrivate(*this))
{
  this->m_Name = QString(/*no tr*/ "Scissors");
  this->m_Title = tr("Scissors");
  this->m_ShowEffectCursorInThreeDView = true;
}

//----------------------------------------------------------------------------
qSlicerSegmentEditorScissorsEffect::~qSlicerSegmentEditorScissorsEffect() = default;

//---------------------------------------------------------------------------
QIcon qSlicerSegmentEditorScissorsEffect::icon()
{
  Q_D(qSlicerSegmentEditorScissorsEffect);

  return d->ScissorsIcon;
}

//---------------------------------------------------------------------------
const QString qSlicerSegmentEditorScissorsEffect::helpText() const
{
  return QString("<html>")
         + tr("Cut through the entire segment from the current viewpoint<br>."
              "<p><ul style=\"margin: 0\">"
              "<li><b>Left-button drag-and-drop:</b> sweep out an outline."
              "<li><b>Esc:</b> cancel."
              "</ul><p>"
              "Editing is available both in slice and 3D views. In slice view cutting will be applied to all slices."
              "<p>");
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffect::setupOptionsFrame()
{
  Q_D(qSlicerSegmentEditorScissorsEffect);

  // Setup widgets corresponding to the parent class of this effect
  Superclass::setupOptionsFrame();

  // Operation

  d->gridLayout = new QGridLayout();
  d->gridLayout->setAlignment(Qt::AlignLeft);

  d->eraseInsideRadioButton = new QRadioButton(tr("Erase inside"));
  d->eraseOutsideRadioButton = new QRadioButton(tr("Erase outside"));
  d->fillInsideRadioButton = new QRadioButton(tr("Fill inside"));
  d->fillOutsideRadioButton = new QRadioButton(tr("Fill outside"));

  d->gridLayout->addWidget(new QLabel(tr("Operation:")), 0, 0);
  d->gridLayout->addWidget(d->eraseInsideRadioButton, 1, 0);
  d->gridLayout->addWidget(d->eraseOutsideRadioButton, 2, 0);
  d->gridLayout->addWidget(d->fillInsideRadioButton, 3, 0);
  d->gridLayout->addWidget(d->fillOutsideRadioButton, 4, 0);

  d->operationGroup = new QButtonGroup(this);
  d->operationGroup->setExclusive(true);
  d->operationGroup->addButton(d->eraseInsideRadioButton, qSlicerSegmentEditorScissorsEffectPrivate::OperationEraseInside);
  d->operationGroup->addButton(d->eraseOutsideRadioButton, qSlicerSegmentEditorScissorsEffectPrivate::OperationEraseOutside);
  d->operationGroup->addButton(d->fillInsideRadioButton, qSlicerSegmentEditorScissorsEffectPrivate::OperationFillInside);
  d->operationGroup->addButton(d->fillOutsideRadioButton, qSlicerSegmentEditorScissorsEffectPrivate::OperationFillOutside);

  QObject::connect(d->operationGroup, SIGNAL(buttonClicked(int)), this, SLOT(setOperation(int)));

  // Shape

  d->freeFormRadioButton = new QRadioButton(tr("Free-form"));
  d->circleRadioButton = new QRadioButton(tr("Circle"));
  d->rectangleRadioButton = new QRadioButton(tr("Rectangle"));
  d->shapeDrawCenteredCheckBox = new QCheckBox(tr("Centered"));
  d->shapeDrawCenteredCheckBox->setToolTip(tr("If checked, click position sets the circle or rectangle center, otherwise click position is at the shape boundary."));

  d->gridLayout->addWidget(new QLabel(tr("Shape:")), 0, 1);
  d->gridLayout->addWidget(d->freeFormRadioButton, 1, 1);
  d->gridLayout->addWidget(d->circleRadioButton, 2, 1);
  d->gridLayout->addWidget(d->rectangleRadioButton, 3, 1);
  d->gridLayout->addWidget(d->shapeDrawCenteredCheckBox, 4, 1);

  d->shapeGroup = new QButtonGroup(this);
  d->shapeGroup->setExclusive(true);
  d->shapeGroup->addButton(d->freeFormRadioButton, qSlicerSegmentEditorScissorsEffectPrivate::ShapeFreeForm);
  d->shapeGroup->addButton(d->circleRadioButton, qSlicerSegmentEditorScissorsEffectPrivate::ShapeCircle);
  d->shapeGroup->addButton(d->rectangleRadioButton, qSlicerSegmentEditorScissorsEffectPrivate::ShapeRectangle);

  QObject::connect(d->shapeGroup, SIGNAL(buttonClicked(int)), this, SLOT(setShape(int)));
  QObject::connect(d->shapeDrawCenteredCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setShapeDrawCentered(int)));

  // Slice cut mode

  d->unlimitedRadioButton = new QRadioButton(tr("Unlimited"));
  d->unlimitedRadioButton->setToolTip(tr("Cut through the entire segmentation. Only used for slice views."));
  d->positiveRadioButton = new QRadioButton(tr("Positive"));
  d->positiveRadioButton->setToolTip(tr("Only positive side of the slice plane is included in cut region. Only used for slice views."));
  d->negativeRadioButton = new QRadioButton(tr("Negative"));
  d->negativeRadioButton->setToolTip(tr("Only negative side of the slice plane is included in cut region. Only used for slice views."));
  d->symmetricRadioButton = new QRadioButton(tr("Symmetric"));
  d->symmetricRadioButton->setToolTip(tr("Cut region is limited to the specified thickness around the slice plane. Only used for slice views."));

  d->sliceCutDepthSpinBox = new qMRMLSpinBox();
  d->sliceCutDepthSpinBox->setToolTip(tr("Thickness of the cut region. 0 means that only the current slice is included. Only used for slice views."));
  d->sliceCutDepthSpinBox->setQuantity("length"); // scene is not available yet
  d->sliceCutDepthSpinBox->setUnitAwareProperties(qMRMLSpinBox::Prefix | qMRMLSpinBox::Suffix | qMRMLSpinBox::Precision | qMRMLSpinBox::MaximumValue);
  d->sliceCutDepthSpinBox->setMinimum(0);
  d->sliceCutDepthSpinBox->setValue(0);
  d->sliceCutDepthSpinBox->setEnabled(false);

  QLabel* sliceCutModeLabel = new QLabel(tr("Slice cut:"));
  sliceCutModeLabel->setToolTip(tr("Restrict cut region in slice views."));
  d->gridLayout->addWidget(sliceCutModeLabel, 0, 2);
  d->gridLayout->addWidget(d->unlimitedRadioButton, 1, 2);
  d->gridLayout->addWidget(d->positiveRadioButton, 2, 2);
  d->gridLayout->addWidget(d->negativeRadioButton, 3, 2);
  d->gridLayout->addWidget(d->symmetricRadioButton, 4, 2);
  d->gridLayout->addWidget(d->sliceCutDepthSpinBox, 5, 2);

  QLabel* applyToAllVisibleSegmentsLabel = new QLabel(tr("Apply to visible segments:"));
  applyToAllVisibleSegmentsLabel->setToolTip(tr("Apply scissor effect to all visible segments from top to bottom. \
                                          After pressing 'Apply': Please be patient - this may be time-consuming. \
                                          Progress will be shown as status message. "));
  d->gridLayout->addWidget(applyToAllVisibleSegmentsLabel, 5, 0);
  d->applyToAllVisibleSegmentsCheckBox = new QCheckBox(" ");
  d->applyToAllVisibleSegmentsCheckBox->setObjectName(QString::fromUtf8("applyToAllVisibleSegmentsCheckBox"));
  d->gridLayout->addWidget(d->applyToAllVisibleSegmentsCheckBox, 5, 1);

  this->addOptionsWidget(d->gridLayout);

  d->sliceCutModeGroup = new QButtonGroup(this);
  d->sliceCutModeGroup->setExclusive(true);
  d->sliceCutModeGroup->addButton(d->unlimitedRadioButton, qSlicerSegmentEditorScissorsEffectPrivate::SliceCutModeUnlimited);
  d->sliceCutModeGroup->addButton(d->positiveRadioButton, qSlicerSegmentEditorScissorsEffectPrivate::SliceCutModePositive);
  d->sliceCutModeGroup->addButton(d->negativeRadioButton, qSlicerSegmentEditorScissorsEffectPrivate::SliceCutModeNegative);
  d->sliceCutModeGroup->addButton(d->symmetricRadioButton, qSlicerSegmentEditorScissorsEffectPrivate::SliceCutModeSymmetric);

  QObject::connect(d->sliceCutModeGroup, SIGNAL(buttonClicked(int)), this, SLOT(setSliceCutMode(int)));
  QObject::connect(d->sliceCutDepthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onSliceCutDepthChanged(double)));
  QObject::connect(d->applyToAllVisibleSegmentsCheckBox, SIGNAL(clicked()), this, SLOT(updateMRMLFromGUI()));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffect::setMRMLDefaults()
{
  Q_D(qSlicerSegmentEditorScissorsEffect);
  Superclass::setMRMLDefaults();
  this->setParameterDefault("ApplyToAllVisibleSegments", 0);
  this->setParameterDefault("Operation", d->ConvertOperationToString(qSlicerSegmentEditorScissorsEffectPrivate::OperationEraseInside));
  this->setParameterDefault("Shape", d->ConvertShapeToString(qSlicerSegmentEditorScissorsEffectPrivate::ShapeFreeForm));
  this->setParameterDefault("SliceCutMode", d->ConvertSliceCutModeToString(qSlicerSegmentEditorScissorsEffectPrivate::SliceCutModeUnlimited));
  this->setParameterDefault("SliceCutDepthMm", 0);
  this->setParameterDefault("ShapeDrawCentered", 0);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffect::updateGUIFromMRML()
{
  Q_D(qSlicerSegmentEditorScissorsEffect);

  Superclass::updateGUIFromMRML();

  if (!this->active())
  {
    // updateGUIFromMRML is called when the effect is activated
    return;
  }

  if (!this->scene())
  {
    return;
  }

  int operationIndex = d->ConvertOperationFromString(QString(this->parameter("Operation")));
  if (d->operationGroup->button(operationIndex))
  {
    bool wasBlocked = d->operationGroup->blockSignals(true);
    d->operationGroup->button(operationIndex)->setChecked(true);
    d->operationGroup->blockSignals(wasBlocked);
  }

  int shapeIndex = d->ConvertShapeFromString(QString(this->parameter("Shape")));
  if (d->shapeGroup->button(shapeIndex))
  {
    bool wasBlocked = d->shapeGroup->blockSignals(true);
    d->shapeGroup->button(shapeIndex)->setChecked(true);
    d->shapeGroup->blockSignals(wasBlocked);
  }

  bool wasBlocked = d->shapeDrawCenteredCheckBox->blockSignals(true);
  d->shapeDrawCenteredCheckBox->setCheckState(this->integerParameter("ShapeDrawCentered") ? Qt::Checked : Qt::Unchecked);
  d->shapeDrawCenteredCheckBox->setEnabled(shapeIndex == qSlicerSegmentEditorScissorsEffectPrivate::ShapeCircle || //
                                           shapeIndex == qSlicerSegmentEditorScissorsEffectPrivate::ShapeRectangle);
  d->shapeDrawCenteredCheckBox->blockSignals(wasBlocked);

  int sliceCutModeIndex = d->ConvertSliceCutModeFromString(QString(this->parameter("SliceCutMode")));
  if (d->sliceCutModeGroup->button(sliceCutModeIndex))
  {
    bool wasBlocked = d->sliceCutModeGroup->blockSignals(true);
    d->sliceCutModeGroup->button(sliceCutModeIndex)->setChecked(true);
    d->sliceCutModeGroup->blockSignals(wasBlocked);
  }

  wasBlocked = d->sliceCutDepthSpinBox->blockSignals(true);
  d->sliceCutDepthSpinBox->setMRMLScene(this->scene());
  d->sliceCutDepthSpinBox->setValue(this->doubleParameter("SliceCutDepthMm"));
  d->sliceCutDepthSpinBox->setEnabled(sliceCutModeIndex == qSlicerSegmentEditorScissorsEffectPrivate::SliceCutModeSymmetric);
  d->sliceCutDepthSpinBox->blockSignals(wasBlocked);

  d->applyToAllVisibleSegmentsCheckBox->blockSignals(true);
  d->applyToAllVisibleSegmentsCheckBox->setChecked(this->integerParameter("ApplyToAllVisibleSegments"));
  d->applyToAllVisibleSegmentsCheckBox->blockSignals(wasBlocked);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffect::setOperation(int operationIndex)
{
  Q_D(qSlicerSegmentEditorScissorsEffect);
  this->setParameter("Operation", d->ConvertOperationToString(operationIndex));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffect::setShape(int shapeIndex)
{
  Q_D(qSlicerSegmentEditorScissorsEffect);
  QString shape = d->ConvertShapeToString(shapeIndex);
  this->setParameter("Shape", shape);
  d->shapeDrawCenteredCheckBox->setEnabled(                              //
    shapeIndex == qSlicerSegmentEditorScissorsEffectPrivate::ShapeCircle //
    || shapeIndex == qSlicerSegmentEditorScissorsEffectPrivate::ShapeRectangle);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffect::setShapeDrawCentered(int checkState)
{
  Q_D(qSlicerSegmentEditorScissorsEffect);
  this->setParameter("ShapeDrawCentered", checkState == Qt::Checked);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffect::setSliceCutMode(int sliceCutModeIndex)
{
  Q_D(qSlicerSegmentEditorScissorsEffect);
  this->setParameter("SliceCutMode", d->ConvertSliceCutModeToString(sliceCutModeIndex));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffect::onSliceCutDepthChanged(double value)
{
  this->setParameter("SliceCutDepthMm", value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffect::updateMRMLFromGUI()
{
  Q_D(qSlicerSegmentEditorScissorsEffect);

  Superclass::updateMRMLFromGUI();

  QString operation = d->ConvertOperationToString(d->operationGroup->checkedId());
  QString shape = d->ConvertShapeToString(d->shapeGroup->checkedId());
  QString sliceCutMode = d->ConvertSliceCutModeToString(d->sliceCutModeGroup->checkedId());
  int applyToAllVisibleSegments = d->applyToAllVisibleSegmentsCheckBox->isChecked() ? 1 : 0;
  this->setParameter("ApplyToAllVisibleSegments", applyToAllVisibleSegments);
  this->setParameter("Operation", operation.toUtf8().constData());
  this->setParameter("Shape", shape.toUtf8().constData());
  this->setParameter("SliceCutMode", sliceCutMode.toUtf8().constData());
  this->setParameter("SliceCutDepthMm", d->sliceCutDepthSpinBox->value());
  this->setParameter("ShapeDrawCentered", d->shapeDrawCenteredCheckBox->checkState() == Qt::Checked);
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect* qSlicerSegmentEditorScissorsEffect::clone()
{
  return new qSlicerSegmentEditorScissorsEffect();
}

//---------------------------------------------------------------------------
void qSlicerSegmentEditorScissorsEffect::deactivate()
{
  Q_D(qSlicerSegmentEditorScissorsEffect);
  Superclass::deactivate();
  d->clearScissorsPipelines();
}

//---------------------------------------------------------------------------
bool qSlicerSegmentEditorScissorsEffect::processInteractionEvents(vtkRenderWindowInteractor* callerInteractor, unsigned long eid, qMRMLWidget* viewWidget)
{
  Q_D(qSlicerSegmentEditorScissorsEffect);

  bool abortEvent = false;

  // This effect only supports interactions in the 2D slice views currently
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (!sliceWidget && !threeDWidget)
  {
    return abortEvent;
  }
  ScissorsPipeline* pipeline = d->scissorsPipelineForWidget(viewWidget);
  if (!pipeline)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to create pipeline";
    return abortEvent;
  }

  bool anyModifierKeyPressed = (callerInteractor->GetShiftKey() || callerInteractor->GetControlKey() || callerInteractor->GetAltKey());

  if (eid == vtkCommand::LeftButtonPressEvent && !anyModifierKeyPressed)
  {
    // Warn the user if current segment is not visible
    int confirmedEditingAllowed = this->confirmCurrentSegmentVisible();
    if (confirmedEditingAllowed == NotConfirmed || confirmedEditingAllowed == ConfirmedWithDialog)
    {
      // If user had to move the mouse to click on the popup, so we cannot continue with painting
      // from the current mouse position. User will need to click again.
      // The dialog is not displayed again for the same segment.

      // The event has to be aborted, because otherwise there would be a LeftButtonPressEvent without a matching
      // LeftButtonReleaseEvent (as the popup window got the release button event) and so the view would be stuck
      // in view rotation mode.
      abortEvent = true;

      return abortEvent;
    }

    pipeline->IsDragging = true;
    // this->cursorOff(viewWidget);
    vtkVector2i eventPosition;
    callerInteractor->GetEventPosition(eventPosition[0], eventPosition[1]);
    d->createGlyph(pipeline, eventPosition);
    abortEvent = true;
  }
  else if (eid == vtkCommand::MouseMoveEvent)
  {
    if (pipeline->IsDragging)
    {
      vtkVector2i eventPosition;
      callerInteractor->GetEventPosition(eventPosition[0], eventPosition[1]);
      d->updateGlyphWithNewPosition(pipeline, eventPosition, false);
      this->scheduleRender(viewWidget);
      abortEvent = true;
    }
  }
  else if (eid == vtkCommand::LeftButtonReleaseEvent)
  {
    if (pipeline->IsDragging)
    {
      pipeline->IsDragging = false;
      vtkVector2i eventPosition;
      callerInteractor->GetEventPosition(eventPosition[0], eventPosition[1]);
      d->updateGlyphWithNewPosition(pipeline, eventPosition, true);
      // this->cursorOn(viewWidget);

      // Paint on modifier labelmap
      vtkCellArray* lines = pipeline->PolyData->GetLines();
      vtkOrientedImageData* modifierLabelmap = defaultModifierLabelmap();
      if (lines->GetNumberOfCells() > 0 && modifierLabelmap)
      {
        d->paintApply(viewWidget);
        qSlicerSegmentEditorAbstractEffect::scheduleRender(viewWidget);
      }
      abortEvent = true;
    }
  }
  return abortEvent;
}
