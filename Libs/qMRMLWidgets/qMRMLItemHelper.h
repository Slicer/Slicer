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

#ifndef __qMRMLItemHelper_h
#define __qMRMLItemHelper_h

// Qt includes
#include <Qt>
#include <QVariant>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

// qMRMLWidgets predefs
class qMRMLAbstractItemHelper;
class qMRMLAbstractItemHelperPrivate;
class qMRMLRootItemHelperPrivate;
class qMRMLAbstractSceneItemHelperPrivate;
class qMRMLAbstractNodeItemHelperPrivate;
class qMRMLVariantArrayItemHelperPrivate;
class qMRMLProxyItemHelper;
class qMRMLProxyItemHelperPrivate;
class qMRMLExtraItemsHelper;
class qMRMLExtraItemsHelperPrivate;

// MRML predefs
class vtkMRMLScene;
class vtkMRMLNode;

// VTK predefs
class vtkCollection;
class vtkObject;
class vtkStdString;
class vtkVariantArray;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractItemHelperFactory
{
public:
  virtual ~qMRMLAbstractItemHelperFactory(){}
  virtual qMRMLAbstractItemHelper* createItem(vtkObject* object, int column, int row = -1)const = 0;
  /// create a qMRMLRootItemHelper by default. Overwrite if you want a
  /// different behavior.
  virtual qMRMLAbstractItemHelper* createRootItem(vtkMRMLScene* scene)const;
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractItemHelper
{
public:
  virtual ~qMRMLAbstractItemHelper();
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
  virtual int row(const qMRMLAbstractItemHelper* child) const;
  virtual bool setData(const QVariant &value, int role = Qt::EditRole);
  virtual vtkObject* object()const = 0;
  virtual bool operator==(const qMRMLAbstractItemHelper& helper)const;
protected:
  friend class qMRMLProxyItemHelper;
  qMRMLAbstractItemHelper(int column, const qMRMLAbstractItemHelperFactory* factory, int row);
  
  const qMRMLAbstractItemHelperFactory* factory()const;
  /// here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
protected:
  QScopedPointer<qMRMLAbstractItemHelperPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLAbstractItemHelper);
  Q_DISABLE_COPY(qMRMLAbstractItemHelper);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractSceneItemHelper : public qMRMLAbstractItemHelper
{
public:
  virtual ~qMRMLAbstractSceneItemHelper();
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual vtkObject* object()const;
  virtual vtkMRMLScene* mrmlScene()const;

protected: 
  qMRMLAbstractSceneItemHelper(vtkMRMLScene* scene, int column, const qMRMLAbstractItemHelperFactory* factory, int row);
protected:
  QScopedPointer<qMRMLAbstractSceneItemHelperPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLAbstractSceneItemHelper);
  Q_DISABLE_COPY(qMRMLAbstractSceneItemHelper);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLAbstractNodeItemHelper : public qMRMLAbstractItemHelper
{
public:
  virtual ~qMRMLAbstractNodeItemHelper();
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual vtkObject* object() const;
  virtual vtkMRMLNode* mrmlNode()const;
  virtual qMRMLAbstractItemHelper* parent() const = 0;
  virtual bool setData(const QVariant &value, int role = Qt::EditRole);
protected:
  qMRMLAbstractNodeItemHelper(vtkMRMLNode* node, int column, const qMRMLAbstractItemHelperFactory* factory, int row);

protected:
  QScopedPointer<qMRMLAbstractNodeItemHelperPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLAbstractNodeItemHelper);
  Q_DISABLE_COPY(qMRMLAbstractNodeItemHelper);
};

// FIXME: doesn't need to be derived, does it ?
//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLRootItemHelper : public qMRMLAbstractItemHelper
{
public:
  virtual ~qMRMLRootItemHelper();
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
  virtual int childCount() const;
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual bool hasChildren() const;
  virtual vtkObject* object()const;
  virtual qMRMLAbstractItemHelper* parent() const;
  virtual int row() const;

protected:
  friend class qMRMLAbstractItemHelperFactory;
  qMRMLRootItemHelper(vtkMRMLScene* scene, const qMRMLAbstractItemHelperFactory* factory);

  /// here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
  vtkMRMLScene* mrmlScene()const;
protected:
  QScopedPointer<qMRMLRootItemHelperPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLRootItemHelper);
  Q_DISABLE_COPY(qMRMLRootItemHelper);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLProxyItemHelper : public qMRMLAbstractItemHelper
{
public:
  virtual ~qMRMLProxyItemHelper();
  virtual bool canReparent(qMRMLAbstractItemHelper* newParent)const;
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
  virtual int childCount() const;
  int column() const;
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual bool hasChildren() const;
  virtual qMRMLAbstractItemHelper* parent() const;
  virtual bool reparent(qMRMLAbstractItemHelper* newParent);
  int row() const;
  virtual bool setData(const QVariant &value, int role = Qt::EditRole);
  virtual vtkObject* object()const;
  virtual bool operator==(const qMRMLAbstractItemHelper& helper)const;
protected:
  qMRMLProxyItemHelper(qMRMLAbstractItemHelper* proxy);
  /// here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
  qMRMLAbstractItemHelper* proxy()const;
protected:
  QScopedPointer<qMRMLProxyItemHelperPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLProxyItemHelper);
  Q_DISABLE_COPY(qMRMLProxyItemHelper);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLVariantArrayItemHelper : public qMRMLAbstractItemHelper
{
public:
  qMRMLVariantArrayItemHelper(vtkVariantArray* array, int column, const qMRMLAbstractItemHelperFactory* factory, int row);
  virtual ~qMRMLVariantArrayItemHelper();
  virtual QVariant data(int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags() const;
  virtual qMRMLAbstractItemHelper* parent() const;
  virtual bool setData(const QVariant &value, int role = Qt::EditRole);
  virtual vtkObject* object()const;
  static void createProperties(vtkVariantArray& properties, 
                               vtkObject* parent, 
                               const vtkStdString& title,
                               Qt::ItemFlags flags = Qt::ItemIsEnabled);
protected:
  vtkMRMLScene* mrmlScene()const;

protected:
  QScopedPointer<qMRMLVariantArrayItemHelperPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLVariantArrayItemHelper);
  Q_DISABLE_COPY(qMRMLVariantArrayItemHelper);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLExtraItemsHelper : public qMRMLProxyItemHelper
{
public:
  qMRMLExtraItemsHelper(vtkCollection* preItems, vtkCollection* postItems,qMRMLAbstractItemHelper* proxy);
  virtual ~qMRMLExtraItemsHelper();
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
  virtual int childCount() const;
  virtual bool hasChildren() const;
  vtkCollection* preItems()const;
  vtkCollection* postItems()const;
protected:
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
protected:
  QScopedPointer<qMRMLExtraItemsHelperPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLExtraItemsHelper);
  Q_DISABLE_COPY(qMRMLExtraItemsHelper);
};

#endif
