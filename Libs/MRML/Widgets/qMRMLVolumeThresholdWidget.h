/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qMRMLVolumeThresholdWidget_h
#define __qMRMLVolumeThresholdWidget_h

/// CTK includes
#include "qMRMLVolumeWidget.h"
class qMRMLVolumeThresholdWidgetPrivate;

class QMRML_WIDGETS_EXPORT qMRMLVolumeThresholdWidget
  : public qMRMLVolumeWidget
{
  Q_OBJECT
  Q_PROPERTY(int  autoThreshold READ autoThreshold WRITE setAutoThreshold)
  Q_PROPERTY(double lowerThreshold READ lowerThreshold WRITE setLowerThreshold)
  Q_PROPERTY(double upperThreshold READ upperThreshold WRITE setUpperThreshold)

public:
  /// Constructors
  typedef qMRMLVolumeWidget Superclass;
  explicit qMRMLVolumeThresholdWidget(QWidget* parent=nullptr);
  ~qMRMLVolumeThresholdWidget() override;

  enum ControlMode
  {
    Auto = 0,
    Manual = 1,
    Off =2
  };

  ControlMode autoThreshold() const;
  void setAutoThreshold(ControlMode autoWindowLevel);

  /// Is the thresholding activated
  bool isOff()const;

  ///
  /// Get lowerThreshold
  double lowerThreshold()const;

  ///
  /// Get upperThreshold
  double upperThreshold()const;

signals:
  ///
  /// Signal sent if the lowerThreshold/upperThreshold value is updated
  void thresholdValuesChanged(double lowerThreshold, double upperThreshold);
  ///
  /// Signal sent if the auto/manual value is updated
  void autoThresholdValueChanged(int value);

public slots:
  /// Set Auto/Manual mode
  void setAutoThreshold(int autoThreshold);

  ///
  /// Set lowerThreshold
  void setLowerThreshold(double lowerThreshold);

  ///
  /// Set upperThreshold
  void setUpperThreshold(double upperThreshold);

  ///
  /// Set lowerThreshold/upperThreshold in once
  void setThreshold(double lowerThreshold, double upperThreshold);

protected:
  /// Update the widget from volume display node properties.
  void updateWidgetFromMRMLDisplayNode() override;

  ///
  /// Set sliders range
  void setMinimum(double min);
  void setMaximum(double max);

private:
  Q_DECLARE_PRIVATE(qMRMLVolumeThresholdWidget);
  Q_DISABLE_COPY(qMRMLVolumeThresholdWidget);
};

#endif
