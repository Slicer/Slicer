#include "qVTKAbstractPythonManager.h"

//-----------------------------------------------------------------------------
qVTKAbstractPythonManager::qVTKAbstractPythonManager(QObject* _parent) : Superclass(_parent)
{

}

//-----------------------------------------------------------------------------
qVTKAbstractPythonManager::~qVTKAbstractPythonManager()
{
}

//-----------------------------------------------------------------------------
QStringList qVTKAbstractPythonManager::pythonPaths()
{
  QStringList paths;
  paths << Superclass::pythonPaths(); 
  return paths; 
}

//-----------------------------------------------------------------------------
void qVTKAbstractPythonManager::preInitialization()
{
  Superclass::preInitialization();
//   this->registerCPPClassForPythonQt("vtkObject");
}
