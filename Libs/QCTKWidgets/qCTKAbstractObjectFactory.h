#ifndef __qCTKAbstractObjectFactory_h
#define __qCTKAbstractObjectFactory_h 

#include <QString>
#include <QHash>
#include <QDebug>

#include <iostream>

#include "qSlicerBaseGUI2Win32Header.h"

//----------------------------------------------------------------------------
namespace{
// Description:
template<typename BaseClassType, typename ClassType>
BaseClassType *CreateObject()
{
  return new ClassType;
}
}

//----------------------------------------------------------------------------
template<typename BaseClassType, typename UniqueIdType>
class QCTK_WIDGETS_EXPORT qCTKAbstractObjectFactory
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
    this->registeredObjectMap[ uniqueId ] = &CreateObject<BaseClassType, ClassType>; 
    }
  
  // Description:
  // Create an instance of the object
  BaseClassType *Create(UniqueIdType uniqueId)
    {
    ConstIterator iter = this->registeredObjectMap.find(uniqueId); 
    
    if ( iter == this->registeredObjectMap.constEnd())
      {
      return NULL;
      }
    return (iter.value())();
    }

private:
  QHash<UniqueIdType, CreateObjectFunc> registeredObjectMap;
}; 

#endif
