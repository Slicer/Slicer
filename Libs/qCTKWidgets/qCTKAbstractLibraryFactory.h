#ifndef __qCTKAbstractLibraryFactory_h
#define __qCTKAbstractLibraryFactory_h

#include "qCTKAbstractFactory.h"

#include <QLibrary>
#include <QFileInfo>
#include <QDebug>

//----------------------------------------------------------------------------
template<typename BaseClassType>
class qCTKFactoryLibraryItem : public qCTKAbstractFactoryItem<BaseClassType>
{
protected:
  typedef typename QHash<QString, void*>::const_iterator ConstIterator;
  typedef typename QHash<QString, void*>::iterator       Iterator;

public:
  qCTKFactoryLibraryItem(const QString& key, const QString& path):
    qCTKAbstractFactoryItem<BaseClassType>(key),Path(path){}
  virtual bool load()
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
  QString path() { return this->Path; }
  virtual QString loadErrorString() { return this->Library.errorString();}

  void setSymbols(const QStringList& symbols) { this->Symbols = symbols; }

  //-----------------------------------------------------------------------------
  // Description:
  // Resolve symbols
  void resolve()
    {
    foreach(const QString& symbol, this->Symbols)
      {
      // Sanity checks
      if (symbol.isEmpty()) { continue; }

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
    // Description:
    // Get symbol address
    void * symbolAddress(const QString& symbol)
    {
      ConstIterator iter = this->ResolvedSymbols.find(symbol);

      Q_ASSERT(iter != this->ResolvedSymbols.constEnd());
      if ( iter == this->ResolvedSymbols.constEnd())
        {
        return 0;
        }
      return iter.value();
    }

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
  // Description:
  // Constructor
  qCTKAbstractLibraryFactory():
    qCTKAbstractFactory<BaseClassType>(){}
  virtual ~qCTKAbstractLibraryFactory(){}

  //-----------------------------------------------------------------------------
  // Description:
  // Set the list of symbols
  void setSymbols(const QStringList& symbols) { this->Symbols = symbols; }

  //-----------------------------------------------------------------------------
  // Description:
  // Register a plugin in the factory
  virtual bool registerLibrary(const QString& key, const QString& path)
    {
    // Check if already registered
    if (this->getItem(key))
      {
      return false;
      }
    QSharedPointer<FactoryItemType> item =
      QSharedPointer<FactoryItemType>(new FactoryItemType(key, path));
    item->setSymbols(this->Symbols);
    return this->registerItem(item);
    }


private:
  qCTKAbstractLibraryFactory(const qCTKAbstractLibraryFactory &);  // Not implemented
  void operator=(const qCTKAbstractLibraryFactory&); // Not implemented

  QStringList Symbols;
};

#endif
