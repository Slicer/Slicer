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

#ifndef __qSlicerMarkupsROIWidget_h
#define __qSlicerMarkupsROIWidget_h

// Qt includes
#include <QWidget>

// Markups widgets includes
#include "qSlicerMarkupsAdditionalOptionsWidget.h"
#include "qSlicerMarkupsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkMRMLAnnotationROINode;
class vtkMRMLNode;
class vtkMRMLMarkupsROINode;
class qSlicerMarkupsROIWidgetPrivate;

class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT qSlicerMarkupsROIWidget : public qSlicerMarkupsAdditionalOptionsWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerMarkupsAdditionalOptionsWidget Superclass;
  qSlicerMarkupsROIWidget(QWidget* parent=nullptr);
  ~qSlicerMarkupsROIWidget() override;

  /// Returns the current MRML ROI node
  vtkMRMLMarkupsROINode* mrmlROINode()const;

  /// Gets the name of the additional options widget type
  const QString getAdditionalOptionsWidgetTypeName() override { return "ROI"; }

  void setExtent(double min, double max);
  void setExtent(double minLR, double maxLR,
                 double minPA, double maxPA,
                 double minIS, double maxIS);

  /// Updates the widget based on information from MRML.
  void updateWidgetFromMRML() override;

  /// Checks whether a given node can be handled by the widget
  bool canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const override;

public slots:

  /// Set the MRML node of interest
  void setMRMLMarkupsNode(vtkMRMLMarkupsNode* node) override;

  /// Sets the vtkMRMLMarkupsNode to operate on.
  void setMRMLMarkupsNode(vtkMRMLNode* node) override;

  /// Turn on/off the visibility of the ROI node
  void setDisplayClippingBox(bool visible);

  /// Turn on/off the tracking mode of the sliders.
  /// The ROI node will be updated only when the slider handles are released.
  void setInteractiveMode(bool interactive);
signals:
  void displayClippingBoxChanged(bool);

protected slots:
  /// Internal function to update the widgets based on the ROI node
  void onMRMLNodeModified();
  /// Internal function to update the ROI node based on the sliders
  void updateROI();
  /// Internal function to update the ROIDisplay node
  void onMRMLDisplayNodeModified();
  /// Internal function to update type of ROI
  void onROITypeParameterChanged();

protected:
  qSlicerMarkupsROIWidget(qSlicerMarkupsROIWidgetPrivate &d, QWidget* parent=nullptr);
  void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsROIWidget);
  Q_DISABLE_COPY(qSlicerMarkupsROIWidget);

};

#endif
