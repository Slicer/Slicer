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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerScalarOverlayIOOptionsWidget_h
#define __qSlicerScalarOverlayIOOptionsWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerIOOptionsWidget.h"
#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerScalarOverlayIOOptionsWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerScalarOverlayIOOptionsWidget :
  public qSlicerIOOptionsWidget
{
  Q_OBJECT
public:
  typedef qSlicerIOOptionsWidget Superclass;
  qSlicerScalarOverlayIOOptionsWidget(QWidget *parent=0);
  virtual ~qSlicerScalarOverlayIOOptionsWidget();

  virtual bool isValid()const;

protected slots:
  void updateProperties();

protected:
  QScopedPointer<qSlicerScalarOverlayIOOptionsWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScalarOverlayIOOptionsWidget);
  Q_DISABLE_COPY(qSlicerScalarOverlayIOOptionsWidget);
};

#endif
