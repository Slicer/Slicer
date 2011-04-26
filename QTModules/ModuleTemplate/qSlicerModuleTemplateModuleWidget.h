/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerModuleTemplateModuleWidget_h
#define __qSlicerModuleTemplateModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerModuleTemplateModuleExport.h"

class qSlicerModuleTemplateModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ModuleTemplate
class Q_SLICER_QTMODULES_MODULETEMPLATE_EXPORT qSlicerModuleTemplateModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerModuleTemplateModuleWidget(QWidget *parent=0);
  virtual ~qSlicerModuleTemplateModuleWidget();

public slots:


protected:
  QScopedPointer<qSlicerModuleTemplateModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerModuleTemplateModuleWidget);
  Q_DISABLE_COPY(qSlicerModuleTemplateModuleWidget);
};

#endif
