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
#include <QButtonGroup>

// qMRML includes
#include "qMRMLClipNodeWidget.h"
#include "qMRMLNodeComboBox.h"
#include "ui_qMRMLClipNodeWidget.h"

// MRML includes
#include <vtkMRMLClipNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qMRMLClipNodeWidgetPrivate : public Ui_qMRMLClipNodeWidget
{
  Q_DECLARE_PUBLIC(qMRMLClipNodeWidget);

protected:
  qMRMLClipNodeWidget* const q_ptr;

public:
  qMRMLClipNodeWidgetPrivate(qMRMLClipNodeWidget& object);
  void init();

  vtkWeakPointer<vtkMRMLClipNode> MRMLClipNode;
  bool IsUpdatingWidgetFromMRML;
};

//------------------------------------------------------------------------------
qMRMLClipNodeWidgetPrivate::qMRMLClipNodeWidgetPrivate(qMRMLClipNodeWidget& object)
  : q_ptr(&object)
{
  this->IsUpdatingWidgetFromMRML = false;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidgetPrivate::init()
{
  Q_Q(qMRMLClipNodeWidget);
  this->setupUi(q);

  QButtonGroup* clipTypeGroup = new QButtonGroup(q);
  clipTypeGroup->addButton(this->UnionRadioButton);
  clipTypeGroup->addButton(this->IntersectionRadioButton);

  QObject::connect(this->UnionRadioButton, SIGNAL(toggled(bool)), q, SLOT(updateNodeClipType()));
  QObject::connect(this->IntersectionRadioButton, SIGNAL(toggled(bool)), q, SLOT(updateNodeClipType()));

  q->setEnabled(this->MRMLClipNode != nullptr);
}

//------------------------------------------------------------------------------
qMRMLClipNodeWidget::qMRMLClipNodeWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLClipNodeWidgetPrivate(*this))
{
  Q_D(qMRMLClipNodeWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLClipNodeWidget::~qMRMLClipNodeWidget() = default;

//------------------------------------------------------------------------------
vtkMRMLClipNode* qMRMLClipNodeWidget::mrmlClipNode() const
{
  Q_D(const qMRMLClipNodeWidget);
  return d->MRMLClipNode;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::setMRMLClipNode(vtkMRMLNode* node)
{
  this->setMRMLClipNode(vtkMRMLClipNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::setMRMLClipNode(vtkMRMLClipNode* clipNode)
{
  Q_D(qMRMLClipNodeWidget);
  qvtkReconnect(d->MRMLClipNode, clipNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->MRMLClipNode = clipNode;
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::setClipType(int type)
{
  Q_D(qMRMLClipNodeWidget);
  if (!d->MRMLClipNode)
  {
    return;
  }
  d->MRMLClipNode->SetClipType(type);
}

//------------------------------------------------------------------------------
int qMRMLClipNodeWidget::clipType() const
{
  Q_D(const qMRMLClipNodeWidget);
  return d->UnionRadioButton->isChecked() ? vtkMRMLClipNode::ClipUnion : vtkMRMLClipNode::ClipIntersection;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLClipNodeWidget);
  if (d->IsUpdatingWidgetFromMRML)
  {
    return;
  }

  this->setEnabled(d->MRMLClipNode);
  if (!d->MRMLClipNode)
  {
    return;
  }

  bool oldUpdating = d->IsUpdatingWidgetFromMRML;
  d->IsUpdatingWidgetFromMRML = true;

  bool wasBlocking = d->UnionRadioButton->blockSignals(true);
  d->UnionRadioButton->setChecked(d->MRMLClipNode->GetClipType() == vtkMRMLClipNode::ClipUnion);
  d->UnionRadioButton->blockSignals(wasBlocking);

  wasBlocking = d->IntersectionRadioButton->blockSignals(true);
  d->IntersectionRadioButton->setChecked(d->MRMLClipNode->GetClipType() == vtkMRMLClipNode::ClipIntersection);
  d->IntersectionRadioButton->blockSignals(wasBlocking);

  bool needToUpdateReferences = this->needToUpdateClippingNodeFrame();
  if (needToUpdateReferences)
  {
    this->updateClippingNodeFrame();
  }

  QList<QButtonGroup*> clipButtonGroups = d->ClipNodeFrame->findChildren<QButtonGroup*>("ClipButtonGroup");
  for (QButtonGroup* clipButtonGroup : clipButtonGroups)
  {
    std::map<QAbstractButton*, bool> buttonWasBlocking;
    for (QAbstractButton* button : clipButtonGroup->buttons())
    {
      buttonWasBlocking[button] = button->blockSignals(true);
    }

    int nodeIndex = clipButtonGroup->property("Index").toInt();
    int clipState = d->MRMLClipNode->GetNthClippingNodeState(nodeIndex);
    for (QAbstractButton* button : clipButtonGroup->buttons())
    {
      switch (clipState)
      {
        case vtkMRMLClipNode::ClipPositiveSpace: button->setChecked(button->objectName() == "PositiveRadioButton"); break;
        case vtkMRMLClipNode::ClipNegativeSpace: button->setChecked(button->objectName() == "NegativeRadioButton"); break;
        case vtkMRMLClipNode::ClipOff: button->setChecked(button->objectName() == "OffRadioButton"); break;
        default: break;
      }
    }

    for (QAbstractButton* button : clipButtonGroup->buttons())
    {
      button->blockSignals(buttonWasBlocking[button]);
    }
  }

  d->IsUpdatingWidgetFromMRML = oldUpdating;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::updateClippingNodeFrame()
{
  Q_D(qMRMLClipNodeWidget);
  if (!d->MRMLClipNode)
  {
    return;
  }

  QStringList clipNodeClasses;
  clipNodeClasses << "vtkMRMLSliceNode";
  clipNodeClasses << "vtkMRMLMarkupsPlaneNode";
  clipNodeClasses << "vtkMRMLMarkupsROINode";
  clipNodeClasses << "vtkMRMLModelNode";

  QLayoutItem* item;
  while ((item = d->ClipNodeFrame->layout()->takeAt(0)))
  {
    QWidget* widget = item->widget();
    widget->setParent(nullptr);
    widget->setVisible(false);
    widget->deleteLater();
  }

  for (int i = 0; i < d->MRMLClipNode->GetNumberOfClippingNodes() + 1; ++i)
  {
    vtkMRMLNode* clippingNode = nullptr;
    if (i < d->MRMLClipNode->GetNumberOfClippingNodes())
    {
      clippingNode = d->MRMLClipNode->GetNthClippingNode(i);
    }
    qMRMLNodeComboBox* clipNodeSelector = new qMRMLNodeComboBox();
    clipNodeSelector->setNoneEnabled(true);
    clipNodeSelector->setAddEnabled(false);
    clipNodeSelector->setMRMLScene(this->mrmlScene());
    clipNodeSelector->setNodeTypes(clipNodeClasses);
    clipNodeSelector->setCurrentNode(clippingNode);
    QObject::connect(clipNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(updateClippingNodeFromWidget()));
    clipNodeSelector->setProperty("NewSelector", QVariant(clippingNode == nullptr));

    QHBoxLayout* clipNodeLayout = new QHBoxLayout();
    clipNodeLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* clipNodeFrame = new QWidget();
    clipNodeFrame->setLayout(clipNodeLayout);
    clipNodeFrame->setContentsMargins(0, 0, 0, 0);
    clipNodeFrame->layout()->addWidget(clipNodeSelector);

    QList<QObject*> objectsList;

    QButtonGroup* clipTypeGroup = new QButtonGroup(clipNodeFrame);
    clipTypeGroup->setObjectName("ClipButtonGroup");
    objectsList << clipTypeGroup;

    QRadioButton* offClipButton = new QRadioButton();
    offClipButton->setObjectName("OffRadioButton");
    offClipButton->setText("Off");
    clipTypeGroup->addButton(offClipButton);
    clipNodeFrame->layout()->addWidget(offClipButton);
    QObject::connect(offClipButton, SIGNAL(toggled(bool)), this, SLOT(updateClippingNodeFromWidget()));
    objectsList << offClipButton;

    QRadioButton* positiveClipButton = new QRadioButton();
    positiveClipButton->setObjectName("PositiveRadioButton");
    positiveClipButton->setText("Positive");
    positiveClipButton->setIcon(QIcon(":/Icons/GreySpacePositive.png"));
    clipTypeGroup->addButton(positiveClipButton);
    clipNodeFrame->layout()->addWidget(positiveClipButton);
    QObject::connect(positiveClipButton, SIGNAL(toggled(bool)), this, SLOT(updateClippingNodeFromWidget()));
    objectsList << positiveClipButton;

    QRadioButton* negativeClipButton = new QRadioButton();
    negativeClipButton->setObjectName("NegativeRadioButton");
    negativeClipButton->setText("Negative");
    negativeClipButton->setIcon(QIcon(":/Icons/GreySpaceNegative.png"));
    clipTypeGroup->addButton(negativeClipButton);
    clipNodeFrame->layout()->addWidget(negativeClipButton);
    QObject::connect(negativeClipButton, SIGNAL(toggled(bool)), this, SLOT(updateClippingNodeFromWidget()));
    objectsList << negativeClipButton;

    for (QObject* object : objectsList)
    {
      QWidget* widget = qobject_cast<QWidget*>(object);
      if (widget)
      {
        widget->setEnabled(clippingNode != nullptr);
      }

      if (clippingNode)
      {
        object->setProperty("ID", QVariant(qMRMLWidget::safeQStringFromUtf8Ptr(clippingNode->GetID())));
      }
      object->setProperty("Index", QVariant(i));
    }

    d->ClipNodeFrame->layout()->addWidget(clipNodeFrame);
  }
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::updateMRMLFromWidget()
{
  Q_D(qMRMLClipNodeWidget);
  if (d->IsUpdatingWidgetFromMRML)
  {
    return;
  }

  if (!d->MRMLClipNode)
  {
    return;
  }

  MRMLNodeModifyBlocker blocker(d->MRMLClipNode);
  d->MRMLClipNode->SetClipType(this->clipType());
  this->updateClippingNodeFromWidget();
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::updateClippingNodeFromWidget()
{
  Q_D(qMRMLClipNodeWidget);
  if (!d->MRMLClipNode)
  {
    return;
  }

  MRMLNodeModifyBlocker blocker(d->MRMLClipNode);

  if (this->needToUpdateClippingNodeFrame())
  {
    std::vector<vtkMRMLNode*> widgetClippingNodes;
    QList<qMRMLNodeComboBox*> clipNodeComboBoxes = d->ClipNodeFrame->findChildren<qMRMLNodeComboBox*>();
    for (qMRMLNodeComboBox* clipNodeComboBox : clipNodeComboBoxes)
    {
      vtkMRMLNode* clippingNode = clipNodeComboBox->currentNode();
      if (!clippingNode)
      {
        continue;
      }
      widgetClippingNodes.push_back(clippingNode);
    }

    d->MRMLClipNode->RemoveAllClippingNodeIDs();
    for (vtkMRMLNode* clippingNode : widgetClippingNodes)
    {
      d->MRMLClipNode->AddAndObserveClippingNodeID(clippingNode->GetID());
    }
  }

  QList<QButtonGroup*> clipButtonGroups = d->ClipNodeFrame->findChildren<QButtonGroup*>("ClipButtonGroup");
  for (QButtonGroup* clipButtonGroup : clipButtonGroups)
  {
    QAbstractButton* checkedButton = clipButtonGroup->checkedButton();
    if (!checkedButton)
    {
      continue;
    }

    std::string clipNodeID = checkedButton->property("ID").toString().toStdString();
    if (clipNodeID.empty())
    {
      continue;
    }

    int clipNodeIndex = checkedButton->property("Index").toInt();
    vtkMRMLNode* clippingNode = this->mrmlClipNode()->GetNthClippingNode(clipNodeIndex);
    if (!clippingNode)
    {
      continue;
    }

    int clipState = vtkMRMLClipNode::ClipPositiveSpace;
    if (checkedButton->objectName() == "OffRadioButton")
    {
      clipState = vtkMRMLClipNode::ClipOff;
    }
    else if (checkedButton->objectName() == "PositiveRadioButton")
    {
      clipState = vtkMRMLClipNode::ClipPositiveSpace;
    }
    else if (checkedButton->objectName() == "NegativeRadioButton")
    {
      clipState = vtkMRMLClipNode::ClipNegativeSpace;
    }
    d->MRMLClipNode->SetNthClippingNodeState(clipNodeIndex, clipState);
  }

  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
bool qMRMLClipNodeWidget::needToUpdateClippingNodeFrame() const
{
  Q_D(const qMRMLClipNodeWidget);

  std::vector<vtkMRMLNode*> currentClippingNodes;
  for (int i = 0; i < d->MRMLClipNode->GetNumberOfClippingNodes(); ++i)
  {
    currentClippingNodes.push_back(d->MRMLClipNode->GetNthClippingNode(i));
  }

  QList<qMRMLNodeComboBox*> clipNodeComboBoxes = d->ClipNodeFrame->findChildren<qMRMLNodeComboBox*>();
  if (clipNodeComboBoxes.size() == 0)
  {
    return true;
  }

  std::vector<vtkMRMLNode*> widgetClippingNodes;
  for (qMRMLNodeComboBox* clipNodeComboBox : clipNodeComboBoxes)
  {
    vtkMRMLNode* clipNode = clipNodeComboBox->currentNode();
    if (clipNodeComboBox->property("NewSelector").toBool())
    {
      if (clipNode)
      {
        // A node has been selected in the new node combobox.
        return true;
      }
      continue;
    }
    widgetClippingNodes.push_back(clipNode);
  }

  bool needToUpdateReferences = widgetClippingNodes.size() != currentClippingNodes.size();
  for (int i = 0; i < currentClippingNodes.size() && !needToUpdateReferences; ++i)
  {
    if (currentClippingNodes[i] != widgetClippingNodes[i])
    {
      needToUpdateReferences = true;
    }
  }

  return needToUpdateReferences;
}

//------------------------------------------------------------------------------
int qMRMLClipNodeWidget::clipState(vtkMRMLNode* node) const
{
  Q_D(const qMRMLClipNodeWidget);
  if (!d->MRMLClipNode)
  {
    return vtkMRMLClipNode::ClipOff;
  }
  return d->MRMLClipNode->GetClippingNodeState(node);
}

//------------------------------------------------------------------------------
int qMRMLClipNodeWidget::clipState(const char* nodeID) const
{
  Q_D(const qMRMLClipNodeWidget);
  if (!d->MRMLClipNode)
  {
    return vtkMRMLClipNode::ClipOff;
  }
  return d->MRMLClipNode->GetClippingNodeState(nodeID);
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::setClipState(vtkMRMLNode* node, int state)
{
  Q_D(const qMRMLClipNodeWidget);
  if (!d->MRMLClipNode)
  {
    return;
  }
  d->MRMLClipNode->SetClippingNodeState(node, state);
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::setClipState(const char* nodeID, int state)
{
  Q_D(const qMRMLClipNodeWidget);
  if (!d->MRMLClipNode)
  {
    return;
  }
  d->MRMLClipNode->SetClippingNodeState(nodeID, state);
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::updateNodeClipType()
{
  this->setClipType(this->clipType());
}
