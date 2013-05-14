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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QInputDialog>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerColorsModuleWidget.h"
#include "ui_qSlicerColorsModuleWidget.h"

// qMRMLWidget includes
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"

// Slicer logic includes
#include <vtkSlicerColorLogic.h>

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLProceduralColorNode.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkLookupTable.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>

//-----------------------------------------------------------------------------
class qSlicerColorsModuleWidgetPrivate: public Ui_qSlicerColorsModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerColorsModuleWidget);
protected:
  qSlicerColorsModuleWidget* const q_ptr;

public:
  qSlicerColorsModuleWidgetPrivate(qSlicerColorsModuleWidget& obj);
  virtual ~qSlicerColorsModuleWidgetPrivate();
  vtkSlicerColorLogic* colorLogic()const;
  void setDefaultColorNode();

  vtkScalarBarWidget* ScalarBarWidget;
};

//-----------------------------------------------------------------------------
qSlicerColorsModuleWidgetPrivate::qSlicerColorsModuleWidgetPrivate(qSlicerColorsModuleWidget& object)
  : q_ptr(&object)
{
  this->ScalarBarWidget = vtkScalarBarWidget::New();
  this->ScalarBarWidget->GetScalarBarActor()->SetOrientationToVertical();
  this->ScalarBarWidget->GetScalarBarActor()->SetNumberOfLabels(11);
  this->ScalarBarWidget->GetScalarBarActor()->SetTitle("(mm)");
  this->ScalarBarWidget->GetScalarBarActor()->SetLabelFormat(" %#8.3f");
  
  // it's a 2d actor, position it in screen space by percentages
  this->ScalarBarWidget->GetScalarBarActor()->SetPosition(0.1, 0.1);
  this->ScalarBarWidget->GetScalarBarActor()->SetWidth(0.1);
  this->ScalarBarWidget->GetScalarBarActor()->SetHeight(0.8);
}

//-----------------------------------------------------------------------------
qSlicerColorsModuleWidgetPrivate::~qSlicerColorsModuleWidgetPrivate()
{
  if (this->ScalarBarWidget)
    {
    this->ScalarBarWidget->Delete();
    this->ScalarBarWidget = 0;
    }
}

//-----------------------------------------------------------------------------
vtkSlicerColorLogic* qSlicerColorsModuleWidgetPrivate::colorLogic()const
{
  Q_Q(const qSlicerColorsModuleWidget);
  return vtkSlicerColorLogic::SafeDownCast(q->logic()); 
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidgetPrivate::setDefaultColorNode()
{
  Q_Q(qSlicerColorsModuleWidget);
  if (!q->mrmlScene() ||
      !this->ColorTableComboBox ||
      this->ColorTableComboBox->currentNode() != 0)
    {
    return;
    }
  const char *defaultID = this->colorLogic()->GetDefaultLabelMapColorNodeID();
  vtkMRMLColorNode *defaultNode = vtkMRMLColorNode::SafeDownCast(
    q->mrmlScene()->GetNodeByID(defaultID));
  this->ColorTableComboBox->setCurrentNode(defaultNode);
}

//-----------------------------------------------------------------------------
qSlicerColorsModuleWidget::qSlicerColorsModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerColorsModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerColorsModuleWidget::~qSlicerColorsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setup()
{
  Q_D(qSlicerColorsModuleWidget);

  d->setupUi(this);

  QIcon copyIcon = this->style()->standardIcon(QStyle::SP_FileDialogNewFolder);
  d->CopyColorNodeButton->setIcon(copyIcon);

  d->VTKScalarBar->setScalarBarWidget(d->ScalarBarWidget);

  connect(d->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onMRMLColorNodeChanged(vtkMRMLNode*)));
  connect(d->NumberOfColorsSpinBox, SIGNAL(editingFinished()),
          this, SLOT(setNumberOfColors()));
  connect(d->LUTRangeWidget, SIGNAL(valuesChanged(double,double)),
          this, SLOT(setLookupTableRange(double,double)));
  connect(d->CopyColorNodeButton, SIGNAL(clicked()),
          this, SLOT(copyCurrentColorNode()));

  qSlicerApplication * app = qSlicerApplication::application();
  if (app && app->layoutManager())
    {
    qMRMLThreeDView* threeDView = app->layoutManager()->threeDWidget(0)->threeDView();
    vtkRenderer* activeRenderer = app->layoutManager()->activeThreeDRenderer();
    if (activeRenderer)
      {
      d->ScalarBarWidget->SetInteractor(activeRenderer->GetRenderWindow()->GetInteractor());
      }
    connect(d->VTKScalarBar, SIGNAL(modified()), threeDView, SLOT(scheduleRender()));
    }

  // Select the default color node
  d->setDefaultColorNode();
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setMRMLScene(vtkMRMLScene *scene)
{
  Q_D(qSlicerColorsModuleWidget);
  this->qSlicerAbstractModuleWidget::setMRMLScene(scene);
  d->setDefaultColorNode();
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setCurrentColorNode(vtkMRMLNode* colorNode)
{
  Q_D(qSlicerColorsModuleWidget);
  d->ColorTableComboBox->setCurrentNode(colorNode);
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::onMRMLColorNodeChanged(vtkMRMLNode* newColorNode)
{
  Q_D(qSlicerColorsModuleWidget);
  vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(newColorNode);
  if (!colorNode)
    {
    d->NumberOfColorsSpinBox->setEnabled(false);
    d->LUTRangeWidget->setEnabled(false);
    d->CopyColorNodeButton->setEnabled(false);
    return;
    }

  d->CopyColorNodeButton->setEnabled(true);
  d->NumberOfColorsSpinBox->setEnabled(
    colorNode->GetType() == vtkMRMLColorTableNode::User);
  d->NumberOfColorsSpinBox->setValue(colorNode->GetNumberOfColors());

  Q_ASSERT(d->NumberOfColorsSpinBox->value() == colorNode->GetNumberOfColors());

  if (colorNode->GetLookupTable())
    {
    d->LUTRangeWidget->setEnabled(true);
    double* range = colorNode->GetLookupTable()->GetRange();
    d->LUTRangeWidget->setRange(
      qMin(range[0], -1024.),qMax(range[1], 3071.));
    d->LUTRangeWidget->setValues(range[0], range[1]);
    d->ScalarBarWidget->GetScalarBarActor()->SetLookupTable(colorNode->GetLookupTable());
    }
  else
    {
    d->LUTRangeWidget->setEnabled(false);
    d->LUTRangeWidget->setValues(0.,0.);
    vtkMRMLProceduralColorNode *procColorNode = vtkMRMLProceduralColorNode::SafeDownCast(colorNode);
    if (procColorNode)
      {
      d->ScalarBarWidget->GetScalarBarActor()->SetLookupTable(procColorNode->GetColorTransferFunction());
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setNumberOfColors()
{
  Q_D(qSlicerColorsModuleWidget);
  if (!d->NumberOfColorsSpinBox->isEnabled())
    {
    return;
    }
  int newNumber = d->NumberOfColorsSpinBox->value();
  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(
    d->ColorTableComboBox->currentNode());
  colorTableNode->SetNumberOfColors(newNumber);
  colorTableNode->GetLookupTable()->SetRange(0, newNumber - 1);
  // update the slider manually
  bool blocked = d->LUTRangeWidget->blockSignals(true);
  d->LUTRangeWidget->setRange(0, newNumber - 1);
  d->LUTRangeWidget->blockSignals(blocked);
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setLookupTableRange(double min, double max)
{
  Q_D(qSlicerColorsModuleWidget);
  if (!d->LUTRangeWidget->isEnabled())
    {
    return;
    }
  vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(d->ColorTableComboBox->currentNode());
  Q_ASSERT(colorNode);
  colorNode->GetLookupTable()->SetRange(min, max);
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::copyCurrentColorNode()
{
  Q_D(qSlicerColorsModuleWidget);
  vtkMRMLColorNode* currentNode = vtkMRMLColorNode::SafeDownCast(
    d->ColorTableComboBox->currentNode());
  Q_ASSERT(currentNode);
  QString newColorName = QInputDialog::getText(
    this, "Transfer function name",
    "Please select a new name for the transfer function to create from copy",
    QLineEdit::Normal,
    QString(currentNode->GetName()) + QString("Copy"));
  if (newColorName.isEmpty())
    {
    return;
    }
  
  vtkMRMLColorTableNode *colorNode = d->colorLogic()->CopyNode(currentNode, newColorName.toLatin1());
  this->mrmlScene()->AddNode(colorNode);
  colorNode->Delete();
  d->ColorTableComboBox->setCurrentNode(colorNode);
}
