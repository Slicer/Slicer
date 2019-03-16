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

#ifndef __qMRMLWindowLevelWidget_h
#define __qMRMLWindowLevelWidget_h

// Qt includes

// CTK includes
#include "qMRMLVolumeWidget.h"
class qMRMLWindowLevelWidgetPrivate;

class QMRML_WIDGETS_EXPORT qMRMLWindowLevelWidget
  : public qMRMLVolumeWidget
{
  Q_OBJECT

  Q_PROPERTY(ControlMode autoWindowLevel READ autoWindowLevel WRITE setAutoWindowLevel)
  Q_PROPERTY(double window READ window WRITE setWindow)
  Q_PROPERTY(double level READ level WRITE setLevel)
  Q_PROPERTY(double minimumValue READ minimumValue WRITE setMinimumValue)
  Q_PROPERTY(double maximumValue READ maximumValue WRITE setMaximumValue)
  Q_ENUMS(ControlMode)

public:
  /// Constructors
  typedef qMRMLVolumeWidget Superclass;
  explicit qMRMLWindowLevelWidget(QWidget* parentWidget = nullptr);
  ~qMRMLWindowLevelWidget() override;

  enum ControlMode
  {
    Auto = 0,
    Manual = 1,
    ManualMinMax =2
  };

  /// Set Auto/Manual mode
  void setAutoWindowLevel(ControlMode autoWindowLevel);
  ControlMode autoWindowLevel() const;

  ///
  /// Get window
  double window()const;

  ///
  /// Get level
  double level()const;

  ///
  /// Get minimum of the range
  double minimumValue()const;

  ///
  /// Get maximum of the range
  double maximumValue()const;

signals:
  ///
  /// Signal sent if the window/level value is updated
  void windowLevelValuesChanged(double window, double level);
  ///
  /// Signal sent if the auto/manual value is updated
  void autoWindowLevelValueChanged(ControlMode value);

public slots:
  /// Set Auto/Manual mode
  void setAutoWindowLevel(int autoWindowLevel);

  ///
  /// Set window
  void setWindow(double window);

  ///
  /// Set level
  void setLevel(double level);

  ///
  /// Set window/level in once
  void setWindowLevel(double window, double level);

  ///
  /// Set min/max range
  void setMinMaxRangeValue(double min, double max);
  void setMinimumValue(double min);
  void setMaximumValue(double max);

protected:
  /// Update the widget from volume display node properties.
  void updateWidgetFromMRMLDisplayNode() override;

private:
  Q_DECLARE_PRIVATE(qMRMLWindowLevelWidget);
  Q_DISABLE_COPY(qMRMLWindowLevelWidget);
};

#endif
