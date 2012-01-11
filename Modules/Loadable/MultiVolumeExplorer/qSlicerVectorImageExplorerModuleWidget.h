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

#ifndef __qSlicerVectorImageExplorerModuleWidget_h
#define __qSlicerVectorImageExplorerModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVectorImageExplorerModuleExport.h"

class qSlicerVectorImageExplorerModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_VECTORIMAGEEXPLORER_EXPORT qSlicerVectorImageExplorerModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVectorImageExplorerModuleWidget(QWidget *parent=0);
  virtual ~qSlicerVectorImageExplorerModuleWidget();

public slots:


protected:
  QScopedPointer<qSlicerVectorImageExplorerModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerVectorImageExplorerModuleWidget);
  Q_DISABLE_COPY(qSlicerVectorImageExplorerModuleWidget);
};

#endif
