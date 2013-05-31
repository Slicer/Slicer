/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.


==============================================================================*/

#ifndef __qSlicerTractographyDisplayModuleWidget_h
#define __qSlicerTractographyDisplayModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerTractographyDisplayModuleWidgetsExport.h"

class qSlicerTractographyDisplayModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLFiberBundleNode;

/// \ingroup Slicer_QtModules_Volumes
class Q_SLICER_MODULE_TRACTOGRAPHYDISPLAY_WIDGETS_EXPORT qSlicerTractographyDisplayModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTractographyDisplayModuleWidget(QWidget *parent=0);
  virtual ~qSlicerTractographyDisplayModuleWidget();

  vtkMRMLFiberBundleNode fiberBundleNode()const;
  double PercentageOfFibersShown()const;

  virtual void exit();

public slots:
  void setFiberBundleNode(vtkMRMLNode*);
  void setFiberBundleNode(vtkMRMLFiberBundleNode*);
  void setPercentageOfFibersShown(double);
  void setSolidTubeColor(bool);

signals:
  void percentageOfFibersShownChanged(double);
  void currentNodeChanged(vtkMRMLNode*);
  void currentNodeChanged(vtkMRMLFiberBundleNode*);

protected:
  virtual void setup();

protected:
  QScopedPointer<qSlicerTractographyDisplayModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTractographyDisplayModuleWidget);
  Q_DISABLE_COPY(qSlicerTractographyDisplayModuleWidget);
};

#endif
