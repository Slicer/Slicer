#ifndef __qSlicerAbstractCoreModule_h
#define __qSlicerAbstractCoreModule_h

#include "qSlicerAbstractModule.h"

#include <qCTKPimpl.h>

#include "qSlicerBaseQTBaseWin32Header.h"

class qSlicerAbstractCoreModulePrivate;

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerAbstractCoreModule : public qSlicerAbstractModule
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerAbstractCoreModule(QObject *parent=0);

  virtual void printAdditionalInfo();

  virtual QString name() const;

protected:
  virtual void setup();

  virtual void setName(const QString& name){}

  // Description:
  // Convenient method to return slicer wiki URL
  QString slicerWikiUrl()const{ return "http://www.slicer.org/slicerWiki/index.php"; }

private:
  QCTK_DECLARE_PRIVATE(qSlicerAbstractCoreModule);
};

#endif
