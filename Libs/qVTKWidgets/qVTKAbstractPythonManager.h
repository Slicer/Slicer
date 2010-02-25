#ifndef __qVTKAbstractPythonManager_h
#define __qVTKAbstractPythonManager_h

// qCTK includes
#include <qCTKAbstractPythonManager.h>

#include "qVTKWidgetsExport.h"

class PythonQtObjectPtr;

class QVTK_WIDGETS_EXPORT qVTKAbstractPythonManager : public qCTKAbstractPythonManager
{
  Q_OBJECT

public:
  typedef qCTKAbstractPythonManager Superclass;
  qVTKAbstractPythonManager(QObject* parent=NULL);
  ~qVTKAbstractPythonManager();
  
protected:

  virtual QStringList pythonPaths();
  virtual void preInitialization();

};

#endif
