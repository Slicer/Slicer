#ifndef __qMRMLItemHelper_h
#define __qMRMLItemHelper_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
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

class qMRMLSceneItemHelperPrivate;
class qMRMLNodeItemHelperPrivate;
class qMRMLNodeCategoryItemHelperPrivate;
class qMRMLRootItemHelperPrivate;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractItemHelper
{
public:
  qMRMLAbstractItemHelper(int column = -1);
  virtual bool canReparent(qMRMLAbstractItemHelper* newParent)const;
  virtual qMRMLAbstractItemHelper* child(int row = 0, int column = 0) const;
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
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
private:
  QCTK_DECLARE_PRIVATE(qMRMLAbstractItemHelper);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractSceneItemHelper : public qMRMLAbstractItemHelper
{
public:
  qMRMLAbstractSceneItemHelper(vtkMRMLScene* scene, int column = -1);
  
  //virtual qMRMLAbstractItemHelper* child(int row = 0, int column = 0) const;
  //virtual int childCount() const;
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  //virtual bool hasChildren() const;
  virtual vtkObject* object()const;
  virtual vtkMRMLScene* mrmlScene()const;
  // parent() MUST be reimplemented  
  //virtual qMRMLAbstractItemHelper* parent() const;

protected:
  // here we know for sure that child is a child of this.
  //virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
private:
  QCTK_DECLARE_PRIVATE(qMRMLAbstractSceneItemHelper);
};




//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractNodeItemHelper : public qMRMLAbstractItemHelper
{
public:
  qMRMLAbstractNodeItemHelper(vtkMRMLNode* node, int column = -1);

  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual vtkObject* object() const;
  virtual vtkMRMLNode* mrmlNode()const;
  virtual qMRMLAbstractItemHelper* parent() const = 0;
  virtual bool setData(const QVariant &value, int role = Qt::EditRole);
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
  
  virtual qMRMLAbstractItemHelper* child(int row, int column = 0) const;
  virtual int childCount() const;
  virtual int column() const;
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual bool hasChildren() const;
  virtual qMRMLAbstractItemHelper* parent() const;
  virtual int row() const;
  virtual bool setData(const QVariant &value, int role = Qt::EditRole);
protected:
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;

private:
  QCTK_DECLARE_PRIVATE(qMRMLNodeCategoryItemHelper);
};
*/


//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractRootItemHelper : public qMRMLAbstractItemHelper
{
public:
  qMRMLAbstractRootItemHelper(vtkMRMLScene* scene);
  // child MUST be reimplemented
  virtual qMRMLAbstractItemHelper* child(int row = 0, int column = 0) const;
  virtual int childCount() const;
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual bool hasChildren() const;
  virtual vtkObject* object()const;
  virtual qMRMLAbstractItemHelper* parent() const;
  virtual int row() const;

protected:
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
  vtkMRMLScene* mrmlScene()const;
private:
  QCTK_DECLARE_PRIVATE(qMRMLAbstractRootItemHelper);
};

/*
//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLRootItemHelper : public qMRMLAbstractSceneItemHelper
{
public:
  qMRMLRootItemHelper(vtkMRMLScene* scene, bool topLevelScene = true);
  
  virtual qMRMLAbstractItemHelper* child(int row = 0, int column = 0) const;
  virtual int childCount() const;
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual bool hasChildren() const;
  virtual qMRMLAbstractItemHelper* parent() const;
  virtual int row() const;

protected:
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;

private:
  QCTK_DECLARE_PRIVATE(qMRMLRootItemHelper);
};
*/

#endif
