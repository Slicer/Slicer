/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
class QAction;

class qMRMLColorModelPrivate;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLColorModel : public QStandardItemModel
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(bool noneEnabled READ noneEnabled WRITE setNoneEnabled)

public:
  typedef QAbstractItemModel Superclass;
  qMRMLColorModel(QObject *parent=0);
  virtual ~qMRMLColorModel();
  
  enum ItemDataRole{
    ColorEntryRole = Qt::UserRole,
    PointerRole,
    ColorRole
  };
 
  enum Columns{
    ColorColumn = 0,
    LabelColumn = 1,
    OpacityColumn = 2
  };

  void setMRMLColorNode(vtkMRMLColorNode* node);
  vtkMRMLColorNode* mrmlColorNode()const;

  /// Set/Get NoneEnabled flags
  /// An additional item is added into the menu list, where the user can select "None".
  void setNoneEnabled(bool enable);
  bool noneEnabled()const;

  /// Control wether or not displaying the label in the color column
  void setLabelInColorColumn(bool enable);
  bool isLabelInColorColumn()const;

  /// Return the vtkMRMLNode associated to the node index.
  /// -1 if the node index is not a MRML node (i.e. vtkMRMLScene, extra item...)
  inline int colorFromIndex(const QModelIndex &nodeIndex)const;
  int colorFromItem(QStandardItem* nodeItem)const;
  
  QStandardItem* itemFromColor(int color, int column = 0)const;
  QModelIndexList indexes(int color)const;
  
  inline QColor qcolorFromIndex(const QModelIndex& nodeIndex)const;
  inline QColor qcolorFromItem(QStandardItem* nodeItem)const;
  QColor qcolorFromColor(int color)const;
protected slots:
  void onMRMLColorNodeModified(vtkObject* node);
  void onItemChanged(QStandardItem * item);

protected:

  qMRMLColorModel(qMRMLColorModelPrivate* pimpl, QObject *parent=0);
  //virtual void insertColor(int color);
  virtual void updateItemFromColor(QStandardItem* item, int color, int column);
  virtual void updateColorFromItem(int color, QStandardItem* item);
  virtual void updateNode();
  //virtual void populateNode();

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
