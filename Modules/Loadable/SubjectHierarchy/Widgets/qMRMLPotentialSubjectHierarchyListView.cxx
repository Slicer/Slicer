/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// QT includes
#include <QSortFilterProxyModel>


// qMRML includes
#include "qMRMLPotentialSubjectHierarchyListView.h"
#include "qMRMLScenePotentialSubjectHierarchyModel.h"
#include "qMRMLSortFilterPotentialSubjectHierarchyProxyModel.h"

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qMRMLPotentialSubjectHierarchyListViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLPotentialSubjectHierarchyListView);
protected:
  qMRMLPotentialSubjectHierarchyListView* const q_ptr;
public:
  qMRMLPotentialSubjectHierarchyListViewPrivate(qMRMLPotentialSubjectHierarchyListView& object);
  void init();

  qMRMLSortFilterPotentialSubjectHierarchyProxyModel* SortFilterModel;
};

//------------------------------------------------------------------------------
qMRMLPotentialSubjectHierarchyListViewPrivate::qMRMLPotentialSubjectHierarchyListViewPrivate(qMRMLPotentialSubjectHierarchyListView& object)
  : q_ptr(&object)
{
  this->SortFilterModel = NULL;
}

//------------------------------------------------------------------------------
void qMRMLPotentialSubjectHierarchyListViewPrivate::init()
{
  Q_Q(qMRMLPotentialSubjectHierarchyListView);

  qMRMLScenePotentialSubjectHierarchyModel* sceneModel = new qMRMLScenePotentialSubjectHierarchyModel(q);

  this->SortFilterModel = new qMRMLSortFilterPotentialSubjectHierarchyProxyModel(q);
  this->SortFilterModel->setSourceModel(sceneModel);

  q->setDragDropMode(QAbstractItemView::DragOnly);

  q->QListView::setModel(this->SortFilterModel);
}

//------------------------------------------------------------------------------
qMRMLPotentialSubjectHierarchyListView::qMRMLPotentialSubjectHierarchyListView(QWidget *_parent)
  : QListView(_parent)
  , d_ptr(new qMRMLPotentialSubjectHierarchyListViewPrivate(*this))
{
  Q_D(qMRMLPotentialSubjectHierarchyListView);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLPotentialSubjectHierarchyListView::~qMRMLPotentialSubjectHierarchyListView()
{
}

//------------------------------------------------------------------------------
void qMRMLPotentialSubjectHierarchyListView::setMRMLScene(vtkMRMLScene* scene)
{
  QSortFilterProxyModel* sortModel = qobject_cast<QSortFilterProxyModel*>(this->model());
  qMRMLSceneModel* mrmlModel = qobject_cast<qMRMLSceneModel*>(sortModel->sourceModel());
  Q_ASSERT(mrmlModel);

  mrmlModel->setMRMLScene(scene);
  if (scene)
    {
    this->setRootIndex(sortModel->index(0, 0));
    sortModel->invalidate();
    }
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLPotentialSubjectHierarchyListView::mrmlScene()const
{
  QSortFilterProxyModel* sortModel = qobject_cast<QSortFilterProxyModel*>(this->model());
  Q_ASSERT(qobject_cast<const qMRMLSceneModel*>(sortModel->sourceModel()));
  return qobject_cast<const qMRMLSceneModel*>(sortModel->sourceModel())->mrmlScene();
}

//--------------------------------------------------------------------------
qMRMLSortFilterProxyModel* qMRMLPotentialSubjectHierarchyListView::sortFilterProxyModel()const
{
  Q_D(const qMRMLPotentialSubjectHierarchyListView);
  Q_ASSERT(d->SortFilterModel);
  return d->SortFilterModel;
}
