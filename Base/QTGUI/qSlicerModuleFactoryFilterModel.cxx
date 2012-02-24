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
#include <QBrush>
#include <QSortFilterProxyModel>

// QtGUI includes
#include "qSlicerModuleFactoryFilterModel.h"

// --------------------------------------------------------------------------
// qSlicerModuleFactoryFilterModelPrivate

//-----------------------------------------------------------------------------
class qSlicerModuleFactoryFilterModelPrivate
{
  Q_DECLARE_PUBLIC(qSlicerModuleFactoryFilterModel);
protected:
  qSlicerModuleFactoryFilterModel* const q_ptr;

public:
  qSlicerModuleFactoryFilterModelPrivate(qSlicerModuleFactoryFilterModel& object);

  bool FilterToLoad;
  bool FilterToIgnore;
  bool FilterLoaded;
  bool FilterIgnored;
  bool FilterFailed;
};

// --------------------------------------------------------------------------
// qSlicerModulesListViewPrivate methods

// --------------------------------------------------------------------------
qSlicerModuleFactoryFilterModelPrivate::qSlicerModuleFactoryFilterModelPrivate(qSlicerModuleFactoryFilterModel& object)
  :q_ptr(&object)
{
  this->FilterToLoad = false;
  this->FilterLoaded = false;
  this->FilterToIgnore = false;
  this->FilterIgnored = false;
  this->FilterFailed = false;
}

// --------------------------------------------------------------------------
qSlicerModuleFactoryFilterModel::qSlicerModuleFactoryFilterModel(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerModuleFactoryFilterModelPrivate(*this))
{
  this->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

// --------------------------------------------------------------------------
qSlicerModuleFactoryFilterModel::~qSlicerModuleFactoryFilterModel()
{
}

// --------------------------------------------------------------------------
bool qSlicerModuleFactoryFilterModel::filterToLoad()const
{
  Q_D(const qSlicerModuleFactoryFilterModel);
  return d->FilterToLoad;
}

// --------------------------------------------------------------------------
void qSlicerModuleFactoryFilterModel::setFilterToLoad(bool filter)
{
  Q_D(qSlicerModuleFactoryFilterModel);
  d->FilterToLoad = filter;
  this->invalidate();
}
// --------------------------------------------------------------------------
bool qSlicerModuleFactoryFilterModel::filterToIgnore()const
{
  Q_D(const qSlicerModuleFactoryFilterModel);
  return d->FilterToIgnore;
}

// --------------------------------------------------------------------------
void qSlicerModuleFactoryFilterModel::setFilterToIgnore(bool filter)
{
  Q_D(qSlicerModuleFactoryFilterModel);
  d->FilterToIgnore = filter;
  this->invalidate();
}

// --------------------------------------------------------------------------
bool qSlicerModuleFactoryFilterModel::filterLoaded()const
{
  Q_D(const qSlicerModuleFactoryFilterModel);
  return d->FilterLoaded;
}

// --------------------------------------------------------------------------
void qSlicerModuleFactoryFilterModel::setFilterLoaded(bool filter)
{
  Q_D(qSlicerModuleFactoryFilterModel);
  d->FilterLoaded = filter;
  this->invalidate();
}

// --------------------------------------------------------------------------
bool qSlicerModuleFactoryFilterModel::filterIgnored()const
{
  Q_D(const qSlicerModuleFactoryFilterModel);
  return d->FilterIgnored;
}

// --------------------------------------------------------------------------
void qSlicerModuleFactoryFilterModel::setFilterIgnored(bool filter)
{
  Q_D(qSlicerModuleFactoryFilterModel);
  d->FilterIgnored = filter;
  this->invalidate();
}

// --------------------------------------------------------------------------
bool qSlicerModuleFactoryFilterModel::filterFailed()const
{
  Q_D(const qSlicerModuleFactoryFilterModel);
  return d->FilterFailed;
}

// --------------------------------------------------------------------------
void qSlicerModuleFactoryFilterModel::setFilterFailed(bool filter)
{
  Q_D(qSlicerModuleFactoryFilterModel);
  d->FilterFailed = filter;
  this->invalidate();
}

// --------------------------------------------------------------------------
bool qSlicerModuleFactoryFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent)const
{
  Q_D(const qSlicerModuleFactoryFilterModel);
  QModelIndex sourceIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);
  if (d->FilterToLoad)
    {
    if (this->sourceModel()->data(sourceIndex, Qt::CheckStateRole).toUInt() == Qt::Checked)
      {
      return false;
      }
    }
  if (d->FilterToIgnore)
    {
    if (this->sourceModel()->data(sourceIndex, Qt::CheckStateRole).toUInt() == Qt::Unchecked)
      {
      return false;
      }
    }
  if (d->FilterLoaded)
    {
    if (this->sourceModel()->data(sourceIndex, Qt::ForegroundRole).value<QBrush>() == QBrush())
      {
      return false;
      }
    }
  if (d->FilterIgnored)
    {
    if (this->sourceModel()->data(sourceIndex, Qt::ForegroundRole).value<QBrush>() != QBrush() &&
        this->sourceModel()->data(sourceIndex, Qt::ForegroundRole).value<QBrush>() != QBrush(Qt::red))
      {
      return false;
      }
    }
  if (d->FilterFailed)
    {
    if (this->sourceModel()->data(sourceIndex, Qt::ForegroundRole).value<QBrush>() == QBrush(Qt::red))
      {
      return false;
      }
    }

  return this->Superclass::filterAcceptsRow(sourceRow, sourceParent);
}
