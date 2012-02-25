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

  bool ShowToLoad;
  bool ShowToIgnore;
  bool ShowLoaded;
  bool ShowIgnored;
  bool ShowFailed;
};

// --------------------------------------------------------------------------
// qSlicerModulesListViewPrivate methods

// --------------------------------------------------------------------------
qSlicerModuleFactoryFilterModelPrivate::qSlicerModuleFactoryFilterModelPrivate(qSlicerModuleFactoryFilterModel& object)
  :q_ptr(&object)
{
  this->ShowToLoad = true;
  this->ShowLoaded = true;
  this->ShowToIgnore = true;
  this->ShowIgnored = true;
  this->ShowFailed = true;
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
bool qSlicerModuleFactoryFilterModel::showToLoad()const
{
  Q_D(const qSlicerModuleFactoryFilterModel);
  return d->ShowToLoad;
}

// --------------------------------------------------------------------------
void qSlicerModuleFactoryFilterModel::setShowToLoad(bool show)
{
  Q_D(qSlicerModuleFactoryFilterModel);
  d->ShowToLoad = show;
  this->invalidate();
}

// --------------------------------------------------------------------------
bool qSlicerModuleFactoryFilterModel::showToIgnore()const
{
  Q_D(const qSlicerModuleFactoryFilterModel);
  return d->ShowToIgnore;
}

// --------------------------------------------------------------------------
void qSlicerModuleFactoryFilterModel::setShowToIgnore(bool show)
{
  Q_D(qSlicerModuleFactoryFilterModel);
  d->ShowToIgnore = show;
  this->invalidate();
}

// --------------------------------------------------------------------------
bool qSlicerModuleFactoryFilterModel::showLoaded()const
{
  Q_D(const qSlicerModuleFactoryFilterModel);
  return d->ShowLoaded;
}

// --------------------------------------------------------------------------
void qSlicerModuleFactoryFilterModel::setShowLoaded(bool show)
{
  Q_D(qSlicerModuleFactoryFilterModel);
  d->ShowLoaded = show;
  this->invalidate();
}

// --------------------------------------------------------------------------
bool qSlicerModuleFactoryFilterModel::showIgnored()const
{
  Q_D(const qSlicerModuleFactoryFilterModel);
  return d->ShowIgnored;
}

// --------------------------------------------------------------------------
void qSlicerModuleFactoryFilterModel::setShowIgnored(bool show)
{
  Q_D(qSlicerModuleFactoryFilterModel);
  d->ShowIgnored = show;
  this->invalidate();
}

// --------------------------------------------------------------------------
bool qSlicerModuleFactoryFilterModel::showFailed()const
{
  Q_D(const qSlicerModuleFactoryFilterModel);
  return d->ShowFailed;
}

// --------------------------------------------------------------------------
void qSlicerModuleFactoryFilterModel::setShowFailed(bool show)
{
  Q_D(qSlicerModuleFactoryFilterModel);
  d->ShowFailed = show;
  this->invalidate();
}

// --------------------------------------------------------------------------
bool qSlicerModuleFactoryFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent)const
{
  Q_D(const qSlicerModuleFactoryFilterModel);
  QModelIndex sourceIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);
  if (!d->ShowToLoad)
    {
    if (this->sourceModel()->data(sourceIndex, Qt::CheckStateRole).toUInt() == Qt::Checked)
      {
      return false;
      }
    }
  if (!d->ShowToIgnore)
    {
    if (this->sourceModel()->data(sourceIndex, Qt::CheckStateRole).toUInt() == Qt::Unchecked)
      {
      return false;
      }
    }
  if (!d->ShowLoaded)
    {
    if (this->sourceModel()->data(sourceIndex, Qt::ForegroundRole).value<QBrush>() == QBrush())
      {
      return false;
      }
    }
  if (!d->ShowIgnored)
    {
    if (this->sourceModel()->data(sourceIndex, Qt::ForegroundRole).value<QBrush>() != QBrush() &&
        this->sourceModel()->data(sourceIndex, Qt::ForegroundRole).value<QBrush>() != QBrush(Qt::red))
      {
      return false;
      }
    }
  if (!d->ShowFailed)
    {
    if (this->sourceModel()->data(sourceIndex, Qt::ForegroundRole).value<QBrush>() == QBrush(Qt::red))
      {
      return false;
      }
    }

  return this->Superclass::filterAcceptsRow(sourceRow, sourceParent);
}
