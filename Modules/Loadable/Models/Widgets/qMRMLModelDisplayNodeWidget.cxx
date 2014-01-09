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

// QT includes
#include <QColor>
#include <QDebug>

// qMRML includes
#include "qMRMLModelDisplayNodeWidget.h"
#include "ui_qMRMLModelDisplayNodeWidget.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelHierarchyNode.h>

// VTK includes
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Models
class qMRMLModelDisplayNodeWidgetPrivate: public Ui_qMRMLModelDisplayNodeWidget
{
  Q_DECLARE_PUBLIC(qMRMLModelDisplayNodeWidget);

protected:
  qMRMLModelDisplayNodeWidget* const q_ptr;

public:
  qMRMLModelDisplayNodeWidgetPrivate(qMRMLModelDisplayNodeWidget& object);
  void init();

  vtkSmartPointer<vtkMRMLModelDisplayNode> MRMLModelDisplayNode;
};

//------------------------------------------------------------------------------
qMRMLModelDisplayNodeWidgetPrivate::qMRMLModelDisplayNodeWidgetPrivate(qMRMLModelDisplayNodeWidget& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidgetPrivate::init()
{
  Q_Q(qMRMLModelDisplayNodeWidget);
  this->setupUi(q);
  
  QObject::connect(this->ScalarsVisibilityCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setScalarsVisibility(bool)));
  QObject::connect(this->ActiveScalarComboBox, SIGNAL(currentArrayChanged(QString)),
                   q, SLOT(setActiveScalarName(QString)));
  QObject::connect(this->ScalarsColorNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(setScalarsColorNode(vtkMRMLNode*)));
  q->setEnabled(this->MRMLModelDisplayNode.GetPointer() != 0);

  this->MRMLDisplayNodeWidget->setSelectedVisible(false);
}

//------------------------------------------------------------------------------
qMRMLModelDisplayNodeWidget::qMRMLModelDisplayNodeWidget(QWidget *_parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLModelDisplayNodeWidgetPrivate(*this))
{
  Q_D(qMRMLModelDisplayNodeWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLModelDisplayNodeWidget::~qMRMLModelDisplayNodeWidget()
{
}


//------------------------------------------------------------------------------
vtkMRMLModelDisplayNode* qMRMLModelDisplayNodeWidget::mrmlModelDisplayNode()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->MRMLModelDisplayNode;
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMRMLModelOrHierarchyNode(vtkMRMLNode* node)
{
  // can be set from a model node or a model hierarchy node
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(node);
  vtkMRMLModelHierarchyNode *hierarchyNode = vtkMRMLModelHierarchyNode::SafeDownCast(node);
  vtkMRMLModelDisplayNode *modelDisplayNode = 0;
  if (modelNode)
    {
    if (modelNode->GetDisplayNode())
      {
      modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
      }
    }
  else if (hierarchyNode)
    {
    modelDisplayNode = hierarchyNode->GetModelDisplayNode();
    }
  this->setMRMLModelDisplayNode(modelDisplayNode);
  
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMRMLModelDisplayNode(vtkMRMLNode* node)
{
  this->setMRMLModelDisplayNode(vtkMRMLModelDisplayNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMRMLModelDisplayNode(vtkMRMLModelDisplayNode* ModelDisplayNode)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  qvtkReconnect(d->MRMLModelDisplayNode, ModelDisplayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  d->MRMLModelDisplayNode = ModelDisplayNode;
  d->MRMLDisplayNodeWidget->setMRMLDisplayNode(ModelDisplayNode);
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarsVisibility(bool visible)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->MRMLModelDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLModelDisplayNode->SetScalarVisibility(visible);
}

//------------------------------------------------------------------------------
bool qMRMLModelDisplayNodeWidget::scalarsVisibility()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->ScalarsVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setActiveScalarName(const QString& arrayName)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->MRMLModelDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLModelDisplayNode->SetActiveScalarName(arrayName.toLatin1());
}

//------------------------------------------------------------------------------
QString qMRMLModelDisplayNodeWidget::activeScalarName()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  // TODO: use currentArrayName()
  vtkAbstractArray* array = d->ActiveScalarComboBox->currentArray();
  return array ? array->GetName() : "";
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarsColorNode(vtkMRMLNode* colorNode)
{
  this->setScalarsColorNode(vtkMRMLColorNode::SafeDownCast(colorNode));
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarsColorNode(vtkMRMLColorNode* colorNode)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->MRMLModelDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLModelDisplayNode->SetAndObserveColorNodeID(colorNode ? colorNode->GetID() : NULL);
}

//------------------------------------------------------------------------------
vtkMRMLColorNode* qMRMLModelDisplayNodeWidget::scalarsColorNode()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return vtkMRMLColorNode::SafeDownCast(
    d->ScalarsColorNodeComboBox->currentNode());
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLModelDisplayNodeWidget);
  this->setEnabled(d->MRMLModelDisplayNode.GetPointer() != 0);
  if (!d->MRMLModelDisplayNode.GetPointer())
    {
    return;
    }
  if (d->ScalarsVisibilityCheckBox->isChecked() !=
      d->MRMLModelDisplayNode->GetScalarVisibility())
    {
    d->ScalarsVisibilityCheckBox->setChecked(
      d->MRMLModelDisplayNode->GetScalarVisibility());
    }
  bool wasBlocking = d->ActiveScalarComboBox->blockSignals(true);
  d->ActiveScalarComboBox->setDataSet(
    d->MRMLModelDisplayNode->GetInputPolyData());
  d->ActiveScalarComboBox->blockSignals(wasBlocking);
  if (d->ActiveScalarComboBox->currentArrayName() !=
      d->MRMLModelDisplayNode->GetActiveScalarName())
    {
    d->ActiveScalarComboBox->setCurrentArray(
      d->MRMLModelDisplayNode->GetActiveScalarName());
    }
  // set the scalar range info
  QString scalarRangeString;
  if (!d->ActiveScalarComboBox->currentArrayName().isEmpty())
    {
    vtkPointData *pointData = NULL;
    if (d->MRMLModelDisplayNode->GetInputPolyData())
      {
      pointData = d->MRMLModelDisplayNode->GetInputPolyData()->GetPointData();
      }
    if (pointData &&
        pointData->GetArray(d->MRMLModelDisplayNode->GetActiveScalarName()))
      {
      double *range = pointData->GetArray(
        d->MRMLModelDisplayNode->GetActiveScalarName())->GetRange();
      if (range)
        {
        scalarRangeString = QString::number(range[0]) +
          QString(", ") +
          QString::number(range[1]);
        }
      }
    }
  d->ActiveScalarRangeLabel->setText(scalarRangeString);

  if (d->ScalarsColorNodeComboBox->mrmlScene() !=
      d->MRMLModelDisplayNode->GetScene())
    {
    d->ScalarsColorNodeComboBox->setMRMLScene(
      d->MRMLModelDisplayNode->GetScene());
    }
  if (d->ScalarsColorNodeComboBox->currentNodeID() !=
      d->MRMLModelDisplayNode->GetColorNodeID())
    {
    d->ScalarsColorNodeComboBox->setCurrentNodeID(
      d->MRMLModelDisplayNode->GetColorNodeID());
    }
}
