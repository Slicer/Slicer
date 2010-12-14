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
#include <QDebug>
#include <QMouseEvent>

// CTK includes
#include "ctkModelTester.h"

// qMRML includes
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLSceneTransformModel.h"
#include "qMRMLSceneAnnotationModel.h"
#include "qMRMLTreeWidget.h"

#include "qMRMLAnnotationTreeWidget.h"

// MRML includes
#include "vtkMRMLNode.h"

//------------------------------------------------------------------------------
class qMRMLAnnotationTreeWidgetPrivate
{
  Q_DECLARE_PUBLIC(qMRMLAnnotationTreeWidget);
protected:
  qMRMLAnnotationTreeWidget* const q_ptr;
public:
  qMRMLAnnotationTreeWidgetPrivate(qMRMLAnnotationTreeWidget& object);
  void init();

  qMRMLSceneAnnotationModel*           SceneModel;
  qMRMLSortFilterProxyModel* SortFilterModel;
};

//------------------------------------------------------------------------------
qMRMLAnnotationTreeWidgetPrivate::qMRMLAnnotationTreeWidgetPrivate(qMRMLAnnotationTreeWidget& object)
  : q_ptr(&object)
{
  this->SceneModel = 0;
  this->SortFilterModel = 0;
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidgetPrivate::init()
{
  Q_Q(qMRMLAnnotationTreeWidget);
  //p->qMRMLTreeWidget::setModel(new qMRMLItemModel(p));
  //this->SceneModel = new qMRMLSceneAnnotationModel(q);
  //this->SceneModel->setColumnCount(6);

  //this->SceneModel->setListenNodeModifiedEvent(true);
  this->SceneModel = new qMRMLSceneAnnotationModel(q);
  q->setSceneModel(this->SceneModel, "Annotation");
  //this->SortFilterModel = new qMRMLSortFilterProxyModel(q);
  // we only want to show vtkMRMLAnnotationNodes and vtkMRMLAnnotationHierarchyNodes
  QStringList nodeTypes = QStringList();
  nodeTypes.append("vtkMRMLAnnotationNode");
  nodeTypes.append("vtkMRMLAnnotationHierarchyNode");

  //this->SortFilterModel->setNodeTypes(nodeTypes);
  q->setNodeTypes(nodeTypes);
  this->SortFilterModel = q->sortFilterProxyModel();

  //this->SortFilterModel->setSourceModel(this->SceneModel);
  //q->qMRMLTreeWidget::setModel(this->SortFilterModel);

  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);
  //QObject::connect(q, SIGNAL(activated(const QModelIndex&)),
  //                 q, SLOT(onActivated(const QModelIndex&)));
  //QObject::connect(q, SIGNAL(clicked(const QModelIndex&)),
  //                 q, SLOT(onActivated(const QModelIndex&)));


  QObject::connect(q, SIGNAL(clicked(const QModelIndex& )),
                   q, SLOT(onClicked(const QModelIndex&)));


  q->setUniformRowHeights(true);
}

//------------------------------------------------------------------------------
qMRMLAnnotationTreeWidget::qMRMLAnnotationTreeWidget(QWidget *_parent)
  :qMRMLTreeWidget(_parent)
  , d_ptr(new qMRMLAnnotationTreeWidgetPrivate(*this))
{
  Q_D(qMRMLAnnotationTreeWidget);
  d->init();

  // we need to enable mouse tracking to set the appropriate cursor while mouseMove occurs
  this->setMouseTracking(true);
}

//------------------------------------------------------------------------------
qMRMLAnnotationTreeWidget::~qMRMLAnnotationTreeWidget()
{
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLAnnotationTreeWidget);
  Q_ASSERT(d->SortFilterModel);
  // only qMRMLSceneModel needs the scene, the other proxies don't care.
  d->SceneModel->setMRMLScene(scene);

  this->expandAll();
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::onActivated(const QModelIndex& index)
{
  //Q_D(qMRMLAnnotationTreeWidget);
  //Q_ASSERT(d->SortFilterModel);
  this->qMRMLTreeWidget::onActivated(index);
  //emit currentNodeChanged(d->SortFilterModel->mrmlNode(index));
}


//------------------------------------------------------------------------------
//
// Click and selected event handling
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::onClicked(const QModelIndex& index)
{

  Q_D(qMRMLAnnotationTreeWidget);

  // if the user clicked on a hierarchy, set this as the active one
  // this means, new annotations or new user-created hierarchies will be created
  // as childs of this one

  if(d->SortFilterModel->mrmlNode(index)->IsA("vtkMRMLAnnotationHierarchyNode"))
    {
    this->m_Logic->SetActiveHierarchyNode(vtkMRMLAnnotationHierarchyNode::SafeDownCast(d->SortFilterModel->mrmlNode(index)));
    }

  // check if user clicked on icon, this can happen even after we marked a hierarchy as active
  if (index.column() == qMRMLSceneAnnotationModel::VisibilityColumn)
    {
    // user wants to toggle the visibility of the annotation
    this->onVisibilityColumnClicked(d->SortFilterModel->mrmlNode(index));
    }
  else if (index.column() == qMRMLSceneAnnotationModel::LockColumn)
    {
    // user wants to toggle the un-/lock of the annotation
    this->onLockColumnClicked(d->SortFilterModel->mrmlNode(index));
    }
  else if (index.column() == qMRMLSceneAnnotationModel::EditColumn)
    {
    // user wants to edit the properties of this annotation
    this->m_Widget->propertyEditButtonClicked(QString(d->SortFilterModel->mrmlNode(index)->GetID()));
    }

}

//------------------------------------------------------------------------------
const char* qMRMLAnnotationTreeWidget::firstSelectedNode()
{
  Q_D(qMRMLAnnotationTreeWidget);
  QModelIndexList selected = this->selectedIndexes();

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return 0;
    }

  // now get the first selected item
  QModelIndex index = selected.first();

  // check if it is a valid node
  if (!d->SortFilterModel->mrmlNode(index))
    {
    return 0;
    }

  return d->SortFilterModel->mrmlNode(index)->GetID();
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::toggleLockForSelected()
{
  Q_D(qMRMLAnnotationTreeWidget);
  QModelIndexList selected = this->selectedIndexes();

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return;
    }

  for (int i = 0; i < selected.size(); ++i) {

    // we need to prevent looping through all columns
    // there we only update once a row
    if (selected.at(i).column() == qMRMLSceneAnnotationModel::LockColumn)
      {

      vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(d->SortFilterModel->mrmlNode(selected.at(i)));

      if (annotationNode)
        {
        this->onLockColumnClicked(annotationNode);
        }

      }

  }

}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::toggleVisibilityForSelected()
{
  Q_D(qMRMLAnnotationTreeWidget);
  QModelIndexList selected = this->selectedIndexes();

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return;
    }

  for (int i = 0; i < selected.size(); ++i)
    {

    // we need to prevent looping through all columns
    // there we only update once a row
    if (selected.at(i).column() == qMRMLSceneAnnotationModel::VisibilityColumn)
      {

      vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(d->SortFilterModel->mrmlNode(selected.at(i)));

      if (annotationNode)
        {
        this->onVisibilityColumnClicked(annotationNode);
        }

      }

    } // for loop

}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::deleteSelected()
{
  Q_D(qMRMLAnnotationTreeWidget);
  QModelIndexList selected = this->selectedIndexes();

  QStringList markedForDeletion;

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return;
    }

  // case: delete a hierarchy only, if it is the only selection
  // warning: all directly under this hierarchy laying annotation nodes will be lost
  // if there are other hierarchies underneath the one which gets deleted, they will get reparented
  if (selected.size()==1)
    {
    // only one item was selected, is this a hierarchy?
    vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(d->SortFilterModel->mrmlNode(selected.first()));

    if (hierarchyNode)
      {
      // this is exciting!!

      // TODO

      // all done, bail out
      return;
      }
    // if this is not a hierarchyNode, treat this single selection as a normal case

    }
  // end hierarchy case


  // case:: delete all selected annotationNodes but no hierarchies
  for (int i = 0; i < selected.size(); ++i)
    {

    // we need to prevent looping through all columns
    // there we only update once a row
    if (selected.at(i).column() == qMRMLSceneAnnotationModel::VisibilityColumn)
      {

      vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(d->SortFilterModel->mrmlNode(selected.at(i)));

      if (annotationNode)
        {

        // we mark this one for deletion
        markedForDeletion.append(QString(annotationNode->GetID()));

        }

      }
    } // for

  // we parsed the complete selection and saved all mrmlIds to delete
  // now, it is safe to delete
  for (int j=0; j < markedForDeletion.size(); ++j)
    {

    vtkMRMLAnnotationNode* annotationNodeToDelete = vtkMRMLAnnotationNode::SafeDownCast(this->m_Logic->GetMRMLScene()->GetNodeByID(markedForDeletion.at(j).toLatin1()));
    this->m_Logic->RemoveAnnotationNode(annotationNodeToDelete);

    }

}

//------------------------------------------------------------------------------
// Return the selected annotations as a collection of mrmlNodes
//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::selectedAsCollection(vtkCollection* collection)
{

  if (!collection)
    {
    return;
    }

  Q_D(qMRMLAnnotationTreeWidget);
  QModelIndexList selected = this->selectedIndexes();

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return;
    }

  qSort(selected.begin(),selected.end());

  for (int i = 0; i < selected.size(); ++i)
    {

      // we need to prevent looping through all columns
      // there we only update once a row
      if (selected.at(i).column() == qMRMLSceneAnnotationModel::VisibilityColumn)
        {

        vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(d->SortFilterModel->mrmlNode(selected.at(i)));

        //if (node->IsA("vtkMRMLAnnotationNode"))
         // {
        collection->AddItem(node);
         // }

        }

    } // for

}

//------------------------------------------------------------------------------
//
// MouseMove event handling
//
//------------------------------------------------------------------------------

#ifndef QT_NO_CURSOR
//------------------------------------------------------------------------------
bool qMRMLAnnotationTreeWidget::viewportEvent(QEvent* e)
{

  // reset the cursor if we leave the viewport
  if(e->type() == QEvent::Leave)
    {
    setCursor(QCursor());
    }

  return QTreeView::viewportEvent(e);
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::mouseMoveEvent(QMouseEvent* e)
{
  this->QTreeView::mouseMoveEvent(e);

  // get the index of the current column
  QModelIndex index = indexAt(e->pos());

  if (index.column() == qMRMLSceneAnnotationModel::VisibilityColumn || index.column() == qMRMLSceneAnnotationModel::LockColumn || index.column() == qMRMLSceneAnnotationModel::EditColumn)
    {
    // we are over a column with a clickable icon
    // let's change the cursor
    QCursor handCursor(Qt::PointingHandCursor);
    this->setCursor(handCursor);
    // .. and bail out
    return;
    }
  else if(this->cursor().shape() == Qt::PointingHandCursor)
    {
    // if we are NOT over such a column and we already have a changed cursor,
    // reset it!
    this->setCursor(QCursor());
    }

}
#endif

//------------------------------------------------------------------------------
//
// Layout and behavior customization
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::hideScene()
{
  Q_D(qMRMLAnnotationTreeWidget);

  // first, we set the root index to the mrmlScene
  // this works also if the scene is not defined yet
  QModelIndex root = d->SceneModel->mrmlSceneIndex();

  if (this->m_Logic)
    {
    // if the logic is already registered, we look for the first annotationHierarchyNode
    vtkMRMLNode* toplevelNode = this->m_Logic->GetMRMLScene()->GetNthNodeByClass(0,"vtkMRMLAnnotationHierarchyNode");

    if (toplevelNode)
      {
      // if we find it, we use it as the root index
      root = d->SceneModel->indexes(toplevelNode)[0];
      }
    }

  this->setRootIndex(d->SortFilterModel->mapFromSource(root));


  // set the column widths
  this->header()->setResizeMode(qMRMLSceneAnnotationModel::DummyColumn, (QHeaderView::ResizeToContents));
  this->header()->setResizeMode(qMRMLSceneAnnotationModel::VisibilityColumn, (QHeaderView::ResizeToContents));
  this->header()->setResizeMode(qMRMLSceneAnnotationModel::LockColumn, (QHeaderView::ResizeToContents));
  this->header()->setResizeMode(qMRMLSceneAnnotationModel::EditColumn, (QHeaderView::ResizeToContents));
  this->header()->setResizeMode(qMRMLSceneAnnotationModel::ValueColumn, (QHeaderView::ResizeToContents));
  this->header()->setResizeMode(qMRMLSceneAnnotationModel::TextColumn, (QHeaderView::ResizeToContents));

  // hide the strange columns
  this->hideColumn(6);
  this->hideColumn(7);
  this->hideColumn(8);
  this->hideColumn(9);
  this->hideColumn(10);
  this->hideColumn(11);


}

//------------------------------------------------------------------------------
//
// In-Place Editing of Annotations
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::onVisibilityColumnClicked(vtkMRMLNode* node)
{

  if (!node)
    {
    // no node found!
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);

  if (annotationNode)
    {
    // this is a valid annotationNode
    annotationNode->SetVisible(!annotationNode->GetVisible());

    }


  // TODO move to logic
  vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);

  if (hierarchyNode)
    {
    vtkCollection* children = vtkCollection::New();
    hierarchyNode->GetChildrenDisplayableNodes(children);

    children->InitTraversal();
    for (int i=0; i<children->GetNumberOfItems(); ++i)
      {
      vtkMRMLAnnotationNode* childNode = vtkMRMLAnnotationNode::SafeDownCast(children->GetItemAsObject(i));
      if (childNode)
        {
        // this is a valid annotation child node
        //
        childNode->SetVisible(!childNode->GetVisible());
        }
      } // for loop

    } // if hierarchyNode


}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::onLockColumnClicked(vtkMRMLNode* node)
{

  if (!node)
    {
    // no node found!
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);

  if (annotationNode)
    {
    // this is a valid annotationNode
    annotationNode->SetLocked(!annotationNode->GetLocked());

    }


  // TODO move to logic
  vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);

  if (hierarchyNode)
    {
    vtkCollection* children = vtkCollection::New();
    hierarchyNode->GetChildrenDisplayableNodes(children);

    children->InitTraversal();
    for (int i=0; i<children->GetNumberOfItems(); ++i)
      {
      vtkMRMLAnnotationNode* childNode = vtkMRMLAnnotationNode::SafeDownCast(children->GetItemAsObject(i));
      if (childNode)
        {
        // this is a valid annotation child node
        //
        childNode->SetLocked(!childNode->GetLocked());
        }
      } // for loop

    } // if hierarchyNode


}

//------------------------------------------------------------------------------
//
// Connections to other classes
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Set and observe the GUI widget
//-----------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::setAndObserveWidget(qSlicerAnnotationModuleWidget* widget)
{
  if (!widget)
    {
    return;
    }

  Q_D(qMRMLAnnotationTreeWidget);

  this->m_Widget = widget;


  // propagate down to model
  d->SceneModel->setAndObserveWidget(this->m_Widget);

}

//-----------------------------------------------------------------------------
/// Set and observe the logic
//-----------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::setAndObserveLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  if (!logic)
    {
    return;
    }

  Q_D(qMRMLAnnotationTreeWidget);

  this->m_Logic = logic;

  // propagate down to model
  d->SceneModel->setAndObserveLogic(this->m_Logic);

}
