#ifndef __qCTKAbstractObjectFactory_h
#define __qCTKAbstractObjectFactory_h 

#include "qCTKAbstractFactory.h"

#include <QDebug>

//----------------------------------------------------------------------------
namespace{
// Description:
// Function in charge of instanciating an object of type: ClassType
template<typename BaseClassType, typename ClassType>
BaseClassType *instanciateObject()
{
  return new ClassType;
}
}

//----------------------------------------------------------------------------
template<typename BaseClassType, typename ClassType>
class qCTKFactoryObjectItem : public qCTKAbstractFactoryItem<BaseClassType>
{
protected:
  typedef BaseClassType *(*InstanciateObjectFunc)();
public:
  qCTKFactoryObjectItem(const QString& key):qCTKAbstractFactoryItem<BaseClassType>(key){}
  virtual bool load()
    {
    this->instanciateObjectFunc = &instanciateObject<BaseClassType, ClassType>;
    return true; 
    }
protected:
  virtual BaseClassType* instanciator()
    {
    return this->instanciateObjectFunc();
    }
private:
  InstanciateObjectFunc instanciateObjectFunc; 
};

//----------------------------------------------------------------------------
template<typename BaseClassType>
class qCTKAbstractObjectFactory : public qCTKAbstractFactory<BaseClassType>
{
public:
  //-----------------------------------------------------------------------------
  // Description:
  // Constructor
  qCTKAbstractObjectFactory(){}
  virtual ~qCTKAbstractObjectFactory(){}
  
  //-----------------------------------------------------------------------------
  // Description:
  // Register an object in the factory
  template<typename ClassType>
  bool registerQObject()
    {
    return this->registerObject<ClassType>(ClassType::staticMetaObject.className());
    }

  //-----------------------------------------------------------------------------
  // Description:
  // Register an object in the factory
  template<typename ClassType>
  bool registerObject(const QString& key)
    {
    // Check if already registered
    if (this->get(key))
      { 
      return false; 
      }
    QSharedPointer<qCTKFactoryObjectItem<BaseClassType, ClassType> > objectItem = 
      QSharedPointer<qCTKFactoryObjectItem<BaseClassType, ClassType> >(
        new qCTKFactoryObjectItem<BaseClassType, ClassType>(key) );
    return this->registerItem(objectItem);
    }

private:
  qCTKAbstractObjectFactory(const qCTKAbstractObjectFactory &);  // Not implemented
  void operator=(const qCTKAbstractObjectFactory&); // Not implemented 
}; 

#endif
