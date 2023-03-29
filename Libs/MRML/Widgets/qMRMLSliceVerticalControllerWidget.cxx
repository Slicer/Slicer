/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
  and development was supported by the Center for Intelligent Image-guided Interventions (CI3).

==============================================================================*/

// qMRML includes
#include "qMRMLSliceVerticalControllerWidget.h"
#include "ui_qMRMLSliceVerticalControllerWidget.h"
#include "qMRMLSliderWidget.h"

// CTK includes
#include <ctkDoubleSpinBox.h>
#include <ctkDoubleSlider.h>

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceLayerLogic.h>

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLUnitNode.h>

//------------------------------------------------------------------------------
class qMRMLSliceVerticalControllerWidgetPrivate: public Ui_qMRMLSliceVerticalControllerWidget
{
  Q_DECLARE_PUBLIC(qMRMLSliceVerticalControllerWidget);

public:
  qMRMLSliceVerticalControllerWidgetPrivate(qMRMLSliceVerticalControllerWidget& object);
  ~qMRMLSliceVerticalControllerWidgetPrivate();

  void init();
  void setSelectionNode();
  void onSliceLogicModifiedEvent();
  void updateSliceOffsetSliderVisibility();

protected:
  qMRMLSliceVerticalControllerWidget* const q_ptr;
  vtkSmartPointer<vtkMRMLSliceLogic>  SliceLogic;
  vtkMRMLSelectionNode*               SelectionNode;
  /// Slicer offset resolution without applying display scaling.
  double                              SliceOffsetResolution{1.0};
  bool                                ShowSliceOffsetSlider{true};
};

//---------------------------------------------------------------------------
qMRMLSliceVerticalControllerWidgetPrivate::qMRMLSliceVerticalControllerWidgetPrivate(qMRMLSliceVerticalControllerWidget& object)
  : q_ptr(&object)
{
}

//---------------------------------------------------------------------------
qMRMLSliceVerticalControllerWidgetPrivate::~qMRMLSliceVerticalControllerWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidgetPrivate::init()
{
  Q_Q(qMRMLSliceVerticalControllerWidget);
  this->setupUi(q);

  this->SliceVerticalOffsetSlider->setTracking(false);
  this->SliceVerticalOffsetSlider->setToolTip(qMRMLSliceVerticalControllerWidget::tr("Slice distance from RAS origin"));
  this->updateSliceOffsetSliderVisibility();

  // Connect Slice offset slider
  QObject::connect(this->SliceVerticalOffsetSlider, SIGNAL(valueChanged(double)),
                   q, SLOT(setSliceOffsetValue(double)), Qt::QueuedConnection);
  QObject::connect(this->SliceVerticalOffsetSlider, SIGNAL(valueIsChanging(double)),
                   q, SLOT(trackSliceOffsetValue(double)), Qt::QueuedConnection);

  vtkNew<vtkMRMLSliceLogic> defaultLogic;
  defaultLogic->SetMRMLApplicationLogic(vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->GetMRMLApplicationLogic());
  q->setSliceLogic(defaultLogic.GetPointer());
  q->setFixedWidth(12);
}

// --------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidgetPrivate::setSelectionNode()
{
  Q_Q(qMRMLSliceVerticalControllerWidget);

  vtkMRMLSelectionNode* selectionNode = nullptr;
  if (q->mrmlScene())
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      q->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
    }

  this->SelectionNode = selectionNode;
}

// --------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidgetPrivate::onSliceLogicModifiedEvent()
{
  Q_Q(qMRMLSliceVerticalControllerWidget);

  vtkMRMLSliceNode* newSliceNode = this->SliceLogic ? this->SliceLogic->GetSliceNode() : nullptr;
  // Enable/disable widget
  q->setDisabled(newSliceNode == nullptr);

  double offsetRange[2] = {-1.0, 1.0};
  double offsetResolution = 1.0;
  if (!this->SliceLogic || !this->SliceLogic->GetSliceOffsetRangeResolution(offsetRange, offsetResolution))
    {
    return;
    }

  bool wasBlocking = this->SliceVerticalOffsetSlider->blockSignals(true);
  q->setSliceOffsetRange(offsetRange[0], offsetRange[1]);
  q->setSliceOffsetResolution(offsetResolution);
  this->SliceVerticalOffsetSlider->setValue(this->SliceLogic->GetSliceOffset());
  this->SliceVerticalOffsetSlider->blockSignals(wasBlocking);

  emit q->renderRequested();
}

// --------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidgetPrivate::updateSliceOffsetSliderVisibility()
{
  Q_Q(qMRMLSliceVerticalControllerWidget);

  this->SliceVerticalOffsetSlider->setVisible(this->ShowSliceOffsetSlider);
  q->setVisible(this->ShowSliceOffsetSlider);
}

// --------------------------------------------------------------------------
// qMRMLSliceVerticalControllerWidget methods

// --------------------------------------------------------------------------
qMRMLSliceVerticalControllerWidget::qMRMLSliceVerticalControllerWidget(QWidget *_parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLSliceVerticalControllerWidgetPrivate(*this))
{
  Q_D(qMRMLSliceVerticalControllerWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLSliceVerticalControllerWidget::~qMRMLSliceVerticalControllerWidget() = default;

//---------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLSliceVerticalControllerWidget);

  if (this->mrmlScene() == newScene)
    {
    return;
    }

  d->SliceLogic->SetMRMLScene(newScene);
  d->setSelectionNode();

  this->Superclass::setMRMLScene(newScene);
}

//---------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidget::setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode)
{
  Q_D(qMRMLSliceVerticalControllerWidget);
  // eventually calls vtkMRMLSliceLogic::ModifiedEvent which
  // eventually calls onSliceLogicModified.
  d->SliceLogic->SetSliceNode(newSliceNode);
  if (newSliceNode && newSliceNode->GetScene())
    {
    this->setMRMLScene(newSliceNode->GetScene());
    }
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* qMRMLSliceVerticalControllerWidget::mrmlSliceNode()const
{
  Q_D(const qMRMLSliceVerticalControllerWidget);
  return d->SliceLogic->GetSliceNode();
}

//---------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidget::setSliceLogic(vtkMRMLSliceLogic * newSliceLogic)
{
  Q_D(qMRMLSliceVerticalControllerWidget);
  if (d->SliceLogic == newSliceLogic)
    {
    return;
    }

  this->qvtkReconnect(d->SliceLogic, newSliceLogic, vtkCommand::ModifiedEvent,
                      this, SLOT(onSliceLogicModifiedEvent()));

  d->SliceLogic = newSliceLogic;

  if (d->SliceLogic && d->SliceLogic->GetMRMLScene())
    {
    this->setMRMLScene(d->SliceLogic->GetMRMLScene());
    }

  this->onSliceLogicModifiedEvent();
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic *qMRMLSliceVerticalControllerWidget::sliceLogic()
{
  Q_D(qMRMLSliceVerticalControllerWidget);
  return d->SliceLogic;
}

//---------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidget::setSliceOffsetRange(double min, double max)
{
  Q_D(qMRMLSliceVerticalControllerWidget);
  d->SliceVerticalOffsetSlider->setRange(min, max);
}

//---------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidget::setSliceOffsetResolution(double resolution)
{
  Q_D(qMRMLSliceVerticalControllerWidget);
  d->SliceOffsetResolution = resolution;
  resolution = qMax(resolution, 0.00000001);
  double displayCoeffiecient = 1.0;
  if (d->SelectionNode && this->mrmlScene())
    {
    vtkMRMLUnitNode* unitNode = vtkMRMLUnitNode::SafeDownCast(this->mrmlScene()->GetNodeByID(d->SelectionNode->GetUnitNodeID("length")));
    if (unitNode)
      {
      displayCoeffiecient = unitNode->GetDisplayCoefficient();
      }
    }
  d->SliceVerticalOffsetSlider->setSingleStep(resolution * displayCoeffiecient);
  d->SliceVerticalOffsetSlider->setPageStep(resolution * displayCoeffiecient);
}

//---------------------------------------------------------------------------
double qMRMLSliceVerticalControllerWidget::sliceOffsetResolution()
{
  Q_D(qMRMLSliceVerticalControllerWidget);
  return d->SliceOffsetResolution;
}

// --------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidget::setSliceOffsetValue(double offset)
{
  Q_D(qMRMLSliceVerticalControllerWidget);
  if (!d->SliceLogic)
    {
    return;
    }
  //qDebug() << "qMRMLSliceVerticalControllerWidget::setSliceOffsetValue:" << offset;

  // This prevents desynchronized update of displayable managers during user interaction
  // (ie. slice intersection widget or segmentations lagging behind during slice translation)
  vtkMRMLApplicationLogic* applicationLogic =
    vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->GetMRMLApplicationLogic();
  if (applicationLogic)
    {
    applicationLogic->PauseRender();
    }

  d->SliceLogic->StartSliceOffsetInteraction();
  d->SliceLogic->SetSliceOffset(offset);
  d->SliceLogic->EndSliceOffsetInteraction();

  if (applicationLogic)
    {
    applicationLogic->ResumeRender();
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidget::trackSliceOffsetValue(double offset)
{
  Q_D(qMRMLSliceVerticalControllerWidget);
  if (!d->SliceLogic)
    {
    return;
    }
  //qDebug() << "qMRMLSliceVerticalControllerWidget::trackSliceOffsetValue";

  // This prevents desynchronized update of displayable managers during user interaction
  // (ie. slice intersection widget or segmentations lagging behind during slice translation)
    vtkMRMLApplicationLogic* applicationLogic =
    vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->GetMRMLApplicationLogic();
  if (applicationLogic)
    {
    applicationLogic->PauseRender();
    }

  d->SliceLogic->StartSliceOffsetInteraction();
  d->SliceLogic->SetSliceOffset(offset);

  if (applicationLogic)
    {
    applicationLogic->ResumeRender();
  }
}

// --------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidget::onSliceLogicModifiedEvent()
{
  Q_D(qMRMLSliceVerticalControllerWidget);
  d->onSliceLogicModifiedEvent();
}

// --------------------------------------------------------------------------
qMRMLSliderWidget* qMRMLSliceVerticalControllerWidget::sliceVerticalOffsetSlider()
{
  Q_D(qMRMLSliceVerticalControllerWidget);
  return d->SliceVerticalOffsetSlider;
}

//-----------------------------------------------------------------------------
bool qMRMLSliceVerticalControllerWidget::showSliceOffsetSlider()const
{
  Q_D(const qMRMLSliceVerticalControllerWidget);
  return d->ShowSliceOffsetSlider;
}

//-----------------------------------------------------------------------------
void qMRMLSliceVerticalControllerWidget::setShowSliceOffsetSlider(bool show)
{
  Q_D(qMRMLSliceVerticalControllerWidget);
  d->ShowSliceOffsetSlider = show;
  d->updateSliceOffsetSliderVisibility();
}
