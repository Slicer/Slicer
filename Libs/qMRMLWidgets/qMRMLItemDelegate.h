/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

#ifndef __qMRMLItemDelegate_h
#define __qMRMLItemDelegate_h

// Qt includes
#include <QStyledItemDelegate>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class QMRML_WIDGETS_EXPORT qMRMLItemDelegate: public QStyledItemDelegate
{
  Q_OBJECT
public:
  qMRMLItemDelegate(QObject *parent = 0);
  
  bool is0To1Value(const QModelIndex& index)const;

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const;

  virtual QSize sizeHint(const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;

  void updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const;

  virtual bool eventFilter(QObject *object, QEvent *event);

  // We make initStyleOption public so it can be used by qMRMLTreeView
  using QStyledItemDelegate::initStyleOption;

protected slots:
  void commitSenderData();
  void commitAndClose();
};

#endif
