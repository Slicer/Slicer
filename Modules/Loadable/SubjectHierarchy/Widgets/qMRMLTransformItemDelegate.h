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

#ifndef __qMRMLTransformItemDelegate_h
#define __qMRMLTransformItemDelegate_h

// Qt includes
#include <QStyledItemDelegate>
#include <QMenu>

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class vtkMRMLScene;
class QActionGroup;
class QCloseEvent;
class QShowEvent;

/// \brief Item Delegate for MRML parent transform property
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLTransformItemDelegate: public QStyledItemDelegate
{
  Q_OBJECT
public:
  qMRMLTransformItemDelegate(QObject *parent = nullptr);
  ~qMRMLTransformItemDelegate() override;

  void setMRMLScene(vtkMRMLScene* scene);

  /// Determine if the current index contains a transform
  bool isTransform(const QModelIndex& index)const;

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

  /// Set a fixed row height. Useful if uniform row heights is turned on, but the
  /// desired row height is different than that of the first row (often scene).
  /// Set value to -1 to disable fixed row height (this is the default)
  void setFixedRowHeight(int height);

  // We make initStyleOption public so it can be used by the tree view
  using QStyledItemDelegate::initStyleOption;

signals:
  void removeTransformsFromBranchOfCurrentItem();
  void hardenTransformOnBranchOfCurrentItem();

protected slots:
  void commitAndClose();

  void transformActionSelected();

protected:
  vtkMRMLScene* MRMLScene;
  QActionGroup* TransformActionGroup;
  QAction* NoneAction;
  QAction* HardenAction;
  int FixedRowHeight;
};

//------------------------------------------------------------------------------
/// \brief Special menu class that repositions itself to the cursor position when shown
///
/// This is needed because after creating the editor widget, the show function is called
/// by the delegate mechanism. However, the menu shows up in global position instead of
/// local and need to be moved. The cursor position is a convenient place to move it to.
class DelegateMenu : public QMenu
{
  Q_OBJECT
public:
  explicit DelegateMenu(QWidget* parent = nullptr);
  QString SelectedTransformNodeID;
protected:
  void showEvent(QShowEvent* event) override;
signals:
  void closed();
};

#endif
