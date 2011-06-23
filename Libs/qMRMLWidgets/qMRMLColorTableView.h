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

#ifndef __qMRMLColorTableView_h
#define __qMRMLColorTableView_h

// Qt includes
#include <QTableView>
#include <QItemDelegate>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class QSortFilterProxyModel;
class qMRMLColorTableViewPrivate;
class qMRMLColorModel;
class vtkMRMLColorNode;
class vtkMRMLNode;

class QMRML_WIDGETS_EXPORT qMRMLColorTableView : public QTableView
{
  Q_OBJECT
  Q_PROPERTY(bool showOnlyNamedColors READ showOnlyNamedColors WRITE setShowOnlyNamedColors)
public:
  qMRMLColorTableView(QWidget *parent=0);
  virtual ~qMRMLColorTableView();

  vtkMRMLColorNode* mrmlColorNode()const;
  qMRMLColorModel* colorModel()const;
  QSortFilterProxyModel* sortFilterProxyModel()const;
  
  bool showOnlyNamedColors()const;

public slots:
  void setMRMLColorNode(vtkMRMLColorNode* colorNode);
  /// Utility function to simply connect signals/slots with Qt Designer
  void setMRMLColorNode(vtkMRMLNode* colorNode);
  
  void setShowOnlyNamedColors(bool);

protected slots:
  void onDoubleClicked(const QModelIndex& index);

protected:
  QScopedPointer<qMRMLColorTableViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLColorTableView);
  Q_DISABLE_COPY(qMRMLColorTableView);
};

class OpacityDelegate: public QItemDelegate
{
  Q_OBJECT
public:
  OpacityDelegate(QObject *parent = 0);
  
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const;

  void updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const;
 };

#endif
