/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: qCTKTestApplication.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#include "qCTKTestApplication.h"

#include <stdio.h>

#include <QTimer>
#include <QWidget>
#include <QKeyEvent>
#include <QMouseEvent>

int qCTKTestApplication::Error = 0;

//-----------------------------------------------------------------------------
qCTKTestApplication::qCTKTestApplication(int _argc, char** _argv)
{
  qInstallMsgHandler(qCTKTestApplication::messageHandler);
  
  // CMake generated driver removes argv[0], 
  // so let's put a dummy back in
  this->Argv.append("qCTKTestApplication");
  for(int i=0; i<_argc; i++)
    {
    this->Argv.append(_argv[i]);
    }
  for(int j=0; j<this->Argv.size(); j++)
    {
    this->Argvp.append(this->Argv[j].data());
    }
  this->Argc = this->Argvp.size();
  this->App = new QApplication(this->Argc, this->Argvp.data());
}

//-----------------------------------------------------------------------------
qCTKTestApplication::~qCTKTestApplication()
{
  delete this->App;
  qInstallMsgHandler(0);
}

//-----------------------------------------------------------------------------
int qCTKTestApplication::exec(bool reportErrorsOnExit)
{
  if(QCoreApplication::arguments().contains("--exit"))
    {
    QTimer::singleShot(100, QApplication::instance(), 
                       SLOT(quit()));
    }
    
  int ret = QApplication::exec();
  if (reportErrorsOnExit)
    {
    return Error + ret;
    }
  else
    {
    return ret;
    }
}

//-----------------------------------------------------------------------------
void qCTKTestApplication::messageHandler(QtMsgType type, const char *msg)
{
  switch(type)
  {
  case QtDebugMsg:
    fprintf(stderr, "Debug: %s\n", msg);
    break;
  case QtWarningMsg:
    fprintf(stderr, "Warning: %s\n", msg);
    Error++;
    break;
  case QtCriticalMsg:
    fprintf(stderr, "Critical: %s\n", msg);
    Error++;
    break;
  case QtFatalMsg:
    fprintf(stderr, "Fatal: %s\n", msg);
    abort();
  }
}

//-----------------------------------------------------------------------------
void qCTKTestApplication::delay(int ms)
{
  if(ms > 0)
  {
    QTimer::singleShot(ms, QApplication::instance(), SLOT(quit()));
    QApplication::exec();
  }
}

//-----------------------------------------------------------------------------
bool qCTKTestApplication::simulateEvent(QWidget* w, QEvent* e)
{
  bool status = QApplication::sendEvent(w, e);
  QApplication::processEvents();
  return status;
}

//-----------------------------------------------------------------------------
void qCTKTestApplication::keyUp(QWidget* w, Qt::Key key, Qt::KeyboardModifiers mod, int ms)
{
  if(!w)
    return;
  delay(ms);
  QString text;
  char off = 'a';
  if(mod & Qt::ShiftModifier)
    off = 'A';
  if(key >= Qt::Key_A && key <= Qt::Key_Z)
    {
    text.append(QChar::fromAscii(key - Qt::Key_A + off));
    }
  QKeyEvent e(QEvent::KeyRelease, key, mod, text);
  if(!simulateEvent(w, &e))
    {
    qWarning("keyUp not handled\n");
    }
}

//-----------------------------------------------------------------------------
void qCTKTestApplication::keyDown(QWidget* w, Qt::Key key, Qt::KeyboardModifiers mod, int ms)
{
  if(!w)
    return;
  delay(ms);
  QString text;
  char off = 'a';
  if(mod & Qt::ShiftModifier)
    off = 'A';
  if(key >= Qt::Key_A && key <= Qt::Key_Z)
    {
    text.append(QChar::fromAscii(key - Qt::Key_A + off));
    }
  QKeyEvent e(QEvent::KeyPress, key, mod, text);
  if(!simulateEvent(w, &e))
    {
    qWarning("keyDown not handled\n");
    }
}

//-----------------------------------------------------------------------------
void qCTKTestApplication::keyClick(QWidget* w, Qt::Key key, Qt::KeyboardModifiers mod, int ms)
{
  delay(ms);
  keyDown(w, key, mod, 0);
  keyUp(w, key, mod, 0);
}

//-----------------------------------------------------------------------------
void qCTKTestApplication::mouseDown(QWidget* w, QPoint pos, Qt::MouseButton btn,
                        Qt::KeyboardModifiers mod, int ms)
{
  delay(ms);
  QMouseEvent e(QEvent::MouseButtonPress, pos, btn, btn, mod);
  if(!simulateEvent(w, &e))
    {
    qWarning("mouseDown not handled\n");
    }
}

//-----------------------------------------------------------------------------
void qCTKTestApplication::mouseUp(QWidget* w, QPoint pos, Qt::MouseButton btn,
                      Qt::KeyboardModifiers mod, int ms)
{
  delay(ms);
  QMouseEvent e(QEvent::MouseButtonRelease, pos, btn, btn, mod);
  if(!simulateEvent(w, &e))
    {
    qWarning("mouseUp not handled\n");
    }
}

//-----------------------------------------------------------------------------
void qCTKTestApplication::mouseMove(QWidget* w, QPoint pos, Qt::MouseButton btn,
                        Qt::KeyboardModifiers mod, int ms)
{
  delay(ms);
  QMouseEvent e(QEvent::MouseMove, pos, btn, btn, mod);
  if(!simulateEvent(w, &e))
    {
    qWarning("mouseMove not handled\n");
    }
}

//-----------------------------------------------------------------------------
void qCTKTestApplication::mouseClick(QWidget* w, QPoint pos, Qt::MouseButton btn,
                         Qt::KeyboardModifiers mod, int ms)
{
  delay(ms);
  mouseDown(w, pos, btn, mod, 0);
  mouseUp(w, pos, btn, mod, 0);
}

//-----------------------------------------------------------------------------
void qCTKTestApplication::mouseDClick(QWidget* w, QPoint pos, Qt::MouseButton btn,
                         Qt::KeyboardModifiers mod, int ms)
{
  delay(ms);
  QMouseEvent e(QEvent::MouseButtonDblClick, pos, btn, btn, mod);
  if(!simulateEvent(w, &e))
    {
    qWarning("mouseMove not handled\n");
    }
}
