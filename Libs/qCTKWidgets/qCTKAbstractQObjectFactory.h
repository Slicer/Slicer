/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKAbstractQObjectFactory_h
#define __qCTKAbstractQObjectFactory_h

#include "qCTKAbstractObjectFactory.h"

#include <QDebug>

//----------------------------------------------------------------------------
template<typename BaseClassType>
class qCTKAbstractQObjectFactory : public qCTKAbstractObjectFactory<BaseClassType>
{
public:
  //-----------------------------------------------------------------------------
  /// 
  /// Constructor/Desctructor
  explicit qCTKAbstractQObjectFactory();
  virtual ~qCTKAbstractQObjectFactory();

  //----------------------------------------------------------------------------
  /// 
  /// Create an instance of the object
  virtual BaseClassType * instantiate(const QString& itemKey);

  //----------------------------------------------------------------------------
  /// 
  /// Uninstanciate the object
  virtual void uninstantiate(const QString& itemKey);

  //-----------------------------------------------------------------------------
  /// 
  /// Register an object in the factory
  /// Note: The parameter 'key' passed by reference will be updated with the associated object name
  template<typename ClassType>
  bool registerQObject(QString& key);

private:
  qCTKAbstractQObjectFactory(const qCTKAbstractQObjectFactory &);  /// Not implemented
  void operator=(const qCTKAbstractQObjectFactory&); /// Not implemented
};

#include "qCTKAbstractQObjectFactory.txx"

#endif
