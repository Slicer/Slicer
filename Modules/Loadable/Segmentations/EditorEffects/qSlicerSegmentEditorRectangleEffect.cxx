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
#include "qSlicerSegmentEditorRectangleEffect.h"

#include "vtkOrientedImageData.h"
#include "vtkMRMLSegmentEditorNode.h"

// Qt includes
#include <QDebug>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor2D.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkVector.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkEventBroker.h>
#include "qMRMLSliceView.h"

// Slicer includes
#include "qMRMLSliceWidget.h"

//-----------------------------------------------------------------------------
/// Visualization objects and pipeline for each slice view for rectangle drawing
class RectanglePipeline: public QObject
{
public:
  RectanglePipeline()
    {
    this->IsDragging = false;
    this->PolyData = vtkPolyData::New();
    this->Mapper = vtkPolyDataMapper2D::New();
    this->Mapper->SetInputData(this->PolyData);
    this->Actor = vtkActor2D::New();
    this->Actor->SetMapper(this->Mapper);
    this->Actor->VisibilityOff();
    vtkProperty2D* rectangleProperty = this->Actor->GetProperty();
    rectangleProperty->SetColor(1,1,0);
    rectangleProperty->SetLineWidth(1);
    this->DragStartXyPosition.Set(0, 0);
    this->CurrentXyPosition.Set(0, 0);
    };
  ~RectanglePipeline()
    {
    this->Actor->Delete();
    this->Actor = NULL;
    this->Mapper->Delete();
    this->Mapper = NULL;
    this->PolyData->Delete();
    this->PolyData = NULL;
    };
public:
  bool IsDragging;
  vtkVector2i DragStartXyPosition;
  vtkVector2i CurrentXyPosition;
  vtkActor2D* Actor;
  vtkPolyDataMapper2D* Mapper;
  vtkPolyData* PolyData;
};

//-----------------------------------------------------------------------------
class qSlicerSegmentEditorRectangleEffectPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSegmentEditorRectangleEffect);
protected:
  qSlicerSegmentEditorRectangleEffect* const q_ptr;
public:
  qSlicerSegmentEditorRectangleEffectPrivate(qSlicerSegmentEditorRectangleEffect& object);
  ~qSlicerSegmentEditorRectangleEffectPrivate();
public:
  /// Draw rectangle glyph
  void createRectangleGlyph(qMRMLSliceWidget* sliceWidget, RectanglePipeline* rectangle);
  /// Update rectangle glyph based on positions
  void updateRectangleGlyph(RectanglePipeline* rectangle);
protected:
  /// Get rectangle object for widget. Create if does not exist
  RectanglePipeline* rectanglePipelineForWidget(qMRMLSliceWidget* sliceWidget);
  /// Delete all rectangle pipelines
  void clearRectanglePipelines();
public:
  QIcon RectangleIcon;
  QMap<qMRMLSliceWidget*, RectanglePipeline*> RectanglePipelines;
};

//-----------------------------------------------------------------------------
qSlicerSegmentEditorRectangleEffectPrivate::qSlicerSegmentEditorRectangleEffectPrivate(qSlicerSegmentEditorRectangleEffect& object)
  : q_ptr(&object)
{
  this->RectangleIcon = QIcon(":Icons/Rectangle.png");
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorRectangleEffectPrivate::~qSlicerSegmentEditorRectangleEffectPrivate()
{
  this->clearRectanglePipelines();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorRectangleEffectPrivate::clearRectanglePipelines()
{
  Q_Q(qSlicerSegmentEditorRectangleEffect);
  QMapIterator<qMRMLSliceWidget*, RectanglePipeline*> it(this->RectanglePipelines);
  while (it.hasNext())
    {
    it.next();
    qMRMLWidget* viewWidget = it.key();
    RectanglePipeline* pipeline = it.value();
    q->removeActor2D(viewWidget, pipeline->Actor);
    delete pipeline;
    }
  this->RectanglePipelines.clear();
}

//-----------------------------------------------------------------------------
RectanglePipeline* qSlicerSegmentEditorRectangleEffectPrivate::rectanglePipelineForWidget(qMRMLSliceWidget* sliceWidget)
{
  Q_Q(qSlicerSegmentEditorRectangleEffect);

  if (this->RectanglePipelines.contains(sliceWidget))
    {
    return this->RectanglePipelines[sliceWidget];
    }

  // Create rectangle if does not yet exist
  RectanglePipeline* rectangle = new RectanglePipeline();
  this->createRectangleGlyph(sliceWidget, rectangle);

  vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(sliceWidget);
  if (!renderer)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get renderer!";
    }
  else
    {
    q->addActor2D(sliceWidget, rectangle->Actor);
    }

  this->RectanglePipelines[sliceWidget] = rectangle;
  return rectangle;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorRectangleEffectPrivate::createRectangleGlyph(
    qMRMLSliceWidget* sliceWidget,
    RectanglePipeline* rectangle)
{
  Q_Q(qSlicerSegmentEditorRectangleEffect);
  Q_UNUSED(sliceWidget);

  // Clear rectangle glyph in case it was already created
  rectangle->PolyData->Initialize();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
  rectangle->PolyData->SetPoints(points);
  rectangle->PolyData->SetLines(lines);

  int previousPointIndex = -1;
  int firstPointIndex = -1;
  int pointIndex = -1;
  for (int corner=0; corner<4; ++corner)
    {
    pointIndex = points->InsertNextPoint(0.0, 0.0, 0.0);
    if (previousPointIndex != -1)
      {
      vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
      idList->InsertNextId(previousPointIndex);
      idList->InsertNextId(pointIndex);
      rectangle->PolyData->InsertNextCell(VTK_LINE, idList);
      }
    previousPointIndex = pointIndex;
    if (firstPointIndex == -1)
      {
      firstPointIndex = pointIndex;
      }
    }

  // Make the last line in the polydata
  vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
  idList->InsertNextId(pointIndex);
  idList->InsertNextId(firstPointIndex);
  rectangle->PolyData->InsertNextCell(VTK_LINE, idList);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorRectangleEffectPrivate::updateRectangleGlyph(RectanglePipeline* rectangle)
{
  if (rectangle->IsDragging)
    {
    vtkPoints* points = rectangle->PolyData->GetPoints();
    points->SetPoint(0, rectangle->DragStartXyPosition[0], rectangle->DragStartXyPosition[1], 0.0);
    points->SetPoint(1, rectangle->DragStartXyPosition[0], rectangle->CurrentXyPosition[1], 0.0);
    points->SetPoint(2, rectangle->CurrentXyPosition[0], rectangle->CurrentXyPosition[1], 0.0);
    points->SetPoint(3, rectangle->CurrentXyPosition[0], rectangle->DragStartXyPosition[1], 0.0);
    rectangle->Actor->VisibilityOn();
    }
  else
    {
    rectangle->Actor->VisibilityOff();
    }
}

//----------------------------------------------------------------------------
qSlicerSegmentEditorRectangleEffect::qSlicerSegmentEditorRectangleEffect(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSegmentEditorRectangleEffectPrivate(*this) )
{
  this->m_Name = QString("Rectangle");
}

//----------------------------------------------------------------------------
qSlicerSegmentEditorRectangleEffect::~qSlicerSegmentEditorRectangleEffect()
{
}

//---------------------------------------------------------------------------
QIcon qSlicerSegmentEditorRectangleEffect::icon()
{
  Q_D(qSlicerSegmentEditorRectangleEffect);

  return d->RectangleIcon;
}

//---------------------------------------------------------------------------
QString const qSlicerSegmentEditorRectangleEffect::helpText()const
{
  return QString("Add a rectangular area to selected segment.\nLeft-click and drag to sweep out an outline, release the button when ready.");
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect* qSlicerSegmentEditorRectangleEffect::clone()
{
  return new qSlicerSegmentEditorRectangleEffect();
}

//---------------------------------------------------------------------------
void qSlicerSegmentEditorRectangleEffect::deactivate()
{
  Q_D(qSlicerSegmentEditorRectangleEffect);
  Superclass::deactivate();
  d->clearRectanglePipelines();
}

//---------------------------------------------------------------------------
bool qSlicerSegmentEditorRectangleEffect::processInteractionEvents(
  vtkRenderWindowInteractor* callerInteractor,
  unsigned long eid,
  qMRMLWidget* viewWidget )
{
  Q_D(qSlicerSegmentEditorRectangleEffect);
  bool abortEvent = false;

  // This effect only supports interactions in the 2D slice views currently
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  if (!sliceWidget)
    {
    return abortEvent;
    }
  RectanglePipeline* rectangle = d->rectanglePipelineForWidget(sliceWidget);
  if (!rectangle)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to create rectangle!";
    return abortEvent;
    }

  if (eid == vtkCommand::LeftButtonPressEvent)
    {
    rectangle->IsDragging = true;
    this->cursorOff(sliceWidget);
    callerInteractor->GetEventPosition(rectangle->DragStartXyPosition[0], rectangle->DragStartXyPosition[1]);
    callerInteractor->GetEventPosition(rectangle->CurrentXyPosition[0], rectangle->CurrentXyPosition[1]);
    d->updateRectangleGlyph(rectangle);
    abortEvent = true;
    }
  else if (eid == vtkCommand::MouseMoveEvent)
    {
    if (rectangle->IsDragging)
      {
      callerInteractor->GetEventPosition(rectangle->CurrentXyPosition[0], rectangle->CurrentXyPosition[1]);
      d->updateRectangleGlyph(rectangle);
      sliceWidget->sliceView()->scheduleRender();
      abortEvent = true;
      }
    }
  else if (eid == vtkCommand::LeftButtonReleaseEvent)
    {
    rectangle->IsDragging = false;
    d->updateRectangleGlyph(rectangle);
    this->cursorOn(sliceWidget);

    // Paint rectangle on modifier labelmap
    vtkCellArray* lines = rectangle->PolyData->GetLines();
    vtkOrientedImageData* modifierLabelmap = defaultModifierLabelmap();
    if (lines->GetNumberOfCells() > 0 && modifierLabelmap)
      {
      this->saveStateForUndo();
      this->appendPolyMask(modifierLabelmap, rectangle->PolyData, sliceWidget);
      this->modifySelectedSegmentByLabelmap(modifierLabelmap, ModificationModeAdd);
      }
    abortEvent = true;
    }
  return abortEvent;
}
