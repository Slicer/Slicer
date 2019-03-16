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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLSpinBox_h
#define __qMRMLSpinBox_h

// CTK includes
#include <ctkDoubleSpinBox.h>
#include <ctkVTKObject.h>

// qMRML includes
#include <qMRMLWidgetsExport.h>

// VTK includes
class vtkMRMLNode;
class vtkMRMLScene;

// qMRMLSpinBox includes
class qMRMLSpinBoxPrivate;

/// \class qMRMLSpinBox
/// \brief Extend the ctkDoubleSpinBox to integrate units support
///
/// This custom widgets extends the ctkDoubleSpinBox widget to integrate the
/// unit support within Slicer. By default, this widget behaves just like
/// a normal ctkDoubleSpinBox.
///
/// To use the units, one needs to set what kind of quantity this widget should
/// look for. For example, when dealing with world positions, the quantity is
/// probably going to be "length". Once a scene is set to this widget,
/// it listens to the changes made upon the selection node to extract the
/// unit properties related to its quantity and update accordingly.
///
/// To allow even more customisation, one can set which properties of the
/// spinbox are updated by units and which aren't.
///
/// \sa qMRMLSliderWidget, qMRMLCoordinatesWidget
class QMRML_WIDGETS_EXPORT qMRMLSpinBox : public ctkDoubleSpinBox
{
  Q_OBJECT
  QVTK_OBJECT

  /// Get/Set the quantity is used to determine what unit the spinbox is in.
  /// This determines the spinbox properties like minimum, maximum,
  /// single step, prefix and suffix.
  // \sa setQuantity(), quantity()
  // \sa setUnitAwareProperties(), unitAwareProperties()
  Q_PROPERTY(QString quantity READ quantity WRITE setQuantity)

  /// Get/Set the properties that will be determined by units.
  /// If a property is aware of units, it will update itself to the unit's
  /// property value automatically. Otherwise, this property is left to be
  /// changed by its accessors. All flags are on by default.
  /// \sa setQuantity(), quantity()
  // \sa setUnitAwareProperties(), unitAwareProperties()
  Q_FLAGS(UnitAwareProperty UnitAwareProperties)
  Q_PROPERTY(UnitAwareProperties unitAwareProperties READ unitAwareProperties WRITE setUnitAwareProperties)

public:
  typedef ctkDoubleSpinBox Superclass;

  /// Construct an empty qMRMLSpinBox with a null scene.
  explicit qMRMLSpinBox(QWidget* parent = nullptr);
  ~qMRMLSpinBox() override;

  enum UnitAwareProperty
    {
    None = 0x00,
    Prefix = 0x01,
    Suffix = 0x02,
    Precision = 0x04,
    MinimumValue = 0x08,
    MaximumValue = 0x10,
    Scaling = 0x20,
    };
  Q_DECLARE_FLAGS(UnitAwareProperties, UnitAwareProperty)

  /// Get MRML scene that has been set by setMRMLScene(). Default is no scene.
  /// \sa setMRMLScene()
  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;

  QString quantity()const;

  UnitAwareProperties unitAwareProperties()const;

public slots:
  void setQuantity(const QString& baseName);

  /// Set the scene the spinbox listens to.
  /// \sa mrmlScene()
  virtual void setMRMLScene(vtkMRMLScene* scene);

  void setUnitAwareProperties(UnitAwareProperties flags);

protected slots:
  void updateWidgetFromUnitNode();

protected:
  QScopedPointer<qMRMLSpinBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSpinBox);
  Q_DISABLE_COPY(qMRMLSpinBox);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(qMRMLSpinBox::UnitAwareProperties)

#endif
