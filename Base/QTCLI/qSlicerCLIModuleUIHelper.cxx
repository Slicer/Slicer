/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QDebug>
#include <QCheckBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QToolButton>
#include <QSizePolicy>

// CTK includes
#include <ctkDirectoryButton.h>
#include <ctkFlowLayout.h>
#include <ctkPathLineEdit.h>
#include <ctkSliderWidget.h>
#include <ctkDoubleSpinBox.h>

// qMRML includes
#include <qMRMLNodeComboBox.h>

/// ModuleDescriptionParser includes

// MRML includes
#include <vtkMRMLCommandLineModuleNode.h>
#include <vtkMRMLScene.h>

// ITK includes

// Slicer includes
#include "qSlicerCLIModuleUIHelper.h"
#include "qSlicerCLIModuleWidget.h"

/// STD includes
#include <limits>

//-----------------------------------------------------------------------------
qSlicerWidgetValueWrapper::qSlicerWidgetValueWrapper(const QString& _name, const QString& _label, QObject* parentObject)
  : QObject(parentObject)
  , Name(_name)
  , Label(_label)
{
}

//-----------------------------------------------------------------------------
qSlicerWidgetValueWrapper::~qSlicerWidgetValueWrapper() = default;

//-----------------------------------------------------------------------------
#define WIDGET_VALUE_WRAPPER(_NAME, _WIDGET, _GETTER, _SETTER, _CONVERTER, _NOTIFY)           \
  namespace                                                                                   \
  {                                                                                           \
  class _NAME##WidgetValueWrapper : public qSlicerWidgetValueWrapper                          \
  {                                                                                           \
  public:                                                                                     \
    _NAME##WidgetValueWrapper(const QString& _name, const QString& _label, _WIDGET* _widget)  \
      : qSlicerWidgetValueWrapper(_name, _label, _widget)                                     \
    {                                                                                         \
      Q_ASSERT(_widget);                                                                      \
      this->Widget = _widget;                                                                 \
      this->connect(this->Widget, SIGNAL(_NOTIFY), this, SIGNAL(valueChanged()));             \
    }                                                                                         \
    virtual QVariant value()                                                                  \
    {                                                                                         \
      QVariant _value(this->Widget->_GETTER());                                               \
      return _value;                                                                          \
    }                                                                                         \
    virtual void setValue(const QString& _value)                                              \
    {                                                                                         \
      /* setting a value have side effects (such as caret moving to the end of the widget) */ \
      /* so only modify a widget if the value is actually changed */                          \
      if (this->Widget->_GETTER() != qSlicerWidgetValueWrapper::to##_CONVERTER(_value))       \
      {                                                                                       \
        this->Widget->_SETTER(qSlicerWidgetValueWrapper::to##_CONVERTER(_value));             \
      }                                                                                       \
    }                                                                                         \
    _WIDGET* Widget;                                                                          \
  };                                                                                          \
  }

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(IntegerWithoutConstraints, QSpinBox, value, setValue, Int, valueChanged(int));
WIDGET_VALUE_WRAPPER(IntegerWithConstraints, ctkSliderWidget, value, setValue, Int, valueChanged(double));
WIDGET_VALUE_WRAPPER(Boolean, QCheckBox, isChecked, setChecked, Bool, toggled(bool));
WIDGET_VALUE_WRAPPER(FloatWithoutConstraints, ctkDoubleSpinBox, value, setValue, Double, valueChanged(double));
WIDGET_VALUE_WRAPPER(FloatWithConstraints, ctkSliderWidget, value, setValue, Double, valueChanged(double));
WIDGET_VALUE_WRAPPER(DoubleWithoutConstraints, ctkDoubleSpinBox, value, setValue, Double, valueChanged(double));
WIDGET_VALUE_WRAPPER(DoubleWithConstraints, ctkSliderWidget, value, setValue, Double, valueChanged(double));
WIDGET_VALUE_WRAPPER(String, QLineEdit, text, setText, String, textChanged(const QString&));
WIDGET_VALUE_WRAPPER(Point, qMRMLNodeComboBox, currentNodeID, setCurrentNodeID, String, currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(PointFile,
                     qMRMLNodeComboBox,
                     currentNodeID,
                     setCurrentNodeID,
                     String,
                     currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(Region, qMRMLNodeComboBox, currentNodeID, setCurrentNodeID, String, currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(Image, qMRMLNodeComboBox, currentNodeID, setCurrentNodeID, String, currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(Geometry,
                     qMRMLNodeComboBox,
                     currentNodeID,
                     setCurrentNodeID,
                     String,
                     currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(Table, qMRMLNodeComboBox, currentNodeID, setCurrentNodeID, String, currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(Transform,
                     qMRMLNodeComboBox,
                     currentNodeID,
                     setCurrentNodeID,
                     String,
                     currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(Directory, ctkDirectoryButton, directory, setDirectory, String, directoryChanged(QString));
WIDGET_VALUE_WRAPPER(File, ctkPathLineEdit, currentPath, setCurrentPath, String, currentPathChanged(QString));
WIDGET_VALUE_WRAPPER(Enumeration, ButtonGroupWidgetWrapper, checkedValue, setCheckedValue, String, valueChanged());
WIDGET_VALUE_WRAPPER(Measurement,
                     qMRMLNodeComboBox,
                     currentNodeID,
                     setCurrentNodeID,
                     String,
                     currentNodeIDChanged(QString));

//-----------------------------------------------------------------------------
#define INSTANCIATE_WIDGET_VALUE_WRAPPER(_NAME, _PARAM_NAME, _LABEL, _WIDGET_INSTANCE)                       \
  qSlicerWidgetValueWrapper* wrapper = new _NAME##WidgetValueWrapper(_PARAM_NAME, _LABEL, _WIDGET_INSTANCE); \
  Q_Q(qSlicerCLIModuleUIHelper);                                                                             \
  QObject::connect(wrapper, SIGNAL(valueChanged()), q, SLOT(onValueChanged()));                              \
  this->WidgetValueWrappers.push_back(wrapper);

//-----------------------------------------------------------------------------
ButtonGroupWidgetWrapper::ButtonGroupWidgetWrapper(QWidget* _parent)
  : QWidget(_parent)
{
  this->ButtonGroup = new QButtonGroup(this);
  this->connect(this->ButtonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(valueChanged()));
}

//-----------------------------------------------------------------------------
QButtonGroup* ButtonGroupWidgetWrapper::buttonGroup() const
{
  return this->ButtonGroup;
}

//-----------------------------------------------------------------------------
QString ButtonGroupWidgetWrapper::checkedValue()
{
  QAbstractButton* button = this->ButtonGroup->checkedButton();
  if (button)
  {
    return button->property("EnumeratedValue").toString();
  }
  else
  {
    // Handle case where <string-enumeration> has no <default> element
    return QString("");
  }
}

//-----------------------------------------------------------------------------
void ButtonGroupWidgetWrapper::setCheckedValue(const QString& value)
{
  foreach (QAbstractButton* button, this->ButtonGroup->buttons())
  {
    if (button->property("EnumeratedValue").toString() == value)
    {
      button->setChecked(true);
      break;
    }
  }
}

//-----------------------------------------------------------------------------
class qSlicerCLIModuleUIHelperPrivate
{
  Q_DECLARE_PUBLIC(qSlicerCLIModuleUIHelper);

protected:
  qSlicerCLIModuleUIHelper* const q_ptr;

public:
  typedef qSlicerCLIModuleUIHelperPrivate Self;
  qSlicerCLIModuleUIHelperPrivate(qSlicerCLIModuleUIHelper& object);

  /// Convenient typedefs
  typedef std::vector<std::string>::const_iterator ElementConstIterator;
  typedef std::vector<std::string>::iterator ElementIterator;

  ///
  /// Create widget corresponding to the different parameters
  QWidget* createIntegerTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createBooleanTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createFloatTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createDoubleTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createStringTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createPointTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createPointFileTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createRegionTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createImageTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createGeometryTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createTableTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createTransformTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createDirectoryTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createFileTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createEnumerationTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createMeasurementTagWidget(const ModuleParameter& moduleParameter);

  /// Return true if the None option of the widget associated with the parameter should
  /// be enabled
  static bool isNoneEnabled(const ModuleParameter& moduleParameter);

  static bool isOutputChannel(const ModuleParameter& moduleParameter);

  ///
  /// Convenient method allowing to retrieve the node type associated
  /// with the parameter type
  static QString nodeTypeFromMap(const QHash<QString, QString>& map,
                                 const QString& attribute,
                                 const QString& defaultValue = QString());

  ///
  /// Initialize the maps containing the mapping
  /// parameter type -> MRML node type (classname)
  static void initializeMaps();

  /// Map used to store the different relation
  ///  parameter type -> MRML node type
  static bool MapInitialized;
  static QHash<QString, QString> ImageTypeAttributeToNodeType;
  static QHash<QString, QString> PointTypeAttributeToNodeType;
  static QHash<QString, QString> GeometryTypeAttributeToNodeType;
  static QHash<QString, QString> TableTypeAttributeToNodeType;
  static QHash<QString, QString> TransformTypeAttributeToNodeType;

  /// List of wrapper allowing to update the CommandLineModuleNode
  QList<qSlicerWidgetValueWrapper*> WidgetValueWrappers;

  /// Pointer to the associated Command Line module widget
  qSlicerCLIModuleWidget* CLIModuleWidget;
};

//-----------------------------------------------------------------------------
// qSlicerCLIModuleUIHelperPrivate methods

//-----------------------------------------------------------------------------
bool qSlicerCLIModuleUIHelperPrivate::MapInitialized = false;
QHash<QString, QString> qSlicerCLIModuleUIHelperPrivate::ImageTypeAttributeToNodeType;
QHash<QString, QString> qSlicerCLIModuleUIHelperPrivate::PointTypeAttributeToNodeType;
QHash<QString, QString> qSlicerCLIModuleUIHelperPrivate::GeometryTypeAttributeToNodeType;
QHash<QString, QString> qSlicerCLIModuleUIHelperPrivate::TableTypeAttributeToNodeType;
QHash<QString, QString> qSlicerCLIModuleUIHelperPrivate::TransformTypeAttributeToNodeType;

//-----------------------------------------------------------------------------
qSlicerCLIModuleUIHelperPrivate::qSlicerCLIModuleUIHelperPrivate(qSlicerCLIModuleUIHelper& object)
  : q_ptr(&object)
{
  this->CLIModuleWidget = nullptr;
  Self::initializeMaps();
}

//-----------------------------------------------------------------------------
QString qSlicerCLIModuleUIHelperPrivate::nodeTypeFromMap(const QHash<QString, QString>& map,
                                                         const QString& attribute,
                                                         const QString& defaultValue)
{
  QHash<QString, QString>::const_iterator i = map.constFind(attribute);

  if (i == map.constEnd())
  {
    return defaultValue;
  }
  return i.value();
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleUIHelperPrivate::initializeMaps()
{
  if (Self::MapInitialized)
  {
    return;
  }

  // Image type attribute mapping
  Self::ImageTypeAttributeToNodeType["scalar"] = "vtkMRMLScalarVolumeNode";
  Self::ImageTypeAttributeToNodeType["label"] = "vtkMRMLLabelMapVolumeNode";
  Self::ImageTypeAttributeToNodeType["vector"] = "vtkMRMLVectorVolumeNode";
  Self::ImageTypeAttributeToNodeType["tensor"] = "vtkMRMLDiffusionTensorVolumeNode";
  Self::ImageTypeAttributeToNodeType["diffusion-weighted"] = "vtkMRMLDiffusionWeightedVolumeNode";
  Self::ImageTypeAttributeToNodeType["signal"] = "vtkMRMLMultiVolumeNode";
  Self::ImageTypeAttributeToNodeType["multichannel"] = "vtkMRMLMultiVolumeNode";
  Self::ImageTypeAttributeToNodeType["dynamic-contrast-enhanced"] = "vtkMRMLMultiVolumeNode";

  // Markups type attribute mapping
  Self::PointTypeAttributeToNodeType["point"] = "vtkMRMLMarkupsFiducialNode";
  Self::PointTypeAttributeToNodeType["line"] = "vtkMRMLMarkupsLineNode";
  Self::PointTypeAttributeToNodeType["angle"] = "vtkMRMLMarkupsAngleNode";
  Self::PointTypeAttributeToNodeType["curve"] = "vtkMRMLMarkupsCurveNode";
  Self::PointTypeAttributeToNodeType["closedcurve"] = "vtkMRMLMarkupsClosedCurveNode";

  // Geometry type attribute mapping
  Self::GeometryTypeAttributeToNodeType["fiberbundle"] = "vtkMRMLFiberBundleNode";
  Self::GeometryTypeAttributeToNodeType["model"] = "vtkMRMLModelNode";

  // Table type attribute mapping
  Self::TableTypeAttributeToNodeType["color"] = "vtkMRMLColorNode";

  // Table type attribute mapping
  Self::TransformTypeAttributeToNodeType["linear"] = "vtkMRMLLinearTransformNode";
  Self::TransformTypeAttributeToNodeType["nonlinear"] = "vtkMRMLGridTransformNode";
  Self::TransformTypeAttributeToNodeType["bspline"] = "vtkMRMLBSplineTransformNode";

  Self::MapInitialized = true;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createIntegerTagWidget(const ModuleParameter& moduleParameter)
{
  if (isOutputChannel(moduleParameter))
  {
    return createStringTagWidget(moduleParameter);
  }

  int value = QString::fromStdString(moduleParameter.GetValue()).toInt();
  int step = 1;
  // Since, ctkDoubleSlider checks that MIN_INT < doubleValue / this->SingleStep < MAX_INT
  // Assuming the singlestep won't be smaller than 0.01, the min/max range set from the helper
  // is divided by 100.
  int min = std::numeric_limits<int>::min() / 100;
  int max = std::numeric_limits<int>::max() / 100;
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();
  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());

  QWidget* widget = nullptr;
  if (!withConstraints)
  {
    QSpinBox* spinBox = new QSpinBox;
    spinBox->setSingleStep(step);
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    widget = spinBox;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(IntegerWithoutConstraints, _name, _label, spinBox);
  }
  else
  {
    QString minAsStr = QString::fromStdString(moduleParameter.GetMinimum());
    if (!minAsStr.isEmpty())
    {
      min = minAsStr.toInt();
    }

    QString maxAsStr = QString::fromStdString(moduleParameter.GetMaximum());
    if (!maxAsStr.isEmpty())
    {
      max = maxAsStr.toInt();
    }

    QString stepAsStr = QString::fromStdString(moduleParameter.GetStep());
    if (!stepAsStr.isEmpty())
    {
      step = stepAsStr.toInt();
    }

    ctkSliderWidget* slider = new ctkSliderWidget;
    slider->setDecimals(0);
    slider->setSingleStep(step);
    slider->setTickInterval(step);
    slider->setRange(min, max);
    slider->setValue(value);
    widget = slider;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(IntegerWithConstraints, _name, _label, slider);
  }

  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createBooleanTagWidget(const ModuleParameter& moduleParameter)
{
  if (isOutputChannel(moduleParameter))
  {
    return createStringTagWidget(moduleParameter);
  }

  QString valueAsStr = QString::fromStdString(moduleParameter.GetValue());
  QCheckBox* widget = new QCheckBox;
  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  widget->setChecked(valueAsStr == "true");
  INSTANCIATE_WIDGET_VALUE_WRAPPER(Boolean, _name, _label, widget);
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createFloatTagWidget(const ModuleParameter& moduleParameter)
{
  if (isOutputChannel(moduleParameter))
  {
    return createStringTagWidget(moduleParameter);
  }

  QString valueAsStr = QString::fromStdString(moduleParameter.GetValue());
  float value = valueAsStr.toFloat();
  float step = 0.1;
  // Since, ctkDoubleSlider checks that MIN_INT < doubleValue / this->SingleStep < MAX_INT
  // Assuming the singlestep won't be smaller than 0.01, the min/max range set from the helper
  // is divided by 100.
  float min = -std::numeric_limits<float>::max() / 100;
  float max = std::numeric_limits<float>::max() / 100;
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();
  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  int decimals = valueAsStr.indexOf('.') != -1 ? valueAsStr.length() - valueAsStr.indexOf('.') - 1 : 2;

  QWidget* widget = nullptr;
  if (!withConstraints)
  {
    ctkDoubleSpinBox* spinBox = new ctkDoubleSpinBox;
    spinBox->setDecimals(decimals);
    spinBox->setDecimalsOption(ctkDoubleSpinBox::DecimalsByKey | ctkDoubleSpinBox::DecimalsByShortcuts);
    spinBox->setSingleStep(step);
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    widget = spinBox;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(FloatWithoutConstraints, _name, _label, spinBox);
  }
  else
  {
    QString minAsStr = QString::fromStdString(moduleParameter.GetMinimum());
    if (!minAsStr.isEmpty())
    {
      min = minAsStr.toFloat();
      if (minAsStr.indexOf('.') != -1)
      {
        decimals = qMax(decimals, minAsStr.length() - minAsStr.indexOf('.') - 1);
      }
    }

    QString maxAsStr = QString::fromStdString(moduleParameter.GetMaximum());
    if (!maxAsStr.isEmpty())
    {
      max = maxAsStr.toFloat();
      if (maxAsStr.indexOf('.') != -1)
      {
        decimals = qMax(decimals, maxAsStr.length() - maxAsStr.indexOf('.') - 1);
      }
    }

    QString stepAsStr = QString::fromStdString(moduleParameter.GetStep());
    if (!stepAsStr.isEmpty())
    {
      step = stepAsStr.toFloat();
      if (stepAsStr.indexOf('.') != -1)
      {
        decimals = qMax(decimals, stepAsStr.length() - stepAsStr.indexOf('.') - 1);
      }
    }

    ctkSliderWidget* slider = new ctkSliderWidget;
    slider->setDecimals(decimals);
    slider->spinBox()->setDecimalsOption(ctkDoubleSpinBox::DecimalsByKey | ctkDoubleSpinBox::DecimalsByShortcuts);
    slider->setTickInterval(step);
    slider->setSingleStep(step);
    slider->setRange(min, max);
    slider->setValue(value);
    widget = slider;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(FloatWithConstraints, _name, _label, slider);
  }
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createDoubleTagWidget(const ModuleParameter& moduleParameter)
{
  if (isOutputChannel(moduleParameter))
  {
    return createStringTagWidget(moduleParameter);
  }

  QString valueAsStr = QString::fromStdString(moduleParameter.GetValue());
  double value = valueAsStr.toDouble();
  double step = 0.1;
  // Since, ctkDoubleSlider checks that MIN_INT < doubleValue / this->SingleStep < MAX_INT
  // Assuming the singlestep won't be smaller than 0.01, the min/max range set from the helper
  // is divided by 100.
  double min = -std::numeric_limits<double>::max() / 100;
  double max = std::numeric_limits<double>::max() / 100;
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();
  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  int decimals = valueAsStr.indexOf('.') != -1 ? valueAsStr.length() - valueAsStr.indexOf('.') - 1 : 2;

  QWidget* widget = nullptr;
  if (!withConstraints)
  {
    ctkDoubleSpinBox* spinBox = new ctkDoubleSpinBox;
    spinBox->setDecimals(decimals);
    spinBox->setDecimalsOption(ctkDoubleSpinBox::DecimalsByKey | ctkDoubleSpinBox::DecimalsByShortcuts);
    spinBox->setSingleStep(step);
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    widget = spinBox;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(DoubleWithoutConstraints, _name, _label, spinBox);
  }
  else
  {
    QString minAsStr = QString::fromStdString(moduleParameter.GetMinimum());
    if (!minAsStr.isEmpty())
    {
      min = minAsStr.toFloat();
      if (minAsStr.indexOf('.') != -1)
      {
        decimals = qMax(decimals, minAsStr.length() - minAsStr.indexOf('.') - 1);
      }
    }

    QString maxAsStr = QString::fromStdString(moduleParameter.GetMaximum());
    if (!maxAsStr.isEmpty())
    {
      max = maxAsStr.toFloat();
      if (maxAsStr.indexOf('.') != -1)
      {
        decimals = qMax(decimals, maxAsStr.length() - maxAsStr.indexOf('.') - 1);
      }
    }

    QString stepAsStr = QString::fromStdString(moduleParameter.GetStep());
    if (!stepAsStr.isEmpty())
    {
      step = stepAsStr.toFloat();
      if (stepAsStr.indexOf('.') != -1)
      {
        decimals = qMax(decimals, stepAsStr.length() - stepAsStr.indexOf('.') - 1);
      }
    }
    ctkSliderWidget* slider = new ctkSliderWidget;
    slider->setDecimals(decimals);
    slider->spinBox()->setDecimalsOption(ctkDoubleSpinBox::DecimalsByKey | ctkDoubleSpinBox::DecimalsByShortcuts);
    slider->setSingleStep(step);
    slider->setTickInterval(step);
    slider->setRange(min, max);
    slider->setValue(value);
    widget = slider;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(DoubleWithConstraints, _name, _label, slider);
  }
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createStringTagWidget(const ModuleParameter& moduleParameter)
{
  QString valueAsStr = QString::fromStdString(moduleParameter.GetValue());
  QLineEdit* widget = new QLineEdit;
  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  widget->setText(valueAsStr);
  if (isOutputChannel(moduleParameter))
  {
    widget->setReadOnly(true);
  }
  INSTANCIATE_WIDGET_VALUE_WRAPPER(String, _name, _label, widget);
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createPointTagWidget(const ModuleParameter& moduleParameter)
{
  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox* widget = new qMRMLNodeComboBox;
  QStringList nodeTypes;
  nodeTypes += "vtkMRMLMarkupsFiducialNode";

  widget->setNodeTypes(nodeTypes);
  // TODO - title + " FiducialList"
  widget->setNoneEnabled(this->isNoneEnabled(moduleParameter));
  widget->setBaseName(_label);
  // Markups can be added without switching to another module
  // (just by adding a new markup node to the scene and placing a few landmarks)
  // therefore it makes sense to enable adding nodes.
  widget->setAddEnabled(true);
  widget->setRenameEnabled(true);

  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(
    this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Point, _name, _label, widget);

  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createPointFileTagWidget(const ModuleParameter& moduleParameter)
{
  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox* widget = new qMRMLNodeComboBox;

  QString type = QString::fromStdString(moduleParameter.GetType());
  if (type == "any")
  {
    // Add all of the other concrete volume node types
    // TODO: it would be nicer to iterate through all the registered node classes in the scene
    // and add all nodes that are derived from vtkMRMLVolumeNode.
    widget->setNodeTypes(QStringList() << "vtkMRMLMarkupsFiducialNode"
                                       << "vtkMRMLMarkupsLineNode"
                                       << "vtkMRMLMarkupsCurveNode"
                                       << "vtkMRMLMarkupsClosedCurveNode"
                                       << "vtkMRMLMarkupsAngleNode");
  }
  else
  {
    QString nodeType = Self::nodeTypeFromMap(Self::PointTypeAttributeToNodeType, type, "vtkMRMLMarkupsFiducialNode");
    widget->setNodeTypes(QStringList(nodeType));
  }

  // If "type" is specified, only display nodes of type nodeType
  // and don't display subclasses
  if (!type.isEmpty() && type != "any")
  {
    widget->setShowChildNodeTypes(false);
  }

  // TODO - title + " FiducialList"
  widget->setNoneEnabled(this->isNoneEnabled(moduleParameter));
  widget->setBaseName(_label);
  // Markups can be added without switching to another module
  // (just by adding a new markup node to the scene and placing a few landmarks)
  // therefore it makes sense to enable adding nodes.
  widget->setAddEnabled(true);
  widget->setRenameEnabled(true);

  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(
    this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(PointFile, _name, _label, widget);

  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createRegionTagWidget(const ModuleParameter& moduleParameter)
{
  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox* widget = new qMRMLNodeComboBox;
  QStringList nodeTypes;
  nodeTypes += "vtkMRMLMarkupsROINode";
  nodeTypes += "vtkMRMLROIListNode";
  widget->setNodeTypes(QStringList(nodeTypes));
  // TODO - title + " RegionList"
  widget->setNoneEnabled(this->isNoneEnabled(moduleParameter));
  widget->setBaseName(_label);
  // Region can be added without switching to another module
  // therefore it makes sense to enable adding nodes.
  widget->setAddEnabled(true);
  widget->setRenameEnabled(true);

  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(
    this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Region, _name, _label, widget);

  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createImageTagWidget(const ModuleParameter& moduleParameter)
{
  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (channel != "input" && channel != "output")
  {
    qWarning() << "ImageTag - Unknown channel:" << channel;
    return nullptr;
  }

  QString type = QString::fromStdString(moduleParameter.GetType());

  qMRMLNodeComboBox* widget = new qMRMLNodeComboBox;
  if (type == "any")
  {
    // Add all of the other concrete volume node types
    // TODO: it would be nicer to iterate through all the registered node classes in the scene
    // and add all nodes that are derived from vtkMRMLVolumeNode.
    widget->setNodeTypes(QStringList() << "vtkMRMLScalarVolumeNode"
                                       << "vtkMRMLLabelMapVolumeNode"
                                       << "vtkMRMLSegmentationNode"
                                       << "vtkMRMLVectorVolumeNode"
                                       << "vtkMRMLDiffusionTensorVolumeNode"
                                       << "vtkMRMLDiffusionWeightedVolumeNode");
  }
  else if (type == "scalar")
  {
    // TODO: it would be nicer to be able to set multiple node types in Self::nodeTypeFromMap
    widget->setNodeTypes(QStringList() << "vtkMRMLScalarVolumeNode"
                                       << "vtkMRMLLabelMapVolumeNode");
  }
  else
  {
    QString nodeType = Self::nodeTypeFromMap(Self::ImageTypeAttributeToNodeType, type, "vtkMRMLScalarVolumeNode");
    // If node type is vtkMRMLMultiVolumeNode then allow selecting volume sequences, too
    if (nodeType == "vtkMRMLMultiVolumeNode")
    {
      QStringList nodeTypes;
      nodeTypes << nodeType << "vtkMRMLSequenceNode";
      widget->setNodeTypes(nodeTypes);
      widget->addAttribute("vtkMRMLSequenceNode", "DataNodeClassName", "vtkMRMLScalarVolumeNode");
    }
    else if (nodeType == "vtkMRMLLabelMapVolumeNode")
    {
      QStringList nodeTypes;
      nodeTypes << nodeType << "vtkMRMLSegmentationNode";
      widget->setNodeTypes(nodeTypes);
    }
    else
    {
      widget->setNodeTypes(QStringList(nodeType));
    }
  }

  // TODO - title + " Volume"

  QString imageLabel = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString imageName = QString::fromStdString(moduleParameter.GetName());

  // If "type" is specified, only display nodes of type nodeType
  // (e.g. vtkMRMLScalarVolumeNode), don't display subclasses
  // (e.g. vtkMRMLDiffusionTensorVolumeNode)
  if (!type.isEmpty() && type != "any")
  {
    widget->setShowChildNodeTypes(false);
  }
  widget->setNoneEnabled(this->isNoneEnabled(moduleParameter));
  // Being able to create an image for the input is meaningless as the created
  // volume would be empty (useless as an input).
  // However, if it's an output, the result would be saved into the newly
  // created node.
  widget->setAddEnabled(channel != "input");
  widget->setRenameEnabled(true);
  widget->setBaseName(imageLabel);
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());

  QObject::connect(
    this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Image, imageName, imageLabel, widget);

  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createGeometryTagWidget(const ModuleParameter& moduleParameter)
{
  QString type = QString::fromStdString(moduleParameter.GetType());
  QString nodeType = Self::nodeTypeFromMap(Self::GeometryTypeAttributeToNodeType, type, "vtkMRMLModelNode");

  bool multiple = (moduleParameter.GetMultiple() == "true");
  bool aggregate = (moduleParameter.GetAggregate() == "true");

  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (channel != "input" && channel != "output")
  {
    qWarning() << "GeometryTag - Unknown channel:" << channel;
    return nullptr;
  }

  if (multiple && aggregate)
  {
    nodeType = "vtkMRMLModelHierarchyNode";
  }

  // TODO - title + " Model"

  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox* widget = new qMRMLNodeComboBox;
  widget->setShowHidden(false);
  widget->setNodeTypes(QStringList(nodeType));
  widget->setRenameEnabled(true);
  widget->setNoneEnabled(this->isNoneEnabled(moduleParameter));
  widget->setAddEnabled(channel != "input");
  widget->setBaseName(_label);
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(
    this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Geometry, _name, _label, widget);

  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createTableTagWidget(const ModuleParameter& moduleParameter)
{
  QString type = QString::fromStdString(moduleParameter.GetType());
  QString nodeType = Self::nodeTypeFromMap(Self::TableTypeAttributeToNodeType, type, "vtkMRMLTableNode");
  if (nodeType.isEmpty())
  {
    qWarning() << "TableTag - Unknown type:" << type;
    return nullptr;
  }

  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (channel != "input" && channel != "output")
  {
    qWarning() << "TableTag - Unknown channel:" << channel;
    return nullptr;
  }

  // TODO - title + " Table"

  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox* widget = new qMRMLNodeComboBox;
  widget->setNodeTypes(QStringList(nodeType));
  widget->setNoneEnabled(this->isNoneEnabled(moduleParameter));
  widget->setAddEnabled(channel != "input");
  widget->setRenameEnabled(true);
  widget->setBaseName(_label);
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(
    this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Table, _name, _label, widget);

  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createMeasurementTagWidget(const ModuleParameter& moduleParameter)
{
  QString type = QString::fromStdString(moduleParameter.GetType());
  QString nodeType = "vtkMRMLTableNode";
  if (nodeType.isEmpty())
  {
    qWarning() << "MeasurementTag - Unknown type:" << type;
    return nullptr;
  }

  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (channel != "input" && channel != "output")
  {
    qWarning() << "MeasurementTag - Unknown channel:" << channel;
    return nullptr;
  }

  // TODO - title + " Measurement"

  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox* widget = new qMRMLNodeComboBox;
  widget->setNodeTypes(QStringList(nodeType));
  widget->setNoneEnabled(this->isNoneEnabled(moduleParameter));
  widget->setAddEnabled(channel != "input");
  widget->setRenameEnabled(true);
  widget->setBaseName(_label);
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(
    this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Measurement, _name, _label, widget);

  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createTransformTagWidget(const ModuleParameter& moduleParameter)
{
  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (channel != "input" && channel != "output")
  {
    qWarning() << "TransformTag - Unknown channel:" << channel;
    return nullptr;
  }

  QString type = QString::fromStdString(moduleParameter.GetType());

  QString defaultNodeType = "vtkMRMLTransformNode";
  QString nodeType = Self::nodeTypeFromMap(Self::TransformTypeAttributeToNodeType, type, defaultNodeType);
  // TODO - title + " Transform"

  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox* widget = new qMRMLNodeComboBox;
  widget->setNoneEnabled(this->isNoneEnabled(moduleParameter));
  widget->setAddEnabled(channel != "input");
  widget->setRenameEnabled(true);
  if (nodeType == "vtkMRMLTransformNode" && widget->addEnabled())
  {
    // When any kind of transform can be added, allow creating
    // any kind of transform node types.
    widget->setNodeTypes(QStringList() << "vtkMRMLTransformNode"
                                       << "vtkMRMLLinearTransformNode"
                                       << "vtkMRMLGridTransformNode"
                                       << "vtkMRMLBSplineTransformNode");
  }
  else
  {
    widget->setNodeTypes(QStringList(nodeType));
  }
  widget->setBaseName(_label);
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(
    this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Transform, _name, _label, widget);

  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createDirectoryTagWidget(const ModuleParameter& moduleParameter)
{
  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());

  ctkDirectoryButton* widget = new ctkDirectoryButton();
  widget->setCaption(QString("Select %1 ...").arg(_name));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Directory, _name, _label, widget);

  widget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createFileTagWidget(const ModuleParameter& moduleParameter)
{
  QString label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString name = QString::fromStdString(moduleParameter.GetName());

  QStringList fileExtensions;
  const std::vector<std::string> fileExtensionsStd = moduleParameter.GetFileExtensions();
  if (!fileExtensionsStd.empty())
  {
    QString customFilter(qSlicerCLIModuleUIHelper::tr("Compatible Files") + " (");
    for (std::vector<std::string>::const_iterator it = fileExtensionsStd.begin(); it != fileExtensionsStd.end(); ++it)
    {
      if (it != fileExtensionsStd.begin())
      {
        customFilter.append(" ");
      }
      customFilter.append(QString("*") + it->c_str());
    }
    customFilter.append(")");
    fileExtensions << customFilter;
  }
  fileExtensions << QString(qSlicerCLIModuleUIHelper::tr("All Files") + " (*.*)");

  QWidget* widget = new QWidget;
  ctkPathLineEdit* pathLineEdit = new ctkPathLineEdit(name, fileExtensions, ctkPathLineEdit::Files, widget);

  if (isOutputChannel(moduleParameter))
  {
    pathLineEdit->setFilters(pathLineEdit->filters() | ctkPathLineEdit::Writable);
  }

  INSTANCIATE_WIDGET_VALUE_WRAPPER(File, name, label, pathLineEdit);

  QHBoxLayout* hBoxLayout = new QHBoxLayout;
  hBoxLayout->setContentsMargins(0, 0, 0, 0);
  hBoxLayout->addWidget(pathLineEdit);
  widget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
  widget->setLayout(hBoxLayout);
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createEnumerationTagWidget(const ModuleParameter& moduleParameter)
{
  if (isOutputChannel(moduleParameter))
  {
    return createStringTagWidget(moduleParameter);
  }

  QString defaultValue = QString::fromStdString(moduleParameter.GetDefault());

  // iterate over each element in this parameter
  ElementConstIterator sBeginIt = moduleParameter.GetElements().begin();
  ElementConstIterator sEndIt = moduleParameter.GetElements().end();

  QString _label = this->CLIModuleWidget->translate(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  ButtonGroupWidgetWrapper* widget = new ButtonGroupWidgetWrapper;
  ctkFlowLayout* _layout = new ctkFlowLayout(widget);
  widget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);

  for (ElementConstIterator sIt = sBeginIt; sIt != sEndIt; ++sIt)
  {
    QString value = QString::fromStdString(*sIt);
    QRadioButton* radio = new QRadioButton(this->CLIModuleWidget->translate(value.toStdString()), widget);
    _layout->addWidget(radio);
    radio->setChecked(defaultValue == value);
    // enumValue may differ from displayed text when the application is
    // translated or text is modified from outside the application
    // (such as KDE does, to inject keyboard shortcuts)
    radio->setProperty("EnumeratedValue", value);
    // Add radio button to button group
    widget->buttonGroup()->addButton(radio);
  }
  widget->setLayout(_layout);
  INSTANCIATE_WIDGET_VALUE_WRAPPER(Enumeration, _name, _label, widget);
  return widget;
}

//-----------------------------------------------------------------------------
bool qSlicerCLIModuleUIHelperPrivate::isNoneEnabled(const ModuleParameter& moduleParameter)
{
  // As CLI logic is implemented now, index arguments are always required,
  // therefore do not enable selecting 'None' for those.
  bool indexArgument = !moduleParameter.GetIndex().empty();
  return !indexArgument;
}

//-----------------------------------------------------------------------------
bool qSlicerCLIModuleUIHelperPrivate::isOutputChannel(const ModuleParameter& moduleParameter)
{
  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (!channel.isEmpty() && channel != "input" && channel != "output")
  {
    qWarning() << moduleParameter.GetName().c_str() << " - Unknown channel: " << qPrintable(channel)
               << " - assuming input channel";
    return false;
  }
  return channel == "output";
}

//-----------------------------------------------------------------------------
// qSlicerCLIModuleUIHelper methods

//-----------------------------------------------------------------------------
qSlicerCLIModuleUIHelper::qSlicerCLIModuleUIHelper(qSlicerCLIModuleWidget* cliModuleWidget)
  : QObject(cliModuleWidget)
  , d_ptr(new qSlicerCLIModuleUIHelperPrivate(*this))
{
  Q_D(qSlicerCLIModuleUIHelper);

  Q_ASSERT(cliModuleWidget);
  d->CLIModuleWidget = cliModuleWidget;
}

//-----------------------------------------------------------------------------
qSlicerCLIModuleUIHelper::~qSlicerCLIModuleUIHelper() = default;

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelper::createTagWidget(const ModuleParameter& moduleParameter)
{
  Q_D(qSlicerCLIModuleUIHelper);

  Q_ASSERT(moduleParameter.GetHidden() != "true");

  QWidget* widget = nullptr;

  if (moduleParameter.GetTag() == "integer")
  {
    widget = d->createIntegerTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "boolean")
  {
    widget = d->createBooleanTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "float")
  {
    widget = d->createFloatTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "double")
  {
    widget = d->createDoubleTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "string" || moduleParameter.GetTag() == "integer-vector"
           || moduleParameter.GetTag() == "float-vector" || moduleParameter.GetTag() == "double-vector"
           || moduleParameter.GetTag() == "string-vector")
  {
    widget = d->createStringTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "point")
  {
    widget = d->createPointTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "pointfile")
  {
    widget = d->createPointFileTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "region")
  {
    widget = d->createRegionTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "image")
  {
    widget = d->createImageTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "geometry")
  {
    widget = d->createGeometryTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "table")
  {
    widget = d->createTableTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "measurement")
  {
    widget = d->createMeasurementTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "transform")
  {
    widget = d->createTransformTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "directory")
  {
    widget = d->createDirectoryTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "file")
  {
    widget = d->createFileTagWidget(moduleParameter);
  }
  else if (moduleParameter.GetTag() == "string-enumeration" || moduleParameter.GetTag() == "integer-enumeration"
           || moduleParameter.GetTag() == "float-enumeration" || moduleParameter.GetTag() == "double-enumeration")
  {
    widget = d->createEnumerationTagWidget(moduleParameter);
  }

  if (widget)
  {
    QString description = d->CLIModuleWidget->translate(moduleParameter.GetDescription());
    widget->setToolTip(description);
    QString widgetName = QString::fromStdString(moduleParameter.GetName());
    widget->setObjectName(widgetName);
  }

  return widget;
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleUIHelper::updateMRMLCommandLineModuleNode(vtkMRMLCommandLineModuleNode* commandLineModuleNode)
{
  Q_D(qSlicerCLIModuleUIHelper);
  Q_ASSERT(commandLineModuleNode);

  // Block ModifyEvent to be fired, only fire 1 event at the end.
  int disabledModify = commandLineModuleNode->StartModify();

  foreach (qSlicerWidgetValueWrapper* widgetValueWrapper, d->WidgetValueWrappers)
  {
    this->setCommandLineModuleParameter(commandLineModuleNode, widgetValueWrapper->name(), widgetValueWrapper->value());
  }
  // Just in case setCommandLineModuleParameter() has not generated any
  // blocked modify event.
  // TODO: ensure it is really useful to force a modify event
  // commandLineModuleNode->Modified();
  commandLineModuleNode->EndModify(disabledModify);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleUIHelper ::setCommandLineModuleParameter(vtkMRMLCommandLineModuleNode* commandLineModuleNode,
                                                              const QString& name,
                                                              const QVariant& value)
{
  Q_D(qSlicerCLIModuleUIHelper);
  QVariant::Type type = value.type();
  if (type == QVariant::Bool)
  {
    commandLineModuleNode->SetParameterAsBool(name.toUtf8(), value.toBool());
  }
  else if (type == QVariant::Int)
  {
    commandLineModuleNode->SetParameterAsInt(name.toUtf8(), value.toInt());
  }
  else if (type == QVariant::Double)
  {
    commandLineModuleNode->SetParameterAsDouble(name.toUtf8(), value.toDouble());
  }
  else if (type == QVariant::String)
  {
    QString valueAsString = value.toString();
    vtkMRMLNode* node = nullptr;
    if (valueAsString.startsWith("vtkMRML"))
    {
      if (d->CLIModuleWidget->mrmlScene())
      {
        node = d->CLIModuleWidget->mrmlScene()->GetNodeByID(valueAsString.toUtf8());
      }
      else
      {
        qWarning() << Q_FUNC_INFO << "cannot set node by ID, scene is not set";
      }
    }
    if (node)
    {
      commandLineModuleNode->SetParameterAsNode(name.toUtf8(), node);
    }
    else
    {
      commandLineModuleNode->SetParameterAsString(name.toUtf8(), value.toString().toStdString());
    }
  }
  else
  {
    qDebug() << "Unknown widget value type:" << type;
  }
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleUIHelper::updateUi(vtkMRMLCommandLineModuleNode* commandLineModuleNode)
{
  Q_D(qSlicerCLIModuleUIHelper);

  // Disable widgets if no module node is selected
  foreach (qSlicerWidgetValueWrapper* valueWrapper, d->WidgetValueWrappers)
  {
    QWidget* widget = dynamic_cast<QWidget*>(valueWrapper->parent());
    if (!widget)
    {
      continue;
    }
    widget->setEnabled(commandLineModuleNode != nullptr);
  }

  if (!commandLineModuleNode)
  {
    return;
  }

  foreach (qSlicerWidgetValueWrapper* valueWrapper, d->WidgetValueWrappers)
  {
    QString value = QString::fromStdString(commandLineModuleNode->GetParameterAsString(valueWrapper->name().toUtf8()));
    valueWrapper->setValue(value);
  }
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleUIHelper::setValue(const QString& name, const QVariant& value)
{
  Q_D(qSlicerCLIModuleUIHelper);
  foreach (qSlicerWidgetValueWrapper* valueWrapper, d->WidgetValueWrappers)
  {
    if (name == valueWrapper->name())
    {
      valueWrapper->setValue(value.toString());
    }
  }
}
//-----------------------------------------------------------------------------
void qSlicerCLIModuleUIHelper::onValueChanged()
{
  QString name;
  QVariant value;

  qSlicerWidgetValueWrapper* wrapper = qobject_cast<qSlicerWidgetValueWrapper*>(this->sender());
  if (wrapper)
  {
    name = wrapper->name();
    value = wrapper->value();
  }

  emit this->valueChanged(name, value);
}
