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
  QString path();
  virtual QString loadErrorString();

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
  // Description:
  // Constructor
  explicit qCTKAbstractPluginFactory();
  virtual ~qCTKAbstractPluginFactory();

  // Description:
  // Register a plugin in the factory
  virtual bool registerLibrary(const QFileInfo& file, QString& key);

private:
  qCTKAbstractPluginFactory(const qCTKAbstractPluginFactory &);  // Not implemented
  void operator=(const qCTKAbstractPluginFactory&); // Not implemented
};

#include "qCTKAbstractPluginFactory.txx"

#endif
