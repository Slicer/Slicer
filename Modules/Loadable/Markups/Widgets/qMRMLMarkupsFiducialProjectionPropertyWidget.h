/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 5R42CA137886
  It was then updated for the Markups module by Nicole Aucoin, BWH.

==============================================================================*/

#ifndef __qMRMLMarkupsFiducialProjectionPropertyWidget_h
#define __qMRMLMarkupsFiducialProjectionPropertyWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Slicer includes
#include "qMRMLWidget.h"

#include "qSlicerMarkupsModuleWidgetsExport.h"

class qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate;
class vtkMRMLMarkupsDisplayNode;
class vtkMRMLMarkupsNode;

/// \ingroup Slicer_QtModules_Markups
class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT
qMRMLMarkupsFiducialProjectionPropertyWidget
  : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  qMRMLMarkupsFiducialProjectionPropertyWidget(QWidget *newParent = nullptr);
  ~qMRMLMarkupsFiducialProjectionPropertyWidget() override;

public slots:
  void setMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode);
  void setMRMLMarkupsDisplayNode(vtkMRMLMarkupsDisplayNode* markupsDisplayNode);
  void setProjectionVisibility(bool showProjection);
  void setProjectionColor(QColor newColor);
  void setUseFiducialColor(bool useFiducialColor);
  void setOutlinedBehindSlicePlane(bool outlinedBehind);
  void setProjectionOpacity(double opacity);

protected slots:
  void updateWidgetFromDisplayNode();

protected:
  QScopedPointer<qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLMarkupsFiducialProjectionPropertyWidget);
  Q_DISABLE_COPY(qMRMLMarkupsFiducialProjectionPropertyWidget);

};

#endif
