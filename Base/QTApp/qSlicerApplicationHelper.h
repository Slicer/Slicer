/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

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

#ifndef __qSlicerApplicationHelper_h
#define __qSlicerApplicationHelper_h

// Qt includes
#include <QObject>

#include "qSlicerBaseQTAppExport.h"

class ctkPythonConsole;
class qSlicerModuleFactoryManager;

class Q_SLICER_BASE_QTAPP_EXPORT qSlicerApplicationHelper : public QObject
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  typedef qSlicerApplicationHelper Self;

  qSlicerApplicationHelper(QObject * parent = 0);
  virtual ~qSlicerApplicationHelper();

  static void setupModuleFactoryManager(qSlicerModuleFactoryManager * moduleFactoryManager);

  static void showMRMLEventLoggerWidget();

  static void initializePythonConsole(ctkPythonConsole* pythonConsole);

private:
  Q_DISABLE_COPY(qSlicerApplicationHelper);
};

#endif
