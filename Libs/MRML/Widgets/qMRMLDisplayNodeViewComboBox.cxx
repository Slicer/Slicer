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
#include <vtkMRMLViewNode.h>

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
  virtual void init();
  vtkSmartPointer<vtkMRMLDisplayNode> MRMLDisplayNode;
};

// -----------------------------------------------------------------------------
qMRMLDisplayNodeViewComboBoxPrivate
::qMRMLDisplayNodeViewComboBoxPrivate(qMRMLDisplayNodeViewComboBox& object)
  : q_ptr(&object)
{
}

// -----------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBoxPrivate::init()
{
  Q_Q(qMRMLDisplayNodeViewComboBox);
  q->setNodeTypes(QStringList(QString("vtkMRMLViewNode")));
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
qMRMLDisplayNodeViewComboBox::~qMRMLDisplayNodeViewComboBox()
{
}

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
  this->setMRMLScene(d->MRMLDisplayNode ? d->MRMLDisplayNode->GetScene() : 0);
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
  this->setEnabled(this->mrmlScene() != 0 && d->MRMLDisplayNode != 0);
  if (!d->MRMLDisplayNode)
    {
    return;
    }
  bool wasBlocking = this->blockSignals(true);
  bool modified = false;
  for (int i = 0; i < this->nodeCount(); ++i)
    {
    vtkMRMLNode* view = this->nodeFromIndex(i);
    Q_ASSERT(view);
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
}

// --------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBox::updateMRMLFromWidget()
{
  Q_D(qMRMLDisplayNodeViewComboBox);
  if (!d->MRMLDisplayNode)
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
    foreach (vtkMRMLViewNode* viewNode, this->checkedViewNodes())
      {
      d->MRMLDisplayNode->AddViewNodeID(viewNode ? viewNode->GetID() : 0);
      }
    foreach (vtkMRMLViewNode* viewNode, this->uncheckedViewNodes())
      {
      d->MRMLDisplayNode->RemoveViewNodeID(viewNode ? viewNode->GetID() : 0);
      }
    }

  d->MRMLDisplayNode->EndModify(wasModifying);
}

// --------------------------------------------------------------------------
QList<vtkMRMLViewNode*> qMRMLDisplayNodeViewComboBox::checkedViewNodes()const
{
  Q_D(const qMRMLDisplayNodeViewComboBox);
  QList<vtkMRMLViewNode*> res;
  foreach(vtkMRMLNode* checkedNode, this->checkedNodes())
    {
    res << vtkMRMLViewNode::SafeDownCast(checkedNode);
    }
  return res;
}

// --------------------------------------------------------------------------
QList<vtkMRMLViewNode*> qMRMLDisplayNodeViewComboBox::uncheckedViewNodes()const
{
  Q_D(const qMRMLDisplayNodeViewComboBox);
  QList<vtkMRMLViewNode*> res;
  foreach(vtkMRMLNode* checkedNode, this->uncheckedNodes())
    {
    res << vtkMRMLViewNode::SafeDownCast(checkedNode);
    }
  return res;
}
