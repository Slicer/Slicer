#ifndef __qMRMLItemHelper_h
#define __qMRMLItemHelper_h

#include "qCTKPimpl.h"
#include "qMRMLWidgetsWin32Header.h"

#include <Qt>
#include <QVariant>

class vtkObject;
class vtkMRMLScene;
class vtkMRMLNode;
class qMRMLAbstractItemHelperPrivate;

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
protected:
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
private:
  QCTK_DECLARE_PRIVATE(qMRMLAbstractItemHelper);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLSceneItemHelper : public qMRMLAbstractItemHelper
{
public:
  qMRMLSceneItemHelper(vtkMRMLScene* scene, int column = -1);
  
  virtual qMRMLAbstractItemHelper* child(int row = 0, int column = 0) const;
  virtual int childCount() const;
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual bool hasChildren() const;
  virtual vtkObject* object()const;
  virtual vtkMRMLScene* mrmlScene()const;
  virtual qMRMLAbstractItemHelper* parent() const;

protected:
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
private:
  QCTK_DECLARE_PRIVATE(qMRMLSceneItemHelper);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLNodeItemHelper : public qMRMLAbstractItemHelper
{
public:
  qMRMLNodeItemHelper(vtkMRMLNode* node, int column = -1);

  virtual bool canReparent(qMRMLAbstractItemHelper* newParent)const;    
  virtual qMRMLAbstractItemHelper* child(int row = 0, int column = 0) const;
  virtual int childCount() const;
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual bool hasChildren() const;
  virtual vtkObject* object() const;
  virtual vtkMRMLNode* mrmlNode()const;
  virtual qMRMLAbstractItemHelper* parent() const;
  virtual bool reparent(qMRMLAbstractItemHelper* newParent);
  virtual bool setData(const QVariant &value, int role = Qt::EditRole);
protected:
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
private:
  QCTK_DECLARE_PRIVATE(qMRMLNodeItemHelper);
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
class QMRML_WIDGETS_EXPORT qMRMLRootItemHelper : public qMRMLSceneItemHelper
{
public:
  qMRMLRootItemHelper(bool topLevelScene, vtkMRMLScene* scene);
  
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


#endif
