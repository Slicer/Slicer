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

#ifndef __qSlicerWelcomeModuleWidget_h
#define __qSlicerWelcomeModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerWelcomeModuleExport.h"

class qSlicerWelcomeModuleWidgetPrivate;

class Q_SLICER_QTMODULES_WELCOME_EXPORT qSlicerWelcomeModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerWelcomeModuleWidget(QWidget *parent=0);
  virtual ~qSlicerWelcomeModuleWidget();

protected:
  virtual void setup();

protected:
  QScopedPointer<qSlicerWelcomeModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerWelcomeModuleWidget);
  Q_DISABLE_COPY(qSlicerWelcomeModuleWidget);
};

#endif
