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

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerColorsModuleWidget.h"
#include "ui_qSlicerColorsModuleWidget.h"

// qMRMLWidget includes
#include "qMRMLColorModel.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"
#include "qMRMLSliceView.h"
#include "qMRMLSliceWidget.h"
#include "qMRMLSortFilterColorProxyModel.h"

// qMRMLWidget Colors includes
#include "qMRMLColorLegendDisplayNodeWidget.h"

// Slicer logic includes
#include <vtkSlicerColorLogic.h>

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLColorLegendDisplayNode.h>
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLAbstractDisplayableManager.h>
#include <vtkMRMLColorLegendDisplayableManager.h>

// VTK includes
#include <vtkBorderRepresentation.h>
#include <vtkColorTransferFunction.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkScalarBarWidget.h>
#include <vtkSlicerScalarBarActor.h>

// STD includes
#include <cstring>

//-----------------------------------------------------------------------------
class qSlicerColorsModuleWidgetPrivate : public Ui_qSlicerColorsModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerColorsModuleWidget);

protected:
  qSlicerColorsModuleWidget* const q_ptr;

public:
  qSlicerColorsModuleWidgetPrivate(qSlicerColorsModuleWidget& obj);
  virtual ~qSlicerColorsModuleWidgetPrivate();
  vtkSlicerColorLogic* colorLogic() const;
  void setDefaultColorNode();

  vtkWeakPointer<vtkMRMLDisplayableNode> DisplayableNode; /// Current displayable node
  vtkWeakPointer<vtkMRMLColorLegendDisplayNode>
    ColorLegendNode; /// color legend display node for a current displayable node
};

//-----------------------------------------------------------------------------
qSlicerColorsModuleWidgetPrivate::qSlicerColorsModuleWidgetPrivate(qSlicerColorsModuleWidget& object)
  : q_ptr(&object)
{}

//-----------------------------------------------------------------------------
qSlicerColorsModuleWidgetPrivate::~qSlicerColorsModuleWidgetPrivate() {}

//-----------------------------------------------------------------------------
vtkSlicerColorLogic* qSlicerColorsModuleWidgetPrivate::colorLogic() const
{
  Q_Q(const qSlicerColorsModuleWidget);
  return vtkSlicerColorLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidgetPrivate::setDefaultColorNode()
{
  Q_Q(qSlicerColorsModuleWidget);
  if (!q->mrmlScene() || !this->ColorTableComboBox || this->ColorTableComboBox->currentNode() != nullptr)
  {
    return;
  }
  const char* defaultID = this->colorLogic()->GetDefaultLabelMapColorNodeID();
  vtkMRMLColorNode* defaultNode = vtkMRMLColorNode::SafeDownCast(q->mrmlScene()->GetNodeByID(defaultID));
  this->ColorTableComboBox->setCurrentNode(defaultNode);
}

//-----------------------------------------------------------------------------
qSlicerColorsModuleWidget::qSlicerColorsModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerColorsModuleWidgetPrivate(*this))
{}

//-----------------------------------------------------------------------------
qSlicerColorsModuleWidget::~qSlicerColorsModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setup()
{
  Q_D(qSlicerColorsModuleWidget);

  d->setupUi(this);

  d->CopyColorNodeButton->setIcon(QIcon(":Icons/SlicerCopyColor.png"));
  d->AddColorTableNodeButton->setIcon(QIcon(":Icons/SlicerNewColor.png"));
  d->AddNewColorButton->setIcon(QIcon(":Icons/Add.png"));
  d->RemoveCurrentColorButton->setIcon(QIcon(":Icons/Remove.png"));

  connect(
    d->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onMRMLColorNodeChanged(vtkMRMLNode*)));
  // We update on editingFinished (when the spinbox loses focus) not immediately when the value changes.
  // This is useful because colors do not get deleted when the number is temporarily decreased during editing.
  connect(d->NumberOfColorsSpinBox, SIGNAL(editingFinished()), this, SLOT(updateNumberOfColors()));
  connect(d->LUTRangeWidget, SIGNAL(valuesChanged(double, double)), this, SLOT(setLookupTableRange(double, double)));
  connect(d->CopyColorNodeButton, SIGNAL(clicked()), this, SLOT(copyCurrentColorNode()));
  connect(d->AddColorTableNodeButton, SIGNAL(clicked()), this, SLOT(addNewColorTableNode()));
  connect(d->AddNewColorButton, SIGNAL(clicked()), this, SLOT(addNewColorInCurrentNode()));
  connect(d->RemoveCurrentColorButton, SIGNAL(clicked()), this, SLOT(removeCurrentColorEntry()));

  double validBounds[4] = { VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, 0., 1. };
  d->ContinuousScalarsToColorsWidget->view()->setValidBounds(validBounds);
  d->ContinuousScalarsToColorsWidget->view()->addColorTransferFunction(nullptr);

  connect(d->DisplayableNodeComboBox,
          SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this,
          SLOT(onDisplayableNodeChanged(vtkMRMLNode*)));

  connect(d->CreateColorLegendButton, SIGNAL(clicked()), this, SLOT(createColorLegend()));
  connect(d->DeleteColorLegendButton, SIGNAL(clicked()), this, SLOT(deleteColorLegend()));
  connect(d->UseCurrentColorsButton, SIGNAL(clicked()), this, SLOT(useCurrentColorsForColorLegend()));

  // Select the default color node
  d->setDefaultColorNode();
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setMRMLScene(vtkMRMLScene* scene)
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
    d->AddNewColorButton->setEnabled(false);
    d->RemoveCurrentColorButton->setEnabled(false);
    d->NumberOfColorsSpinBox->setEnabled(false);
    d->NumberOfColorsSpinBox->setValue(0);
    d->LUTRangeWidget->setEnabled(false);
    d->LUTRangeWidget->setValues(0., 0.);
    d->CopyColorNodeButton->setEnabled(false);
    d->ContinuousScalarsToColorsWidget->setEnabled(false);
    return;
  }

  d->CopyColorNodeButton->setEnabled(true);

  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(colorNode);
  vtkMRMLProceduralColorNode* procColorNode = vtkMRMLProceduralColorNode::SafeDownCast(colorNode);
  if (colorTableNode && !procColorNode)
  {
    // hide the procedural display, show the color table
    // freesurfer nodes are bit of a special case, they're defined
    // procedurally, but provide a look up table rather than a
    // color transfer function
    d->ContinuousScalarsToColorsWidget->hide();
    d->ColorTableFrame->show();
    d->EditColorsCollapsibleButton->setText(tr("Discrete table"));

    bool editable = (colorNode->GetType() == vtkMRMLColorTableNode::User);

    // number of colors
    d->NumberOfColorsSpinBox->setEnabled(editable);
    d->NumberOfColorsSpinBox->setValue(colorNode->GetNumberOfColors());
    Q_ASSERT(d->NumberOfColorsSpinBox->value() == colorNode->GetNumberOfColors());

    d->AddNewColorButton->setEnabled(editable);
    d->RemoveCurrentColorButton->setEnabled(editable);

    // set the range and the input for the color widget depending on if it's a freesurfer node or a color table node
    double* range = nullptr;
    d->LUTRangeWidget->setEnabled(editable);
    if (colorTableNode && colorTableNode->GetLookupTable())
    {
      range = colorTableNode->GetLookupTable()->GetRange();
    }
    disconnect(
      d->LUTRangeWidget, SIGNAL(valuesChanged(double, double)), this, SLOT(setLookupTableRange(double, double)));
    if (range)
    {
      // Make the range a bit (10%) larger than the values to allow some room for
      // adjustment. More adjustment can be done by manually setting the range on the GUI.
      double rangeMargin = (range[1] - range[0]) * 0.1;
      if (rangeMargin == 0)
      {
        rangeMargin = 10.0;
      }
      d->LUTRangeWidget->setRange(range[0] - rangeMargin, range[1] + rangeMargin);
      d->LUTRangeWidget->setValues(range[0], range[1]);
    }
    else
    {
      d->LUTRangeWidget->setEnabled(false);
      d->LUTRangeWidget->setValues(0., 0.);
    }
    connect(d->LUTRangeWidget, SIGNAL(valuesChanged(double, double)), this, SLOT(setLookupTableRange(double, double)));
    // update the annotations from the superclass color node since this is a
    // color table or freesurfer color node
    int numberOfColors = colorNode->GetNumberOfColors();
    vtkNew<vtkIntArray> indexArray;
    indexArray->SetNumberOfValues(numberOfColors);
    vtkNew<vtkStringArray> stringArray;
    stringArray->SetNumberOfValues(numberOfColors);
    for (int colorIndex = 0; colorIndex < numberOfColors; ++colorIndex)
    {
      indexArray->SetValue(colorIndex, colorIndex);
      stringArray->SetValue(colorIndex, colorNode->GetColorName(colorIndex));
    }
  }
  else if (procColorNode && !colorTableNode)
  {
    // hide and disable the color table display, show the continuous one
    d->NumberOfColorsSpinBox->setEnabled(false);
    d->NumberOfColorsSpinBox->setValue(0);
    d->LUTRangeWidget->setEnabled(false);
    d->LUTRangeWidget->setValues(0., 0.);
    d->ColorTableFrame->hide();
    d->ContinuousScalarsToColorsWidget->show();
    d->EditColorsCollapsibleButton->setText(tr("Continuous scale"));

    // set the color transfer function to the widget
    bool editable = procColorNode->GetType() == vtkMRMLColorNode::User; // only allow editing of user types
    d->ContinuousScalarsToColorsWidget->view()->setColorTransferFunctionToPlots(
      procColorNode->GetColorTransferFunction(), editable);
    d->ContinuousScalarsToColorsWidget->setEnabled(editable);
  }
  else
  {
    // not a valid type of color node
    d->LUTRangeWidget->setValues(0., 0.);
  }
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
  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(d->ColorTableComboBox->currentNode());
  if (colorTableNode)
  {
    colorTableNode->SetNumberOfColors(newNumber);
  }
  else
  {
    qWarning() << "updateNumberOfColors: please select a discrete color table node to adjust the number of colors";
  }
}

//-----------------------------------------------------------------------------
void qSlicerColorsModuleWidget::setLookupTableRange(double min, double max)
{
  Q_D(qSlicerColorsModuleWidget);

  vtkMRMLNode* currentNode = d->ColorTableComboBox->currentNode();
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
  vtkMRMLColorNode* currentNode = vtkMRMLColorNode::SafeDownCast(d->ColorTableComboBox->currentNode());
  Q_ASSERT(currentNode);
  QString newColorName = QInputDialog::getText(this,
                                               "Color node name",
                                               "Please select a new name for the color node copy",
                                               QLineEdit::Normal,
                                               QString(currentNode->GetName()) + QString("Copy"));
  if (newColorName.isEmpty())
  {
    return;
  }

  vtkMRMLColorNode* colorNode = nullptr;
  if (currentNode->IsA("vtkMRMLColorTableNode") || currentNode->IsA("vtkMRMLFreeSurferProceduralColorNode"))
  {
    colorNode = d->colorLogic()->CopyNode(currentNode, newColorName.toUtf8());
  }
  else if (currentNode->IsA("vtkMRMLProceduralColorNode"))
  {
    colorNode = d->colorLogic()->CopyProceduralNode(currentNode, newColorName.toUtf8());
  }
  else
  {
    qWarning() << Q_FUNC_INFO << "Current node not of a color node type "
               << "that can be copied. It's a " << currentNode->GetClassName()
               << ", not a procedural or color table node";
    return;
  }
  if (!this->mrmlScene()->AddNode(colorNode))
  {
    qWarning() << Q_FUNC_INFO << "Failed to add new node to scene";
  }
  colorNode->Delete();
  if (colorNode->GetID())
  {
    d->ColorTableComboBox->setCurrentNode(colorNode);
  }
}

//-----------------------------------------------------------
void qSlicerColorsModuleWidget::addNewColorTableNode()
{
  Q_D(qSlicerColorsModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << "failed: Invalid scene";
    return;
  }

  // Get name (it is hard to rename later)
  std::string nodeName = this->mrmlScene()->GenerateUniqueName("ColorTable");
  QString newColorName = QInputDialog::getText(this,
                                               "Color node name",
                                               "Please select a name for the color node",
                                               QLineEdit::Normal,
                                               QString::fromStdString(nodeName));
  if (newColorName.isEmpty())
  {
    return;
  }

  // Create and add new empty color table node
  vtkSmartPointer<vtkMRMLColorTableNode> newColorTableNode = vtkSmartPointer<vtkMRMLColorTableNode>::Take(
    vtkMRMLColorTableNode::SafeDownCast(this->mrmlScene()->CreateNodeByClass("vtkMRMLColorTableNode")));
  if (!newColorTableNode)
  {
    qCritical() << Q_FUNC_INFO << "failed: could not instantiate vtkMRMLColorTableNode";
    return;
  }
  newColorTableNode->SetTypeToUser();

  newColorTableNode->SetHideFromEditors(false); // makes the color table show up in "Save data" window
  newColorTableNode->SetName(newColorName.toStdString().c_str());

  if (!this->mrmlScene()->AddNode(newColorTableNode))
  {
    qCritical() << Q_FUNC_INFO << "Failed to add new node to scene";
  }

  // Select new node in module
  d->ColorTableComboBox->setCurrentNode(newColorTableNode);
}

//-----------------------------------------------------------
void qSlicerColorsModuleWidget::addNewColorInCurrentNode()
{
  Q_D(qSlicerColorsModuleWidget);
  vtkMRMLColorTableNode* currentNode = vtkMRMLColorTableNode::SafeDownCast(d->ColorTableComboBox->currentNode());
  Q_ASSERT(currentNode);

  // Add a color to the current (User type) color table, at the end
  int newNumber = currentNode->GetNumberOfColors() + 1;
  if (newNumber == 1)
  {
    // First color in the color table, leave it undefined because label==0 corresponds to the background color
    // and add one more color to the table that the user will set to something meaningful.
    newNumber++;
  }
  currentNode->SetNumberOfColors(newNumber);
  //: This is the default name for a new color in a color table
  currentNode->SetColor(newNumber - 1, tr("Unnamed").toStdString().c_str(), 0.5, 0.5, 0.5, 1.0);
  // Update spinbox on GUI as well
  QSignalBlocker blocker(d->NumberOfColorsSpinBox);
  d->NumberOfColorsSpinBox->setValue(newNumber);
}

//-----------------------------------------------------------
void qSlicerColorsModuleWidget::removeCurrentColorEntry()
{
  Q_D(qSlicerColorsModuleWidget);
  vtkMRMLColorTableNode* currentNode = vtkMRMLColorTableNode::SafeDownCast(d->ColorTableComboBox->currentNode());
  Q_ASSERT(currentNode);
  int colorIndex = -1;
  if (d->ColorTableFrame->isVisible())
  {
    QModelIndex colorModelIndex = d->ColorView->sortFilterProxyModel()->mapToSource(d->ColorView->currentIndex());
    colorIndex = d->ColorView->colorModel()->colorFromIndex(colorModelIndex);
  }
  if (colorIndex >= 0)
  {
    currentNode->RemoveColor(colorIndex);
  }
}

//-----------------------------------------------------------
void qSlicerColorsModuleWidget::onDisplayableNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerColorsModuleWidget);
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  qvtkReconnect(d->DisplayableNode,
                displayableNode,
                vtkMRMLDisplayableNode::DisplayModifiedEvent,
                this,
                SLOT(updateColorLegendFromMRML()));
  d->DisplayableNode = displayableNode;
  updateColorLegendFromMRML();
}

//-----------------------------------------------------------
void qSlicerColorsModuleWidget::updateColorLegendFromMRML()
{
  Q_D(qSlicerColorsModuleWidget);
  vtkMRMLDisplayNode* displayNode = nullptr;
  if (d->DisplayableNode)
  {
    displayNode = d->DisplayableNode->GetDisplayNode();
  }
  d->ColorLegendNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(d->DisplayableNode);
  d->CreateColorLegendButton->setVisible(displayNode && !d->ColorLegendNode);
  d->UseCurrentColorsButton->setVisible(displayNode && d->ColorLegendNode);
  d->DeleteColorLegendButton->setEnabled(d->ColorLegendNode);
  // d->ColorLegendDisplayNodeWidget->setEnabled(false);
  d->ColorLegendDisplayNodeWidget->setMRMLColorLegendDisplayNode(d->ColorLegendNode);
}

//-----------------------------------------------------------
void qSlicerColorsModuleWidget::createColorLegend()
{
  Q_D(qSlicerColorsModuleWidget);
  if (!d->DisplayableNode || !d->colorLogic() || d->ColorLegendNode)
  {
    return;
  }
  vtkMRMLDisplayNode* displayNode = d->DisplayableNode->GetDisplayNode();
  if (!displayNode)
  {
    return;
  }
  if (!displayNode->GetColorNode())
  {
    // If there is no color node selected in this display node then choose the current color node
    this->useCurrentColorsForColorLegend();
  }
  vtkMRMLColorLegendDisplayNode* colorLegendDisplayNode =
    vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(d->DisplayableNode);
  if (!colorLegendDisplayNode)
  {
    qWarning() << "createColorLegend: failed to add display node to scene";
  }
}

//-----------------------------------------------------------
void qSlicerColorsModuleWidget::deleteColorLegend()
{
  Q_D(qSlicerColorsModuleWidget);
  if (!d->ColorLegendNode || !d->ColorLegendNode->GetScene())
  {
    return;
  }
  d->ColorLegendNode->GetScene()->RemoveNode(d->ColorLegendNode);
}

//-----------------------------------------------------------
void qSlicerColorsModuleWidget::useCurrentColorsForColorLegend()
{
  Q_D(qSlicerColorsModuleWidget);
  vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(d->ColorTableComboBox->currentNode());
  if (!colorNode)
  {
    return;
  }
  vtkMRMLDisplayNode* displayNode = d->DisplayableNode->GetDisplayNode();
  if (!displayNode)
  {
    return;
  }
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());
}

//-----------------------------------------------------------
bool qSlicerColorsModuleWidget::setEditedNode(vtkMRMLNode* node,
                                              QString role /* = QString()*/,
                                              QString context /* = QString()*/)
{
  Q_D(qSlicerColorsModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkMRMLColorNode::SafeDownCast(node))
  {
    d->ColorTableComboBox->setCurrentNode(node);
    return true;
  }

  return false;
}
