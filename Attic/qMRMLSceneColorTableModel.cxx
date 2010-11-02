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

// Qt includes
#include <QDebug>
#include <QFile>
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
#include <vtkMRMLColorNode.h>
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
  virtual qMRMLAbstractItemHelper* createItem(vtkObject* object, int column, int row = -1)const;
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
                                const qMRMLAbstractItemHelperFactory* factory, int row);
};

// qMRMLColorTableItemHelperFactory
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLColorTableItemHelperFactory
::createItem(vtkObject* object, int column, int row)const
{
  if (!object)
    {
    Q_ASSERT(object);
    return 0;
    }
  if (object->IsA("vtkMRMLColorNode"))
    {
    return new qMRMLColorTableNodeItemHelper(vtkMRMLNode::SafeDownCast(object), column, this, row);
    }
  else
    {
    return this->qMRMLCategoryItemHelperFactory::createItem(object, column, row);
    }
  return 0;
}

// qMRMLColorTableNodeItemHelper
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
qMRMLColorTableNodeItemHelper
::qMRMLColorTableNodeItemHelper(vtkMRMLNode* node,
                                int itemColumn,
                                const qMRMLAbstractItemHelperFactory* _factory, int _row)
  :qMRMLCategoryNodeItemHelper(node, itemColumn, _factory, _row)
{
  Q_ASSERT(node);
}

//------------------------------------------------------------------------------
QVariant qMRMLColorTableNodeItemHelper::data(int role)const
{
  if (role == Qt::DecorationRole)
    {
    vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(this->mrmlNode());
    QString iconFileName(":" + QString(colorNode->GetName()));
    if (!QFile::exists(iconFileName))
      {
      iconFileName = ":blankLUT";
      }
    return QIcon(iconFileName);
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

