/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QSignalSpy>

// SlicerQt includes
#include "qSlicerDirectoryListView.h"

// STD includes
#include <cstdlib>
#include <iostream>

int qSlicerDirectoryListViewTest1(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  qSlicerDirectoryListView widget;

  QSignalSpy spy(&widget, SIGNAL(directoryListChanged()));

  if (widget.directoryList().count() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with directoryList() method !" << std::endl;
    return EXIT_FAILURE;
    }

  QStringList paths;
  widget.setDirectoryList(paths);

  if (spy.count() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryList() method !" << std::endl;
    return EXIT_FAILURE;
    }

  paths << "." << "/should-not-exist" << ".";
  widget.setDirectoryList(paths);

  if (spy.count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryList() method !" << std::endl;
    return EXIT_FAILURE;
    }

  if (widget.directoryList().count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with directoryList/setDirectoryList methods !" << std::endl;
    return EXIT_FAILURE;
    }

  QString current = widget.directoryList().at(0);
  QString expected = QString(".");
  if (current != expected)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with directoryList/setDirectoryList methods !\n"
              << " current[0]:" << qPrintable(current) << "\n"
              << " expected[0]:" << qPrintable(expected) << std::endl;
    return EXIT_FAILURE;
    }

  current = widget.directoryList(true).at(0);
  expected = QDir::current().absolutePath();
  if (current != expected)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryList() method !\n"
              << " current[0]:" << qPrintable(current) << "\n"
              << " expected[0]:" << qPrintable(expected) << std::endl;
    return EXIT_FAILURE;
    }

  widget.removeDirectory(".");

  if (spy.count() != 2)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with removeDirectory() method !" << std::endl;
    return EXIT_FAILURE;
    }

  if (widget.directoryList().count() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with removeDirectory() method !" << std::endl;
    return EXIT_FAILURE;
    }

  paths.clear();
  paths << ".";
  widget.setDirectoryList(paths);

  if (spy.count() != 3)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryList() method !" << std::endl;
    return EXIT_FAILURE;
    }

  if (widget.selectedDirectoryList().size() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with selectedDirectoryList/setDirectoryList  method !" << std::endl;
    return EXIT_FAILURE;
    }

  widget.selectAllDirectories();

  if (widget.selectedDirectoryList().size() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with selectAllDirectories/selectedDirectoryList methods !" << std::endl;
    return EXIT_FAILURE;
    }

  current = widget.selectedDirectoryList().at(0);
  expected = QString(".");
  if (current != expected)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with selectAllDirectories/selectedDirectoryList methods !\n"
              << " current[0]:" << qPrintable(current) << "\n"
              << " expected[0]:" << qPrintable(expected) << std::endl;
    return EXIT_FAILURE;
    }

  current = widget.selectedDirectoryList(true).at(0);
  expected = QDir::current().absolutePath();
  if (current != expected)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with selectAllDirectories/selectedDirectoryList methods !\n"
              << " current[0]:" << qPrintable(current) << "\n"
              << " expected[0]:" << qPrintable(expected) << std::endl;
    return EXIT_FAILURE;
    }

  
  return EXIT_SUCCESS;
}

