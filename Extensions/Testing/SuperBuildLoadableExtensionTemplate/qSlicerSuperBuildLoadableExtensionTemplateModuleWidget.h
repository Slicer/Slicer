/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerSuperBuildLoadableExtensionTemplateModuleWidget_h
#define __qSlicerSuperBuildLoadableExtensionTemplateModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerSuperBuildLoadableExtensionTemplateModuleExport.h"

class qSlicerSuperBuildLoadableExtensionTemplateModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_SUPERBUILDLOADABLEEXTENSIONTEMPLATE_EXPORT qSlicerSuperBuildLoadableExtensionTemplateModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerSuperBuildLoadableExtensionTemplateModuleWidget(QWidget *parent=0);
  virtual ~qSlicerSuperBuildLoadableExtensionTemplateModuleWidget();

public slots:


protected:
  QScopedPointer<qSlicerSuperBuildLoadableExtensionTemplateModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerSuperBuildLoadableExtensionTemplateModuleWidget);
  Q_DISABLE_COPY(qSlicerSuperBuildLoadableExtensionTemplateModuleWidget);
};

#endif
