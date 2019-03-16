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

#ifndef __qMRMLUnitWidget_h
#define __qMRMLUnitWidget_h

// CTK includes
#include <ctkVTKObject.h>

// MRML includes
class vtkMRMLNode;

// qMRML includes
#include <qMRMLWidget.h>
class qMRMLNodeComboBox;

// Qt includes
class QWidget;

// Unit includes
#include "qSlicerUnitsModuleWidgetsExport.h"
class qMRMLUnitWidgetPrivate;
class vtkSlicerUnitsLogic;

class Q_SLICER_MODULE_UNITS_WIDGETS_EXPORT qMRMLUnitWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
  /// This property controls the name of the unit node.
  /// \sa name(), setName(), nameChanged()
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
  /// This property controls the quantity of the unit node.
  /// \sa quantity(), setQuantity(), quantityChanged()
  Q_PROPERTY(QString quantity READ quantity WRITE setQuantity NOTIFY quantityChanged)
  /// This property controls the prefix of the unit node.
  /// \sa prefix(), setPrefix(), prefixChanged()
  Q_PROPERTY(QString prefix READ prefix WRITE setPrefix NOTIFY prefixChanged)
  /// This property controls the suffix of the unit node.
  /// \sa suffix(), setSuffix(), suffixChanged()
  Q_PROPERTY(QString suffix READ suffix WRITE setSuffix NOTIFY suffixChanged)
  /// This property controls the precision of the unit node.
  /// \sa precision(), setPrecision(), precisionChanged()
  Q_PROPERTY(int precision READ precision WRITE setPrecision NOTIFY precisionChanged)
  /// This property controls the minimum of the unit node.
  /// \sa minimum(), setMinimum(), minimumChanged()
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum NOTIFY minimumChanged)
  /// This property controls the maximum of the unit node.
  /// \sa maximum(), setMaximum(), maximumChanged()
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum NOTIFY maximumChanged)
  /// This property controls the displayed coefficient of the unit node
  /// \sa coefficient(), setCoefficient(), coefficientChanged()
  Q_PROPERTY(double coefficient READ coefficient WRITE setCoefficient NOTIFY coefficientChanged)
  /// This property controls the displayed offset of the unit node.
  /// \sa offset(), setOffset(), offsetChanged()
  Q_PROPERTY(double offset READ offset WRITE setOffset NOTIFY offsetChanged)

  /// Set/Get what are the current unit node displayed property by the widget.
  /// Hidden properties can still be accessed programmatically.
  /// By default, all the properties are visible.
  /// \sa editableProperties
  Q_PROPERTY(UnitProperties displayedProperties READ displayedProperties WRITE setDisplayedProperties)
  /// This property control which property is editable by the user.
  /// All the properties except quantity are editable by default.e
  /// \sa displayedProperties
  Q_PROPERTY(UnitProperties editableProperties READ editableProperties WRITE setEditableProperties)

  Q_FLAGS(UnitProperty UnitProperties)

public:
  typedef qMRMLWidget Superclass;
  qMRMLUnitWidget(QWidget *parent=nullptr);
  ~qMRMLUnitWidget() override;

  /// Return the name property value.
  /// \sa name
  QString name() const;
  /// Return the quantity property value.
  /// \sa quantity
  QString quantity() const;
  /// Return the prefix property value.
  /// \sa prefix
  QString prefix() const;
  /// Return the suffix property value.
  /// \sa suffix
  QString suffix() const;
  /// Return the precision property value.
  /// \sa precision
  int precision() const;
  /// Return the minimum property value.
  /// \sa minimum
  double minimum() const;
  /// Return the maximum property value.
  /// \sa maximum
  double maximum() const;
  /// Return the coefficient property value.
  /// \sa setCoefficient
  double coefficient() const;
  /// Return the offset property value.
  /// \sa setOffset
  double offset() const;

  enum UnitProperty
    {
    None = 0x000,
    Preset = 0x001,
    Name = 0x002,
    Quantity = 0x004,
    Precision = 0x008,
    Prefix = 0x010,
    Suffix = 0x020,
    Minimum = 0x040,
    Maximum = 0x080,
    Coefficient = 0x100,
    Offset = 0x200,
    All = 0xfff,
    };
  Q_DECLARE_FLAGS(UnitProperties, UnitProperty)

  /// Return the displayedProperties property value.
  /// \sa displayedProperties
  UnitProperties displayedProperties() const;
  /// Return the editableProperties property value.
  /// \sa editableProperties
  UnitProperties editableProperties() const;

  /// Set the units logic scene for the preset comboxes
  void setMRMLScene(vtkMRMLScene* unitsLogicScene) override;

  Q_INVOKABLE vtkMRMLNode* currentNode() const;

public slots:
  void setCurrentNode(vtkMRMLNode* unitNode);

  /// Set the name property value.
  /// \sa name
  void setName(const QString &);
  /// Set the quantity property value.
  /// \sa quantity
  void setQuantity(const QString &);
  /// Set the prefix property value.
  /// \sa prefix
  void setPrefix(const QString &);
  /// Set the suffix property value.
  /// \sa suffix
  void setSuffix(const QString &);
  /// Set the precision property value.
  /// \sa precision
  void setPrecision(int);
  /// Set the minimum property value.
  /// \sa minimum
  void setMinimum(double);
  /// Set the maximum property value.
  /// \sa maximum
  void setMaximum(double);
  /// Set the coefficient property value.
  /// \sa coefficient
  void setCoefficient(double);
  /// Set the offset property value.
  /// \sa offset
  void setOffset(double);

  /// Apply a preset to the node.
  void setUnitFromPreset(vtkMRMLNode* presetNode);

  /// Set the displayedProperties property value.
  /// \sa displayedProperties
  void setDisplayedProperties(UnitProperties properties);
  /// Set the editableProperties property value.
  /// \sa editableProperties
  void setEditableProperties(UnitProperties properties);

protected slots:
  void updateWidgetFromNode();

signals:
  void nameChanged(QString);
  void quantityChanged(QString);
  void prefixChanged(QString);
  void suffixChanged(QString);
  void precisionChanged(int);
  void minimumChanged(double);
  void maximumChanged(double);
  void coefficientChanged(double);
  void offsetChanged(double);

protected:
  QScopedPointer<qMRMLUnitWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLUnitWidget);
  Q_DISABLE_COPY(qMRMLUnitWidget);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(qMRMLUnitWidget::UnitProperties)

#endif
