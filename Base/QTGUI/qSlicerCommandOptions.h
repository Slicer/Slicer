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

#ifndef __qSlicerCommandOptions_h
#define __qSlicerCommandOptions_h

// Slicer includes
#include "qSlicerCoreCommandOptions.h"

#include "qSlicerBaseQTGUIExport.h"

class qSlicerCommandOptionsPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerCommandOptions : public qSlicerCoreCommandOptions
{
  Q_OBJECT
  Q_PROPERTY(bool noSplash READ noSplash CONSTANT)
  Q_PROPERTY(bool disableToolTips READ disableToolTips CONSTANT)
  Q_PROPERTY(bool noMainWindow READ noMainWindow CONSTANT)
  Q_PROPERTY(bool showPythonInteractor READ showPythonInteractor CONSTANT)
  Q_PROPERTY(bool enableQtTesting READ enableQtTesting CONSTANT)
  Q_PROPERTY(bool exitAfterStartup READ exitAfterStartup CONSTANT)
public:
  typedef qSlicerCoreCommandOptions Superclass;
  qSlicerCommandOptions();
  ~qSlicerCommandOptions() override = default;

  bool noSplash()const;

  bool disableToolTips()const;

  bool noMainWindow()const;

  bool showPythonInteractor()const;

  bool enableQtTesting()const;

  bool exitAfterStartup()const;

protected:
  void addArguments() override;

};

#endif
