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

#ifndef __qSlicerAppMainWindow_p_h
#define __qSlicerAppMainWindow_p_h

// Slicer includes
#include "qSlicerAppMainWindow.h"
#include "qSlicerMainWindow_p.h"

class qSlicerModuleSelectorToolBar;
class qSlicerAppMainWindowPrivate;
class qSlicerLayoutManager;

//-----------------------------------------------------------------------------
class Q_SLICER_APP_EXPORT qSlicerAppMainWindowPrivate
  : public qSlicerMainWindowPrivate
{
  Q_DECLARE_PUBLIC(qSlicerAppMainWindow);
public:
  typedef qSlicerMainWindowPrivate Superclass;
  qSlicerAppMainWindowPrivate(qSlicerAppMainWindow& object);
  ~qSlicerAppMainWindowPrivate() override;

  void init() override;
  /// Reimplemented for custom behavior
  void setupUi(QMainWindow * mainWindow) override;
};

#endif
