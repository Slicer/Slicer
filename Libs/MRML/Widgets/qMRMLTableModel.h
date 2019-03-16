/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef __qMRMLTableModel_h
#define __qMRMLTableModel_h

// Qt includes
#include <QStandardItemModel>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLTableNode;
class QAction;

class qMRMLTableModelPrivate;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLTableModel : public QStandardItemModel
{
  Q_OBJECT
  QVTK_OBJECT
  Q_ENUMS(ItemDataRole)
  Q_PROPERTY(bool transposed READ transposed WRITE setTransposed)

public:
  typedef QAbstractItemModel Superclass;
  qMRMLTableModel(QObject *parent=nullptr);
  ~qMRMLTableModel() override;

  enum ItemDataRole{
    SortRole = Qt::UserRole + 1
  };

  void setMRMLTableNode(vtkMRMLTableNode* node);
  vtkMRMLTableNode* mrmlTableNode()const;

  /// Set/Get transposed flag
  /// If transposed is true then columns of the MRML table are added as rows in the model.
  void setTransposed(bool transposed);
  bool transposed()const;

  /// Return the VTK table cell associated to the node index.
  void updateMRMLFromModel(QStandardItem* item);

  /// Update the entire table from the MRML node
  void updateModelFromMRML();

  /// Get MRML table index from model index
  int mrmlTableRowIndex(QModelIndex modelIndex)const;

  /// Get MRML table index from model index
  int mrmlTableColumnIndex(QModelIndex modelIndex)const;

  /// Delete entire row or column from the MRML table that contains item in the selection.
  /// Returns the number of deleted rows or columns.
  /// If removeModelRow is true then entire model rows are deleted, otherwise entire
  /// model columns are deleted.
  int removeSelectionFromMRML(QModelIndexList selection, bool removeModelRow);

protected slots:
  void onMRMLTableNodeModified(vtkObject* node);
  void onItemChanged(QStandardItem * item);

protected:

  qMRMLTableModel(qMRMLTableModelPrivate* pimpl, QObject *parent=nullptr);

  static void onMRMLNodeEvent(vtkObject* vtk_obj, unsigned long event,
                              void* client_data, void* call_data);
protected:
  QScopedPointer<qMRMLTableModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLTableModel);
  Q_DISABLE_COPY(qMRMLTableModel);
};

#endif
