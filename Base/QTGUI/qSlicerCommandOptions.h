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

// SlicerQt includes
#include "qSlicerCoreCommandOptions.h"

#include "qSlicerBaseQTGUIExport.h"

class qSlicerCommandOptionsPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerCommandOptions : public qSlicerCoreCommandOptions
{
  Q_OBJECT
  Q_PROPERTY(bool noSplash READ noSplash)
  Q_PROPERTY(bool disableToolTips READ disableToolTips)
  Q_PROPERTY(bool showPythonInteractor READ showPythonInteractor)
public:
  typedef qSlicerCoreCommandOptions Superclass;
  qSlicerCommandOptions();
  virtual ~qSlicerCommandOptions(){}

  bool noSplash()const;

  bool disableToolTips()const;

  bool noMainWindow()const;

  bool showPythonInteractor()const;

protected:
  virtual void addArguments();

};

#endif
