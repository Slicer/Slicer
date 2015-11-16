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
#include <QDebug>
#include <QInputDialog>

// CTK includes
#include <ctkVTKScalarsToColorsView.h>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerColorsModuleWidget.h"
#include "ui_qSlicerColorsModuleWidget.h"

// qMRMLWidget includes
#include "qMRMLColorModel.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"

// Slicer logic includes
#include <vtkSlicerColorLogic.h>
#include <vtkSlicerScalarBarActor.h>

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLFreeSurferProceduralColorNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkLookupTable.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
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
  vtkSlicerScalarBarActor* ScalarBarActor;
};

//-----------------------------------------------------------------------------
qSlicerColorsModuleWidgetPrivate::qSlicerColorsModuleWidgetPrivate(qSlicerColorsModuleWidget& object)
  : q_ptr(&object)
{
  this->ScalarBarWidget = vtkScalarBarWidget::New();
  this->ScalarBarActor = vtkSlicerScalarBarActor::New();
  this->ScalarBarWidget->SetScalarBarActor(this->ScalarBarActor);
  this->ScalarBarActor->SetOrientationToVertical();
  this->ScalarBarActor->SetNumberOfLabels(11);
  this->ScalarBarActor->SetTitle("(mm)");

  // it's a 2d actor, position it in screen space by percentages
  this->ScalarBarActor->SetPosition(0.1, 0.1);
  this->ScalarBarActor->SetWidth(0.1);
  this->ScalarBarActor->SetHeight(0.8);
}

//-----------------------------------------------------------------------------
qSlicerColorsModuleWidgetPrivate::~qSlicerColorsModuleWidgetPrivate()
{
  if (this->ScalarBarWidget)
    {
    this->ScalarBarWidget->Delete();
    this->ScalarBarWidget = 0;
    }
  if (this->ScalarBarActor)
  {
    this->ScalarBarActor->Delete();
    this->ScalarBarActor = 0;
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

  d->CopyColorNodeButton->setIcon(QIcon(":Icons/SlicerCopyColor.png"));

  d->VTKScalarBar->setScalarBarWidget(d->ScalarBarWidget);

  connect(d->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onMRMLColorNodeChanged(vtkMRMLNode*)));
  connect(d->NumberOfColorsSpinBox, SIGNAL(editingFinished()),
          this, SLOT(updateNumberOfColors()));
  connect(d->LUTRangeWidget, SIGNAL(valuesChanged(double,double)),
          this, SLOT(setLookupTableRange(double,double)));
  connect(d->CopyColorNodeButton, SIGNAL(clicked()),
          this, SLOT(copyCurrentColorNode()));

#if (VTK_MAJOR_VERSION > 5)
  if (d->UseColorNameAsLabelCheckBox->isChecked())
    {
    // string format
    d->ScalarBarActor->SetLabelFormat(" %.8s");
    }
  else
    {
    // number format
    d->ScalarBarActor->SetLabelFormat(" %#8.3f");
    }
  connect(d->UseColorNameAsLabelCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(setUseColorNameAsLabel(bool)));
#else
  d->UseColorNameAsLabelCheckBox->setEnabled(0);
#endif
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

  double validBounds[4] = {VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, 0., 1.};
  d->ContinuousScalarsToColorsWidget->view()->setValidBounds(validBounds);
  d->ContinuousScalarsToColorsWidget->view()->addColorTransferFunction(0);

  // Select the default color node
  d->setDefaultColorNode();
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setMRMLScene(vtkMRMLScene *scene)
{
  Q_D(qSlicerColorsModuleWidget);
  this->qSlicerAbstractModuleWidget::setMRMLScene(scene);
  d->setDefaultColorNode();

  // make sure the table view has the logic set so that it can access terminologies
  if (d->ColorView &&
      d->ColorView->colorModel())
    {
    d->ColorView->colorModel()->setMRMLColorLogic(d->colorLogic());
    }
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setCurrentColorNode(vtkMRMLNode* colorNode)
{
  Q_D(qSlicerColorsModuleWidget);
  d->ColorTableComboBox->setCurrentNode(colorNode);
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setUseColorNameAsLabel(bool useColorName)
{
  Q_D(qSlicerColorsModuleWidget);
#if (VTK_MAJOR_VERSION <= 5)
  d->ScalarBarActor->SetUseColorNameAsLabel(useColorName);
#else
  if (useColorName)
    {
    // text string format
    d->ScalarBarActor->SetLabelFormat(" %.8s");
    }
  else
    {
    // number format
    d->ScalarBarActor->SetLabelFormat(" %#8.3f");
    }
  d->ScalarBarActor->SetUseAnnotationAsLabel(useColorName);
#endif
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::onMRMLColorNodeChanged(vtkMRMLNode* newColorNode)
{
  Q_D(qSlicerColorsModuleWidget);

  vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(newColorNode);
  if (!colorNode)
    {
    d->NumberOfColorsSpinBox->setEnabled(false);
    d->NumberOfColorsSpinBox->setValue(0);
    d->LUTRangeWidget->setEnabled(false);
    d->LUTRangeWidget->setValues(0.,0.);
    d->CopyColorNodeButton->setEnabled(false);
    d->ContinuousScalarsToColorsWidget->setEnabled(false);
    d->VTKScalarBar->setTitle("(mm)");
    return;
    }

  d->CopyColorNodeButton->setEnabled(true);

  vtkMRMLColorTableNode *colorTableNode = vtkMRMLColorTableNode::SafeDownCast(colorNode);
  vtkMRMLProceduralColorNode *procColorNode = vtkMRMLProceduralColorNode::SafeDownCast(colorNode);
  vtkMRMLFreeSurferProceduralColorNode *fsColorNode = vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode);

  if (colorTableNode != NULL || fsColorNode != NULL)
    {
    // hide the procedural display, show the color table
    // freesurfer nodes are bit of a special case, they're defined
    // procedurally, but provide a look up table rather than a
    // color transfer function
    d->ContinuousDisplayCollapsibleButton->setCollapsed(true);
    d->ContinuousDisplayCollapsibleButton->setEnabled(false);
    d->ContinuousScalarsToColorsWidget->setEnabled(false);
    d->DisplayCollapsibleButton->setCollapsed(false);
    d->DisplayCollapsibleButton->setEnabled(true);

    // number of colors
    d->NumberOfColorsSpinBox->setEnabled(
      colorNode->GetType() == vtkMRMLColorTableNode::User);
    d->NumberOfColorsSpinBox->setValue(colorNode->GetNumberOfColors());
    Q_ASSERT(d->NumberOfColorsSpinBox->value() == colorNode->GetNumberOfColors());

    // set the range and the input for the scalar bar widget depending on if it's a freesurfer node or a color table node
    double *range = NULL;
    d->LUTRangeWidget->setEnabled(colorNode->GetType() == vtkMRMLColorTableNode::User);
    if (colorTableNode && colorTableNode->GetLookupTable())
      {
      range = colorTableNode->GetLookupTable()->GetRange();
      d->ScalarBarActor->SetLookupTable(colorTableNode->GetLookupTable());
      }
    else if (fsColorNode && fsColorNode->GetLookupTable())
      {
      range = fsColorNode->GetScalarsToColors()->GetRange();
      d->ScalarBarActor->SetLookupTable(fsColorNode->GetScalarsToColors());
      }
    if (range)
      {
      d->LUTRangeWidget->setRange(
        qMin(range[0], -1024.),qMax(range[1], 3071.));
      d->LUTRangeWidget->setValues(range[0], range[1]);
      }
    else
      {
      d->LUTRangeWidget->setEnabled(false);
      d->LUTRangeWidget->setValues(0.,0.);
      }
#if (VTK_MAJOR_VERSION > 5)
    // update the annotations from the superclass color node since this is a
    // color table or freesurfer color node
    int numberOfColors = colorNode->GetNumberOfColors();
    vtkIntArray* indexArray = vtkIntArray::New();
    indexArray->SetNumberOfValues(numberOfColors);
    vtkStringArray* stringArray = vtkStringArray::New();
    stringArray->SetNumberOfValues(numberOfColors);
    for (int colorIndex=0; colorIndex<numberOfColors; ++colorIndex)
      {
      indexArray->SetValue(colorIndex, colorIndex);
      stringArray->SetValue(colorIndex, colorNode->GetColorName(colorIndex));
      }
    d->ScalarBarActor->GetLookupTable()->SetAnnotations(indexArray, stringArray);
    indexArray->Delete();
    stringArray->Delete();
#endif
    }
  else if (procColorNode != NULL)
    {
    // hide and disable the color table display, show the continuous one
    d->NumberOfColorsSpinBox->setEnabled(false);
    d->NumberOfColorsSpinBox->setValue(0);
    d->LUTRangeWidget->setEnabled(false);
    d->LUTRangeWidget->setValues(0.,0.);
    d->DisplayCollapsibleButton->setCollapsed(true);
    d->DisplayCollapsibleButton->setEnabled(false);
    d->ContinuousDisplayCollapsibleButton->setCollapsed(false);
    d->ContinuousDisplayCollapsibleButton->setEnabled(true);

    // set the color transfer function to the widget
    d->ContinuousScalarsToColorsWidget->view()->setColorTransferFunctionToPlots(procColorNode->GetColorTransferFunction());

    // only allow editing of user types
    d->ContinuousScalarsToColorsWidget->setEnabled(
        procColorNode->GetType() == vtkMRMLColorNode::User);

    // set the lookup table on the scalar bar widget actor
    if (procColorNode->GetColorTransferFunction())
      {
      d->ScalarBarActor->SetLookupTable(procColorNode->GetColorTransferFunction());
      }
    }
  else
    {
    // not a valid type of color node
    d->LUTRangeWidget->setValues(0.,0.);
    }

  // add the color name to the scalar bar title
  std::string title = std::string(colorNode->GetName()) + std::string(" (mm)");
  d->VTKScalarBar->setTitle(title.c_str());
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::updateNumberOfColors()
{
  Q_D(qSlicerColorsModuleWidget);
  if (!d->NumberOfColorsSpinBox->isEnabled())
    {
    return;
    }
  int newNumber = d->NumberOfColorsSpinBox->value();
  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(
    d->ColorTableComboBox->currentNode());
  if (colorTableNode)
    {
    colorTableNode->SetNumberOfColors(newNumber);
    }
  else
    {
    qWarning() << "updateNumberOfColors: please select a discrete color table node to adjust the number of colors";
    }
  // update the slider which will trigger updating the table on the node
  d->LUTRangeWidget->setRange(0, newNumber - 1);
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setLookupTableRange(double min, double max)
{
  Q_D(qSlicerColorsModuleWidget);

  vtkMRMLNode *currentNode = d->ColorTableComboBox->currentNode();
  if (!currentNode)
    {
    return;
    }

  vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(currentNode);
  if (colorNode && colorNode->GetLookupTable())
    {
    colorNode->GetLookupTable()->SetRange(min, max);
    }
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::copyCurrentColorNode()
{
  Q_D(qSlicerColorsModuleWidget);
  vtkMRMLColorNode* currentNode = vtkMRMLColorNode::SafeDownCast(
    d->ColorTableComboBox->currentNode());
  Q_ASSERT(currentNode);
  QString newColorName = QInputDialog::getText(
    this, "Color node name",
    "Please select a new name for the color node copy",
    QLineEdit::Normal,
    QString(currentNode->GetName()) + QString("Copy"));
  if (newColorName.isEmpty())
    {
    return;
    }

  vtkMRMLColorNode *colorNode = NULL;
  if (currentNode->IsA("vtkMRMLColorTableNode") ||
      currentNode->IsA("vtkMRMLFreeSurferProceduralColorNode"))
    {
    colorNode = d->colorLogic()->CopyNode(currentNode, newColorName.toLatin1());
    }
  else if (currentNode->IsA("vtkMRMLProceduralColorNode"))
    {
    colorNode = d->colorLogic()->CopyProceduralNode(currentNode, newColorName.toLatin1());
    }
  else
    {
    qWarning() << "CopyCurrentColorNode: current node not of a color node type "
               << "that can be copied. It's a " << currentNode->GetClassName()
               << ", not a procedural or color table node";
    return;
    }
  if (!this->mrmlScene()->AddNode(colorNode))
    {
    qWarning() << "CopyCurrentColroNode: failed to add new node to scene";
    }
  colorNode->Delete();
  if (colorNode->GetID())
    {
    d->ColorTableComboBox->setCurrentNode(colorNode);
    }
}
