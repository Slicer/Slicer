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

#ifndef __qMRMLSceneTransformModel_h
#define __qMRMLSceneTransformModel_h

#include "qMRMLSceneModel.h"

class qMRMLSceneTransformModelPrivate;

class QMRML_WIDGETS_EXPORT qMRMLSceneTransformModel : public qMRMLSceneModel
{
  Q_OBJECT

public:
  qMRMLSceneTransformModel(QObject *parent=0);
  virtual ~qMRMLSceneTransformModel();

  ///
  static vtkMRMLNode* parentNode(vtkMRMLNode* node);
  static int          nodeIndex(vtkMRMLNode* node);
  /// fast function that only check the type of the node to know if it can be a child.
  static bool         canBeAChild(vtkMRMLNode* node);
  /// fast function that only check the type of the node to know if it can be a parent.
  static bool         canBeAParent(vtkMRMLNode* node);
  /// if newParent == 0, set the node into the vtkMRMLScene
  static bool         reparent(vtkMRMLNode* node, vtkMRMLNode* newParent);

  virtual Qt::DropActions supportedDropActions()const;

protected:
  virtual void populateScene();
  virtual void insertNode(vtkMRMLNode* node);
  using qMRMLSceneModel::insertNode;
  virtual void updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column);
  virtual void updateNodeFromItem(vtkMRMLNode* node, QStandardItem* item);

private:
  Q_DECLARE_PRIVATE(qMRMLSceneTransformModel);
  Q_DISABLE_COPY(qMRMLSceneTransformModel);
};

#endif
