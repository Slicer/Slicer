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

#ifndef __qSlicerMarkupsSettingsPanel_h
#define __qSlicerMarkupsSettingsPanel_h

// CTK includes
#include <ctkVTKObject.h>
#include <ctkSettingsPanel.h>

// Markups includes
#include "qSlicerMarkupsModuleExport.h"
class qSlicerMarkupsSettingsPanelPrivate;
class vtkSlicerMarkupsLogic;

class Q_SLICER_QTMODULES_MARKUPS_EXPORT qSlicerMarkupsSettingsPanel
  : public ctkSettingsPanel
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(QString defaultGlyphType READ defaultGlyphType WRITE setDefaultGlyphType NOTIFY defaultGlyphTypeChanged)
  Q_PROPERTY(QColor defaultUnselectedColor READ defaultUnselectedColor WRITE setDefaultUnselectedColor NOTIFY defaultUnselectedColorChanged)
  Q_PROPERTY(QColor defaultSelectedColor READ defaultSelectedColor WRITE setDefaultSelectedColor NOTIFY defaultSelectedColorChanged)
  Q_PROPERTY(double defaultGlyphScale READ defaultGlyphScale WRITE setDefaultGlyphScale NOTIFY defaultGlyphScaleChanged)
  Q_PROPERTY(double defaultTextScale READ defaultTextScale WRITE setDefaultTextScale NOTIFY defaultTextScaleChanged)
  Q_PROPERTY(double defaultOpacity READ defaultOpacity WRITE setDefaultOpacity NOTIFY defaultOpacityChanged)

public:
  /// Superclass typedef
  typedef ctkSettingsPanel Superclass;

  /// Constructor
  explicit qSlicerMarkupsSettingsPanel(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerMarkupsSettingsPanel() override;

  /// Markups logic is synchronized with the settings.
  /// \sa vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeGlyphType
  /// \sa vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeGlyphScale
  /// \sa vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeTextScale
  /// \sa vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeOpacity
  /// \sa vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeColor
  /// \sa vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSelectedColor
  void setMarkupsLogic(vtkSlicerMarkupsLogic* logic);
  vtkSlicerMarkupsLogic* markupsLogic()const;

  QString defaultGlyphType() const;
  QColor defaultUnselectedColor() const;
  QColor defaultSelectedColor() const;
  double defaultGlyphScale() const;
  double defaultTextScale() const;
  double defaultOpacity() const;

public slots:
  void setDefaultGlyphType(const QString& type);
  void setDefaultUnselectedColor(const QColor color);
  void setDefaultSelectedColor(const QColor color);
  void setDefaultGlyphScale(const double scale);
  void setDefaultTextScale(const double scale);
  void setDefaultOpacity(const double scale);

signals:
  void defaultGlyphTypeChanged(const QString&);
  void defaultUnselectedColorChanged(QColor);
  void defaultSelectedColorChanged(QColor);
  void defaultGlyphScaleChanged(const double);
  void defaultTextScaleChanged(const double);
  void defaultOpacityChanged(const double);

protected slots:
  void onMarkupsLogicModified();

  void onDefaultGlyphTypeChanged(int);
  void updateMarkupsLogicDefaultGlyphType();

  void onDefaultSelectedColorChanged(QColor);
  void updateMarkupsLogicDefaultSelectedColor();

  void onDefaultUnselectedColorChanged(QColor);
  void updateMarkupsLogicDefaultUnselectedColor();

  void onDefaultGlyphScaleChanged(double);
  void updateMarkupsLogicDefaultGlyphScale();

  void onDefaultTextScaleChanged(double);
  void updateMarkupsLogicDefaultTextScale();

  void onDefaultOpacityChanged(double);
  void updateMarkupsLogicDefaultOpacity();

protected:
  QScopedPointer<qSlicerMarkupsSettingsPanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsSettingsPanel);
  Q_DISABLE_COPY(qSlicerMarkupsSettingsPanel);
};

#endif
