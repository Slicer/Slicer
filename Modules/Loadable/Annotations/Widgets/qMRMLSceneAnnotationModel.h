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

#ifndef __qMRMLSceneAnnotationModel_h
#define __qMRMLSceneAnnotationModel_h

// qMRML includes
#include "qMRMLSceneDisplayableModel.h"

// Annotations includes
#include "qSlicerAnnotationsModuleWidgetsExport.h"
class qMRMLSceneAnnotationModelPrivate;

// Logic includes
class vtkSlicerAnnotationModuleLogic;

// MRML includes
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_Annotation
/// \sa vtkMRMLAnnotationNode, vtkSlicerAnnotationLogic
class Q_SLICER_MODULE_ANNOTATIONS_WIDGETS_EXPORT qMRMLSceneAnnotationModel
  : public qMRMLSceneDisplayableModel
{
  Q_OBJECT
  /// This property holds the column index where the annotation lock property is
  /// controlled.
  /// 2 by default.
  /// \sa lockColumn(), setLockColumn(),
  /// editColumn, valueColumn, textColumn,
  /// vtkMRMLAnnotationNode::Locked
  Q_PROPERTY (int lockColumn READ lockColumn WRITE setLockColumn)
  /// This property holds the column index where the button to edit annotation
  /// node advanced property is displayed.
  /// 3 by default.
  /// \sa editColumn(), setEditColumn(),
  /// lockColumn, valueColumn, textColumn
  Q_PROPERTY (int editColumn READ editColumn WRITE setEditColumn)
  /// This property holds the column index where the annotation measurement is
  /// displayed.
  /// 4 by default.
  /// \sa valueColumn(), setValueColumn(),
  /// lockColumn, editColumn, textColumn
  Q_PROPERTY (int valueColumn READ valueColumn WRITE setValueColumn)
  /// This property holds the column index where the annotation description is
  /// displayed.
  /// 6 by default.
  /// \sa textColumn(), setTextColumn(),
  /// lockColumn, editColumn, valueColumn
  Q_PROPERTY (int textColumn READ textColumn WRITE setTextColumn)

public:
  typedef qMRMLSceneDisplayableModel Superclass;
  qMRMLSceneAnnotationModel(QObject *parent=0);
  virtual ~qMRMLSceneAnnotationModel();

  // Register the logic
  void setLogic(vtkSlicerAnnotationModuleLogic* logic);

  /// Return the lock column.
  /// \sa lockColumn, setLockColumn
  int lockColumn()const;
  /// Set the lock column. Resizes the number of columns if needed.
  /// \sa lockColumn, lockColumn()
  void setLockColumn(int column);

  /// Return the edit column.
  /// \sa editColumn, setEditColumn
  int editColumn()const;
  /// Set the edit column. Resizes the number of columns if needed.
  /// \sa editColumn, editColumn()
  void setEditColumn(int column);

  /// Return the value column.
  /// \sa valueColumn, setValueColumn
  int valueColumn()const;
  /// Set the value column. Resizes the number of columns if needed.
  /// \sa valueColumn, valueColumn()
  void setValueColumn(int column);

  /// Return the text column.
  /// \sa textColumn, setTextColumn
  int textColumn()const;
  /// Set the text column. Resizes the number of columns if needed.
  /// \sa textColumn, textColumn()
  void setTextColumn(int column);

  virtual vtkMRMLNode* parentNode(vtkMRMLNode* node)const;
  virtual bool canBeAParent(vtkMRMLNode* node)const;

protected:
  qMRMLSceneAnnotationModel(qMRMLSceneAnnotationModelPrivate* pimpl,
                             QObject *parent=0);

  virtual void updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column);

  virtual void updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item);

  virtual QFlags<Qt::ItemFlag> nodeFlags(vtkMRMLNode* node, int column)const;
  virtual int maxColumnId()const;

private:
  Q_DECLARE_PRIVATE(qMRMLSceneAnnotationModel);
  Q_DISABLE_COPY(qMRMLSceneAnnotationModel);
};

#endif
