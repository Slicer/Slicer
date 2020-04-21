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
#include <QHeaderView>
#include <QMessageBox>
#include <QMouseEvent>

// CTK includes
//#include "ctkModelTester.h"

// qMRML includes
//#include "qMRMLSortFilterProxyModel.h"
//#include "qMRMLSceneTransformModel.h"
//#include "qMRMLTreeView.h"

// Annotations includes
#include "qMRMLAnnotationTreeView.h"
#include "qMRMLSceneAnnotationModel.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkSlicerAnnotationModuleLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotation
class qMRMLAnnotationTreeViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLAnnotationTreeView);
protected:
  qMRMLAnnotationTreeView* const q_ptr;
public:
  qMRMLAnnotationTreeViewPrivate(qMRMLAnnotationTreeView& object);
  void init();

  qMRMLSceneAnnotationModel* SceneModel;
  qMRMLSortFilterProxyModel* SortFilterModel;
};

//------------------------------------------------------------------------------
qMRMLAnnotationTreeViewPrivate::qMRMLAnnotationTreeViewPrivate(qMRMLAnnotationTreeView& object)
  : q_ptr(&object)
{
  this->SceneModel = nullptr;
  this->SortFilterModel = nullptr;
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeViewPrivate::init()
{
  Q_Q(qMRMLAnnotationTreeView);
  //p->qMRMLTreeView::setModel(new qMRMLItemModel(p));
  //this->SceneModel = new qMRMLSceneAnnotationModel(q);
  //this->SceneModel->setColumnCount(6);

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

  q->setShowScene(false);
  //this->SortFilterModel->setSourceModel(this->SceneModel);
  //q->qMRMLTreeView::setModel(this->SortFilterModel);

  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);

  QObject::connect(q, SIGNAL(clicked(QModelIndex)),
                   q, SLOT(onClicked(QModelIndex)));

}

//------------------------------------------------------------------------------
qMRMLAnnotationTreeView::qMRMLAnnotationTreeView(QWidget *_parent)
  :qMRMLTreeView(_parent)
  , d_ptr(new qMRMLAnnotationTreeViewPrivate(*this))
{
  Q_D(qMRMLAnnotationTreeView);
  d->init();

  // we need to enable mouse tracking to set the appropriate cursor while mouseMove occurs
  this->setMouseTracking(true);
}

//------------------------------------------------------------------------------
qMRMLAnnotationTreeView::~qMRMLAnnotationTreeView() = default;

//------------------------------------------------------------------------------
//
// Click and selected event handling
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeView::onSelectionChanged(const QItemSelection & selected,
                                                 const QItemSelection & deselected)
{
  Q_D(qMRMLAnnotationTreeView);

  // if the user clicked on a hierarchy, set this as the active one
  // this means, new annotations or new user-created hierarchies will be created
  // as children of this one
  vtkMRMLNode* newCurrentNode = nullptr;
  if (selected.indexes().count() > 0)
    {
    newCurrentNode = d->SortFilterModel->mrmlNodeFromIndex(selected.indexes()[0]);
    }
  vtkMRMLNode* newActiveNode =
    this->annotationModel()->activeHierarchyNode(newCurrentNode);
  this->m_Logic->SetActiveHierarchyNodeID(
    newActiveNode ? newActiveNode->GetID() : nullptr);

  this->Superclass::onSelectionChanged(selected, deselected);
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeView::onClicked(const QModelIndex& index)
{

  Q_D(qMRMLAnnotationTreeView);

  vtkMRMLNode *mrmlNode = d->SortFilterModel->mrmlNodeFromIndex(index);
  if (!mrmlNode)
    {
    return;
    }

  // check if user clicked on icon, this can happen even after we marked a
  // hierarchy as active
  if (index.column() == d->SceneModel->checkableColumn())
    {
    // Let the superclass view to handle the event, it will update the item
    // which will update the node.
    }
  else if (index.column() == d->SceneModel->visibilityColumn())
    {
    // Let the superclass view to handle the event.
    // user wants to toggle the visibility of the annotation
    //this->onVisibilityColumnClicked(mrmlNode);
    }
  else if (index.column() == d->SceneModel->lockColumn())
    {
    // user wants to toggle the un-/lock of the annotation
    this->onLockColumnClicked(mrmlNode);
    }
  else if (index.column() == d->SceneModel->editColumn())
    {
    // user wants to edit the properties of this annotation
    // signal the widget
    this->onPropertyEditButtonClicked(QString(mrmlNode->GetID()));
//    this->m_Widget->propertyEditButtonClicked(QString(mrmlNode->GetID()));
    }

}

//------------------------------------------------------------------------------
const char* qMRMLAnnotationTreeView::firstSelectedNode()
{
  Q_D(qMRMLAnnotationTreeView);
  QModelIndexList selected = this->selectedIndexes();

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return nullptr;
    }

  // now get the first selected item
  QModelIndex index = selected.first();

  // check if it is a valid node
  if (!d->SortFilterModel->mrmlNodeFromIndex(index))
    {
    return nullptr;
    }

  return d->SortFilterModel->mrmlNodeFromIndex(index)->GetID();
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeView::toggleLockForSelected()
{
  Q_D(qMRMLAnnotationTreeView);
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
    if (selected.at(i).column() == d->SceneModel->lockColumn())
      {

      vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
        d->SortFilterModel->mrmlNodeFromIndex(selected.at(i)));

      if (annotationNode)
        {
        this->onLockColumnClicked(annotationNode);
        }

      }

  }

}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeView::toggleVisibilityForSelected()
{
  Q_D(qMRMLAnnotationTreeView);
  QModelIndexList selected = this->selectedIndexes();

  foreach(const QModelIndex& index, selected )
    {
    // we need to prevent looping through all columns
    // there we only update once a row
    if (index.column() == d->SceneModel->visibilityColumn())
      {
      this->toggleVisibility(index);
      }

    } // for loop

}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeView::deleteSelected()
{
  Q_D(qMRMLAnnotationTreeView);
  QModelIndexList selected = this->selectedIndexes();

  QStringList markedForDeletion;

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return;
    }

  // case: delete a hierarchy only, if it is the only selection
  // warning: all directly under this hierarchy laying annotation nodes will be lost
  // if there are other hierarchies underneath the one which gets deleted,
  // they will get reparented

  // the selected count will be number of rows that are highlighted * number
  // of columns (each item in a row is selected when the row is highlighted),
  // so to check for one row being selected, the count has to be 1 * the
  // columnCount
  if (selected.count() == d->SceneModel->columnCount())
    {
    // only one item was selected, is this a hierarchy?
    vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(d->SortFilterModel->mrmlNodeFromIndex(selected.first()));

    if (hierarchyNode)
      {
      // this is exciting!!

      // get confirmation to delete
      QMessageBox msgBox;
      msgBox.setText("Do you really want to delete the selected hierarchy?");
      msgBox.setInformativeText("This includes all directly associated annotations.");
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::No);
      int ret = msgBox.exec();

      if (ret == QMessageBox::Yes)
        {
        this->mrmlScene()->StartState(vtkMRMLScene::BatchProcessState);
        hierarchyNode->DeleteDirectChildren();
        this->mrmlScene()->EndState(vtkMRMLScene::BatchProcessState);

        this->mrmlScene()->RemoveNode(hierarchyNode);

        }
      // all done, bail out
      return;
      }
    // if this is not a hierarchyNode, treat this single selection as a normal case

    }
  // end hierarchy case


  // get confirmation to delete
  QMessageBox msgBox;
  msgBox.setText("Do you really want to delete the selected annotations?");
  msgBox.setInformativeText("This does not include hierarchies.");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  int ret = msgBox.exec();

  if (ret == QMessageBox::No)
    {
    //bail out
    return;
    }

  // case:: delete all selected annotationNodes but no hierarchies
  for (int i = 0; i < selected.count(); ++i)
    {

    // we need to prevent looping through all columns
    // there we only update once a row
    if (selected.at(i).column() == d->SceneModel->visibilityColumn())
      {

      vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(d->SortFilterModel->mrmlNodeFromIndex(selected.at(i)));

      if (annotationNode)
        {

        // we mark this one for deletion
        markedForDeletion.append(QString(annotationNode->GetID()));

        }

      }
    } // for

  // we parsed the complete selection and saved all mrmlIds to delete
  // now, it is safe to delete
  this->mrmlScene()->StartState(vtkMRMLScene::BatchProcessState);
  for (int j=0; j < markedForDeletion.size(); ++j)
    {

    vtkMRMLAnnotationNode* annotationNodeToDelete = vtkMRMLAnnotationNode::SafeDownCast(this->mrmlScene()->GetNodeByID(markedForDeletion.at(j).toUtf8()));
    this->m_Logic->RemoveAnnotationNode(annotationNodeToDelete);

    }
  this->mrmlScene()->EndState(vtkMRMLScene::BatchProcessState);

  this->m_Logic->SetActiveHierarchyNodeID(nullptr);

}

//------------------------------------------------------------------------------
// Return the selected annotations as a collection of mrmlNodes
//------------------------------------------------------------------------------
void qMRMLAnnotationTreeView::selectedAsCollection(vtkCollection* collection)
{

  if (!collection)
    {
    return;
    }

  Q_D(qMRMLAnnotationTreeView);
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
    if (selected.at(i).column() == d->SceneModel->visibilityColumn())
      {
      vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(d->SortFilterModel->mrmlNodeFromIndex(selected.at(i)));
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
bool qMRMLAnnotationTreeView::viewportEvent(QEvent* e)
{

  // reset the cursor if we leave the viewport
  if(e->type() == QEvent::Leave)
    {
    this->setCursor(QCursor());
    }

  return this->Superclass::viewportEvent(e);
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeView::mouseMoveEvent(QMouseEvent* e)
{
  Q_D(qMRMLAnnotationTreeView);
  this->Superclass::mouseMoveEvent(e);

  // get the index of the current column
  QModelIndex index = indexAt(e->pos());

  if (index.column() == d->SceneModel->visibilityColumn() ||
      index.column() == d->SceneModel->lockColumn() ||
      index.column() == d->SceneModel->editColumn())
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
void qMRMLAnnotationTreeView::hideScene()
{
  // set the column widths
  for (int i = 0; i < this->header()->count(); ++i)
    {
    this->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
}

//------------------------------------------------------------------------------
//
// In-Place Editing of Annotations
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeView::toggleVisibility(const QModelIndex& index)
{
  Q_D(qMRMLAnnotationTreeView);
  vtkMRMLNode* node = d->SortFilterModel->mrmlNodeFromIndex(index);
  if (!node)
    {
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);

  if (annotationNode)
    {
    // this is a valid annotationNode
    annotationNode->SetDisplayVisibility(!annotationNode->GetDisplayVisibility());
    }
  else
    {
    this->Superclass::toggleVisibility(index);
    }
  // taking out the switch for hierarchy nodes, do it via the buttons above
/*
  vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);

  if (hierarchyNode)
    {
    this->m_Logic->SetHierarchyAnnotationsVisibleFlag(hierarchyNode, true);
    } // if hierarchyNode
*/
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeView::onLockColumnClicked(vtkMRMLNode* node)
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


  // taking out the switch for hierarchy nodes, do it via the buttons above
/*
  vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);

  if (hierarchyNode)
    {
    this->m_Logic->SetHierarchyAnnotationsLockFlag(hierarchyNode, true);


    } // if hierarchyNode
*/

}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeView::mousePressEvent(QMouseEvent* event)
{
  // skip qMRMLTreeView
  this->Superclass::mousePressEvent(event);
}

//------------------------------------------------------------------------------
//
// Connections to other classes
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Set and observe the logic
//-----------------------------------------------------------------------------
void qMRMLAnnotationTreeView::setLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  if (!logic)
    {
    return;
    }

  Q_D(qMRMLAnnotationTreeView);

  this->m_Logic = logic;

  // propagate down to model
  d->SceneModel->setLogic(this->m_Logic);

}

//-----------------------------------------------------------------------------
/// Annotation model
//-----------------------------------------------------------------------------
qMRMLSceneAnnotationModel* qMRMLAnnotationTreeView::annotationModel()const
{
  Q_D(const qMRMLAnnotationTreeView);
  return d->SceneModel;
}
