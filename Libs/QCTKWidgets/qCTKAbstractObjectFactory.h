#ifndef __qCTKAbstractObjectFactory_h
#define __qCTKAbstractObjectFactory_h 

#include "qCTKWidgetsWin32Header.h"

#include <QString>
#include <QHash>
#include <QDebug>

#include <iostream>

//----------------------------------------------------------------------------
namespace{
// Description:
template<typename BaseClassType, typename ClassType>
BaseClassType *createObject()
{
  return new ClassType;
}
}

//----------------------------------------------------------------------------
template<typename BaseClassType, typename UniqueIdType>
class qCTKAbstractObjectFactory
{
protected:
  typedef BaseClassType *(*CreateObjectFunc)();

public:
  typedef typename QHash<UniqueIdType, CreateObjectFunc>::const_iterator ConstIterator;
  typedef typename QHash<UniqueIdType, CreateObjectFunc>::iterator Iterator;

  // Description:
  // Constructor
  qCTKAbstractObjectFactory(){}
  virtual ~qCTKAbstractObjectFactory(){}
  
  // Description:
  // Register an object in the factory
  template<typename ClassType>
  void registerQObject()
    {
    this->registerObject<ClassType>(ClassType::staticMetaObject.className());
    }
    
  // Description:
  // Register an object in the factory
  template<typename ClassType>
  void registerObject(UniqueIdType uniqueId)
    {
    this->RegisteredObjectMap[ uniqueId ] = &createObject<BaseClassType, ClassType>; 
    }
  
  // Description:
  // Create an instance of the object
  virtual BaseClassType *create(UniqueIdType uniqueId)
    {
    ConstIterator iter = this->RegisteredObjectMap.find(uniqueId); 
    
    if ( iter == this->RegisteredObjectMap.constEnd())
      {
      return NULL;
      }
    return (iter.value())();
    }
    
  // Description:
  // List all registered objects
  void dumpObjectInfo()
  {
    // TODO 
  }

private:
  qCTKAbstractObjectFactory(const qCTKAbstractObjectFactory &);  // Not implemented
  void operator=(const qCTKAbstractObjectFactory&); // Not implemented
  
  QHash<UniqueIdType, CreateObjectFunc> RegisteredObjectMap;
}; 

#endif
