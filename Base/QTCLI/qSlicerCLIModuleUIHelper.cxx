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

// CTK includes
#include <ctkDirectoryButton.h>
#include <ctkFlowLayout.h>
#include <ctkPathLineEdit.h>
#include <ctkSliderWidget.h>
#include <ctkSpinBox.h>

// qMRML includes
#include <qMRMLNodeComboBox.h>

/// ModuleDescriptionParser includes

// MRML includes
#include <vtkMRMLCommandLineModuleNode.h>
#include <vtkMRMLScene.h>

// ITK includes

// SlicerQT includes
#include "qSlicerCLIModuleUIHelper.h"
#include "qSlicerCLIModuleWidget.h"

/// STD includes
#include <limits>

//-----------------------------------------------------------------------------
qSlicerWidgetValueWrapper::qSlicerWidgetValueWrapper(const QString& _name,
                                                     const QString& _label,
                                                     QObject* parentObject)
  :QObject(parentObject),Name(_name), Label(_label)
{

}

//-----------------------------------------------------------------------------
qSlicerWidgetValueWrapper::~qSlicerWidgetValueWrapper()
{
}

//-----------------------------------------------------------------------------
#define WIDGET_VALUE_WRAPPER(_NAME, _WIDGET, _GETTER, _SETTER, _CONVERTER, _NOTIFY) \
  namespace{                                                            \
    class _NAME##WidgetValueWrapper: public qSlicerWidgetValueWrapper   \
    {                                                                   \
    public:                                                             \
      _NAME##WidgetValueWrapper(const QString& _name,                   \
                                const QString& _label, _WIDGET * _widget): \
        qSlicerWidgetValueWrapper(_name, _label, _widget)               \
        {                                                               \
        Q_ASSERT(_widget);                                              \
        this->Widget = _widget;                                         \
        this->connect(this->Widget, SIGNAL(_NOTIFY),                    \
                      this, SIGNAL(valueChanged()));                    \
        }                                                               \
      virtual QVariant value()                                          \
      {                                                                 \
        QVariant _value(this->Widget->_GETTER());                       \
        return _value;                                                  \
      }                                                                 \
      virtual void setValue(const QString& _value)                      \
      {                                                                 \
        this->Widget->_SETTER(qSlicerWidgetValueWrapper::to##_CONVERTER(_value)); \
      }                                                                 \
      _WIDGET* Widget;                                                  \
    };                                                                  \
  }

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(IntegerWithoutConstraints, QSpinBox, value, setValue, Int, valueChanged(int));
WIDGET_VALUE_WRAPPER(IntegerWithConstraints, ctkSliderWidget, value, setValue, Int, valueChanged(double));
WIDGET_VALUE_WRAPPER(Boolean, QCheckBox, isChecked, setChecked, Bool, toggled(bool));
WIDGET_VALUE_WRAPPER(FloatWithoutConstraints, ctkSpinBox, value, setValue, Double, valueChanged(double));
WIDGET_VALUE_WRAPPER(FloatWithConstraints, ctkSliderWidget, value, setValue, Double, valueChanged(double));
WIDGET_VALUE_WRAPPER(DoubleWithoutConstraints, ctkSpinBox, value, setValue, Double, valueChanged(double));
WIDGET_VALUE_WRAPPER(DoubleWithConstraints, ctkSliderWidget, value, setValue, Double, valueChanged(double));
WIDGET_VALUE_WRAPPER(String, QLineEdit, text, setText, String, textChanged(const QString&));
WIDGET_VALUE_WRAPPER(Point, qMRMLNodeComboBox, currentNodeID, setCurrentNodeID, String, currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(Region, qMRMLNodeComboBox, currentNodeID, setCurrentNodeID, String, currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(Image, qMRMLNodeComboBox, currentNodeID, setCurrentNodeID, String, currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(Geometry, qMRMLNodeComboBox, currentNodeID, setCurrentNodeID, String, currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(Table, qMRMLNodeComboBox, currentNodeID, setCurrentNodeID, String, currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(Transform, qMRMLNodeComboBox, currentNodeID, setCurrentNodeID, String, currentNodeIDChanged(QString));
WIDGET_VALUE_WRAPPER(Directory, ctkDirectoryButton, directory, setDirectory, String, directoryChanged(QString));
WIDGET_VALUE_WRAPPER(File, ctkPathLineEdit, currentPath, setCurrentPath, String, currentPathChanged(QString));
WIDGET_VALUE_WRAPPER(Enumeration, ButtonGroupWidgetWrapper, checkedValue, setCheckedValue, String, valueChanged());
WIDGET_VALUE_WRAPPER(Measurement, qMRMLNodeComboBox, currentNodeID, setCurrentNodeID, String, currentNodeIDChanged(QString));

//-----------------------------------------------------------------------------
#define INSTANCIATE_WIDGET_VALUE_WRAPPER(_NAME, _PARAM_NAME, _LABEL, _WIDGET_INSTANCE) \
  qSlicerWidgetValueWrapper* wrapper =                                                 \
    new _NAME##WidgetValueWrapper(_PARAM_NAME, _LABEL, _WIDGET_INSTANCE);              \
  Q_Q(qSlicerCLIModuleUIHelper);                                                       \
  QObject::connect(wrapper, SIGNAL(valueChanged()), q, SLOT(onValueChanged()));        \
  this->WidgetValueWrappers.push_back(wrapper);

//-----------------------------------------------------------------------------
ButtonGroupWidgetWrapper::ButtonGroupWidgetWrapper(QWidget* _parent)
 :QWidget(_parent)
{
  this->ButtonGroup = new QButtonGroup(this);
  this->connect(this->ButtonGroup, SIGNAL(buttonClicked(int)),
                this, SIGNAL(valueChanged()));
}

//-----------------------------------------------------------------------------
QButtonGroup* ButtonGroupWidgetWrapper::buttonGroup()const
{
  return this->ButtonGroup;
}

//-----------------------------------------------------------------------------
QString ButtonGroupWidgetWrapper::checkedValue()
{
  QAbstractButton* button = this->ButtonGroup->checkedButton();
  Q_ASSERT(button);
  return button->text();
}

//-----------------------------------------------------------------------------
void ButtonGroupWidgetWrapper::setCheckedValue(const QString& value)
{
  foreach(QAbstractButton* button, this->ButtonGroup->buttons())
    {
    if (button->text() == value)
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
  typedef std::vector<std::string>::iterator       ElementIterator;
  
  /// 
  /// Create widget corresponding to the different parameters
  QWidget* createIntegerTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createBooleanTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createFloatTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createDoubleTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createStringTagWidget(const ModuleParameter& moduleParameter);
  QWidget* createPointTagWidget(const ModuleParameter& moduleParameter);
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
  static bool shouldEnableNone(const ModuleParameter& moduleParameter);
  
  /// 
  /// Convenient method allowing to retrieve the node type associated
  /// with the parameter type
  static QString nodeTypeFromMap(const QHash<QString, QString>& map,
                                 const QString& attribute,
                                 const QString& defaultValue = QString());

  /// 
  /// Initiliaze the maps containing the mapping
  /// parameter type -> MRML node type (classname)
  static void initializeMaps();
  
  /// Map used to store the different relation
  ///  parameter type -> MRML node type
  static bool MapInitialized; 
  static QHash<QString, QString> ImageTypeAttributeToNodeType;
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
QHash<QString, QString> qSlicerCLIModuleUIHelperPrivate::GeometryTypeAttributeToNodeType;
QHash<QString, QString> qSlicerCLIModuleUIHelperPrivate::TableTypeAttributeToNodeType;
QHash<QString, QString> qSlicerCLIModuleUIHelperPrivate::TransformTypeAttributeToNodeType;

//-----------------------------------------------------------------------------
qSlicerCLIModuleUIHelperPrivate::
  qSlicerCLIModuleUIHelperPrivate(qSlicerCLIModuleUIHelper& object)
  : q_ptr(&object)
{
  this->CLIModuleWidget = 0;
  Self::initializeMaps();
}

//-----------------------------------------------------------------------------
QString qSlicerCLIModuleUIHelperPrivate::nodeTypeFromMap(
  const QHash<QString, QString>& map, const QString& attribute, const QString& defaultValue)
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
  Self::ImageTypeAttributeToNodeType["label"] = "vtkMRMLScalarVolumeNode";
  Self::ImageTypeAttributeToNodeType["vector"] = "vtkMRMLVectorVolumeNode";
  Self::ImageTypeAttributeToNodeType["tensor"] = "vtkMRMLDiffusionTensorVolumeNode";
  Self::ImageTypeAttributeToNodeType["diffusion-weighted"] = "vtkMRMLDiffusionWeightedVolumeNode";
  Self::ImageTypeAttributeToNodeType["signal"] = "vtkMRMLMultiVolumeNode";
  Self::ImageTypeAttributeToNodeType["multichannel"] = "vtkMRMLMultiVolumeNode";
  Self::ImageTypeAttributeToNodeType["dynamic-contrast-enhanced"] = "vtkMRMLMultiVolumeNode";


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
  int value = QString::fromStdString(moduleParameter.GetDefault()).toInt();
  int step = 1; 
  // Since, ctkDoubleSlider checks that MIN_INT < doubleValue / this->SingleStep < MAX_INT
  // Assuming the singlestep won't be smaller than 0.01, the min/max range set from the helper
  // is divided by 100.
  int min = std::numeric_limits<int>::min() / 100;
  int max = std::numeric_limits<int>::max() / 100;
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  
  QWidget * widget = 0; 
  if (!withConstraints)
    {
    QSpinBox * spinBox = new QSpinBox;
    spinBox->setSingleStep(step);
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    widget = spinBox;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(IntegerWithoutConstraints, _name, _label, spinBox);
    }
  else
    {
    QString minAsStr = QString::fromStdString(moduleParameter.GetMinimum());
    if (!minAsStr.isEmpty()) { min = minAsStr.toInt(); }
    
    QString maxAsStr = QString::fromStdString(moduleParameter.GetMaximum());
    if (!maxAsStr.isEmpty()) { max = maxAsStr.toInt(); }
    
    QString stepAsStr = QString::fromStdString(moduleParameter.GetStep());
    if (!stepAsStr.isEmpty()) { step = stepAsStr.toInt(); }
    
    ctkSliderWidget * slider = new ctkSliderWidget;
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
  QString valueAsStr = QString::fromStdString(moduleParameter.GetDefault());
  QCheckBox * widget = new QCheckBox;
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  widget->setChecked(valueAsStr == "true");
  INSTANCIATE_WIDGET_VALUE_WRAPPER(Boolean, _name, _label, widget);
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createFloatTagWidget(const ModuleParameter& moduleParameter)
{
  QString valueAsStr = QString::fromStdString(moduleParameter.GetDefault());
  float value = valueAsStr.toFloat();
  float step = 0.1;
  // Since, ctkDoubleSlider checks that MIN_INT < doubleValue / this->SingleStep < MAX_INT
  // Assuming the singlestep won't be smaller than 0.01, the min/max range set from the helper
  // is divided by 100.
  float min = -std::numeric_limits<float>::max() / 100;
  float max = std::numeric_limits<float>::max() / 100;
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  int decimals = valueAsStr.indexOf('.') != -1 ? valueAsStr.length() - valueAsStr.indexOf('.') -1 : 2;
  
  QWidget * widget = 0;
  if (!withConstraints)
    {
    ctkSpinBox * spinBox = new ctkSpinBox;
    spinBox->setDecimals(decimals);
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
        decimals = qMax(decimals, minAsStr.length() - minAsStr.indexOf('.') -1);
        }
      }

    QString maxAsStr = QString::fromStdString(moduleParameter.GetMaximum());
    if (!maxAsStr.isEmpty())
      {
      max = maxAsStr.toFloat();
      if (maxAsStr.indexOf('.') != -1)
        {
        decimals = qMax(decimals, maxAsStr.length() - maxAsStr.indexOf('.') -1);
        }
      }
    
    QString stepAsStr = QString::fromStdString(moduleParameter.GetStep());
    if (!stepAsStr.isEmpty())
      {
      step = stepAsStr.toFloat();
      if (stepAsStr.indexOf('.') != -1)
        {
        decimals = qMax(decimals, stepAsStr.length() - stepAsStr.indexOf('.') -1);
        }
      }
    
    ctkSliderWidget * slider = new ctkSliderWidget;
    slider->setDecimals(decimals);
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
  QString valueAsStr = QString::fromStdString(moduleParameter.GetDefault());
  double value = valueAsStr.toDouble();
  double step = 0.1;
  // Since, ctkDoubleSlider checks that MIN_INT < doubleValue / this->SingleStep < MAX_INT
  // Assuming the singlestep won't be smaller than 0.01, the min/max range set from the helper
  // is divided by 100.
  double min = -std::numeric_limits<double>::max() / 100;
  double max = std::numeric_limits<double>::max() / 100;
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  int decimals = valueAsStr.indexOf('.') != -1 ? valueAsStr.length() - valueAsStr.indexOf('.') -1 : 2;

  QWidget * widget = 0;
  if (!withConstraints)
    {
    ctkSpinBox * spinBox = new ctkSpinBox;
    spinBox->setDecimals(decimals);
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
        decimals = qMax(decimals, minAsStr.length() - minAsStr.indexOf('.') -1);
        }
      }

    QString maxAsStr = QString::fromStdString(moduleParameter.GetMaximum());
    if (!maxAsStr.isEmpty())
      {
      max = maxAsStr.toFloat();
      if (maxAsStr.indexOf('.') != -1)
        {
        decimals = qMax(decimals, maxAsStr.length() - maxAsStr.indexOf('.') -1);
        }
      }
    
    QString stepAsStr = QString::fromStdString(moduleParameter.GetStep());
    if (!stepAsStr.isEmpty())
      {
      step = stepAsStr.toFloat();
      if (stepAsStr.indexOf('.') != -1)
        {
        decimals = qMax(decimals, stepAsStr.length() - stepAsStr.indexOf('.') -1);
        }
      }
    ctkSliderWidget * slider = new ctkSliderWidget;
    slider->setDecimals(decimals);
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
  QString valueAsStr = QString::fromStdString(moduleParameter.GetDefault());
  QLineEdit * widget = new QLineEdit;
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  widget->setText(valueAsStr);
  INSTANCIATE_WIDGET_VALUE_WRAPPER(String, _name, _label, widget);
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createPointTagWidget(const ModuleParameter& moduleParameter)
{
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox* widget = new qMRMLNodeComboBox;
  QStringList nodeTypes;
  nodeTypes += "vtkMRMLFiducialListNode";
  //TODO - specify to point hierarchy
  nodeTypes += "vtkMRMLAnnotationHierarchyNode";
  widget->setNodeTypes(nodeTypes);
  //TODO - title + " FiducialList"
  //TODO - tparameter->SetNewNodeEnabled(1);
  //TODO - tparameter->SetNoneEnabled(noneEnabled);
  widget->setBaseName(_label);
  widget->setRenameEnabled(true);

  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Point, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createRegionTagWidget(const ModuleParameter& moduleParameter)
{
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox* widget = new qMRMLNodeComboBox;
  widget->setNodeTypes(QStringList("vtkMRMLROIListNode"));
  //TODO - title + " RegionList"
  //TODO - tparameter->SetNewNodeEnabled(1);
  //TODO - tparameter->SetNoneEnabled(noneEnabled);
  widget->setBaseName(_label);
  widget->setRenameEnabled(true);
  
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Region, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createImageTagWidget(const ModuleParameter& moduleParameter)
{
  QString type = QString::fromStdString(moduleParameter.GetType());
  QString nodeType = Self::nodeTypeFromMap(Self::ImageTypeAttributeToNodeType,
                                           type,
                                           "vtkMRMLScalarVolumeNode");
  
  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (channel == "input")
    {
    }
  else if (channel == "output")
    {
    if (type == "any")
      {
//     // Add all of the other concrete volume node types
//     tparameter->AddNodeClass("vtkMRMLVectorVolumeNode",
//                               attrName, attrValue,
//                               (title + " VectorVolume").c_str());
//     tparameter->AddNodeClass("vtkMRMLDiffusionTensorVolumeNode",
//                               attrName, attrValue,
//                               (title + " DiffusionTensorVolume").c_str());
//     tparameter->AddNodeClass("vtkMRMLDiffusionWeightedVolumeNode",
//                               attrName, attrValue,
//                               (title + " DiffusionWeightedVolume").c_str());
      }
    }
  else
    {
    qWarning() << "ImageTag - Unknown channel:" << channel;
    return 0;
    }
    
  // TODO - tparameter->SetNoneEnabled(noneEnabled);
  // TODO - title + " Volume"

  QString imageIndex = QString::fromStdString(moduleParameter.GetIndex());
  QString imageLabel = QString::fromStdString(moduleParameter.GetLabel());
  QString imageName = QString::fromStdString(moduleParameter.GetName());

  qMRMLNodeComboBox * widget = new qMRMLNodeComboBox;
  widget->setNodeTypes(QStringList(nodeType));
  // If "type" is specified, only display nodes of type nodeType
  // (e.g. vtkMRMLScalarVolumeNode), don't display subclasses
  // (e.g. vtkMRMLDiffusionTensorVolumeNode)
  if (!type.isEmpty() &&
      type != "any")
    {
    widget->setShowChildNodeTypes(false);
    }
  // If an index is given, then it means the parameter is required (not optional)
  widget->setNoneEnabled(imageIndex.isEmpty());
  // Being able to create an image for the input is meaningless as the created
  // volume would be empty (useless as an input).
  // However, if it's an output, the result would be saved into the newly
  // created node.
  widget->setAddEnabled(channel != "input");
  widget->setRenameEnabled(true);
  widget->setBaseName(imageLabel);
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());

  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));
  // Specify factory attributes
  if (type == "label")
    {
    widget->addAttribute(nodeType, "LabelMap",QString("1"));
    }

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Image, imageName, imageLabel, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createGeometryTagWidget(const ModuleParameter& moduleParameter)
{
  QString type = QString::fromStdString(moduleParameter.GetType());
  QString nodeType = Self::nodeTypeFromMap(Self::GeometryTypeAttributeToNodeType,
                                           type,
                                           "vtkMRMLModelNode");
                                           
  bool multiple = (moduleParameter.GetMultiple() == "true");
  bool aggregate = (moduleParameter.GetAggregate() == "true");

  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (channel != "input" && channel != "output")
    {
    qWarning() << "GeometryTag - Unknown channel:" << channel;
    return 0; 
    }

  if (multiple && aggregate)
    {
    nodeType = "vtkMRMLModelHierarchyNode";
    }

  // TODO - title + " Model"
  // TODO - SetNoneEnabled(noneEnabled)
  
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox * widget = new qMRMLNodeComboBox;
  widget->setShowHidden(0);
  widget->setNodeTypes(QStringList(nodeType));
  widget->setRenameEnabled(true);
  widget->setBaseName(_label);
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Geometry, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createTableTagWidget(const ModuleParameter& moduleParameter)
{
  QString type = QString::fromStdString(moduleParameter.GetType());
  QString nodeType = Self::nodeTypeFromMap(Self::TableTypeAttributeToNodeType, type);
  if (nodeType.isEmpty())
    {
    qWarning() << "TableTag - Unknown type:" << type;
    return 0; 
    }

  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (channel != "input" && channel != "output")
    {
    qWarning() << "TableTag - Unknown channel:" << channel;
    return 0; 
    }
    
  // TODO - title + " Table"
  // TODO - SetNoneEnabled(1)

  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox * widget = new qMRMLNodeComboBox;
  widget->setNodeTypes(QStringList(nodeType));
  widget->setRenameEnabled(true);
  widget->setBaseName(_label);
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Table, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createMeasurementTagWidget(const ModuleParameter& moduleParameter)
{
  QString type = QString::fromStdString(moduleParameter.GetType());
  QString nodeType = "vtkMRMLDoubleArrayNode";
  if (nodeType.isEmpty())
    {
    qWarning() << "MeasurementTag - Unknown type:" << type;
    return 0; 
    }

  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (channel != "input" && channel != "output")
    {
    qWarning() << "MeasurementTag - Unknown channel:" << channel;
    return 0; 
    }
    
  // TODO - title + " Measurement"
  // TODO - SetNoneEnabled(1)

  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox * widget = new qMRMLNodeComboBox;
  widget->setNodeTypes(QStringList(nodeType));
  widget->setRenameEnabled(true);
  widget->setBaseName(_label);
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

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
    return 0; 
    }

  QString type = QString::fromStdString(moduleParameter.GetType());

  // Note: TransformNode is abstract making it inappropriate for
  // an output type since the node selector must be able to make
  // an instance of the class.  For now, revert to LinearTransformNode.

  QString defaultNodeType =
    (channel == "input" ? "vtkMRMLTransformNode" : "vtkMRMLLinearTransformNode");
  QString nodeType = Self::nodeTypeFromMap(Self::TransformTypeAttributeToNodeType,
                                           type, defaultNodeType);

  QString index = QString::fromStdString(moduleParameter.GetIndex());
  // TODO - title + " Transform"

  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeComboBox * widget = new qMRMLNodeComboBox;
  widget->setNoneEnabled(index.isEmpty());
  widget->setAddEnabled(nodeType != "vtkMRMLTransformNode" && channel != "input");
  widget->setRenameEnabled(true);
  widget->setNodeTypes(QStringList(nodeType));
  widget->setBaseName(_label);
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Transform, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createDirectoryTagWidget(const ModuleParameter& moduleParameter)
{
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  
  ctkDirectoryButton* widget = new ctkDirectoryButton();
  widget->setCaption(QString("Select %1 ...").arg(_name));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Directory, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createFileTagWidget(const ModuleParameter& moduleParameter)
{
  QString label = QString::fromStdString(moduleParameter.GetLabel());
  QString name = QString::fromStdString(moduleParameter.GetName());

  QWidget* widget = new QWidget;
  ctkPathLineEdit* pathLineEdit =
    new ctkPathLineEdit(name, QStringList() << QString("*.*"), ctkPathLineEdit::Files, widget);
  QToolButton* browseButton = new QToolButton(widget);
  browseButton->setText("...");
  QObject::connect(browseButton, SIGNAL(clicked()),
                   pathLineEdit, SLOT(browse()));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(File, name, label, pathLineEdit);

  QHBoxLayout* hBoxLayout = new QHBoxLayout;
  hBoxLayout->setContentsMargins(0,0,0,0);
  hBoxLayout->addWidget(pathLineEdit);
  hBoxLayout->addWidget(browseButton);
  widget->setLayout(hBoxLayout);
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createEnumerationTagWidget(const ModuleParameter& moduleParameter)
{
  QString defaultValue = QString::fromStdString(moduleParameter.GetDefault());
  
  // iterate over each element in this parameter
  ElementConstIterator sBeginIt = moduleParameter.GetElements().begin();
  ElementConstIterator sEndIt = moduleParameter.GetElements().end();

  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  ButtonGroupWidgetWrapper * widget = new ButtonGroupWidgetWrapper;
  ctkFlowLayout * _layout = new ctkFlowLayout(widget);
  widget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);

  for (ElementConstIterator sIt = sBeginIt; sIt != sEndIt; ++sIt)
    {
    QString value = QString::fromStdString(*sIt); 
    QRadioButton * radio = new QRadioButton(value, widget);
    _layout->addWidget(radio);
    radio->setChecked(defaultValue == value);
    // Add radio button to button group 
    widget->buttonGroup()->addButton(radio); 
    }
  widget->setLayout(_layout);
  INSTANCIATE_WIDGET_VALUE_WRAPPER(Enumeration, _name, _label, widget);
  return widget;
}

//-----------------------------------------------------------------------------
bool qSlicerCLIModuleUIHelperPrivate::shouldEnableNone(const ModuleParameter& moduleParameter)
{
  // Parameters with flags can support the None node because they are optional
  if (moduleParameter.GetLongFlag() != "" || moduleParameter.GetFlag() != "")
    {
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
// qSlicerCLIModuleUIHelper methods

//-----------------------------------------------------------------------------
qSlicerCLIModuleUIHelper::qSlicerCLIModuleUIHelper(qSlicerCLIModuleWidget* cliModuleWidget)
  :QObject(cliModuleWidget)
  , d_ptr(new qSlicerCLIModuleUIHelperPrivate(*this))
{
  Q_D(qSlicerCLIModuleUIHelper);

  Q_ASSERT(cliModuleWidget);
  d->CLIModuleWidget = cliModuleWidget;
}

//-----------------------------------------------------------------------------
qSlicerCLIModuleUIHelper::~qSlicerCLIModuleUIHelper()
{
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelper::createTagWidget(const ModuleParameter& moduleParameter)
{
  Q_D(qSlicerCLIModuleUIHelper);
  
  Q_ASSERT(moduleParameter.GetHidden() != "true");

  QWidget * widget = 0;
  
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
  else if (moduleParameter.GetTag() == "string" ||
           moduleParameter.GetTag() == "integer-vector" ||
           moduleParameter.GetTag() == "float-vector" ||
           moduleParameter.GetTag() == "double-vector" ||
           moduleParameter.GetTag() == "string-vector")
    {
    widget = d->createStringTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "point")
    {
    widget = d->createPointTagWidget(moduleParameter);
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
  else if(moduleParameter.GetTag() == "string-enumeration" ||
          moduleParameter.GetTag() == "integer-enumeration" ||
          moduleParameter.GetTag() == "float-enumeration" ||
          moduleParameter.GetTag() == "double-enumeration")
    {
    widget = d->createEnumerationTagWidget(moduleParameter);
    }

  if (widget)
    {
    QString description = QString::fromStdString(moduleParameter.GetDescription());
    widget->setToolTip(description);
    }
  
  return widget; 
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleUIHelper::updateMRMLCommandLineModuleNode(
  vtkMRMLCommandLineModuleNode* commandLineModuleNode)
{
  Q_D(qSlicerCLIModuleUIHelper);
  Q_ASSERT(commandLineModuleNode);

  // Block ModifyEvent to be fired, only fire 1 event at the end.
  int disabledModify = commandLineModuleNode->StartModify();
  
  foreach(qSlicerWidgetValueWrapper* widgetValueWrapper, d->WidgetValueWrappers)
    {
    this->setCommandLineModuleParameter(commandLineModuleNode,
                                        widgetValueWrapper->name(),
                                        widgetValueWrapper->value());
    }
  // Just in case setCommandLineModuleParameter() has not generated any
  // blocked modify event.
  // TODO: ensure it is really useful to force a modify event
  //commandLineModuleNode->Modified();
  commandLineModuleNode->EndModify(disabledModify);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleUIHelper
::setCommandLineModuleParameter(vtkMRMLCommandLineModuleNode* commandLineModuleNode,
                                const QString& name, const QVariant& value)
{
  Q_D(qSlicerCLIModuleUIHelper);
  QVariant::Type type = value.type();
  if (type == QVariant::Bool)
    {
    commandLineModuleNode->SetParameterAsBool(name.toLatin1(),
                                              value.toBool());
    }
  else if (type == QVariant::Int)
    {
    commandLineModuleNode->SetParameterAsInt(name.toLatin1(),
                                             value.toInt());
    }
  else if (type == QVariant::Double)
    {
    commandLineModuleNode->SetParameterAsDouble(name.toLatin1(),
                                                value.toDouble());
    }
  else if (type == QVariant::String)
    {
    QString valueAsString = value.toString();
    vtkMRMLNode* node = valueAsString.startsWith("vtkMRML") ?
      d->CLIModuleWidget->mrmlScene()->GetNodeByID(
        valueAsString.toLatin1()) : 0;
    if (node)
      {
      commandLineModuleNode->SetParameterAsNode(name.toLatin1(),node);
      }
    else
      {
      commandLineModuleNode->SetParameterAsString(name.toLatin1(),
                                                  value.toString().toStdString());
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

  if (!commandLineModuleNode)
    {
    return;
    }

  foreach(qSlicerWidgetValueWrapper* valueWrapper, d->WidgetValueWrappers)
    {
    QString value = QString::fromStdString(
      commandLineModuleNode->GetParameterAsString(valueWrapper->name().toLatin1()));
    valueWrapper->setValue(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleUIHelper::setValue(const QString& name, const QVariant& value)
{
  Q_D(qSlicerCLIModuleUIHelper);
  foreach(qSlicerWidgetValueWrapper* valueWrapper, d->WidgetValueWrappers)
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
