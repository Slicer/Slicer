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

#ifndef __qSlicerExtensionTemplateModuleWidget_h
#define __qSlicerExtensionTemplateModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerExtensionTemplateModuleExport.h"

class qSlicerExtensionTemplateModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_EXTENSIONTEMPLATE_EXPORT qSlicerExtensionTemplateModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerExtensionTemplateModuleWidget(QWidget *parent=0);
  virtual ~qSlicerExtensionTemplateModuleWidget();

public slots:


protected:
  QScopedPointer<qSlicerExtensionTemplateModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionTemplateModuleWidget);
  Q_DISABLE_COPY(qSlicerExtensionTemplateModuleWidget);
};

#endif
