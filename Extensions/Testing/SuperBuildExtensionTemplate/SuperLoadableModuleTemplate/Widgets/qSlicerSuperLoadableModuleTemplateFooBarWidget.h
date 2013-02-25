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

#ifndef __qSlicerSuperLoadableModuleTemplateFooBarWidget_h
#define __qSlicerSuperLoadableModuleTemplateFooBarWidget_h

// Qt includes
#include <QWidget>

// FooBar Widgets includes
#include "qSlicerSuperLoadableModuleTemplateModuleWidgetsExport.h"

class qSlicerSuperLoadableModuleTemplateFooBarWidgetPrivate;

/// \ingroup Slicer_QtModules_SuperLoadableModuleTemplate
class Q_SLICER_MODULE_SUPERLOADABLEMODULETEMPLATE_WIDGETS_EXPORT qSlicerSuperLoadableModuleTemplateFooBarWidget
  : public QWidget
{
  Q_OBJECT
public:
  typedef QWidget Superclass;
  qSlicerSuperLoadableModuleTemplateFooBarWidget(QWidget *parent=0);
  virtual ~qSlicerSuperLoadableModuleTemplateFooBarWidget();

protected slots:

protected:
  QScopedPointer<qSlicerSuperLoadableModuleTemplateFooBarWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSuperLoadableModuleTemplateFooBarWidget);
  Q_DISABLE_COPY(qSlicerSuperLoadableModuleTemplateFooBarWidget);
};

#endif
