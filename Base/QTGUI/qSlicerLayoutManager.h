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

#ifndef __qSlicerLayoutManager_h
#define __qSlicerLayoutManager_h

// CTK includes
#include "qSlicerBaseQTGUIExport.h"

// MRMLWidgets includes
#include <qMRMLLayoutManager.h>

class qSlicerLayoutManagerPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerLayoutManager : public qMRMLLayoutManager
{
  Q_OBJECT
public:
  /// Constructors
  explicit qSlicerLayoutManager(QWidget* widget = nullptr);

  /// Set the directory from which built-in scripted
  /// displayableManagers should be sourced from.
  Q_INVOKABLE void setScriptedDisplayableManagerDirectory(const QString& scriptedDisplayableManagerDirectory);

  void setCurrentModule(const QString& moduleName);

signals:
  void selectModule(const QString& moduleName);

private:
  Q_DECLARE_PRIVATE(qSlicerLayoutManager);
  Q_DISABLE_COPY(qSlicerLayoutManager);
};

#endif
