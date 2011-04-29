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
#include <QHeaderView>
#include <QMessageBox>

// CTK includes
#include "ctkModelTester.h"

// qMRML includes
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLSceneTransformModel.h"
#include "qMRMLSceneViewsModel.h"
#include "qMRMLTreeWidget.h"

#include "qMRMLSceneViewsTreeWidget.h"

// MRML includes
#include "vtkMRMLNode.h"
#include "vtkMRMLSceneViewNode.h"
#include "vtkMRMLHierarchyNode.h"

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SceneViews
class qMRMLSceneViewsTreeWidgetPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSceneViewsTreeWidget);
protected:
  qMRMLSceneViewsTreeWidget* const q_ptr;
public:
  qMRMLSceneViewsTreeWidgetPrivate(qMRMLSceneViewsTreeWidget& object);
  void init();

  qMRMLSceneViewsModel*           SceneModel;
  qMRMLSortFilterProxyModel* SortFilterModel;
};

//------------------------------------------------------------------------------
qMRMLSceneViewsTreeWidgetPrivate::qMRMLSceneViewsTreeWidgetPrivate(qMRMLSceneViewsTreeWidget& object)
  : q_ptr(&object)
{
  this->SceneModel = 0;
  this->SortFilterModel = 0;
}

//------------------------------------------------------------------------------
void qMRMLSceneViewsTreeWidgetPrivate::init()
{
  Q_Q(qMRMLSceneViewsTreeWidget);
  //p->qMRMLTreeWidget::setModel(new qMRMLItemModel(p));
  //this->SceneModel = new qMRMLSceneViewsModel(q);
  //this->SceneModel->setColumnCount(6);

  //this->SceneModel->setListenNodeModifiedEvent(true);

  this->SceneModel = new qMRMLSceneViewsModel(q);
  q->setSceneModel(this->SceneModel, "SceneViews");
  //this->SortFilterModel = new qMRMLSortFilterProxyModel(q);
  // we only want to show vtkMRMLSceneViewNodes and vtkMRMLHierarchyNodes
  QStringList nodeTypes = QStringList();
  nodeTypes.append("vtkMRMLSceneViewNode");
//  nodeTypes.append("vtkMRMLHierarchyNode");
  
  //this->SortFilterModel->setNodeTypes(nodeTypes);
  q->setNodeTypes(nodeTypes);
  this->SortFilterModel = q->sortFilterProxyModel();
  //this->SortFilterModel->setShowHidden(true);

  //this->SortFilterModel->setSourceModel(this->SceneModel);
  //q->qMRMLTreeWidget::setModel(this->SortFilterModel);

  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);

  QObject::connect(q, SIGNAL(clicked(const QModelIndex& )),
                   q, SLOT(onClicked(const QModelIndex&)));


  QObject::connect( q->selectionModel(),
        SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection & ) ),
        q,
        SLOT( onSelectionChanged( const QItemSelection &, const QItemSelection & ) ),
        Qt::DirectConnection );
  
  q->setUniformRowHeights(true);
}

//------------------------------------------------------------------------------
qMRMLSceneViewsTreeWidget::qMRMLSceneViewsTreeWidget(QWidget *_parent)
  :qMRMLTreeWidget(_parent)
  , d_ptr(new qMRMLSceneViewsTreeWidgetPrivate(*this))
{
  Q_D(qMRMLSceneViewsTreeWidget);
  d->init();

  // we need to enable mouse tracking to set the appropriate cursor while mouseMove occurs
  this->setMouseTracking(true);
}

//------------------------------------------------------------------------------
qMRMLSceneViewsTreeWidget::~qMRMLSceneViewsTreeWidget()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneViewsTreeWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSceneViewsTreeWidget);
  Q_ASSERT(d->SortFilterModel);
  // only qMRMLSceneModel needs the scene, the other proxies don't care.
  d->SceneModel->setMRMLScene(scene);

  this->expandAll();
}

//------------------------------------------------------------------------------
//
// Click and selected event handling
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qMRMLSceneViewsTreeWidget::onSelectionChanged(const QItemSelection& index,const QItemSelection& beforeIndex)
{

  Q_UNUSED(index)
  Q_UNUSED(beforeIndex)

  if (index.size() == 0)
    {
    // the user clicked in empty space of the treeView
    // so we set the active hierarchy to the top level one
    this->m_Logic->SetActiveHierarchyNode(0);
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneViewsTreeWidget::onClicked(const QModelIndex& index)
{

  Q_D(qMRMLSceneViewsTreeWidget);

  // if the user clicked on a hierarchy, set this as the active one
  // this means, new scene view or new user-created hierarchies will be created
  // as childs of this one
  if(d->SortFilterModel->mrmlNodeFromIndex(index)->IsA("vtkMRMLHierarchyNode"))
    {
    this->m_Logic->SetActiveHierarchyNode(vtkMRMLHierarchyNode::SafeDownCast(d->SortFilterModel->mrmlNodeFromIndex(index)));
    }
  
  // check if user clicked on icon, this can happen even after we marked a hierarchy as active
  if (index.column() == qMRMLSceneViewsModel::RestoreColumn)
    {
    // user wants to toggle the restore
    this->m_Widget->restoreSceneView(QString(d->SortFilterModel->mrmlNodeFromIndex(index)->GetID()));
    }
  else if (index.column() == qMRMLSceneViewsModel::ThumbnailColumn)
    {
    // user wants to edit the properties of this scene view
    this->m_Widget->editSceneView(QString(d->SortFilterModel->mrmlNodeFromIndex(index)->GetID()));
    }

}

//------------------------------------------------------------------------------
QString qMRMLSceneViewsTreeWidget::firstSelectedNode()const
{
  Q_D(const qMRMLSceneViewsTreeWidget);
  QModelIndexList selected = this->selectedIndexes();

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return QString();
    }

  // now get the first selected item
  QModelIndex index = selected.first();

  // check if it is a valid node
  if (!d->SortFilterModel->mrmlNodeFromIndex(index))
    {
    return QString();
    }

  const char* id = d->SortFilterModel->mrmlNodeFromIndex(index)->GetID();
  return id ? QString(id) : QString();
}

//------------------------------------------------------------------------------
void qMRMLSceneViewsTreeWidget::deleteSelected()
{
  Q_D(qMRMLSceneViewsTreeWidget);
  QModelIndexList selected = this->selectedIndexes();

  QStringList markedForDeletion;

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return;
    }

  // case: delete a hierarchy only, if it is the only selection
  // warning: all directly under this hierarchy laying scene view nodes will be lost
  // if there are other hierarchies underneath the one which gets deleted, they will get reparented
  if (selected.count()==6)
    {
    // only one item was selected, is this a hierarchy?
    vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(d->SortFilterModel->mrmlNodeFromIndex(selected.first()));
    if (hierarchyNode)
      {
      // get confirmation to delete
      QMessageBox msgBox;
      msgBox.setText("Do you really want to delete the selected hierarchy?");
      msgBox.setInformativeText("This includes all directly associated scene views.");
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::No);
      int ret = msgBox.exec();

      if (ret == QMessageBox::Yes)
        {

        hierarchyNode->RemoveHierarchyChildrenNodes();

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
  msgBox.setText("Do you really want to delete the selected SceneView?");
  msgBox.setInformativeText("This does not include hierarchies. This can not be undone.");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  int ret = msgBox.exec();

  if (ret == QMessageBox::No)
    {
    //bail out
    return;
    }

  if (ret == QMessageBox::Yes)
    {
    // delete the selected scene view nodes but no hierarchies
    for (int i = 0; i < selected.count(); ++i)
      {

      // we need to prevent looping through all columns
      // there we only update once a row
      if (selected.at(i).column() ==  qMRMLSceneViewsModel::ThumbnailColumn)
        {
        
        vtkMRMLSceneViewNode* sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(d->SortFilterModel->mrmlNodeFromIndex(selected.at(i)));
        
        if (sceneViewNode)
          {
          // we mark this one for deletion
          markedForDeletion.append(QString(sceneViewNode->GetID()));
          }
        }
      } // for
    
    // we parsed the complete selection and saved all mrmlIds to delete
    // now, it is safe to delete
    for (int j=0; j < markedForDeletion.size(); ++j)
      {
      vtkMRMLSceneViewNode* sceneViewNodeToDelete = vtkMRMLSceneViewNode::SafeDownCast(this->m_Logic->GetMRMLScene()->GetNodeByID(markedForDeletion.at(j).toLatin1()));
      this->m_Logic->RemoveSceneViewNode(sceneViewNodeToDelete);
      }
    this->m_Logic->SetActiveHierarchyNode(0);
    }

}


//------------------------------------------------------------------------------
//
// MouseMove event handling
//
//------------------------------------------------------------------------------

#ifndef QT_NO_CURSOR
//------------------------------------------------------------------------------
bool qMRMLSceneViewsTreeWidget::viewportEvent(QEvent* e)
{

  // reset the cursor if we leave the viewport
  if(e->type() == QEvent::Leave)
    {
    setCursor(QCursor());
    }

  return QTreeView::viewportEvent(e);
}

//------------------------------------------------------------------------------
void qMRMLSceneViewsTreeWidget::mouseMoveEvent(QMouseEvent* e)
{
  this->QTreeView::mouseMoveEvent(e);

  // get the index of the current column
  QModelIndex index = indexAt(e->pos());

  if (index.column() == qMRMLSceneViewsModel::ThumbnailColumn || index.column() == qMRMLSceneViewsModel::RestoreColumn)
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
void qMRMLSceneViewsTreeWidget::setSelectedNode(const QString& id)
{
  Q_D(qMRMLSceneViewsTreeWidget);

  vtkMRMLNode* node = this->mrmlScene()->GetNodeByID(id.toLatin1());

  if (node)
    {
    this->setCurrentIndex(d->SortFilterModel->indexFromMRMLNode(node));
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneViewsTreeWidget::hideScene()
{
  Q_D(qMRMLSceneViewsTreeWidget);

  // first, we set the root index to the mrmlScene
  // this works also if the scene is not defined yet
  QModelIndex root = d->SceneModel->mrmlSceneIndex();

   if (this->m_Logic)
    {
    // if the logic is already registered, we look for the first HierarchyNode
    vtkMRMLNode* toplevelNode = this->m_Logic->GetTopLevelHierarchyNode(0);
    //GetMRMLScene()->GetNthNodeByClass(0,"vtkMRMLHierarchyNode");

    if (toplevelNode)
      {
      // if we find it, we use it as the root index
      root = d->SceneModel->indexes(toplevelNode)[0];
      }
    }
   
  this->setRootIndex(d->SortFilterModel->mapFromSource(root));

  // set the column widths
  this->header()->setResizeMode(qMRMLSceneViewsModel::DummyColumn, (QHeaderView::ResizeToContents));
  this->header()->setResizeMode(qMRMLSceneViewsModel::ThumbnailColumn, (QHeaderView::ResizeToContents));
  this->header()->setResizeMode(qMRMLSceneViewsModel::RestoreColumn, (QHeaderView::ResizeToContents));
  this->header()->setResizeMode(qMRMLSceneViewsModel::NameColumn, (QHeaderView::ResizeToContents));
  this->header()->setResizeMode(qMRMLSceneViewsModel::DescriptionColumn, (QHeaderView::ResizeToContents));

  // hide the strange columns
  this->hideColumn(6);
  this->hideColumn(7);
  this->hideColumn(8);
  this->hideColumn(9);
  this->hideColumn(10);
  //this->hideColumn(11);


}

//------------------------------------------------------------------------------
void qMRMLSceneViewsTreeWidget::mousePressEvent(QMouseEvent* event)
{
  // skip qMRMLTreeWidget
  this->QTreeView::mousePressEvent(event);
}

//------------------------------------------------------------------------------
//
// Connections to other classes
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Set and observe the GUI widget
//-----------------------------------------------------------------------------
void qMRMLSceneViewsTreeWidget::setAndObserveWidget(qSlicerSceneViewsModuleWidget* widget)
{
  if (!widget)
    {
    return;
    }

  //Q_D(qMRMLSceneViewsTreeWidget);

  this->m_Widget = widget;

}

//-----------------------------------------------------------------------------
/// Set and observe the logic
//-----------------------------------------------------------------------------
void qMRMLSceneViewsTreeWidget::setAndObserveLogic(vtkSlicerSceneViewsModuleLogic* logic)
{
  if (!logic)
    {
    return;
    }

  //Q_D(qMRMLSceneViewsTreeWidget);

  this->m_Logic = logic;

}
