// Qt includes
#include <QDebug>
#include <QIcon>
#include <QMap>
#include <QMimeData>
#include <QSharedPointer>
#include <QStack>
#include <QStringList>
#include <QVariant>
#include <QVector>

// qMRML includes
#include "qMRMLItemHelper.h"
#include "qMRMLSceneColorTableModel.h"
#include "qMRMLSceneCategoryModel.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkVariantArray.h>

class qMRMLCategoryItemHelperPrivate;
class vtkCategory;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLColorTableItemHelperFactory : public qMRMLCategoryItemHelperFactory
{
public:
  virtual qMRMLAbstractItemHelper* createItem(vtkObject* object, int column)const;
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLColorTableNodeItemHelper
  :public qMRMLCategoryNodeItemHelper
{
public:
  virtual QVariant data(int role)const;
protected:
  friend class qMRMLColorTableItemHelperFactory;
  qMRMLColorTableNodeItemHelper(vtkMRMLNode* node, int column,
                                const qMRMLAbstractItemHelperFactory* factory);
};

// qMRMLColorTableItemHelperFactory
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLColorTableItemHelperFactory
::createItem(vtkObject* object, int column)const
{
  if (!object)
    {
    Q_ASSERT(object);
    return 0;
    }
  if (object->IsA("vtkMRMLColorTableNode"))
    {
    return new qMRMLColorTableNodeItemHelper(vtkMRMLNode::SafeDownCast(object), column, this);
    }
  else
    {
    return this->qMRMLCategoryItemHelperFactory::createItem(object, column);
    }
  return 0;
}

// qMRMLColorTableNodeItemHelper
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
qMRMLColorTableNodeItemHelper
::qMRMLColorTableNodeItemHelper(vtkMRMLNode* node,
                                int itemColumn,
                                const qMRMLAbstractItemHelperFactory* _factory)
  :qMRMLCategoryNodeItemHelper(node, itemColumn, _factory)
{
  Q_ASSERT(node);
}

//------------------------------------------------------------------------------
QVariant qMRMLColorTableNodeItemHelper::data(int role)const
{
  if (role == Qt::DecorationRole)
    {
    return QIcon(":Icons/VisibleOn");
    }
  return qMRMLAbstractNodeItemHelper::data(role);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneColorTableModel::qMRMLSceneColorTableModel(QObject *vparent)
  :qMRMLSceneTreeModel(new qMRMLColorTableItemHelperFactory, vparent)
{
}

//------------------------------------------------------------------------------
qMRMLSceneColorTableModel::~qMRMLSceneColorTableModel()
{
}

