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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __qMRMLMarkupsPlaneWidget_h
#define __qMRMLMarkupsPlaneWidget_h

// Qt includes
#include <QWidget>

// Markups widgets includes
#include "qMRMLMarkupsAbstractOptionsWidget.h"
#include "qSlicerMarkupsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkMRMLNode;
class vtkMRMLMarkupsPlaneNode;
class qMRMLMarkupsPlaneWidgetPrivate;

class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT qMRMLMarkupsPlaneWidget : public qMRMLMarkupsAbstractOptionsWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLMarkupsAbstractOptionsWidget Superclass;
  qMRMLMarkupsPlaneWidget(QWidget* parent=nullptr);
  ~qMRMLMarkupsPlaneWidget() override;

  /// Returns the current MRML Plane node
  vtkMRMLMarkupsPlaneNode* mrmlPlaneNode()const;

  /// Gets the name of the additional options widget type
  const QString className() const override { return "qMRMLMarkupsPlaneWidget"; }

  /// Checks whether a given node can be handled by the widget
  bool canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const override;

public slots:
  /// Updates the widget based on information from MRML.
  void updateWidgetFromMRML() override;

  /// Set the MRML node of interest
  void setMRMLMarkupsNode(vtkMRMLMarkupsNode* node) override;

  /// Returns an instance of the widget
  qMRMLMarkupsAbstractOptionsWidget* createInstance() const override
  {
    return new qMRMLMarkupsPlaneWidget();
  }

protected slots:
  /// Internal function to update type of Plane
  void onPlaneTypeIndexChanged();
  void onPlaneSizeModeIndexChanged();
  void onPlaneSizeSpinBoxChanged();
  void onPlaneBoundsSpinBoxChanged();
  void onNormalVisibilityCheckBoxChanged();
  void onNormalOpacitySliderChanged();

protected:
  void setup();

protected:
  QScopedPointer<qMRMLMarkupsPlaneWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLMarkupsPlaneWidget);
  Q_DISABLE_COPY(qMRMLMarkupsPlaneWidget);

};

#endif
