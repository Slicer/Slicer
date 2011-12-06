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

#ifndef __qMRMLSceneCategoryModel_h
#define __qMRMLSceneCategoryModel_h

#include "qMRMLSceneModel.h"

class qMRMLSceneCategoryModelPrivate;

class QMRML_WIDGETS_EXPORT qMRMLSceneCategoryModel : public qMRMLSceneModel
{
  Q_OBJECT

public:
  qMRMLSceneCategoryModel(QObject *parent=0);
  virtual ~qMRMLSceneCategoryModel();

  QStandardItem* itemFromCategory(const QString& category)const;
  int categoryCount()const;

protected:
  virtual QStandardItem* insertNode(vtkMRMLNode* node);
  using qMRMLSceneModel::insertNode;
  virtual bool isANode(const QStandardItem * item)const;
  virtual void updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column);
  virtual void updateNodeFromItem(vtkMRMLNode* node, QStandardItem* item);

  virtual void updateItemFromCategory(QStandardItem* item, const QString& category);
  virtual QStandardItem* insertCategory(const QString& category, int row);

private:
  Q_DECLARE_PRIVATE(qMRMLSceneCategoryModel);
  Q_DISABLE_COPY(qMRMLSceneCategoryModel);
};

#endif
