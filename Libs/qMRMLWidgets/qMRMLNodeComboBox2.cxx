/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QApplication>
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QListView>
#include <QStandardItemModel>

// CTK includes
#include <ctkComboBox.h>

// qMRMLWidgets includes
#include "qMRMLNodeComboBox2.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneModel2.h"
#include "qMRMLSortFilterProxyModel2.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// -----------------------------------------------------------------------------
class qMRMLNodeComboBox2Private
{
  Q_DECLARE_PUBLIC(qMRMLNodeComboBox2);
protected:
  qMRMLNodeComboBox2* const q_ptr;
public:
  qMRMLNodeComboBox2Private(qMRMLNodeComboBox2& object);
  void init(QAbstractItemModel* model);
  vtkMRMLNode* mrmlNode(int row)const;
  vtkMRMLNode* mrmlNodeFromIndex(const QModelIndex& index)const;
  void updateDefaultText();
  void updateNoneItem(bool resetRootIndex = true);
  void updateActionItems(bool resetRootIndex = true);
  QString nodeTypeLabel()const;

  QComboBox*        ComboBox;
  qMRMLNodeFactory* MRMLNodeFactory;
  qMRMLSceneModel2*  MRMLSceneModel;
  bool              SelectNodeUponCreation;
  bool              NoneEnabled;
  bool              AddEnabled;
  bool              RemoveEnabled;
  bool              EditEnabled;
private:
  void setModel(QAbstractItemModel* model);
};

// -----------------------------------------------------------------------------
qMRMLNodeComboBox2Private::qMRMLNodeComboBox2Private(qMRMLNodeComboBox2& object)
  : q_ptr(&object)
{
  this->ComboBox = 0;
  this->MRMLNodeFactory = 0;
  this->MRMLSceneModel = 0;
  this->SelectNodeUponCreation = true;
  this->NoneEnabled = false;
  this->AddEnabled = true;
  this->RemoveEnabled = true;
  this->EditEnabled = false;
}

// -----------------------------------------------------------------------------
void qMRMLNodeComboBox2Private::init(QAbstractItemModel* model)
{
  Q_Q(qMRMLNodeComboBox2);
  Q_ASSERT(this->MRMLNodeFactory == 0);
  ctkComboBox* comboBox = new ctkComboBox(q);
  comboBox->setElideMode(Qt::ElideMiddle);
  this->ComboBox = comboBox;

  q->setLayout(new QHBoxLayout);
  q->layout()->addWidget(this->ComboBox);
  q->layout()->setContentsMargins(0,0,0,0);
  q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,
                               QSizePolicy::Fixed,
                               QSizePolicy::ComboBox));
  this->ComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                            QSizePolicy::Expanding,
                                            QSizePolicy::DefaultType));

  this->MRMLNodeFactory = new qMRMLNodeFactory(q);

  QAbstractItemModel* rootModel = model;
  while (qobject_cast<QAbstractProxyModel*>(rootModel) &&
         qobject_cast<QAbstractProxyModel*>(rootModel)->sourceModel())
    {
    rootModel = qobject_cast<QAbstractProxyModel*>(rootModel)->sourceModel();
    }
  this->MRMLSceneModel = qobject_cast<qMRMLSceneModel2*>(rootModel);
  Q_ASSERT(this->MRMLSceneModel);
  // no need to reset the root model index here as the model is not yet set
  this->updateNoneItem(false);
  this->updateActionItems(false);

  qMRMLSortFilterProxyModel2* sortFilterModel = new qMRMLSortFilterProxyModel2(q);
  sortFilterModel->setSourceModel(model);
  this->setModel(sortFilterModel);

  // nodeTypeLabel() works only when the model is set.
  this->updateDefaultText();

  q->connect(this->ComboBox, SIGNAL(currentIndexChanged(int)),
             q, SLOT(emitCurrentNodeChanged(int)));

  q->setEnabled(q->mrmlScene() != 0);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2Private::setModel(QAbstractItemModel* model)
{
  Q_Q(qMRMLNodeComboBox2);
  this->ComboBox->setModel(model);
  q->connect(model, SIGNAL(rowsInserted(const QModelIndex&, int,int)),
             q, SLOT(emitNodesAdded(const QModelIndex&, int, int)));
  q->connect(model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int,int)),
             q, SLOT(emitNodesAboutToBeRemoved(const QModelIndex&, int, int)));
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBox2Private::mrmlNode(int row)const
{
  QModelIndex modelIndex = this->ComboBox->model()->index(
    row, this->ComboBox->modelColumn(), this->ComboBox->rootModelIndex());
  /*
  Q_Q(const qMRMLNodeComboBox2);
  QString nodeId =
    this->ComboBox->itemData(index, qMRML::UIDRole).toString();
  if (nodeId.isEmpty())
    {
    return 0;
    }
  vtkMRMLScene* scene = q->mrmlScene();
  return scene ? scene->GetNodeByID(nodeId.toLatin1().data()) : 0;
  */
  return this->mrmlNodeFromIndex(modelIndex);
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBox2Private::mrmlNodeFromIndex(const QModelIndex& index)const
{
  Q_Q(const qMRMLNodeComboBox2);
  Q_ASSERT(q->model());
  QString nodeId =
    this->ComboBox->model()->data(index, qMRML::UIDRole).toString();
  if (nodeId.isEmpty())
    {
    return 0;
    }
  vtkMRMLScene* scene = q->mrmlScene();
  return scene ? scene->GetNodeByID(nodeId.toLatin1().data()) : 0;
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2Private::updateDefaultText()
{
  ctkComboBox* cb = qobject_cast<ctkComboBox*>(this->ComboBox);
  if (cb)
    {
    cb->setDefaultText(QObject::tr("Select a ") + this->nodeTypeLabel());
    }
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2Private::updateNoneItem(bool resetRootIndex)
{
  Q_UNUSED(resetRootIndex);
  //Q_Q(qMRMLNodeComboBox2);
  QStringList noneItem;
  if (this->NoneEnabled)
    {
    noneItem.append("None");
    }
  QVariant currentNode =
    this->ComboBox->itemData(this->ComboBox->currentIndex(), qMRML::UIDRole);
  this->MRMLSceneModel->setPreItems(noneItem, this->MRMLSceneModel->mrmlSceneItem());
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
void qMRMLNodeComboBox2Private::updateActionItems(bool resetRootIndex)
{
  Q_Q(qMRMLNodeComboBox2);
  Q_UNUSED(resetRootIndex);

  QVariant currentNode =
    this->ComboBox->itemData(this->ComboBox->currentIndex(), qMRML::UIDRole);

  QStringList extraItems;
  if (q->mrmlScene())
    {
    if (this->AddEnabled || this->RemoveEnabled || this->EditEnabled)
      {
      extraItems.append("separator");
      }
    if (this->AddEnabled)
      {
      extraItems.append(QObject::tr("Create new ") + this->nodeTypeLabel());
      }
    if (this->RemoveEnabled)
      {
      extraItems.append(QObject::tr("Delete current ")  + this->nodeTypeLabel());
      }
    if (this->EditEnabled)
      {
      extraItems.append(QObject::tr("Edit current ")  + this->nodeTypeLabel());
      }
    }
  this->MRMLSceneModel->setPostItems(extraItems, this->MRMLSceneModel->mrmlSceneItem());
  QObject::connect(this->ComboBox->view(), SIGNAL(clicked(const QModelIndex& )),
                   q, SLOT(activateExtraItem(const QModelIndex& )),
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

// --------------------------------------------------------------------------
QString qMRMLNodeComboBox2Private::nodeTypeLabel()const
{
  Q_Q(const qMRMLNodeComboBox2);
  QStringList nodeTypes = q->nodeTypes();
  QString label;
  if (q->mrmlScene() && !nodeTypes.isEmpty())
    {
    label = q->mrmlScene()->GetTagByClassName(nodeTypes[0].toLatin1());
    if (label.isEmpty() && nodeTypes[0] == "vtkMRMLVolumeNode")
      {
      label = QObject::tr("Volume");
      }
    }
  if (label.isEmpty())
    {
    label = QObject::tr("Node");
    }
  return label;
}

// --------------------------------------------------------------------------
// qMRMLNodeComboBox2

// --------------------------------------------------------------------------
qMRMLNodeComboBox2::qMRMLNodeComboBox2(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLNodeComboBox2Private(*this))
{
  Q_D(qMRMLNodeComboBox2);
  d->init(new qMRMLSceneModel2(this));
}

// --------------------------------------------------------------------------
qMRMLNodeComboBox2::~qMRMLNodeComboBox2()
{
}

// --------------------------------------------------------------------------
qMRMLNodeComboBox2::qMRMLNodeComboBox2(QAbstractItemModel* sceneModel, QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLNodeComboBox2Private(*this))
{
  Q_D(qMRMLNodeComboBox2);
  d->init(sceneModel);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2::activateExtraItem(const QModelIndex& index)
{
  Q_D(qMRMLNodeComboBox2);
  // FIXME: check the type of the item on a different role instead of the display role
  QString data = this->model()->data(index, Qt::DisplayRole).toString();
  if (data.startsWith(QObject::tr("Create new ")))
    {
    d->ComboBox->hidePopup();
    this->addNode();
    }
  else if (data.startsWith(QObject::tr("Delete current ")))
    {
    d->ComboBox->hidePopup();
    this->removeCurrentNode();
    }
  else if (data.startsWith(QObject::tr("Edit current ")))
    {
    d->ComboBox->hidePopup();
    this->editCurrentNode();
    }
}

//-----------------------------------------------------------------------------
void qMRMLNodeComboBox2::addAttribute(const QString& nodeType,
                                     const QString& attributeName,
                                     const QVariant& attributeValue)
{
  Q_D(qMRMLNodeComboBox2);
  d->MRMLNodeFactory->addAttribute(attributeName, attributeValue.toString());
  this->sortFilterProxyModel()->addAttribute(nodeType, attributeName, attributeValue);
}

//-----------------------------------------------------------------------------
void qMRMLNodeComboBox2::setBaseName(const QString& baseName)
{
  Q_D(qMRMLNodeComboBox2);
  QStringList nodeClasses = this->nodeTypes();
  if (nodeClasses.isEmpty())
    {
    return;
    }
  d->MRMLNodeFactory->setBaseName(nodeClasses[0], baseName);
}

//-----------------------------------------------------------------------------
QString qMRMLNodeComboBox2::baseName()const
{
  Q_D(const qMRMLNodeComboBox2);
  QStringList nodeClasses = this->nodeTypes();
  if (nodeClasses.isEmpty())
    {
    return QString();
    }
  return d->MRMLNodeFactory->baseName(nodeClasses[0]);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2::addNode()
{
  Q_D(qMRMLNodeComboBox2);
  // Create the MRML node via the MRML Scene
  // FIXME, for the moment we create only nodes of the first type, but we should
  // be able to add a node of any type in NodeTypes
  vtkMRMLNode * _node =
    d->MRMLNodeFactory->createNode(this->nodeTypes()[0]);
  // The created node is appended at the bottom of the current list
  Q_ASSERT(_node);
  if (_node && this->selectNodeUponCreation())
    {// select the created node.
    this->setCurrentNode(_node);
    }
  emit this->nodeAddedByUser(_node);
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBox2::currentNode()const
{
  Q_D(const qMRMLNodeComboBox2);
  return d->mrmlNode(d->ComboBox->currentIndex());
}

// --------------------------------------------------------------------------
QString qMRMLNodeComboBox2::currentNodeId()const
{
  vtkMRMLNode* node = this->currentNode();
  return node ? node->GetID() : "";
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2::editCurrentNode()
{
  //Q_D(const qMRMLNodeComboBox2);
  //FIXME
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2::emitCurrentNodeChanged(int currentIndex)
{
  Q_D(qMRMLNodeComboBox2);
  vtkMRMLNode*  node = 0;
  if (qobject_cast<QListView*>(d->ComboBox->view()))
    {
    node = d->mrmlNode(currentIndex);
    }
  else
    {// special case where the view can handle a tree... currentIndex could be
    // from any parent, not only a top level..
    QModelIndex currentViewIndex = d->ComboBox->view()->currentIndex();
    if (currentViewIndex.row() != currentIndex)
      {
      currentViewIndex = d->ComboBox->model()->index(
        currentIndex, d->ComboBox->modelColumn(), currentViewIndex.parent());
      }
    node = d->mrmlNodeFromIndex(currentViewIndex);
    }
  if (!node && ((!d->NoneEnabled &&currentIndex != -1) || (d->NoneEnabled && currentIndex != 0)) )
    {
    this->setCurrentNode(this->nodeFromIndex(this->nodeCount()-1));
    }
  else
    {
    emit currentNodeChanged(node);
    emit currentNodeChanged(node != 0);
    }
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLNodeComboBox2::mrmlScene()const
{
  Q_D(const qMRMLNodeComboBox2);
  return d->MRMLSceneModel->mrmlScene();
}

// --------------------------------------------------------------------------
int qMRMLNodeComboBox2::nodeCount()const
{
  Q_D(const qMRMLNodeComboBox2);
  int extraItemsCount =
    d->MRMLSceneModel->preItems(d->MRMLSceneModel->mrmlSceneItem()).count()
    + d->MRMLSceneModel->postItems(d->MRMLSceneModel->mrmlSceneItem()).count();
  Q_ASSERT(!this->mrmlScene() || d->ComboBox->count() >= extraItemsCount);
  return this->mrmlScene() ? d->ComboBox->count() - extraItemsCount : 0;
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBox2::nodeFromIndex(int index)const
{
  Q_D(const qMRMLNodeComboBox2);
  return d->mrmlNode(d->NoneEnabled ? index + 1 : index);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2::removeCurrentNode()
{
  this->mrmlScene()->RemoveNode(this->currentNode());
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLNodeComboBox2);

  //if (d->MRMLSceneModel->mrmlScene() == scene)
  //  {
  //  return ;
  //  }

  // The Add button is valid only if the scene is non-empty
  //this->setAddEnabled(scene != 0);
  QVariant currentNode = d->ComboBox->itemData(d->ComboBox->currentIndex(), qMRML::UIDRole);

  // Update factory
  d->MRMLNodeFactory->setMRMLScene(scene);
  d->MRMLSceneModel->setMRMLScene(scene);
  d->updateDefaultText();
  d->updateNoneItem(false);
  d->updateActionItems(false);

  // updating the action items reset the root model index. Set it back
  // setting the rootmodel index looses the current item
  d->ComboBox->setRootModelIndex(this->model()->index(0, 0));

  // try to set the current item back
  this->setCurrentNode(currentNode.toString());

  this->setEnabled(scene != 0);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2::setCurrentNode(vtkMRMLNode* newCurrentNode)
{
  this->setCurrentNode(newCurrentNode ? newCurrentNode->GetID() : "");
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2::setCurrentNode(const QString& nodeID)
{
  Q_D(qMRMLNodeComboBox2);
  // a straight forward implementation of setCurrentNode would be:
  //    int index = !nodeID.isEmpty() ? d->ComboBox->findData(nodeID, qMRML::UIDRole) : -1;
  //    if (index == -1 && d->NoneEnabled)
  //      {
  //      index = 0;
  //      }
  //    d->ComboBox->setCurrentIndex(index);
  // However it doesn't work for custom comboxboxes that display non-flat lists
  // (typically if it is a tree model/view)
  // let's use a more generic one
  QModelIndexList indexes = d->ComboBox->model()->match(
    d->ComboBox->model()->index(0, 0), qMRML::UIDRole, nodeID, 1,
    Qt::MatchRecursive | Qt::MatchExactly | Qt::MatchWrap);
  if (indexes.size() == 0)
    {
    d->ComboBox->setRootModelIndex(d->ComboBox->model()->index(0, 0));
    d->ComboBox->setCurrentIndex(d->NoneEnabled ? 0 : -1);
    return;
    }
  //d->ComboBox->setRootModelIndex(indexes[0].parent());
  //d->ComboBox->setCurrentIndex(indexes[0].row());
  d->ComboBox->view()->setCurrentIndex(indexes[0]);
  QKeyEvent event(QEvent::ShortcutOverride, Qt::Key_Enter, Qt::NoModifier);
  QApplication::sendEvent(d->ComboBox->view(), &event);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2::setCurrentNode(int index)
{
  Q_D(qMRMLNodeComboBox2);
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
CTK_SET_CPP(qMRMLNodeComboBox2, bool, setSelectNodeUponCreation, SelectNodeUponCreation);
CTK_GET_CPP(qMRMLNodeComboBox2, bool, selectNodeUponCreation, SelectNodeUponCreation);

// --------------------------------------------------------------------------
QStringList qMRMLNodeComboBox2::nodeTypes()const
{
  qMRMLSortFilterProxyModel2* m = this->sortFilterProxyModel();
  return m ? m->nodeTypes() : QStringList();
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox2::setNodeTypes(const QStringList& _nodeTypes)
{
  Q_D(qMRMLNodeComboBox2);
  this->sortFilterProxyModel()->setNodeTypes(_nodeTypes);
  d->updateDefaultText();
  d->updateActionItems();
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox2::setNoneEnabled(bool enable)
{
  Q_D(qMRMLNodeComboBox2);
  if (d->NoneEnabled == enable)
    {
    return;
    }
  d->NoneEnabled = enable;
  d->updateNoneItem();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox2::noneEnabled()const
{
  Q_D(const qMRMLNodeComboBox2);
  return d->NoneEnabled;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox2::setAddEnabled(bool enable)
{
  Q_D(qMRMLNodeComboBox2);
  if (d->AddEnabled == enable)
    {
    return;
    }
  d->AddEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox2::addEnabled()const
{
  Q_D(const qMRMLNodeComboBox2);
  return d->AddEnabled;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox2::setRemoveEnabled(bool enable)
{
  Q_D(qMRMLNodeComboBox2);
  if (d->RemoveEnabled == enable)
    {
    return;
    }
  d->RemoveEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox2::removeEnabled()const
{
  Q_D(const qMRMLNodeComboBox2);
  return d->RemoveEnabled;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox2::setEditEnabled(bool enable)
{
  Q_D(qMRMLNodeComboBox2);
  if (d->EditEnabled == enable)
    {
    return;
    }
  d->EditEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox2::editEnabled()const
{
  Q_D(const qMRMLNodeComboBox2);
  return d->EditEnabled;
}

//--------------------------------------------------------------------------
qMRMLSortFilterProxyModel2* qMRMLNodeComboBox2::sortFilterProxyModel()const
{
  Q_ASSERT(qobject_cast<qMRMLSortFilterProxyModel2*>(this->model()));
  return qobject_cast<qMRMLSortFilterProxyModel2*>(this->model());
}

//----------------------------------=---------------------------------------
QAbstractItemModel* qMRMLNodeComboBox2::model()const
{
  Q_D(const qMRMLNodeComboBox2);
  return d->ComboBox ? d->ComboBox->model() : 0;
}

//----------------------------------=---------------------------------------
QAbstractItemModel* qMRMLNodeComboBox2::rootModel()const
{
  Q_D(const qMRMLNodeComboBox2);
  return d->MRMLSceneModel;
}

//----------------------------------=---------------------------------------
void qMRMLNodeComboBox2::setComboBox(QComboBox* comboBox)
{
  Q_D(qMRMLNodeComboBox2);
  if (comboBox == d->ComboBox)
    {
    return;
    }

  QAbstractItemModel* oldModel = this->model();
  QComboBox* oldComboBox = d->ComboBox;

  this->layout()->addWidget(comboBox);
  d->ComboBox = comboBox;
  d->setModel(oldModel);

  connect(d->ComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(emitCurrentNodeChanged(int)));
  delete oldComboBox;
}

//----------------------------------=---------------------------------------
QComboBox* qMRMLNodeComboBox2::comboBox()const
{
  Q_D(const qMRMLNodeComboBox2);
  return d->ComboBox;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox2::emitNodesAdded(const QModelIndex & parent, int start, int end)
{
  Q_D(qMRMLNodeComboBox2);
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
void qMRMLNodeComboBox2::emitNodesAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
  Q_D(qMRMLNodeComboBox2);
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
