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

#ifndef __qSlicerModuleFactoryFilterModel_h
#define __qSlicerModuleFactoryFilterModel_h

// Qt includes
#include <QSortFilterProxyModel>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"
class qSlicerModuleFactoryFilterModelPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModuleFactoryFilterModel
  : public QSortFilterProxyModel
{
  Q_OBJECT
  Q_PROPERTY(bool filterToLoad READ filterToLoad WRITE setFilterToLoad)
  Q_PROPERTY(bool filterToIgnore READ filterToIgnore WRITE setFilterToIgnore)
  Q_PROPERTY(bool filterLoaded READ filterLoaded WRITE setFilterLoaded)
  Q_PROPERTY(bool filterIgnored READ filterIgnored WRITE setFilterIgnored)
  Q_PROPERTY(bool filterFailed READ filterFailed WRITE setFilterFailed)
public:
  /// Superclass typedef
  typedef QSortFilterProxyModel Superclass;

  /// Constructor
  explicit qSlicerModuleFactoryFilterModel(QObject* parent = 0);

  /// Destructor
  virtual ~qSlicerModuleFactoryFilterModel();

  bool filterToLoad()const;
  bool filterToIgnore()const;
  bool filterLoaded()const;
  bool filterIgnored()const;
  bool filterFailed()const;

public slots:
  void setFilterToLoad(bool filter);
  void setFilterToIgnore(bool filter);
  void setFilterLoaded(bool filter);
  void setFilterIgnored(bool filter);
  void setFilterFailed(bool filter);

protected:
  QScopedPointer<qSlicerModuleFactoryFilterModelPrivate> d_ptr;

  virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent)const;

private:
  Q_DECLARE_PRIVATE(qSlicerModuleFactoryFilterModel);
  Q_DISABLE_COPY(qSlicerModuleFactoryFilterModel);
};

#endif

