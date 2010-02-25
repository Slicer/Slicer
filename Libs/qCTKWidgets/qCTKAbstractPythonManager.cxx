/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#include "qCTKAbstractPythonManager.h"

//#include "qZGuiPythonQtDecorators.h"

//#include "../extensions/PythonQt_QtAll/PythonQt_QtAll.h"

// QT includes
#include <QDir>
#include <QDebug>

// PythonQT includes
#include <PythonQt.h>

#include <signal.h>

//-----------------------------------------------------------------------------
qCTKAbstractPythonManager::qCTKAbstractPythonManager(QObject* _parent) : Superclass(_parent)
{

}

//-----------------------------------------------------------------------------
qCTKAbstractPythonManager::~qCTKAbstractPythonManager()
{
  PythonQt::cleanup();
}

//-----------------------------------------------------------------------------
PythonQtObjectPtr qCTKAbstractPythonManager::mainContext()
{
  if (!PythonQt::self())
    {
    this->initPythonQt();
    }
  if (PythonQt::self())
    {
    return PythonQt::self()->getMainModule();
    }
  return PythonQtObjectPtr();
}

//-----------------------------------------------------------------------------
void qCTKAbstractPythonManager::initPythonQt()
{
  PythonQt::init(PythonQt::IgnoreSiteModule | PythonQt::RedirectStdOut);

  // Python maps SIGINT (control-c) to its own handler.  We will remap it
  // to the default so that control-c works.
  #ifdef SIGINT
  signal(SIGINT, SIG_DFL);
  #endif

  PythonQtObjectPtr _mainContext = PythonQt::self()->getMainModule();

  this->connect(PythonQt::self(), SIGNAL(pythonStdOut(const QString&)),
                SLOT(printStdout(const QString&)));
  this->connect(PythonQt::self(), SIGNAL(pythonStdErr(const QString&)),
                SLOT(printStderr(const QString&)));

  QStringList initCode;
  initCode << "import sys";
  foreach (QString path, this->pythonPaths())
    {
    initCode << QString("sys.path.append('%1')").arg(QDir::fromNativeSeparators(path));
    }

  _mainContext.evalScript(initCode.join("\n"));

  this->preInitialization();

  emit this->pythonInitialized();
}

//-----------------------------------------------------------------------------
QStringList qCTKAbstractPythonManager::pythonPaths()
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qCTKAbstractPythonManager::preInitialization()
{
}

//-----------------------------------------------------------------------------
void qCTKAbstractPythonManager::registerPythonQtDecorator(QObject* decorator)
{
  PythonQt::self()->addDecorators(decorator);
}

//-----------------------------------------------------------------------------
void qCTKAbstractPythonManager::registerClassForPythonQt(const QMetaObject* metaobject)
{
  PythonQt::self()->registerClass(metaobject);
}

//-----------------------------------------------------------------------------
void qCTKAbstractPythonManager::registerCPPClassForPythonQt(const char* name)
{
  PythonQt::self()->registerCPPClass(name);
}

//-----------------------------------------------------------------------------
QVariant qCTKAbstractPythonManager::executeString(const QString& code)
{
  QVariant ret;
  PythonQtObjectPtr main = qCTKAbstractPythonManager::mainContext();
  if (main)
    {
    ret = main.evalScript(code, Py_single_input);
    }
  return ret;
}

//-----------------------------------------------------------------------------
void qCTKAbstractPythonManager::executeFile(const QString& filename)
{
  PythonQtObjectPtr main = qCTKAbstractPythonManager::mainContext();
  if (main)
    {
    main.evalFile(filename);
    }
}

//-----------------------------------------------------------------------------
void qCTKAbstractPythonManager::addObjectToPythonMain(const QString& name, QObject* obj)
{
  PythonQtObjectPtr main = qCTKAbstractPythonManager::mainContext();
  if (main && obj)
    {
    main.addObject(name, obj);
    }
}

//-----------------------------------------------------------------------------
QVariant qCTKAbstractPythonManager::getVariable(const QString& name)
{
  PythonQtObjectPtr main = qCTKAbstractPythonManager::mainContext();
  if (main)
    {
    return PythonQt::self()->getVariable(main, name);
    }
  return QVariant();
}

//-----------------------------------------------------------------------------
void qCTKAbstractPythonManager::printStdout(const QString& text)
{
  std::cout << qPrintable(text);
}

//-----------------------------------------------------------------------------
void qCTKAbstractPythonManager::printStderr(const QString& text)
{
  std::cout << qPrintable(text);
}
