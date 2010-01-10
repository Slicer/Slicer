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
  // Description:
  // Constructor/Desctructor
  explicit qCTKAbstractQObjectFactory();
  virtual ~qCTKAbstractQObjectFactory();

  //----------------------------------------------------------------------------
  // Description:
  // Create an instance of the object
  virtual BaseClassType * instantiate(const QString& itemKey);

  //----------------------------------------------------------------------------
  // Description:
  // Uninstanciate the object
  virtual void uninstantiate(const QString& itemKey);

  //-----------------------------------------------------------------------------
  // Description:
  // Register an object in the factory
  // Note: The parameter 'key' passed by reference will be updated with the associated object name
  template<typename ClassType>
  bool registerQObject(QString& key);

private:
  qCTKAbstractQObjectFactory(const qCTKAbstractQObjectFactory &);  // Not implemented
  void operator=(const qCTKAbstractQObjectFactory&); // Not implemented
};

#include "qCTKAbstractQObjectFactory.txx"

#endif
