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

  This file was originally developed by Csaba Pinter, EBATINCA, S.L.
  and was funded by by Murat Maga (Seattle Children's Research Institute).

==============================================================================*/

#ifndef __qMRMLSortFilterColorProxyModel_h
#define __qMRMLSortFilterColorProxyModel_h

// MRML includes
#include "qMRMLWidgetsExport.h"

// Qt includes
#include <QSortFilterProxyModel>

// CTK includes
#include <ctkVTKObject.h>
#include <ctkPimpl.h>

class qMRMLSortFilterColorProxyModelPrivate;
class vtkMRMLColorNode;
class vtkMRMLScene;
class QStandardItem;

class QMRML_WIDGETS_EXPORT qMRMLSortFilterColorProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
  QVTK_OBJECT

  /// Filter to show or hide colors that are empty (indicated by Defined flag in color properties)
  /// True by default.
  Q_PROPERTY(bool showEmptyColors READ showEmptyColors WRITE setShowEmptyColors)

public:
  typedef QSortFilterProxyModel Superclass;
  qMRMLSortFilterColorProxyModel(QObject* parent = nullptr);
  ~qMRMLSortFilterColorProxyModel() override;

  Q_INVOKABLE vtkMRMLColorNode* mrmlColorNode() const;

  bool showEmptyColors() const;

  /// Returns true if the item in the row indicated by the given sourceRow and
  /// sourceParent should be included in the model; otherwise returns false.
  /// This method test each item via \a filterAcceptsItem
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

public slots:
  void setShowEmptyColors(bool show);

protected:
  /// Filters items to decide which to display in the view
  virtual bool filterAcceptsItem(int colorIndex) const;

  QStandardItem* sourceItem(const QModelIndex& index) const;

protected:
  QScopedPointer<qMRMLSortFilterColorProxyModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSortFilterColorProxyModel);
  Q_DISABLE_COPY(qMRMLSortFilterColorProxyModel);
};

#endif
