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

#ifndef __qMRMLMarkupsROIWidget_h
#define __qMRMLMarkupsROIWidget_h

// Markups widgets includes
#include "qMRMLMarkupsAbstractOptionsWidget.h"
#include "qSlicerMarkupsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// ------------------------------------------------------------------------------
class vtkMRMLNode;
class vtkMRMLMarkupsROINode;
class qMRMLMarkupsROIWidgetPrivate;

// ------------------------------------------------------------------------------
class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT qMRMLMarkupsROIWidget
: public qMRMLMarkupsAbstractOptionsWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLMarkupsAbstractOptionsWidget Superclass;
  qMRMLMarkupsROIWidget(QWidget* parent=nullptr);
  ~qMRMLMarkupsROIWidget() override;

  /// Returns the current MRML ROI node
  vtkMRMLMarkupsROINode* mrmlROINode()const;

  void setExtent(double min, double max);
  void setExtent(double minLR, double maxLR,
                 double minPA, double maxPA,
                 double minIS, double maxIS);

  /// Gets the name of the additional options widget type
  const QString className() const override {return "qMRMLMarkupsROIWidget";}

  /// Checks whether a given node can be handled by the widget
  bool canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const override;

  /// Get the inside out state.
  bool insideOut();

public slots:
  /// Turn on/off the visibility of the ROI node
  void setDisplayClippingBox(bool visible);

  /// Turn on/off the tracking mode of the sliders.
  /// The ROI node will be updated only when the slider handles are released.
  void setInteractiveMode(bool interactive);

  /// Turn on/off inside out state.
  void setInsideOut(bool insideOut);

  /// Updates the widget on MRML changes
  void updateWidgetFromMRML() override;

  /// Set the MRML node of interest
  void setMRMLMarkupsNode(vtkMRMLMarkupsNode* node) override;

  /// Returns an instance of the widget
  qMRMLMarkupsAbstractOptionsWidget* createInstance() const override
  { return new qMRMLMarkupsROIWidget(); }

signals:
  void displayClippingBoxChanged(bool);

protected slots:
  /// Internal function to update the ROI node based on the sliders
  void updateROI();
  /// Internal function to update the ROIDisplay node
  void onMRMLDisplayNodeModified();
  /// Internal function to update type of ROI
  void onROITypeParameterChanged();

protected:
  void setup();

protected:
  QScopedPointer<qMRMLMarkupsROIWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLMarkupsROIWidget);
  Q_DISABLE_COPY(qMRMLMarkupsROIWidget);
};

#endif
