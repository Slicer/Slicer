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
  virtual ~qMRMLSceneCategoryModel();

};

#endif
