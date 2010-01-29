/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKAbstractLibraryFactory_h
#define __qCTKAbstractLibraryFactory_h

#include "qCTKAbstractFactory.h"

#include <QFileInfo>
#include <QLibrary>
#include <QStringList>

//----------------------------------------------------------------------------
template<typename BaseClassType>
class qCTKFactoryLibraryItem : public qCTKAbstractFactoryItem<BaseClassType>
{
protected:
  typedef typename QHash<QString, void*>::const_iterator ConstIterator;
  typedef typename QHash<QString, void*>::iterator       Iterator;

public:
  explicit qCTKFactoryLibraryItem(const QString& key, const QString& path);
  virtual bool load();
  QString path()const;
  virtual QString loadErrorString()const;

  void setSymbols(const QStringList& symbols);

  //-----------------------------------------------------------------------------
  /// 
  /// Resolve symbols
  void resolve();
  
  //-----------------------------------------------------------------------------
  /// 
  /// Get symbol address
  void* symbolAddress(const QString& symbol)const;

private:
  QLibrary              Library;
  QString               Path;
  QHash<QString, void*> ResolvedSymbols;
  QStringList           Symbols;
};

//----------------------------------------------------------------------------
template<typename BaseClassType, typename FactoryItemType>
class qCTKAbstractLibraryFactory : public qCTKAbstractFactory<BaseClassType>
{
public:
  //-----------------------------------------------------------------------------
  /// 
  /// Constructor
  explicit qCTKAbstractLibraryFactory();
  virtual ~qCTKAbstractLibraryFactory();

  //-----------------------------------------------------------------------------
  /// 
  /// Set the list of symbols
  void setSymbols(const QStringList& symbols);

  //-----------------------------------------------------------------------------
  /// 
  /// Register a plugin in the factory
  virtual bool registerLibrary(const QFileInfo& file, QString& key);

private:
  qCTKAbstractLibraryFactory(const qCTKAbstractLibraryFactory &);  /// Not implemented
  void operator=(const qCTKAbstractLibraryFactory&); /// Not implemented

  QStringList Symbols;
};

#include "qCTKAbstractLibraryFactory.txx"

#endif
