/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKAbstractPluginFactory_h
#define __qCTKAbstractPluginFactory_h

#include "qCTKAbstractFactory.h"

#include <QPluginLoader>
#include <QFileInfo>

//----------------------------------------------------------------------------
template<typename BaseClassType>
class qCTKFactoryPluginItem : public qCTKAbstractFactoryItem<BaseClassType>
{
public:
  explicit qCTKFactoryPluginItem(const QString& key, const QString& path);
  virtual bool load();
  QString path()const;
  virtual QString loadErrorString()const;

protected:
  virtual BaseClassType* instanciator();

private:
  QPluginLoader    Loader;
  QString          Path;
};

//----------------------------------------------------------------------------
template<typename BaseClassType, typename FactoryItemType = qCTKFactoryPluginItem<BaseClassType> >
class qCTKAbstractPluginFactory : public qCTKAbstractFactory<BaseClassType>
{
public:
  /// 
  /// Constructor
  explicit qCTKAbstractPluginFactory();
  virtual ~qCTKAbstractPluginFactory();

  /// 
  /// Register a plugin in the factory
  virtual bool registerLibrary(const QFileInfo& file, QString& key);

private:
  qCTKAbstractPluginFactory(const qCTKAbstractPluginFactory &);  /// Not implemented
  void operator=(const qCTKAbstractPluginFactory&); /// Not implemented
};

#include "qCTKAbstractPluginFactory.txx"

#endif
