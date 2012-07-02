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

// TractographyDisplays includes
#include "qMRMLTractographyDisplayTreeView.h"
#include "qMRMLSceneTractographyDisplayModel.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"

// MRML includes


//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TractographyDisplay
class qMRMLTractographyDisplayTreeViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLTractographyDisplayTreeView);
protected:
  qMRMLTractographyDisplayTreeView* const q_ptr;
public:
  qMRMLTractographyDisplayTreeViewPrivate(qMRMLTractographyDisplayTreeView& object);
  void init();

  qMRMLSceneTractographyDisplayModel*           SceneModel;
  qMRMLSortFilterProxyModel* SortFilterModel;
};

//------------------------------------------------------------------------------
qMRMLTractographyDisplayTreeViewPrivate::qMRMLTractographyDisplayTreeViewPrivate(qMRMLTractographyDisplayTreeView& object)
  : q_ptr(&object)
{
  this->SceneModel = 0;
  this->SortFilterModel = 0;
}

//------------------------------------------------------------------------------
void qMRMLTractographyDisplayTreeViewPrivate::init()
{
  Q_Q(qMRMLTractographyDisplayTreeView);
  //p->qMRMLTreeView::setModel(new qMRMLItemModel(p));
  //this->SceneModel = new qMRMLSceneTractographyDisplayModel(q);
  //this->SceneModel->setColumnCount(6);

  this->SceneModel = new qMRMLSceneTractographyDisplayModel(q);
  q->setSceneModel(this->SceneModel, "TractographyDisplay");

  //this->SortFilterModel = new qMRMLSortFilterProxyModel(q);
  // we only want to show vtkMRMLTractographyDisplayNodes and vtkMRMLTractographyDisplayHierarchyNodes
  QStringList nodeTypes = QStringList();
  nodeTypes.append("vtkMRMLFiberBundleNode");
  //nodeTypes.append("vtkMRMLTractographyDisplayHierarchyNode");

  //this->SortFilterModel->setNodeTypes(nodeTypes);
  q->setNodeTypes(nodeTypes);
  this->SortFilterModel = q->sortFilterProxyModel();

  q->header()->setStretchLastSection(false);
  q->header()->setResizeMode(QHeaderView::ResizeToContents);
  q->header()->setResizeMode(0, QHeaderView::Stretch);

  //this->SortFilterModel->setSourceModel(this->SceneModel);
  //q->qMRMLTreeView::setModel(this->SortFilterModel);

  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);

  //QObject::connect(q, SIGNAL(clicked(QModelIndex)),
  //                 q, SLOT(onClicked(QModelIndex)));

  /***
  QObject::connect( q->selectionModel(),
        SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
        q,
        SLOT(onSelectionChanged(QItemSelection,QItemSelection)),
        Qt::DirectConnection );
  ***/

  q->setUniformRowHeights(true);
}

//------------------------------------------------------------------------------
qMRMLTractographyDisplayTreeView::qMRMLTractographyDisplayTreeView(QWidget *_parent)
  :qMRMLTreeView(_parent)
  , d_ptr(new qMRMLTractographyDisplayTreeViewPrivate(*this))
{
  Q_D(qMRMLTractographyDisplayTreeView);
  d->init();

  // we need to enable mouse tracking to set the appropriate cursor while mouseMove occurs
  this->setMouseTracking(true);
}

//------------------------------------------------------------------------------
qMRMLTractographyDisplayTreeView::~qMRMLTractographyDisplayTreeView()
{
}

#ifndef QT_NO_CURSOR
//------------------------------------------------------------------------------
bool qMRMLTractographyDisplayTreeView::viewportEvent(QEvent* e)
{

  // reset the cursor if we leave the viewport
  if(e->type() == QEvent::Leave)
    {
    setCursor(QCursor());
    }

  return QTreeView::viewportEvent(e);
}


#endif


//------------------------------------------------------------------------------
void qMRMLTractographyDisplayTreeView::onVisibilityColumnClicked(vtkMRMLNode* node)
{

  if (!node)
    {
    // no node found!
    return;
    }
}

//-----------------------------------------------------------------------------
/// Set and observe the logic
//-----------------------------------------------------------------------------
void qMRMLTractographyDisplayTreeView::setLogic(vtkSlicerFiberBundleLogic* logic)
{
  if (!logic)
    {
    return;
    }

  //Q_D(qMRMLTractographyDisplayTreeView);

  this->m_Logic = logic;

  // propagate down to model
  //d->SceneModel->setLogic(this->m_Logic);

}

//------------------------------------------------------------------------------
void qMRMLTractographyDisplayTreeView::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  this->setRootIndex(this->sortFilterProxyModel()->mrmlSceneIndex());
}

//------------------------------------------------------------------------------
bool qMRMLTractographyDisplayTreeView::clickDecoration(const QModelIndex& index)
{
  bool res = false;
  int type = -1;
  QModelIndex sourceIndex = this->sortFilterProxyModel()->mapToSource(index);

  vtkMRMLFiberBundleNode* fbNode = vtkMRMLFiberBundleNode::SafeDownCast(
          this->sortFilterProxyModel()->mrmlNodeFromIndex(index));

  vtkMRMLFiberBundleDisplayNode* lineDisplayNode = fbNode->GetLineDisplayNode();
  vtkMRMLFiberBundleDisplayNode* tubeDisplayNode = fbNode->GetTubeDisplayNode();
  vtkMRMLFiberBundleDisplayNode* glyphDisplayNode = fbNode->GetGlyphDisplayNode();

  qMRMLSceneTractographyDisplayModel* model = dynamic_cast< qMRMLSceneTractographyDisplayModel* >(this->sceneModel());
  
  if (!(sourceIndex.flags() & Qt::ItemIsEnabled))
    {
    res = false;
    }
  else if (sourceIndex.column() == model->lineVisibilityColumn())
    {
    type = 0;
    if (lineDisplayNode)
      {
      lineDisplayNode->SetVisibility(lineDisplayNode->GetVisibility() ? 0 : 1);
      res = true;
      }
  }
  else if (sourceIndex.column() == model->tubeVisibilityColumn())
    {
    type = 1;
    if (tubeDisplayNode)
      {
      tubeDisplayNode->SetVisibility(tubeDisplayNode->GetVisibility() ? 0 : 1);
      res = true;
      }
  }
  else if (sourceIndex.column() == model->tubeIntersectionVisibilityColumn())
    {
    if (tubeDisplayNode)
      {
      tubeDisplayNode->SetSliceIntersectionVisibility(tubeDisplayNode->GetSliceIntersectionVisibility() ? 0 : 1);
      res = true;
      }
  }
  else if (sourceIndex.column() == model->glyphVisibilityColumn())
    {
    type = 2;
    if (glyphDisplayNode)
      {
      glyphDisplayNode->SetVisibility(glyphDisplayNode->GetVisibility() ? 0 : 1);
      res = true;
      }
  }

  if (glyphDisplayNode->GetVisibility() == 0 &&
      tubeDisplayNode->GetVisibility() == 0  &&
      lineDisplayNode->GetVisibility() == 1)
    {
    type = 0;
    }
  if (glyphDisplayNode->GetVisibility() == 0 &&
      tubeDisplayNode->GetVisibility() == 1  &&
      lineDisplayNode->GetVisibility() == 0)
    {
    type = 1;
    }
  if (glyphDisplayNode->GetVisibility() == 1 &&
      tubeDisplayNode->GetVisibility() == 0  &&
      lineDisplayNode->GetVisibility() == 0)
    {
    type = 2;
    }

  if (res)
    {
    emit decorationClicked(index);
    }
  if (type > -1)
    {
    emit visibilityChanged(type);
    }
  return res;
}
