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
  Module:    $RCSfile: QTestApp.h,v $

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

#ifndef __qCTKTestApplication_h
#define __qCTKTestApplication_h

#include <QApplication>
#include <QVector>
#include <QByteArray>

class qCTKTestApplication
{
public:
  qCTKTestApplication(int _argc, char** _argv);
  ~qCTKTestApplication();

  // Description:
  // If reportErrorsOnExit is true, then the return value will
  // be the number of warning messages plus the number of error messages
  // produced by QDebug during execution.
  static int exec(bool reportErrorsOnExit=false);

  static void messageHandler(QtMsgType type, const char *msg);

  static void delay(int ms);

  static bool simulateEvent(QWidget* w, QEvent* e);

  static void keyUp(QWidget* w, Qt::Key key, Qt::KeyboardModifiers mod, int ms);

  static void keyDown(QWidget* w, Qt::Key key, Qt::KeyboardModifiers mod, int ms);

  static void keyClick(QWidget* w, Qt::Key key, Qt::KeyboardModifiers mod, int ms);

  static void mouseDown(QWidget* w, QPoint pos, Qt::MouseButton btn, 
                        Qt::KeyboardModifiers mod, int ms);
  
  static void mouseUp(QWidget* w, QPoint pos, Qt::MouseButton btn, 
                      Qt::KeyboardModifiers mod, int ms);
  
  static void mouseMove(QWidget* w, QPoint pos, Qt::MouseButton btn, 
                        Qt::KeyboardModifiers mod, int ms);

  static void mouseClick(QWidget* w, QPoint pos, Qt::MouseButton btn, 
                         Qt::KeyboardModifiers mod, int ms);

  static void mouseDClick(QWidget* w, QPoint pos, Qt::MouseButton btn, 
                          Qt::KeyboardModifiers mod, int ms);

private:
  QApplication*     App;
  static int        Error;
  QList<QByteArray> Argv;
  QVector<char*>    Argvp;
  int               Argc;
};

#endif
