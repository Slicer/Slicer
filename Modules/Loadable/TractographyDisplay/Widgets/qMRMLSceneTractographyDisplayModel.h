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

#ifndef __qMRMLSceneTractographyDisplayModel_h
#define __qMRMLSceneTractographyDisplayModel_h

#include "qSlicerTractographyDisplayModuleWidgetsExport.h"

// MRMLWidgets includes
#include "qMRMLSceneDisplayableModel.h"
class qMRMLSceneTractographyDisplayModelPrivate;

/// The Visibility icon is in the same column than the name by default.
class Q_SLICER_MODULE_TRACTOGRAPHYDISPLAY_WIDGETS_EXPORT qMRMLSceneTractographyDisplayModel : public qMRMLSceneDisplayableModel
{
  Q_OBJECT
  /// Control in which column vtkMRMLModelDisplayNode::Color are displayed
  /// (Qt::DecorationRole). Even if a vtkMRMLModelNode doesn't have a color
  /// proper, the color of its display node is used. If the model node has
  /// more than one display node and their colors are different, it uses
  /// an invalid color.
  /// A value of -1 (default) hides the column
  Q_PROPERTY (int colorColumn READ colorColumn WRITE setColorColumn)

  // This property holds the column ID where the node tube visisbility is shown.
  /// A value of -1 (default) hides the column.
  Q_PROPERTY (int lineVisibilityColumn READ lineVisibilityColumn WRITE setLineVisibilityColumn)

  // This property holds the column ID where the node tube visisbility is shown.
  /// A value of -1 (default) hides the column.
  Q_PROPERTY (int tubeVisibilityColumn READ tubeVisibilityColumn WRITE setTubeVisibilityColumn)

  // This property holds the column ID where the node tube visisbility is shown.
  /// A value of -1 (default) hides the column.
  Q_PROPERTY (int tubeIntersectionVisibilityColumn READ tubeIntersectionVisibilityColumn WRITE setTubeIntersectionVisibilityColumn)

  // This property holds the column ID where the node glyph visisbility is shown.
  /// A value of -1 (default) hides the column.
  Q_PROPERTY (int glyphVisibilityColumn READ glyphVisibilityColumn WRITE setGlyphVisibilityColumn)

  /// This property holds the column ID where the node opacity is shown.
  /// A value of -1 (default) hides the column.
  //Q_PROPERTY (int opacityColumn READ opacityColumn WRITE setOpacityColumn)

public:
  typedef qMRMLSceneDisplayableModel Superclass;
  qMRMLSceneTractographyDisplayModel(QObject *parent=0);
  virtual ~qMRMLSceneTractographyDisplayModel();

  
  int colorColumn()const;
  void setColorColumn(int column);

  int  lineVisibilityColumn()const;
  void setLineVisibilityColumn(int column);

  int tubeVisibilityColumn()const;
  void setTubeVisibilityColumn(int column);

  int tubeIntersectionVisibilityColumn()const;
  void setTubeIntersectionVisibilityColumn(int column);

  int glyphVisibilityColumn()const;
  void setGlyphVisibilityColumn(int column);

/**
  int opacityColumn()const;
  void setOpacityColumn(int column);
  ///
  virtual vtkMRMLNode* parentNode(vtkMRMLNode* node)const;
  //virtual int          nodeIndex(vtkMRMLNode* node)const;
  /// fast function that only check the type of the node to know if it can be a child.
  virtual bool         canBeAChild(vtkMRMLNode* node)const;
  /// fast function that only check the type of the node to know if it can be a parent.
  virtual bool         canBeAParent(vtkMRMLNode* node)const;
  **/

protected:
  qMRMLSceneTractographyDisplayModel(qMRMLSceneTractographyDisplayModelPrivate* pimpl,
                             QObject *parent=0);

  /// Reimplemented to listen to the displayable DisplayModifiedEvent event for
  /// visibility check state changes.
  //virtual void observeNode(vtkMRMLNode* node);
  virtual QFlags<Qt::ItemFlag> nodeFlags(vtkMRMLNode* node, int column)const;
  virtual void updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column);
  virtual void updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item);
  virtual int maxColumnId()const;

  void updateVilibilityFromNode(QStandardItem* item, vtkMRMLNode* node, bool slice=false);
  void updateVilibilityFromItem(QStandardItem* item, vtkMRMLNode* node, bool slice=false);

private:
  Q_DECLARE_PRIVATE(qMRMLSceneTractographyDisplayModel);
  Q_DISABLE_COPY(qMRMLSceneTractographyDisplayModel);
};

#endif
