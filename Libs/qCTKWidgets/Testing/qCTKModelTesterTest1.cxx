/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// qCTK includes
#include <qCTKModelTester.h>

// QT includes
#include <QApplication>
#include <QList>
#include <QTreeWidget>
#include <QString>
#include <QStringList>

// std includes
#include <stdlib.h>
#include <iostream>

int qCTKModelTesterTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  try
    {
    QTreeWidget treeWidget(0);
    qCTKModelTester tester(treeWidget.model());
    treeWidget.setColumnCount(1);
    QList<QTreeWidgetItem *> items;
    for (int i = 0; i < 10; ++i)
      {
      items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("item: %1").arg(i))));
      }
    treeWidget.insertTopLevelItems(0, items);
    }
  catch (const char* error)
    {
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

