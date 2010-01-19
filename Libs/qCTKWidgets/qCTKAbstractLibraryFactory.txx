#ifndef __qCTKAbstractLibraryFactory_txx
#define __qCTKAbstractLibraryFactory_txx

#include "qCTKAbstractFactory.h"

//----------------------------------------------------------------------------
template<typename BaseClassType>
qCTKFactoryLibraryItem<BaseClassType>::qCTKFactoryLibraryItem(const QString& _key,
                                                              const QString& _path)
  :qCTKAbstractFactoryItem<BaseClassType>(_key)
  ,Path(_path)
{
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
bool qCTKFactoryLibraryItem<BaseClassType>::load()
{
  this->Library.setFileName(this->path());
  bool loaded = this->Library.load();
  if (loaded)
    {
    this->resolve();
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
QString qCTKFactoryLibraryItem<BaseClassType>::path()const
{ 
  return this->Path; 
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
QString qCTKFactoryLibraryItem<BaseClassType>::loadErrorString()const
{ 
  return this->Library.errorString();
}

//----------------------------------------------------------------------------
template<typename BaseClassType>
void qCTKFactoryLibraryItem<BaseClassType>::setSymbols(const QStringList& symbols)
{ 
  this->Symbols = symbols; 
}

//-----------------------------------------------------------------------------
template<typename BaseClassType>
void qCTKFactoryLibraryItem<BaseClassType>::resolve()
{
  foreach(const QString& symbol, this->Symbols)
    {
    // Sanity checks
    if (symbol.isEmpty()) 
      { 
      continue; 
      }
      
    // Make sure the symbols haven't been registered
    if (this->ResolvedSymbols.contains(symbol))
      {
      qWarning() << "Symbol '" << symbol << "' already resolved - Path:" << this->Path;
      continue;
      }
    
    void * resolvedSymbol = this->Library.resolve(symbol.toLatin1());
    if (!resolvedSymbol)
      {
      qWarning() << "Failed to resolve symbol '" << symbol << "' - Path:" << this->Path;
      }
    this->ResolvedSymbols[symbol] = resolvedSymbol;
    }
}

//-----------------------------------------------------------------------------
template<typename BaseClassType>
void* qCTKFactoryLibraryItem<BaseClassType>::symbolAddress(const QString& symbol)const
{
  ConstIterator iter = this->ResolvedSymbols.find(symbol);
  
  Q_ASSERT(iter != this->ResolvedSymbols.constEnd());
  if ( iter == this->ResolvedSymbols.constEnd())
    {
    return 0;
    }
  return iter.value();
}

//-----------------------------------------------------------------------------
template<typename BaseClassType, typename FactoryItemType>
qCTKAbstractLibraryFactory<BaseClassType, FactoryItemType>::qCTKAbstractLibraryFactory()
  :qCTKAbstractFactory<BaseClassType>()
{
}
  
//-----------------------------------------------------------------------------
template<typename BaseClassType, typename FactoryItemType>
qCTKAbstractLibraryFactory<BaseClassType, FactoryItemType>::~qCTKAbstractLibraryFactory()
{
}

//-----------------------------------------------------------------------------
template<typename BaseClassType, typename FactoryItemType>
void qCTKAbstractLibraryFactory<BaseClassType, FactoryItemType>::setSymbols(const QStringList& symbols) 
{
  this->Symbols = symbols; 
}

//-----------------------------------------------------------------------------
template<typename BaseClassType, typename FactoryItemType>
bool qCTKAbstractLibraryFactory<BaseClassType, FactoryItemType>::registerLibrary(const QFileInfo& file, QString& key)
{
  key = file.fileName();
  // Check if already registered
  if (this->item(key))
    {
    return false;
    }
  QSharedPointer<FactoryItemType> _item =
    QSharedPointer<FactoryItemType>(new FactoryItemType(key, file.filePath()));
  _item->setSymbols(this->Symbols);
  return this->registerItem(_item);
}

#endif
