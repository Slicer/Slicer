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

// Slicer includes
#include "qSlicerIOOptionsWidget.h"
#include "qSlicerModelsModuleExport.h"

class qSlicerScalarOverlayIOOptionsWidgetPrivate;

class Q_SLICER_QTMODULES_MODELS_EXPORT qSlicerScalarOverlayIOOptionsWidget
  : public qSlicerIOOptionsWidget
{
  Q_OBJECT
public:
  typedef qSlicerIOOptionsWidget Superclass;
  qSlicerScalarOverlayIOOptionsWidget(QWidget *parent=nullptr);
  ~qSlicerScalarOverlayIOOptionsWidget() override;

  bool isValid()const override;

protected slots:
  void updateProperties();

private:
  Q_DECLARE_PRIVATE_D(qGetPtrHelper(qSlicerIOOptions::d_ptr), qSlicerScalarOverlayIOOptionsWidget);
  Q_DISABLE_COPY(qSlicerScalarOverlayIOOptionsWidget);
};

#endif
