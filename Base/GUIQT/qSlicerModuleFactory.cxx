#include "qSlicerModuleFactory.h" 

#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"

#include "qSlicerTransformsModule.h"

#include <QHash>
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerModuleFactory::qInternal
{
public:
  typedef QHash<QString, QString>::const_iterator MapConstIterator;
  typedef QHash<QString, QString>::iterator       MapIterator;
  
  qInternal()
    {
    }
  QHash<QString, QString> MapTitleToName;
  QHash<QString, QString> MapNameToTitle;
};

//-----------------------------------------------------------------------------
qSlicerModuleFactory::qSlicerModuleFactory() : Superclass()
{
  this->Internal = new qInternal;
  
  this->registerCoreModules(); 
}

//-----------------------------------------------------------------------------
qSlicerModuleFactory::~qSlicerModuleFactory()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerCoreModules()
{
  this->registerModule<qSlicerTransformsModule>();
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
  qDebug() << "qSlicerModuleFactory (" << this << ")"; 
  qDebug() << "MapTitleToName";
  qInternal::MapConstIterator iter = this->Internal->MapTitleToName.constBegin();
  while(iter != this->Internal->MapTitleToName.constEnd())
    {
    qDebug() << "Title:" << iter.key() << "-> Name:" << iter.value(); 
    ++iter;
    }
  qDebug() << "MapNameToTitle"; 
  iter = this->Internal->MapNameToTitle.constBegin();
  while(iter != this->Internal->MapNameToTitle.constEnd())
    {
    qDebug() << "Name:" << iter.key() << "-> Title:" << iter.value(); 
    ++iter;
    }
}

//-----------------------------------------------------------------------------
template<typename ClassType>
void qSlicerModuleFactory::registerModule(/*const QString& moduleTitle*/)
{
  const QString moduleTitle = ClassType::moduleTitle(); 
  
  Q_ASSERT(!this->Internal->MapTitleToName.contains(moduleTitle)); 
  
  QString moduleName = ClassType::staticMetaObject.className(); 
  Q_ASSERT(!this->Internal->MapNameToTitle.contains(moduleName)); 
  

  this->registerQObject<ClassType>();
  
  // Keep track of the relation Title -> moduleName
  this->Internal->MapTitleToName[moduleTitle] = moduleName;
   
  // Keep track of the relation moduleName -> Title
  this->Internal->MapNameToTitle[moduleName] = moduleTitle; 
}

//-----------------------------------------------------------------------------
QString qSlicerModuleFactory::getModuleTitle(const QString & moduleName)
{
  // Lookup module name
  qInternal::MapConstIterator iter = this->Internal->MapNameToTitle.constFind(moduleName); 
  
  if (iter == this->Internal->MapNameToTitle.constEnd())
    {
    //qCritical() << "Failed to retrieve module title given its name:" << moduleName; 
    return QString(); 
    }
  return iter.value(); 
}

//-----------------------------------------------------------------------------
QString qSlicerModuleFactory::getModuleName(const QString & moduleTitle)
{
  // Lookup module name
  qInternal::MapConstIterator iter = this->Internal->MapTitleToName.constFind(moduleTitle); 
  
  if (iter == this->Internal->MapTitleToName.constEnd())
    {
    //qCritical() << "Failed to retrieve module name given its title:" << moduleTitle; 
    return QString(); 
    }
  return iter.value(); 
}

//-----------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleFactory::createModule(const QString& moduleName)
{
  qSlicerAbstractModule* module = this->create(moduleName); 
  if (!module)
    {
    //qCritical() << "Failed to instanciate module:" << moduleName; 
    return 0;
    }
    
  // Get module title
  QString moduleTitle = this->getModuleTitle(moduleName);
  if (moduleTitle.isEmpty())
    {
    return 0; 
    }
    
  module->setWindowTitle(moduleTitle); 
  
  qDebug() << module << " - title:" << moduleTitle; 
  return module; 
}
