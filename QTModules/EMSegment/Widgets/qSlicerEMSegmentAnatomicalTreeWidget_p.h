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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerEMSegmentAnatomicalTreeWidget_p_h
#define __qSlicerEMSegmentAnatomicalTreeWidget_p_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentAnatomicalTreeWidget.h"

// VTK includes
#include <vtkType.h> // For vtkIdType

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentAnatomicalTreeWidgetPrivate;
class QStandardItem;
class QStandardItemModel;
class QToolButton;
class QCheckBox;
class QTreeView;
class vtkMRMLEMSNode;
class vtkMRMLEMSTreeNode;
class vtkMRMLNode;
class vtkMRMLColorTableNode;

//-----------------------------------------------------------------------------
class qSlicerEMSegmentAnatomicalTreeWidgetPrivate :
    public QObject, public ctkPrivate<qSlicerEMSegmentAnatomicalTreeWidget>
{
  Q_OBJECT

public:

  CTK_DECLARE_PUBLIC(qSlicerEMSegmentAnatomicalTreeWidget);
  typedef qSlicerEMSegmentAnatomicalTreeWidgetPrivate Self;
  qSlicerEMSegmentAnatomicalTreeWidgetPrivate();

  enum
    {
    TreeNodeIDRole = Qt::UserRole + 1,
    TreeItemTypeRole
    };

  enum TreeItemType
    {
    StructureNameItemType = 0,
    LabelItemType,
    MRMLIDItemType,
    ClassWeightItemType,
    UpdateClassWeightItemType,
    AtlasWeightItemType,
    AlphaItemType
    };

  enum ColumnIds
    {
    StructureColumn = 0,
    IdColumn,
    LabelColumn,
    ClassWeightColumn,
    UpdateClassWeightColumn,
    AtlasWeightColumn,
    AlphaColumn
    };

  void initializeHorizontalHeader();

  void populateTreeModel(vtkIdType treeNodeId, QStandardItem *item);

  QStandardItem* insertTreeRow(QStandardItem * parentItem, vtkIdType treeNodeId,
                               vtkMRMLEMSTreeNode * treeNode);

  QColor colorFromLabelId(int labelId);

public slots:

  void onTreeItemChanged(QStandardItem * treeItem);

  void onTreeItemSelected(const QModelIndex & index);

public:

  vtkMRMLEMSNode *         EMSNode;

  vtkMRMLColorTableNode *  CurrentColorTableNode;

  QTreeView *              TreeView;
  QStandardItemModel *     TreeModel;
  bool                     StructureNameEditable;
  bool                     LabelColumnVisible;
  bool                     ClassWeightColumnVisible;
  bool                     UpdateClassWeightColumnVisible;
  bool                     AtlasWeightColumnVisible;
  bool                     AlphaColumnVisible;

  QCheckBox *              DisplayMRMLIDsCheckBox;
  QToolButton *            CollapseAllButton;
  QToolButton *            ExpandAllButton;
};

#endif
