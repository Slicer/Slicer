/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// QT includes
#include <QString>
#include <QStringList>

// SlicerQt includes
#include <qSlicerCLIExecutableModuleFactory.h>

// STD includes
#include <cstdlib>
#include <iostream>

#include "TestingMacros.h"

int qSlicerCLIExecutableModuleFactoryTest1(int, char * [] )
{
  QStringList executableNames;
  executableNames << "Threshold.exe"
                  << "threshold.exe"
                  << "Threshold";

  QString expectedModuleName = "threshold";
  qSlicerCLIExecutableModuleFactory factory;
  foreach (const QString& executableName, executableNames)
    {
    QString moduleName = factory.fileNameToKey(executableName);
    if (moduleName != expectedModuleName)
      {
      std::cerr << __LINE__ << " - Error in  extractModuleName()" << std::endl
                            << "executableName = " << qPrintable(executableName) << std::endl
                            << "moduleName = " << qPrintable(moduleName) << std::endl
                            << "expectedModuleName = " << qPrintable(expectedModuleName) << std::endl;
      return EXIT_FAILURE;
      }
    }

  return EXIT_SUCCESS;
}
