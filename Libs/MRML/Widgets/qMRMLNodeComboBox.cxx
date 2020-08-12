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
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QListView>

// CTK includes
#include <ctkComboBox.h>

// MRMLWidgets includes
#include "qMRMLNodeComboBoxDelegate.h"
#include "qMRMLNodeComboBoxMenuDelegate.h"
#include "qMRMLNodeComboBox_p.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneModel.h"

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// --------------------------------------------------------------------------
qMRMLNodeComboBoxPrivate::qMRMLNodeComboBoxPrivate(qMRMLNodeComboBox& object)
  : q_ptr(&object)
{
  this->ComboBox = nullptr;
  this->MRMLNodeFactory = nullptr;
  this->MRMLSceneModel = nullptr;
  this->NoneEnabled = false;
  this->AddEnabled = true;
  this->RemoveEnabled = true;
  this->EditEnabled = false;
  // "Singleton" is the default tag for the interaction singleton node
  this->InteractionNodeSingletonTag = "Singleton";
  this->RenameEnabled = false;

  this->SelectNodeUponCreation = true;
  this->NoneDisplay = qMRMLNodeComboBox::tr("None");
  this->AutoDefaultText = true;
}

// --------------------------------------------------------------------------
qMRMLNodeComboBoxPrivate::~qMRMLNodeComboBoxPrivate() = default;

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxPrivate::init(QAbstractItemModel* model)
{
  Q_Q(qMRMLNodeComboBox);
  Q_ASSERT(this->MRMLNodeFactory == nullptr);

  q->setLayout(new QHBoxLayout);
  q->layout()->setContentsMargins(0,0,0,0);
  q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,
                               QSizePolicy::Fixed,
                               QSizePolicy::ComboBox));

  if (this->ComboBox == nullptr)
    {
    ctkComboBox* comboBox = new ctkComboBox(q);
    comboBox->setElideMode(Qt::ElideMiddle);
    q->setComboBox(comboBox);
    }
  else
    {
    QComboBox* comboBox = this->ComboBox;
    this->ComboBox = nullptr;
    q->setComboBox(comboBox);
    }

  this->MRMLNodeFactory = new qMRMLNodeFactory(q);

  QAbstractItemModel* rootModel = model;
  while (qobject_cast<QAbstractProxyModel*>(rootModel) &&
         qobject_cast<QAbstractProxyModel*>(rootModel)->sourceModel())
    {
    rootModel = qobject_cast<QAbstractProxyModel*>(rootModel)->sourceModel();
    }
  this->MRMLSceneModel = qobject_cast<qMRMLSceneModel*>(rootModel);
  Q_ASSERT(this->MRMLSceneModel);
  // no need to reset the root model index here as the model is not yet set
  this->updateNoneItem(false);
  this->updateActionItems(false);

  qMRMLSortFilterProxyModel* sortFilterModel = new qMRMLSortFilterProxyModel(q);
  sortFilterModel->setSourceModel(model);
  this->setModel(sortFilterModel);

  // nodeTypeLabel() works only when the model is set.
  this->updateDefaultText();

  q->setEnabled(q->mrmlScene() != nullptr);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxPrivate::setModel(QAbstractItemModel* model)
{
  Q_Q(qMRMLNodeComboBox);
  if (model == nullptr)
    {// it's invalid to set a null model to a combobox
    return;
    }
  if (this->ComboBox->model() != model)
    {
    this->ComboBox->setModel(model);
    }
  q->connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
             q, SLOT(emitNodesAdded(QModelIndex,int,int)));
  q->connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
             q, SLOT(emitNodesAboutToBeRemoved(QModelIndex,int,int)));
  q->connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
             q, SLOT(refreshIfCurrentNodeHidden()));
  q->connect(model, SIGNAL(modelReset()), q, SLOT(refreshIfCurrentNodeHidden()));
  q->connect(model, SIGNAL(layoutChanged()), q, SLOT(refreshIfCurrentNodeHidden()));
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBoxPrivate::mrmlNode(int row)const
{
  QModelIndex modelIndex;
  if (qobject_cast<QListView*>(this->ComboBox->view()))
    {
    modelIndex  = this->ComboBox->model()->index(
      row, this->ComboBox->modelColumn(), this->ComboBox->rootModelIndex());
    }
  else
    {// special case where the view can handle a tree... currentIndex could be
    // from any parent, not only a top level..
    modelIndex = this->ComboBox->view()->currentIndex();
    modelIndex = this->ComboBox->model()->index(
      row, this->ComboBox->modelColumn(), modelIndex.parent());
    }
  /*
  Q_Q(const qMRMLNodeComboBox);
  QString nodeId =
    this->ComboBox->itemData(index, qMRMLSceneModel::UIDRole).toString();
  if (nodeId.isEmpty())
    {
    return 0;
    }
  vtkMRMLScene* scene = q->mrmlScene();
  return scene ? scene->GetNodeByID(nodeId.toUtf8()) : 0;
  */
  return this->mrmlNodeFromIndex(modelIndex);
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBoxPrivate::mrmlNodeFromIndex(const QModelIndex& index)const
{
  Q_Q(const qMRMLNodeComboBox);
  Q_ASSERT(q->model());
  QString nodeId =
    this->ComboBox->model()->data(index, qMRMLSceneModel::UIDRole).toString();
  if (nodeId.isEmpty())
    {
    return nullptr;
    }
  vtkMRMLScene* scene = q->mrmlScene();
  return scene ? scene->GetNodeByID(nodeId.toUtf8()) : nullptr;
}

// --------------------------------------------------------------------------
QModelIndexList qMRMLNodeComboBoxPrivate::indexesFromMRMLNodeID(const QString& nodeID)const
{
  return this->ComboBox->model()->match(
    this->ComboBox->model()->index(0, 0), qMRMLSceneModel::UIDRole, nodeID, 1,
    Qt::MatchRecursive | Qt::MatchExactly | Qt::MatchWrap);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxPrivate::updateDefaultText()
{
  Q_Q(const qMRMLNodeComboBox);
  if (!this->AutoDefaultText)
    {
    return;
    }
  ctkComboBox* cb = qobject_cast<ctkComboBox*>(this->ComboBox);
  if (cb)
    {
    // Use the first node type label to give a hint to the user
    // what kind of node is expected
    QString nodeType;
    QStringList nodeTypes = q->nodeTypes();
    if (!nodeTypes.empty())
      {
      nodeType = nodeTypes[0];
      }
    cb->setDefaultText(qMRMLNodeComboBox::tr("Select a ") + q->nodeTypeLabel(nodeType));
    }
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxPrivate::updateNoneItem(bool resetRootIndex)
{
  Q_UNUSED(resetRootIndex);
  //Q_Q(qMRMLNodeComboBox);
  QStringList noneItem;
  if (this->NoneEnabled)
    {
    noneItem.append(this->NoneDisplay);
    }
  //QVariant currentNode =
  //  this->ComboBox->itemData(this->ComboBox->currentIndex(), qMRMLSceneModel::UIDRole);
  //qDebug() << "updateNoneItem: " << this->MRMLSceneModel->mrmlSceneItem();
  if (this->MRMLSceneModel->mrmlSceneItem())
    {
    this->MRMLSceneModel->setPreItems(noneItem, this->MRMLSceneModel->mrmlSceneItem());
    }
/*  if (resetRootIndex)
    {
    this->ComboBox->setRootModelIndex(q->model()->index(0, 0));
    // setting the rootmodel index looses the current item
    // try to set the current item back
    q->setCurrentNode(currentNode.toString());
    }
*/
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxPrivate::updateActionItems(bool resetRootIndex)
{
  Q_Q(qMRMLNodeComboBox);
  Q_UNUSED(resetRootIndex);


  QStringList extraItems;
  if (q->mrmlScene())
    {
    // Action items are not updated when selection is changed, therefore use the actual
    // node type label if there is only one type and use a generic name if there are multiple node types (or none)
    QString nodeType;
    QStringList nodeTypes = q->nodeTypes();
    if (nodeTypes.size()==1)
      {
      nodeType = nodeTypes[0];
      }
    QString label = q->nodeTypeLabel(nodeType);

    if (this->AddEnabled || this->RemoveEnabled || this->EditEnabled
        || this->RenameEnabled || !this->UserMenuActions.empty())
      {
      extraItems.append("separator");
      }
    if (this->RenameEnabled)
      {
      extraItems.append(qMRMLNodeComboBox::tr("Rename current ")  + label);
      }
    if (this->EditEnabled)
      {
      extraItems.append(qMRMLNodeComboBox::tr("Edit current ")  + label);
      }
    if (this->AddEnabled)
      {
      foreach (QString nodeType, q->nodeTypes())
        {
        QString label = q->nodeTypeLabel(nodeType);
        extraItems.append(qMRMLNodeComboBox::tr("Create new ") + label);
        if (this->RenameEnabled)
          {
          extraItems.append(qMRMLNodeComboBox::tr("Create new ") + label + qMRMLNodeComboBox::tr(" as..."));
          }
        }
      }
    if (this->RemoveEnabled)
      {
      extraItems.append(qMRMLNodeComboBox::tr("Delete current ")  + label);
      }
    foreach (QAction *action, this->UserMenuActions)
      {
      extraItems.append(action->text());
      }
    }
  this->MRMLSceneModel->setPostItems(extraItems, this->MRMLSceneModel->mrmlSceneItem());
  QObject::connect(this->ComboBox->view(), SIGNAL(clicked(QModelIndex)),
                   q, SLOT(activateExtraItem(QModelIndex)),
                   Qt::UniqueConnection);
  /*
  if (resetRootIndex)
    {
    this->ComboBox->setRootModelIndex(q->model()->index(0, 0));
    // setting the rootmodel index looses the current item
    // try to set the current item back
    q->setCurrentNode(currentNode.toString());
    }
  */
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBoxPrivate::updateDelegate(bool force)
{
  Q_Q(qMRMLNodeComboBox);
  QStyleOptionComboBox opt;
  opt.editable = this->ComboBox->isEditable();

  if (this->ComboBox->style()->styleHint(
      QStyle::SH_ComboBox_Popup, &opt, this->ComboBox))
    {
      if (force ||
          qobject_cast<qMRMLNodeComboBoxDelegate *>(this->ComboBox->itemDelegate()))
        {
        this->ComboBox->setItemDelegate(
            new qMRMLNodeComboBoxMenuDelegate(q->parent(), q->comboBox()));
        }
    }
  else
    {
      if (force ||
          qobject_cast<qMRMLNodeComboBoxMenuDelegate *>(this->ComboBox->itemDelegate()))
        {
        this->ComboBox->setItemDelegate(
            new qMRMLNodeComboBoxDelegate(q->parent(), q->comboBox()));
        }
    }
}

// --------------------------------------------------------------------------
bool qMRMLNodeComboBoxPrivate::hasPostItem(const QString& name)const
{
  foreach(const QString& item,
          this->MRMLSceneModel->postItems(this->MRMLSceneModel->mrmlSceneItem()))
    {
    if (item.startsWith(name))
      {
      return true;
      }
    }
  return false;
}

// --------------------------------------------------------------------------
// qMRMLNodeComboBox

// --------------------------------------------------------------------------
qMRMLNodeComboBox::qMRMLNodeComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLNodeComboBoxPrivate(*this))
{
  Q_D(qMRMLNodeComboBox);
  d->init(new qMRMLSceneModel(this));
}

// --------------------------------------------------------------------------
qMRMLNodeComboBox::qMRMLNodeComboBox(QAbstractItemModel* sceneModel, QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLNodeComboBoxPrivate(*this))
{
  Q_D(qMRMLNodeComboBox);
  d->init(sceneModel);
}

// --------------------------------------------------------------------------
qMRMLNodeComboBox::qMRMLNodeComboBox(qMRMLNodeComboBoxPrivate* pimpl, QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(pimpl)
{
  Q_D(qMRMLNodeComboBox);
  d->init(new qMRMLSceneModel(this));
}

// --------------------------------------------------------------------------
qMRMLNodeComboBox::~qMRMLNodeComboBox() = default;

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::activateExtraItem(const QModelIndex& index)
{
  Q_D(qMRMLNodeComboBox);
  // FIXME: check the type of the item on a different role instead of the display role
  QString data = this->model()->data(index, Qt::DisplayRole).toString();
  if (d->AddEnabled && data.startsWith(tr("Create new ")) && !data.endsWith(tr(" as...")))
    {
    QString label = data.right(data.length()-tr("Create new ").length());
    QString nodeTypeName;
    foreach (QString nodeType, this->nodeTypes())
      {
      QString foundLabel = this->nodeTypeLabel(nodeType);
      if (foundLabel==label)
        {
        nodeTypeName = nodeType;
        }
      }
    d->ComboBox->hidePopup();
    this->addNode(nodeTypeName);
    }
  else if (d->RemoveEnabled && data.startsWith(tr("Delete current ")))
    {
    d->ComboBox->hidePopup();
    this->removeCurrentNode();
    }
  else if (d->EditEnabled && data.startsWith(tr("Edit current ")))
    {
    d->ComboBox->hidePopup();
    this->editCurrentNode();
    }
  else if (d->RenameEnabled && data.startsWith(tr("Rename current ")))
    {
    d->ComboBox->hidePopup();
    this->renameCurrentNode();
    }
  else if (d->RenameEnabled && d->AddEnabled
           && data.startsWith(tr("Create new ")) && data.endsWith(tr(" as...")))
    {
    // Get the node type label by stripping "Create new" and "as..." from left and right
    QString label = data.mid(tr("Create new ").length(), data.length()-tr("Create new ").length()-tr(" as...").length());
    QString nodeTypeName;
    foreach (QString nodeType, this->nodeTypes())
      {
      QString foundLabel = this->nodeTypeLabel(nodeType);
      if (foundLabel==label)
        {
        nodeTypeName = nodeType;
        }
      }
    d->ComboBox->hidePopup();
    this->addNode(nodeTypeName);
    this->renameCurrentNode();
    }
  else
    {
    // check for user added items
    foreach (QAction *action, d->UserMenuActions)
      {
      if (data.startsWith(action->text()))
        {
        d->ComboBox->hidePopup();
        action->trigger();
        break;
        }
      }
    }

}

//-----------------------------------------------------------------------------
void qMRMLNodeComboBox::addAttribute(const QString& nodeType,
                                     const QString& attributeName,
                                     const QVariant& attributeValue)
{
  Q_D(qMRMLNodeComboBox);

  // Add a warning to make it easier to detect issue in obsolete modules that have not been updated
  // since the "LabelMap" attribute was replaced by vtkMRMLLabelMapVolumeNode.
  // The "LabelMap" attribute filter is not applied to make obsolete modules still usable (if we
  // applied the filter then no volume would show up in the selector; as we ignore it both scalar
  // and labelmap volumes show up, which is just a slight inconvenience).
  // Probably this check can be removed by Summer 2016 (one year after the vtkMRMLLabelMapVolumeNode
  // was introduced).
  if (nodeType=="vtkMRMLScalarVolumeNode" && attributeName=="LabelMap")
  {
    qWarning("vtkMRMLScalarVolumeNode does not have a LabelMap attribute anymore. Update your code according to "
      "http://www.slicer.org/slicerWiki/index.php/Documentation/Labs/Segmentations#Module_update_instructions");
    return;
  }

  d->MRMLNodeFactory->addAttribute(attributeName, attributeValue.toString());
  this->sortFilterProxyModel()->addAttribute(nodeType, attributeName, attributeValue);
}

//-----------------------------------------------------------------------------
void qMRMLNodeComboBox::removeAttribute(const QString& nodeType,
                                     const QString& attributeName)
{
  Q_D(qMRMLNodeComboBox);

  d->MRMLNodeFactory->removeAttribute(attributeName);
  this->sortFilterProxyModel()->removeAttribute(nodeType, attributeName);
}

//-----------------------------------------------------------------------------
void qMRMLNodeComboBox::setBaseName(const QString& baseName, const QString& nodeType /* ="" */ )
{
  Q_D(qMRMLNodeComboBox);
  if (!nodeType.isEmpty())
    {
    d->MRMLNodeFactory->setBaseName(nodeType, baseName);
    return;
    }
  // If no node type is defined then we set the base name for all already specified node types
  QStringList nodeTypes = this->nodeTypes();
  if (nodeTypes.isEmpty())
    {
    qWarning("qMRMLNodeComboBox::setBaseName failed: no node types have been set yet");
    return;
    }
  foreach (QString aNodeType, nodeTypes)
    {
    d->MRMLNodeFactory->setBaseName(aNodeType, baseName);
    }
}

//-----------------------------------------------------------------------------
QString qMRMLNodeComboBox::baseName(const QString& nodeType /* ="" */ )const
{
  Q_D(const qMRMLNodeComboBox);
  if (!nodeType.isEmpty())
    {
    return d->MRMLNodeFactory->baseName(nodeType);
    }
  // If nodeType is not specified then base name of the first node type is returned.
  QStringList nodeClasses = this->nodeTypes();
  if (nodeClasses.isEmpty())
    {
    qWarning("qMRMLNodeComboBox::baseName failed: no node types have been set yet");
    return QString();
    }
  return d->MRMLNodeFactory->baseName(nodeClasses[0]);
}

//-----------------------------------------------------------------------------
void qMRMLNodeComboBox::setNodeTypeLabel(const QString& label, const QString& nodeType)
{
  Q_D(qMRMLNodeComboBox);
  if (nodeType.isEmpty())
    {
    qWarning() << Q_FUNC_INFO << " failed: nodeType is invalid";
    return;
    }
  if (label.isEmpty())
    {
    d->NodeTypeLabels.remove(nodeType);
    }
  else
    {
    d->NodeTypeLabels[nodeType] = label;
    }
  d->updateDefaultText();
  d->updateActionItems();
}

//-----------------------------------------------------------------------------
QString qMRMLNodeComboBox::nodeTypeLabel(const QString& nodeType)const
{
  Q_D(const qMRMLNodeComboBox);
  // If a label was explicitly specified then use that
  if (d->NodeTypeLabels.contains(nodeType))
    {
    return d->NodeTypeLabels[nodeType];
    }
  // Otherwise use the node tag
  if (this->mrmlScene())
    {
    QString label = this->mrmlScene()->GetTagByClassName(nodeType.toUtf8());
    if (!label.isEmpty())
      {
      return label;
      }
    }
  // Special case: for volumes, use "Volume" as label
  if (nodeType == "vtkMRMLVolumeNode")
      {
      return tr("Volume");
      }
  // Otherwise just label the node as "node"
  return tr("node");
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBox::addNode(QString nodeType)
{
  Q_D(qMRMLNodeComboBox);
  if (!this->nodeTypes().contains(nodeType))
    {
    qWarning("qMRMLNodeComboBox::addNode() attempted with node type %s, which is not among the allowed node types", qPrintable(nodeType));
    return nullptr;
    }
  // Create the MRML node via the MRML Scene
  vtkMRMLNode * newNode = d->MRMLNodeFactory->createNode(nodeType);
  // The created node is appended at the bottom of the current list
  if (newNode==nullptr)
    {
    qWarning("qMRMLNodeComboBox::addNode() failed with node type %s", qPrintable(nodeType));
    return nullptr;
    }
  if (this->selectNodeUponCreation())
    {// select the created node.
    this->setCurrentNode(newNode);
    }
  emit this->nodeAddedByUser(newNode);
  return newNode;
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBox::addNode()
{
  if (this->nodeTypes().isEmpty())
    {
    return nullptr;
    }
  return this->addNode(this->nodeTypes()[0]);
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBox::currentNode()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->mrmlNode(d->ComboBox->currentIndex());
}

// --------------------------------------------------------------------------
QString qMRMLNodeComboBox::currentNodeID()const
{
  vtkMRMLNode* node = this->currentNode();
  return node ? node->GetID() : "";
}

// --------------------------------------------------------------------------
QString qMRMLNodeComboBox::currentNodeId()const
{
  qWarning() << "This function is deprecated. Use currentNodeID() instead";
  return this->currentNodeID();
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::editCurrentNode()
{
  Q_D(const qMRMLNodeComboBox);
  vtkMRMLNode* node = this->currentNode();
  emit this->nodeAboutToBeEdited(node);

  if (!d->InteractionNodeSingletonTag.isEmpty())
    {
    vtkMRMLInteractionNode* interactionNode = vtkMRMLInteractionNode::SafeDownCast(
      this->mrmlScene()->GetSingletonNode(d->InteractionNodeSingletonTag.toUtf8(), "vtkMRMLInteractionNode"));
    if (interactionNode)
      {
      interactionNode->EditNode(node);
      }
    else
      {
      qWarning() << Q_FUNC_INFO << " failed: interaction node not found with singleton tag " << d->InteractionNodeSingletonTag;
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::renameCurrentNode()
{
  vtkMRMLNode* node = this->currentNode();
  if (!node)
    {
    return;
    }

  bool ok = false;
  QString newName = QInputDialog::getText(
    this, "Rename " + this->nodeTypeLabel(node->GetClassName()), "New name:",
    QLineEdit::Normal, node->GetName(), &ok);
  if (!ok)
    {
    return;
    }
  node->SetName(newName.toUtf8());
  emit currentNodeRenamed(newName);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::emitCurrentNodeChanged()
{
  Q_D(qMRMLNodeComboBox);
  int currentIndex = d->ComboBox->currentIndex();
  vtkMRMLNode*  node = d->mrmlNode(currentIndex);
  if (!node && ((!d->NoneEnabled &&currentIndex != -1) || (d->NoneEnabled && currentIndex != 0)) )
    {
    // we only set the current node if the new selected is different
    // (not nullptr) to avoid warning in QAccessibleTable::child
    vtkMRMLNode* newSelectedNode = this->nodeFromIndex(this->nodeCount() - 1);
    if (newSelectedNode)
      {
      this->setCurrentNode(newSelectedNode);
      }
    }
  else
    {
    emit currentNodeChanged(node);
    emit currentNodeChanged(node != nullptr);
    emit currentNodeIDChanged(node ? node->GetID() : "");
    }
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::emitNodeActivated(int currentIndex)
{
  Q_D(qMRMLNodeComboBox);
  vtkMRMLNode*  node = d->mrmlNode(currentIndex);
  // Fire only if the user clicked on a node or "None", don't fire the signal
  // if the user clicked on an "action" (post item) like "Add Node".
  if (node || (d->NoneEnabled && currentIndex == 0))
    {
    emit nodeActivated(node);
    }
}
// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLNodeComboBox::mrmlScene()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->MRMLSceneModel->mrmlScene();
}

// --------------------------------------------------------------------------
int qMRMLNodeComboBox::nodeCount()const
{
  Q_D(const qMRMLNodeComboBox);
  int extraItemsCount =
    d->MRMLSceneModel->preItems(d->MRMLSceneModel->mrmlSceneItem()).count()
    + d->MRMLSceneModel->postItems(d->MRMLSceneModel->mrmlSceneItem()).count();
  //qDebug() << d->MRMLSceneModel->invisibleRootItem() << d->MRMLSceneModel->mrmlSceneItem() << d->ComboBox->count() <<extraItemsCount;
  //printStandardItem(d->MRMLSceneModel->invisibleRootItem(), "  ");
  //qDebug() << d->ComboBox->rootModelIndex();
  return this->mrmlScene() ? d->ComboBox->count() - extraItemsCount : 0;
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBox::nodeFromIndex(int index)const
{
  Q_D(const qMRMLNodeComboBox);
  return d->mrmlNode(d->NoneEnabled ? index + 1 : index);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::removeCurrentNode()
{
  this->mrmlScene()->RemoveNode(this->currentNode());
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLNodeComboBox);

  // Be careful when commenting that out. you really need a good reason for
  // forcing a new set. You should probably expose
  // qMRMLSceneModel::UpdateScene() and make sure there is no nested calls
  if (d->MRMLSceneModel->mrmlScene() == scene)
    {
    return ;
    }

  // The Add button is valid only if the scene is non-empty
  //this->setAddEnabled(scene != 0);
  QString oldCurrentNode = d->ComboBox->itemData(d->ComboBox->currentIndex(), qMRMLSceneModel::UIDRole).toString();
  bool previousSceneWasValid = (this->nodeCount() > 0);

  // Update factory
  d->MRMLNodeFactory->setMRMLScene(scene);
  d->MRMLSceneModel->setMRMLScene(scene);
  d->updateDefaultText();
  d->updateNoneItem(false);
  d->updateActionItems(false);

  //qDebug()<< "setMRMLScene:" << this->model()->index(0, 0);
  // updating the action items reset the root model index. Set it back
  // setting the rootmodel index looses the current item
  d->ComboBox->setRootModelIndex(this->model()->index(0, 0));

  // try to set the current item back
  // if there was no node in the scene (or scene not set), then the
  // oldCurrentNode was not meaningful and we probably don't want to
  // set it back. Please consider make it a behavior property if it doesn't fit
  // your need, as this behavior is currently wanted for some cases (
  // vtkMRMLClipModels selector in the Models module)
  if (previousSceneWasValid)
    {
    this->setCurrentNodeID(oldCurrentNode);
    }
  // if the new nodeCount is 0, then let's make sure to select 'invalid' node
  // (None(0) or -1). we can't do nothing otherwise the Scene index (rootmodelIndex)
  // would be selected and "Scene" would be displayed (see vtkMRMLNodeComboboxTest5)
  else
    {
    QString newNodeID = this->currentNodeID();
    if (!d->RequestedNodeID.isEmpty())
      {
      newNodeID = d->RequestedNodeID;
      }
    else if (d->RequestedNode != nullptr && d->RequestedNode->GetID() != nullptr)
      {
      newNodeID = d->RequestedNode->GetID();
      }
    this->setCurrentNodeID(newNodeID);
    }
  d->RequestedNodeID.clear();
  d->RequestedNode = nullptr;

  this->setEnabled(scene != nullptr);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setCurrentNode(vtkMRMLNode* newCurrentNode)
{
  Q_D(qMRMLNodeComboBox);
  if (!this->mrmlScene())
    {
    d->RequestedNodeID.clear();
    d->RequestedNode = newCurrentNode;
    }
  this->setCurrentNodeID(newCurrentNode ? newCurrentNode->GetID() : "");
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setCurrentNode(const QString& nodeID)
{
  qWarning() << "This function is deprecated. Use setCurrentNodeID() instead";
  this->setCurrentNodeID(nodeID);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setCurrentNodeID(const QString& nodeID)
{
  Q_D(qMRMLNodeComboBox);
  if (!this->mrmlScene())
    {
    d->RequestedNodeID = nodeID;
    d->RequestedNode = nullptr;
    }
  // A straight forward implementation of setCurrentNode would be:
  //    int index = !nodeID.isEmpty() ? d->ComboBox->findData(nodeID, qMRMLSceneModel::UIDRole) : -1;
  //    if (index == -1 && d->NoneEnabled)
  //      {
  //      index = 0;
  //      }
  //    d->ComboBox->setCurrentIndex(index);
  // However it doesn't work for custom comboxboxes that display non-flat lists
  // (typically if it is a tree model/view)
  // let's use a more generic one
  QModelIndexList indexes = d->indexesFromMRMLNodeID(nodeID);
  if (indexes.size() == 0)
    {
    QModelIndex sceneIndex = d->ComboBox->model()->index(0, 0);
    d->ComboBox->setRootModelIndex(sceneIndex);
    // The combobox updates the current index of the view only when he needs
    // it (in popup()), however we want the view to be always synchronized
    // with the currentIndex as we use it to know if it has changed. This is
    // why we set it here.
    QModelIndex noneIndex = sceneIndex.child(0, d->ComboBox->modelColumn());
    d->ComboBox->view()->setCurrentIndex(
      d->NoneEnabled ? noneIndex : sceneIndex);
    d->ComboBox->setCurrentIndex(d->NoneEnabled ? 0 : -1);
    return;
    }
  //d->ComboBox->setRootModelIndex(indexes[0].parent());
  //d->ComboBox->setCurrentIndex(indexes[0].row());
  QModelIndex oldIndex = d->ComboBox->view()->currentIndex();
  if (oldIndex != indexes[0])
    {
    d->ComboBox->view()->setCurrentIndex(indexes[0]);
    QKeyEvent event(QEvent::ShortcutOverride, Qt::Key_Enter, Qt::NoModifier);
    // here we conditionally send the event, otherwise, nodeActivated would be
    // fired even if the user didn't manually select the node.
    // Warning: please note that sending a KeyEvent will close the popup menu
    // of the combobox if it is open.
    QApplication::sendEvent(d->ComboBox->view(), &event);
    }
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setCurrentNodeIndex(int index)
{
  Q_D(qMRMLNodeComboBox);
  if (index >= this->nodeCount())
    {
    index = -1;
    }
  if (d->NoneEnabled)
    {
    // If the "None" extra item is present, shift all the indexes
    ++index;
    }
  d->ComboBox->setCurrentIndex(index);
}

//--------------------------------------------------------------------------
CTK_SET_CPP(qMRMLNodeComboBox, bool, setSelectNodeUponCreation, SelectNodeUponCreation);
CTK_GET_CPP(qMRMLNodeComboBox, bool, selectNodeUponCreation, SelectNodeUponCreation);

// --------------------------------------------------------------------------
QStringList qMRMLNodeComboBox::nodeTypes()const
{
  qMRMLSortFilterProxyModel* m = this->sortFilterProxyModel();
  return m ? m->nodeTypes() : QStringList();
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setNodeTypes(const QStringList& _nodeTypes)
{
  Q_D(qMRMLNodeComboBox);

  // Remove empty elements (empty elements may be created accidentally when
  // string lists are constructed in Python)
  QStringList nodeTypesFiltered = _nodeTypes;
  nodeTypesFiltered.removeAll("");

  this->sortFilterProxyModel()->setNodeTypes(nodeTypesFiltered);
  d->updateDefaultText();
  d->updateActionItems();
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setNoneEnabled(bool enable)
{
  Q_D(qMRMLNodeComboBox);
  if (d->NoneEnabled == enable)
    {
    return;
    }
  d->NoneEnabled = enable;
  d->updateNoneItem();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox::noneEnabled()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->NoneEnabled;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setAddEnabled(bool enable)
{
  Q_D(qMRMLNodeComboBox);
  if (d->AddEnabled == enable)
    {
    return;
    }
  if (enable && d->hasPostItem(tr("Create new ")))
    {
    qDebug() << "setAddEnabled: An action starting with name "
             << tr("Create new ") << " already exists. "
                "Not enabling this property.";
    return;
    }
  d->AddEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox::addEnabled()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->AddEnabled;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setRemoveEnabled(bool enable)
{
  Q_D(qMRMLNodeComboBox);
  if (d->RemoveEnabled == enable)
    {
    return;
    }
  if (enable && d->hasPostItem(tr("Delete current ")))
    {
    qDebug() << "setRemoveEnabled: An action starting with name "
             << tr("Delete current ") << " already exists. "
                "Not enabling this property.";
    return;
    }
  d->RemoveEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox::removeEnabled()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->RemoveEnabled;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setEditEnabled(bool enable)
{
  Q_D(qMRMLNodeComboBox);
  if (d->EditEnabled == enable)
    {
    return;
    }
  if (enable && d->hasPostItem(tr("Edit current ")))
    {
    qDebug() << "setEditEnabled: An action starting with name "
             << tr("Edit current ") << " already exists. "
                "Not enabling this property.";
    return;
    }
  d->EditEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox::editEnabled()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->EditEnabled;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setRenameEnabled(bool enable)
{
  Q_D(qMRMLNodeComboBox);
  if (d->RenameEnabled == enable)
    {
    return;
    }
  if (enable && d->hasPostItem(tr("Rename current ")))
    {
    qDebug() << "setRenameEnabled: An action starting with name "
             << tr("Rename current ") << " already exists. "
                "Not enabling this property.";
    return;
    }
  d->RenameEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox::renameEnabled()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->RenameEnabled;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setNoneDisplay(const QString& displayName)
{
  Q_D(qMRMLNodeComboBox);
  if (d->NoneDisplay == displayName)
    {
    return;
    }
  d->NoneDisplay = displayName;
  d->updateNoneItem(false);
}

//--------------------------------------------------------------------------
QString qMRMLNodeComboBox::noneDisplay()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->NoneDisplay;
}

//--------------------------------------------------------------------------
QList<vtkMRMLNode*> qMRMLNodeComboBox::nodes()const
{
  QList<vtkMRMLNode*> nodeList;
  for (int i = 0; i < this->nodeCount(); ++i)
    {
    vtkMRMLNode* node = this->nodeFromIndex(i);
    Q_ASSERT(node);
    if (node)
      {
      nodeList << node;
      }
    }
  return nodeList;
}

//--------------------------------------------------------------------------
qMRMLSortFilterProxyModel* qMRMLNodeComboBox::sortFilterProxyModel()const
{
  Q_ASSERT(qobject_cast<qMRMLSortFilterProxyModel*>(this->model()));
  return qobject_cast<qMRMLSortFilterProxyModel*>(this->model());
}

//--------------------------------------------------------------------------
QAbstractItemModel* qMRMLNodeComboBox::model()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->ComboBox ? d->ComboBox->model() : nullptr;
}

//--------------------------------------------------------------------------
qMRMLSceneModel* qMRMLNodeComboBox::sceneModel()const
{
  Q_ASSERT(this->sortFilterProxyModel());
  return this->sortFilterProxyModel()->sceneModel();
}

//--------------------------------------------------------------------------
QAbstractItemModel* qMRMLNodeComboBox::rootModel()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->MRMLSceneModel;
}

//--------------------------------------------------------------------------
qMRMLNodeFactory* qMRMLNodeComboBox::nodeFactory()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->MRMLNodeFactory;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setComboBox(QComboBox* comboBox)
{
  Q_D(qMRMLNodeComboBox);
  if (comboBox == d->ComboBox)
    {
    return;
    }

  QAbstractItemModel* oldModel = this->model();
  QComboBox* oldComboBox = d->ComboBox;

  this->layout()->addWidget(comboBox);
  d->ComboBox = comboBox;
  d->ComboBox->setFocusProxy(this);
  d->setModel(oldModel);

  connect(d->ComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(emitCurrentNodeChanged()));
  connect(d->ComboBox, SIGNAL(activated(int)),
          this, SLOT(emitNodeActivated(int)));
  d->ComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Expanding,
                                         QSizePolicy::DefaultType));
  delete oldComboBox;

  /// Set the new item delegate to force the highlight in case the item is not
  /// selectable but current.
  if (d->ComboBox)
    {
    d->updateDelegate(
      d->ComboBox->view()->metaObject()->className() == QString("QComboBoxListView"));
    }
}

//--------------------------------------------------------------------------
QComboBox* qMRMLNodeComboBox::comboBox()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->ComboBox;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::emitNodesAdded(const QModelIndex & parent, int start, int end)
{
  Q_D(qMRMLNodeComboBox);
  Q_ASSERT(this->model());
  for(int i = start; i <= end; ++i)
    {
    vtkMRMLNode* node = d->mrmlNodeFromIndex(this->model()->index(start, 0, parent));
    if (node)
      {
      emit nodeAdded(node);
      }
    }
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::emitNodesAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
  Q_D(qMRMLNodeComboBox);
  Q_ASSERT(this->model());
  for(int i = start; i <= end; ++i)
    {
    vtkMRMLNode* node = d->mrmlNodeFromIndex(this->model()->index(start, 0, parent));
    if (node)
      {
      emit nodeAboutToBeRemoved(node);
      }
    }
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::refreshIfCurrentNodeHidden()
{
  /// Sometimes, a node can disappear/hide from the combobox
  /// (qMRMLSortFilterProxyModel) because of a changed property.
  /// If the node is the current node, we need to unselect it because it is
  /// not a valid current node anymore.
  vtkMRMLNode* node = this->currentNode();
  if (!node)
    {
    this->setCurrentNode(nullptr);
    }
}

//--------------------------------------------------------------------------
QComboBox::SizeAdjustPolicy qMRMLNodeComboBox::sizeAdjustPolicy()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->ComboBox->sizeAdjustPolicy();
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy policy)
{
  Q_D(qMRMLNodeComboBox);
  d->ComboBox->setSizeAdjustPolicy(policy);
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::changeEvent(QEvent *event)
{
  Q_D(qMRMLNodeComboBox);
  if(event->type() == QEvent::StyleChange)
    {
    d->updateDelegate();
    }
  this->Superclass::changeEvent(event);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::addMenuAction(QAction *newAction)
{
  Q_D(qMRMLNodeComboBox);

  // is an action with the same text already in the user list?
  foreach (QAction *action, d->UserMenuActions)
    {
    if (action->text() == newAction->text())
      {
      qDebug() << "addMenuAction: duplicate action text of "
               << newAction->text()
               << ", not adding this action";
      return;
      }
    }
  if ((d->AddEnabled
       && newAction->text().startsWith(tr("Create new "))) ||
      (d->RemoveEnabled
       && newAction->text().startsWith(tr("Delete current "))) ||
      (d->EditEnabled
       && newAction->text().startsWith(tr("Edit current "))) ||
      (d->RenameEnabled
       && newAction->text().startsWith(tr("Rename current "))))
    {
    qDebug() << "addMenuAction: warning: the text on this action, "
             << newAction->text()
             << ", matches the start of an enabled default action text and "
                "will not get triggered, not adding it.";
    return;
    }

  d->UserMenuActions.append(newAction);

  // update with the new action
  d->updateActionItems(false);
}

//--------------------------------------------------------------------------
QString qMRMLNodeComboBox::interactionNodeSingletonTag()const
{
  Q_D(const qMRMLNodeComboBox);
  return d->InteractionNodeSingletonTag;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setInteractionNodeSingletonTag(const QString& tag)
{
  Q_D(qMRMLNodeComboBox);
  d->InteractionNodeSingletonTag = tag;
}
