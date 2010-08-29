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
