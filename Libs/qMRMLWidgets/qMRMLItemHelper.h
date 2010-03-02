#ifndef __qMRMLItemHelper_h
#define __qMRMLItemHelper_h

/// qCTK includes
#include <qCTKPimpl.h>

/// QT includes
#include <Qt>
#include <QVariant>

#include "qMRMLWidgetsExport.h"

class vtkObject;
class vtkMRMLScene;
class vtkMRMLNode;
class qMRMLAbstractItemHelperPrivate;
class qMRMLAbstractRootItemHelperPrivate;
class qMRMLAbstractSceneItemHelperPrivate;
class qMRMLAbstractNodeItemHelperPrivate;
class vtkVariantArray;
class qMRMLExtraItemHelperPrivate;

//class qMRMLSceneItemHelperPrivate;
//class qMRMLNodeItemHelperPrivate;
//class qMRMLNodeCategoryItemHelperPrivate;
//class qMRMLRootItemHelperPrivate;

class qMRMLAbstractItemHelper;
class vtkVariantArray;
class vtkStdString;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractItemHelperFactory
{
public:
  virtual qMRMLAbstractItemHelper* createItem(vtkObject* object, int column)const = 0;
  virtual qMRMLAbstractItemHelper* createRootItem(vtkMRMLScene* scene)const = 0;
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractItemHelper
{
public:
  virtual bool canReparent(qMRMLAbstractItemHelper* newParent)const;
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
  virtual int childCount() const;
  virtual int column() const;
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const = 0;
  virtual bool hasChildren() const;
  virtual qMRMLAbstractItemHelper* parent() const;
  virtual bool reparent(qMRMLAbstractItemHelper* newParent);
  virtual int row() const;
  virtual bool setData(const QVariant &value, int role = Qt::EditRole);
  virtual vtkObject* object()const = 0;
  virtual bool operator==(const qMRMLAbstractItemHelper& helper)const;
protected:
  qMRMLAbstractItemHelper(int column, const qMRMLAbstractItemHelperFactory* factory);
  const qMRMLAbstractItemHelperFactory* factory()const;
  /// here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
private:
  QCTK_DECLARE_PRIVATE(qMRMLAbstractItemHelper);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractSceneItemHelper : public qMRMLAbstractItemHelper
{
public:
  
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual vtkObject* object()const;
  virtual vtkMRMLScene* mrmlScene()const;

protected: 
  qMRMLAbstractSceneItemHelper(vtkMRMLScene* scene, int column, const qMRMLAbstractItemHelperFactory* factory);
  //inline void setActions(const QList<QAction*>& actions);
  //const QList<QAction*>& Actions;
private:
  QCTK_DECLARE_PRIVATE(qMRMLAbstractSceneItemHelper);
};
/*
void qMRMLAbstractSceneItemHelper::setActions(const QList<QAction*>& actions)
{
  this->Actions = actions;
}
*/

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractNodeItemHelper : public qMRMLAbstractItemHelper
{
public:
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual vtkObject* object() const;
  virtual vtkMRMLNode* mrmlNode()const;
  virtual qMRMLAbstractItemHelper* parent() const = 0;
  virtual bool setData(const QVariant &value, int role = Qt::EditRole);
protected:
  qMRMLAbstractNodeItemHelper(vtkMRMLNode* node, int column, const qMRMLAbstractItemHelperFactory* factory);

private:
  QCTK_DECLARE_PRIVATE(qMRMLAbstractNodeItemHelper);
};



/*
//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLNodeCategoryItemHelper : public qMRMLAbstractItemHelper
{
public:
  qMRMLNodeCategoryItemHelper(const QString& category);
  virtual ~qMRMLNodeCategoryItemHelper();
  
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
  virtual int childCount() const;
  virtual int column() const;
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual bool hasChildren() const;
  virtual qMRMLAbstractItemHelper* parent() const;
  virtual int row() const;
  virtual bool setData(const QVariant &value, int role = Qt::EditRole);
protected:
  /// here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;

private:
  QCTK_DECLARE_PRIVATE(qMRMLNodeCategoryItemHelper);
};
*/


//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractRootItemHelper : public qMRMLAbstractItemHelper
{
public:
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
  virtual int childCount() const;
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual bool hasChildren() const;
  virtual vtkObject* object()const;
  virtual qMRMLAbstractItemHelper* parent() const;
  virtual int row() const;

protected:
  qMRMLAbstractRootItemHelper(vtkMRMLScene* scene, const qMRMLAbstractItemHelperFactory* factory);
  /// here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
  vtkMRMLScene* mrmlScene()const;
private:
  QCTK_DECLARE_PRIVATE(qMRMLAbstractRootItemHelper);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLExtraItemHelper : public qMRMLAbstractItemHelper
{
public:
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual qMRMLAbstractItemHelper* parent() const;
  virtual bool setData(const QVariant &value, int role = Qt::EditRole);
  virtual vtkObject* object()const;
  static void createExtraItemProperties(vtkVariantArray& properties, 
                              vtkObject* parent, 
                              const vtkStdString& title);
protected:
  friend class qMRMLSceneModelItemHelperFactory;
  qMRMLExtraItemHelper(vtkVariantArray* array, int column, const qMRMLAbstractItemHelperFactory* factory);
  vtkMRMLScene* mrmlScene()const;

private:
  QCTK_DECLARE_PRIVATE(qMRMLExtraItemHelper);
};


#endif
