/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKAbstractObjectFactory_h
#define __qCTKAbstractObjectFactory_h

#include "qCTKAbstractFactory.h"

#include <QDebug>

//----------------------------------------------------------------------------
namespace{
  /// 
  /// Function in charge of instanciating an object of type: ClassType
  template<typename BaseClassType, typename ClassType>
    BaseClassType *instantiateObject()
  {
    return new ClassType;
  }
}

//----------------------------------------------------------------------------
template<typename BaseClassType, typename ClassType>
class qCTKFactoryObjectItem : public qCTKAbstractFactoryItem<BaseClassType>
{
protected:
  typedef BaseClassType *(*InstantiateObjectFunc)();
public:
  explicit qCTKFactoryObjectItem(const QString& key);
  virtual bool load();
protected:
  virtual BaseClassType* instanciator();
private:
  InstantiateObjectFunc instantiateObjectFunc;
};

//----------------------------------------------------------------------------
template<typename BaseClassType>
class qCTKAbstractObjectFactory : public qCTKAbstractFactory<BaseClassType>
{
public:
  //-----------------------------------------------------------------------------
  /// 
  /// Constructor/Desctructor
  explicit qCTKAbstractObjectFactory();
  virtual ~qCTKAbstractObjectFactory();

  //-----------------------------------------------------------------------------
  /// 
  /// Register an object in the factory
  template<typename ClassType>
  bool registerObject(const QString& key);

private:
  qCTKAbstractObjectFactory(const qCTKAbstractObjectFactory &);  /// Not implemented
  void operator=(const qCTKAbstractObjectFactory&); /// Not implemented
};

#include "qCTKAbstractObjectFactory.txx"

#endif
