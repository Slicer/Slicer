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

#ifndef __qMRMLTableItemDelegate_h
#define __qMRMLTableItemDelegate_h

// Qt includes
#include <QStyledItemDelegate>

// CTK includes
class ctkDoubleSpinBox;

// qMRML includes
#include "qMRMLWidgetsExport.h"

/// \brief Item Delegate for MRML table value types
class QMRML_WIDGETS_EXPORT qMRMLTableItemDelegate: public QStyledItemDelegate
{
  Q_OBJECT
public:
  qMRMLTableItemDelegate(QObject *parent = 0);
  virtual ~qMRMLTableItemDelegate();
  typedef QStyledItemDelegate Superclass;

  virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected slots:
  void commitSenderData();
  void commitAndClose();
};

#endif
