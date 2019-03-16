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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLROIWidget_h
#define __qMRMLROIWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

#include "qMRMLWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLROINode;
class qMRMLROIWidgetPrivate;

class QMRML_WIDGETS_EXPORT qMRMLROIWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructors
  explicit qMRMLROIWidget(QWidget* parent);
  ~qMRMLROIWidget() override;

  /// Returns the current MRML ROI node
  vtkMRMLROINode* mrmlROINode()const;

  void setExtent(double min, double max);
public slots:
  /// Set the MRML node of interest
  void setMRMLROINode(vtkMRMLROINode* node);

  /// Utility function that calls setMRMLROINode(vtkMRMLROINode*)
  /// It's useful to connect to vtkMRMLNode* signals when you are sure of
  /// the type
  void setMRMLROINode(vtkMRMLNode* node);

  /// Turn on/off the visibility of the ROI node
  void setDisplayClippingBox(bool visible);

  /// Turn on/off the tracking mode of the sliders.
  /// The ROI node will be updated only when the slider handles are released.
  void setInteractiveMode(bool interactive);

protected slots:
  /// Internal function to update the widgets based on the ROI node
  void onMRMLNodeModified();
  /// Internal function to update the ROI node based on the sliders
  void updateROI();

protected:
  QScopedPointer<qMRMLROIWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLROIWidget);
  Q_DISABLE_COPY(qMRMLROIWidget);
};

#endif
