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

#ifndef __qMRMLItemDelegate_h
#define __qMRMLItemDelegate_h

// Qt includes
#include <QStyledItemDelegate>

// CTK includes
class ctkDoubleSpinBox;

// qMRML includes
#include "qMRMLWidgetsExport.h"

/// \brief Item Delegate for MRML properties
/// Use custom widgets for properties such as colors, opacities...
/// If an index has Qt::DecorationRole set to a QColor, its editor will then be
/// a ctkColorPickerButton linked to a color picker dialog on click
/// If an index has a Qt::EditRole set to a QString that exactly is on the form
/// X.YY where X is 0 or 1 and Y is a digit (0 to 9), then the editor will be
/// a ctkDoubleSpinBox with a slider popping up on mouse hover.
class QMRML_WIDGETS_EXPORT qMRMLItemDelegate: public QStyledItemDelegate
{
  Q_OBJECT
public:
  qMRMLItemDelegate(QObject *parent = nullptr);
  ~qMRMLItemDelegate() override;
  typedef QStyledItemDelegate Superclass;

  bool isColor(const QModelIndex& index)const;
  int colorRole(const QModelIndex& index)const;
  bool is0To1Value(const QModelIndex& index)const;

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

  void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const override;

  QSize sizeHint(const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;

  void updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  bool eventFilter(QObject *object, QEvent *event) override;

  // We make initStyleOption public so it can be used by qMRMLTreeView
  using QStyledItemDelegate::initStyleOption;

protected slots:
  void commitSenderData();
  void commitAndClose();

protected:
  ctkDoubleSpinBox* DummySpinBox;
};

#endif
