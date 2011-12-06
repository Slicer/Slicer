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

#ifndef __qSlicerTractographyModuleWidget_h
#define __qSlicerTractographyModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerTractographyModuleExport.h"

class qSlicerTractographyModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_Tractography
class Q_SLICER_QTMODULES_TRACTOGRAPHY_EXPORT qSlicerTractographyModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTractographyModuleWidget(QWidget *parent=0);
  virtual ~qSlicerTractographyModuleWidget();

public slots:


protected:
  QScopedPointer<qSlicerTractographyModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerTractographyModuleWidget);
  Q_DISABLE_COPY(qSlicerTractographyModuleWidget);
};

#endif
