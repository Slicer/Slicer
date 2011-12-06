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

#ifndef __qMRMLSceneViewsModel_h
#define __qMRMLSceneViewsModel_h

#include "qMRMLSceneHierarchyModel.h"
#include "qSlicerSceneViewsModuleExport.h"
class qMRMLSceneViewsModelPrivate;

/// \ingroup Slicer_QtModules_SceneViews
class Q_SLICER_QTMODULES_SCENEVIEWS_EXPORT qMRMLSceneViewsModel
//  : public qMRMLSceneModel
  : public qMRMLSceneHierarchyModel
{
  Q_OBJECT

public:
  typedef qMRMLSceneHierarchyModel Superclass;
  qMRMLSceneViewsModel(QObject *parent=0);
  virtual ~qMRMLSceneViewsModel();

  // Enum for the different columns
  enum Columns{
    ThumbnailColumn = 0,
    RestoreColumn = 1,
    NameColumn = 2,
    DescriptionColumn = 3
  };

protected:

  virtual void updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item);
  virtual void updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column);
  virtual QFlags<Qt::ItemFlag> nodeFlags(vtkMRMLNode* node, int column)const;

private:
  Q_DISABLE_COPY(qMRMLSceneViewsModel);
};

#endif
