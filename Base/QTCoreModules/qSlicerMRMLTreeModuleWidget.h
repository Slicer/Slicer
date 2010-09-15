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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerMRMLTreeModuleWidget_h
#define __qSlicerMRMLTreeModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerMRMLTreeModuleWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerMRMLTreeModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerMRMLTreeModuleWidget(QWidget *parent=0);
  virtual ~qSlicerMRMLTreeModuleWidget();

protected:
  virtual void setup();

protected:
  QScopedPointer<qSlicerMRMLTreeModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerMRMLTreeModuleWidget);
  Q_DISABLE_COPY(qSlicerMRMLTreeModuleWidget);
};

#endif
