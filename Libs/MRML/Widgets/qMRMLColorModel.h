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

#ifndef __qMRMLColorModel_h
#define __qMRMLColorModel_h

// Qt includes
#include <QStandardItemModel>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLColorNode;
class vtkMRMLColorLogic;
class QAction;

class qMRMLColorModelPrivate;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLColorModel : public QStandardItemModel
{
  Q_OBJECT
  QVTK_OBJECT
  Q_ENUMS(ItemDataRole)
  Q_PROPERTY(bool noneEnabled READ noneEnabled WRITE setNoneEnabled)

  /// The color column contains a Qt::DecorationRole with a pixmap of the color,
  /// the ColorRole with the color QColor, the colorName as Qt::TooltipRole
  /// 0 by default.
  /// \sa colorColumn(), setColorColumn(), labelColumn, opacityColumn,
  /// checkableColumn
  Q_PROPERTY(int colorColumn READ colorColumn WRITE setColorColumn)
  /// The label column contains the colorName as Qt::DisplayRole.
  /// 1 by default.
  /// \sa labelColumn(), setLabelColumn(), colorColumn, opacityColumn,
  /// checkableColumn
  Q_PROPERTY(int labelColumn READ labelColumn WRITE setLabelColumn)
  /// The opacity column contains the color opacity as Qt::DisplayRole
  /// 2 by default.
  /// \sa opacityColumn(), setOpacityColumn(), colorColumn, labelColumn,
  /// checkableColumn
  Q_PROPERTY(int opacityColumn READ opacityColumn WRITE setOpacityColumn)
  /// The checkable column adds a checkbox for each entry.
  /// Note that the checkboxes are unlinked to the color table node.
  /// -1 by default
  /// \sa checkableColumn(), setCheckableColumn(), colorColumn, labelColumn,
  /// opacityColumn
  Q_PROPERTY(int checkableColumn READ checkableColumn WRITE setCheckableColumn)

public:
  typedef QAbstractItemModel Superclass;
  qMRMLColorModel(QObject *parent=nullptr);
  ~qMRMLColorModel() override;

  /// ColorRole is an invivisble role that contains the true color (QColor) when
  /// Qt::DecorationRole contains a pixmap of the color.
  enum ItemDataRole{
    ColorEntryRole = Qt::UserRole,
    PointerRole,
    ColorRole
  };


  void setMRMLColorNode(vtkMRMLColorNode* node);
  vtkMRMLColorNode* mrmlColorNode()const;

  /// Set/Get NoneEnabled flags
  /// An additional item is added into the menu list, where the user can select "None".
  void setNoneEnabled(bool enable);
  bool noneEnabled()const;

  int colorColumn()const;
  void setColorColumn(int column);

  int labelColumn()const;
  void setLabelColumn(int column);

  int opacityColumn()const;
  void setOpacityColumn(int column);

  int checkableColumn()const;
  void setCheckableColumn(int column);

  /// Return the color entry associated to the index.
  /// -1 if the index is not in the model.
  /// \sa colorFromItem(), nameFromColor(), colorFromName()
  inline int colorFromIndex(const QModelIndex &index)const;
  /// Return the color entry associated to the item.
  /// -1 if the item is not in the model.
  /// \sa colorFromIndex(), nameFromColor(), colorFromName()
  int colorFromItem(QStandardItem* item)const;

  QStandardItem* itemFromColor(int color, int column = 0)const;
  QModelIndexList indexes(int color)const;

  inline QColor qcolorFromIndex(const QModelIndex& nodeIndex)const;
  inline QColor qcolorFromItem(QStandardItem* nodeItem)const;
  QColor qcolorFromColor(int color)const;

  /// Return the name of the color \a colorEntry
  /// \sa colorFromName(), colorFromIndex(), colorFromItem()
  QString nameFromColor(int colorEntry)const;
  /// Return the color entry of the first color with name \a name.
  /// \sa nameFromColor(), colorFromIndex(), colorFromItem()
  int colorFromName(const QString& name)const;

  /// Overload the header data method for the vertical header
  /// so that can return the color index rather than the row
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected slots:
  void onMRMLColorNodeModified(vtkObject* node);
  void onItemChanged(QStandardItem * item);

protected:

  qMRMLColorModel(qMRMLColorModelPrivate* pimpl, QObject *parent=nullptr);
  virtual void updateItemFromColor(QStandardItem* item, int color, int column);
  virtual void updateColorFromItem(int color, QStandardItem* item);
  virtual void updateNode();

  static void onMRMLNodeEvent(vtkObject* vtk_obj, unsigned long event,
                              void* client_data, void* call_data);
protected:
  QScopedPointer<qMRMLColorModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLColorModel);
  Q_DISABLE_COPY(qMRMLColorModel);
};

// -----------------------------------------------------------------------------
int qMRMLColorModel::colorFromIndex(const QModelIndex &nodeIndex)const
{
  return this->colorFromItem(this->itemFromIndex(nodeIndex));
}

// -----------------------------------------------------------------------------
QColor qMRMLColorModel::qcolorFromIndex(const QModelIndex &nodeIndex)const
{
  return this->qcolorFromItem(this->itemFromIndex(nodeIndex));
}

// -----------------------------------------------------------------------------
QColor qMRMLColorModel::qcolorFromItem(QStandardItem* nodeItem)const
{
  return this->qcolorFromColor(this->colorFromItem(nodeItem));
}

#endif
