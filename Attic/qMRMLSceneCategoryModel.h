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

#ifndef __qMRMLSceneCategoryModel_h
#define __qMRMLSceneCategoryModel_h

#include "qMRMLSceneTreeModel.h"

class vtkCategory;
class vtkTimeStamp;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLCategoryItemHelperFactory : public qMRMLSceneModelItemHelperFactory
{
public:
  qMRMLCategoryItemHelperFactory();
  virtual ~qMRMLCategoryItemHelperFactory();
  virtual qMRMLAbstractItemHelper* createItem(vtkObject* object, int column, int row)const;

  void updateCategories(vtkMRMLScene* scene);
  vtkCategory* category(const QString& categoryName)const;
  int categoryIndex(const QString& categoryName)const;

  vtkCollection* Categories;
protected:
  vtkTimeStamp* UpdateTime;
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLCategoryNodeItemHelper
  :public qMRMLAbstractNodeItemHelper
{
public:
  virtual qMRMLAbstractItemHelper* parent() const;
protected:
  friend class qMRMLCategoryItemHelperFactory;
  qMRMLCategoryNodeItemHelper(vtkMRMLNode* node, int column,
                              const qMRMLAbstractItemHelperFactory* factory, int row);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLSceneCategoryModel : public qMRMLSceneTreeModel
{
  Q_OBJECT

public:
  qMRMLSceneCategoryModel(QObject *parent=0);
};

#endif
