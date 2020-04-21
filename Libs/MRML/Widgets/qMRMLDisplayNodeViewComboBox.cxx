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

// MRMLWidgets includes
#include "qMRMLDisplayNodeViewComboBox.h"

// MRML includes
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLAbstractViewNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// -----------------------------------------------------------------------------
class qMRMLDisplayNodeViewComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qMRMLDisplayNodeViewComboBox);
protected:
  qMRMLDisplayNodeViewComboBox* const q_ptr;

public:
  qMRMLDisplayNodeViewComboBoxPrivate(qMRMLDisplayNodeViewComboBox& object);
  void init();
  vtkSmartPointer<vtkMRMLDisplayNode> MRMLDisplayNode;
  bool IsUpdatingWidgetFromMRML;
};

// -----------------------------------------------------------------------------
qMRMLDisplayNodeViewComboBoxPrivate
::qMRMLDisplayNodeViewComboBoxPrivate(qMRMLDisplayNodeViewComboBox& object)
  : q_ptr(&object)
  , IsUpdatingWidgetFromMRML(false)
{
}

// -----------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBoxPrivate::init()
{
  Q_Q(qMRMLDisplayNodeViewComboBox);
  q->setNodeTypes(QStringList(QString("vtkMRMLAbstractViewNode")));
  q->setBaseName("View");
  QObject::connect(q, SIGNAL(checkedNodesChanged()),
                   q, SLOT(updateMRMLFromWidget()));
  QObject::connect(q, SIGNAL(nodeAdded(vtkMRMLNode*)),
                   q, SLOT(updateWidgetFromMRML()));
  QObject::connect(q, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode*)),
                   q, SLOT(updateWidgetFromMRML()));
}

// --------------------------------------------------------------------------
// qMRMLDisplayNodeViewComboBox

// --------------------------------------------------------------------------
qMRMLDisplayNodeViewComboBox::qMRMLDisplayNodeViewComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLDisplayNodeViewComboBoxPrivate(*this))
{
  Q_D(qMRMLDisplayNodeViewComboBox);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLDisplayNodeViewComboBox::~qMRMLDisplayNodeViewComboBox() = default;

// --------------------------------------------------------------------------
vtkMRMLDisplayNode* qMRMLDisplayNodeViewComboBox::mrmlDisplayNode()const
{
  Q_D(const qMRMLDisplayNodeViewComboBox);
  return d->MRMLDisplayNode;
}

// --------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBox
::setMRMLDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  Q_D(qMRMLDisplayNodeViewComboBox);
  this->qvtkReconnect(d->MRMLDisplayNode, displayNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRML()));
  d->MRMLDisplayNode = displayNode;
  if (d->MRMLDisplayNode)
    {
    // Only overwrite the scene if the node has a valid scene
    // (otherwise scene may be swapped out to an invalid scene during scene close
    // causing a crash if it happens during a scene model update).
    if (d->MRMLDisplayNode->GetScene())
      {
      this->setMRMLScene(d->MRMLDisplayNode->GetScene());
      }
    }
  else
    {
    this->setMRMLScene(nullptr);
    }
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBox
::setMRMLDisplayNode(vtkMRMLNode* displayNode)
{
  this->setMRMLDisplayNode(
    vtkMRMLDisplayNode::SafeDownCast(displayNode));
}

// --------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBox::updateWidgetFromMRML()
{
  Q_D(qMRMLDisplayNodeViewComboBox);
  this->setEnabled(this->mrmlScene() != nullptr && d->MRMLDisplayNode != nullptr);
  if (!d->MRMLDisplayNode)
    {
    return;
    }
  bool oldUpdating = d->IsUpdatingWidgetFromMRML;
  d->IsUpdatingWidgetFromMRML = true;

  bool wasBlocking = this->blockSignals(true);
  bool modified = false;
  for (int i = 0; i < this->nodeCount(); ++i)
    {
    vtkMRMLNode* view = this->nodeFromIndex(i);
    if (!view)
      {
      // we get here if there is an orphan view node and the scene is closing
      this->setCheckState(view, Qt::Unchecked);
      continue;
      }
    bool check = d->MRMLDisplayNode->IsDisplayableInView(view->GetID());
    Qt::CheckState viewCheckState = check ? Qt::Checked : Qt::Unchecked;
    if (this->checkState(view) != viewCheckState)
      {
      modified = true;
      this->setCheckState(view, viewCheckState);
      }
    }
  this->blockSignals(wasBlocking);
  if (modified)
    {
    emit checkedNodesChanged();
    }
  d->IsUpdatingWidgetFromMRML = oldUpdating;
}

// --------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBox::updateMRMLFromWidget()
{
  Q_D(qMRMLDisplayNodeViewComboBox);
  if (!d->MRMLDisplayNode)
    {
    return;
    }
  if (d->IsUpdatingWidgetFromMRML)
    {
    return;
    }
  int wasModifying = d->MRMLDisplayNode->StartModify();

  if (this->allChecked() || this->noneChecked())
    {
    d->MRMLDisplayNode->RemoveAllViewNodeIDs();
    }
  else
    {
    foreach (vtkMRMLAbstractViewNode* viewNode, this->checkedViewNodes())
      {
      d->MRMLDisplayNode->AddViewNodeID(viewNode ? viewNode->GetID() : nullptr);
      }
    foreach (vtkMRMLAbstractViewNode* viewNode, this->uncheckedViewNodes())
      {
      d->MRMLDisplayNode->RemoveViewNodeID(viewNode ? viewNode->GetID() : nullptr);
      }
    }

  d->MRMLDisplayNode->EndModify(wasModifying);
}

// --------------------------------------------------------------------------
QList<vtkMRMLAbstractViewNode*> qMRMLDisplayNodeViewComboBox::checkedViewNodes()const
{
  QList<vtkMRMLAbstractViewNode*> res;
  foreach(vtkMRMLNode* checkedNode, this->checkedNodes())
    {
    res << vtkMRMLAbstractViewNode::SafeDownCast(checkedNode);
    }
  return res;
}

// --------------------------------------------------------------------------
QList<vtkMRMLAbstractViewNode*> qMRMLDisplayNodeViewComboBox::uncheckedViewNodes()const
{
  QList<vtkMRMLAbstractViewNode*> res;
  foreach(vtkMRMLNode* uncheckedNode, this->uncheckedNodes())
    {
    res << vtkMRMLAbstractViewNode::SafeDownCast(uncheckedNode);
    }
  return res;
}
