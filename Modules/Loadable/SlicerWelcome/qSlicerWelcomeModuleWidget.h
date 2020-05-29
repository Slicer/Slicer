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

#ifndef __qSlicerWelcomeModuleWidget_h
#define __qSlicerWelcomeModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerWelcomeModuleExport.h"

class qSlicerWelcomeModuleWidgetPrivate;

/// \ingroup Slicer_QtModules_SlicerWelcome
class Q_SLICER_QTMODULES_WELCOME_EXPORT qSlicerWelcomeModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerWelcomeModuleWidget(QWidget *parent=nullptr);
  ~qSlicerWelcomeModuleWidget() override;


public slots:

  bool loadNonDicomData();
  bool loadRemoteSampleData();
  bool loadDicomData();
  void editApplicationSettings();
  bool presentTutorials();
  bool exploreLoadedData();

protected:
  void setup() override;

protected slots:
  void loadSource(QWidget*);

protected:
  QScopedPointer<qSlicerWelcomeModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerWelcomeModuleWidget);
  Q_DISABLE_COPY(qSlicerWelcomeModuleWidget);
};

#endif
