/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKAbstractPythonManager_h
#define __qCTKAbstractPythonManager_h

// QT includes
#include <QObject>
#include <QList>
#include <QStringList>

#include "qCTKWidgetsExport.h"

class PythonQtObjectPtr;

class QCTK_WIDGETS_EXPORT qCTKAbstractPythonManager : public QObject
{
  Q_OBJECT

public:
  typedef QObject Superclass;
  qCTKAbstractPythonManager(QObject* _parent=NULL);
  ~qCTKAbstractPythonManager();

  PythonQtObjectPtr mainContext();
  void addObjectToPythonMain(const QString& name, QObject* obj);
  void registerPythonQtDecorator(QObject* decorator);
  void registerClassForPythonQt(const QMetaObject* metaobject);
  void registerCPPClassForPythonQt(const char* name);

  // Description:
  // Execute a python of python code (can be multiple lines separated with newline)
  // and return the result as a QVariant.
  QVariant executeString(const QString& code);

  // Description:
  // Gets the value of the variable looking in the __main__ module.
  // If the variable is not found returns a default initialized QVariant.
  QVariant getVariable(const QString& varName);

  // Description:
  // Execute a python script with the given filename.
  void executeFile(const QString& filename);

signals:

  // Description:
  // This signal is emitted after python is initialized.  Observers can listen
  // for this signal to handle additional initialization steps.
  void pythonInitialized();

protected slots:
  void printStderr(const QString&);
  void printStdout(const QString&);

protected:

  void initPythonQt();

  virtual QStringList     pythonPaths();
  virtual void            preInitialization();

};
#endif
