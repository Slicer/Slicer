/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QLabel>
#include <QDebug>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QButtonGroup>
#include <QPushButton>
#include <QRadioButton>
#include <QHash>

// CTK includes
#include <ctkCollapsibleButton.h>
#include <ctkDoubleSlider.h>
#include <ctkDirectoryButton.h>
#include <qCTKFlowLayout.h>

// qMRML includes
#include <qMRMLNodeSelector.h>
#include <qMRMLNodeFactory.h>

/// ModuleDescriptionParser includes
#include <ModuleDescription.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLCommandLineModuleNode.h>

// ITK includes
#include <itkNumericTraits.h>

// SlicerQT includes
#include "qSlicerCLIModuleUIHelper.h"
#include "qSlicerCLIModuleWidget.h"

/// STD includes
#include <vector>

//-----------------------------------------------------------------------------
namespace
{
class WidgetValueWrapper
{
public:
  WidgetValueWrapper(const QString& _name, const QString& _label):Name(_name), Label(_label){}
  virtual ~WidgetValueWrapper(){}
  virtual QVariant value() = 0;
  QString label(){ return this->Label; }
  QString name(){ return this->Name; }

  virtual void setValue(const QString& _value) = 0;

  static QString toString(const QString& _value)
    {
    return _value;
    }

  static bool toBool(const QString& _value)
    {
    return (_value.compare("true", Qt::CaseInsensitive) == 0); 
    }

  static int toInt(const QString& _value)
    {
    return _value.toInt();
    }

  static double toDouble(const QString& _value)
    {
    return _value.toDouble(); 
    }

  QString Name;
  QString Label;
};
}

//-----------------------------------------------------------------------------
#define WIDGET_VALUE_WRAPPER(_NAME, _WIDGET, _GETTER, _SETTER, _CONVERTER)  \
namespace{                                                                  \
class _NAME##WidgetValueWrapper: public WidgetValueWrapper                  \
{                                                                           \
public:                                                                     \
  _NAME##WidgetValueWrapper(const QString& _name,                           \
                            const QString& _label, _WIDGET * _widget):      \
    WidgetValueWrapper(_name, _label)                                       \
    {                                                                       \
    Q_ASSERT(_widget);                                                      \
    this->Widget = _widget;                                                 \
    }                                                                       \
  virtual QVariant value()                                                  \
    {                                                                       \
    QVariant _value(this->Widget->_GETTER());                               \
    return _value;                                                          \
    }                                                                       \
  virtual void setValue(const QString& _value)                              \
    {                                                                       \
    this->Widget->_SETTER(WidgetValueWrapper::to##_CONVERTER(_value));      \
    }                                                                       \
  _WIDGET* Widget;                                                          \
};                                                                          \
}

//-----------------------------------------------------------------------------
#define INSTANCIATE_WIDGET_VALUE_WRAPPER(_NAME, _PARAM_NAME, _LABEL, _WIDGET_INSTANCE)   \
this->WidgetValueWrappers.push_back(                                                     \
  new _NAME##WidgetValueWrapper(_PARAM_NAME, _LABEL, _WIDGET_INSTANCE));

//-----------------------------------------------------------------------------
class qSlicerCLIModuleUIHelperPrivate: public ctkPrivate<qSlicerCLIModuleUIHelper>
{
public:
  typedef qSlicerCLIModuleUIHelperPrivate Self;
  qSlicerCLIModuleUIHelperPrivate();

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

  ///
  /// Return true if the None option of the widget associated with the parameter should
  /// be enabled
  static bool shouldEnableNone(const ModuleParameter& moduleParameter);
  
  /// 
  /// Convenient method allowing to retrieve the node type associated
  /// with the parameter type
  static QString nodeTypeFromMap(const QString& defaultValue,
                                 const QHash<QString, QString>& map,
                                 const QString& attribute);

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
  QList<WidgetValueWrapper*> WidgetValueWrappers;

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
  qSlicerCLIModuleUIHelperPrivate()
{
  this->CLIModuleWidget = 0;
  Self::initializeMaps();
}

//-----------------------------------------------------------------------------
QString qSlicerCLIModuleUIHelperPrivate::nodeTypeFromMap(const QString& defaultValue,
  const QHash<QString, QString>& map, const QString& attribute)
{
  QHash<QString, QString>::const_iterator i = map.constFind(attribute);

  if (i == map.constEnd())
    {
    return defaultValue; 
    }
  else
    {
    return i.value();
    }
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

  // Geometry type attribute mapping
  Self::GeometryTypeAttributeToNodeType["fiberBundle"] = "vtkMRMLFiberBundleNode";
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
WIDGET_VALUE_WRAPPER(IntegerWithoutConstraints, QSpinBox, value, setValue, Int);
WIDGET_VALUE_WRAPPER(IntegerWithConstraints, QSlider, value, setValue, Int);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createIntegerTagWidget(const ModuleParameter& moduleParameter)
{
  int value = QString::fromStdString(moduleParameter.GetDefault()).toInt();
  int step = 1; 
  int min = itk::NumericTraits<int>::NonpositiveMin() / 100;
  int max = itk::NumericTraits<int>::max() / 100;
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  
  QWidget * widget = 0; 
  if (!withConstraints)
    {
    QSpinBox * spinBox = new QSpinBox;
    spinBox->setSingleStep(step);
    spinBox->setValue(value);
    spinBox->setRange(min, max);
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
    
    QSlider * slider = new QSlider;
    slider->setOrientation(Qt::Horizontal);
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
WIDGET_VALUE_WRAPPER(Boolean, QCheckBox, isChecked, setChecked, Bool);

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
WIDGET_VALUE_WRAPPER(FloatWithoutConstaints, QDoubleSpinBox, value, setValue, Double);
WIDGET_VALUE_WRAPPER(FloatWithConstaints, ctkDoubleSlider, value, setValue, Double);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createFloatTagWidget(const ModuleParameter& moduleParameter)
{
  float value = QString::fromStdString(moduleParameter.GetDefault()).toFloat();
  float step = 0.1;
  float min = itk::NumericTraits<int>::NonpositiveMin() / 100;
  float max = itk::NumericTraits<int>::max() / 100;
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  
  QWidget * widget = 0;
  if (!withConstraints)
    {
    QDoubleSpinBox * spinBox = new QDoubleSpinBox;
    spinBox->setSingleStep(step);
    spinBox->setValue(value);
    spinBox->setRange(min, max);
    widget = spinBox;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(FloatWithoutConstaints, _name, _label, spinBox);
    }
  else
    {
    ctkDoubleSlider * slider = new ctkDoubleSlider;
    slider->setOrientation(Qt::Horizontal);
    slider->setTickInterval(step);
    slider->setSingleStep(step);
    slider->setRange(min, max);
    slider->setValue(value);
    widget = slider;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(FloatWithConstaints, _name, _label, slider);
    }
  return widget; 
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(DoubleWithoutConstaints, QDoubleSpinBox, value, setValue, Double);
WIDGET_VALUE_WRAPPER(DoubleWithConstaints, ctkDoubleSlider, value, setValue, Double);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createDoubleTagWidget(const ModuleParameter& moduleParameter)
{
  double value = QString::fromStdString(moduleParameter.GetDefault()).toDouble();
  double step = 0.1;
  double min = itk::NumericTraits<int>::NonpositiveMin() / 100;
  double max = itk::NumericTraits<int>::max() / 100;
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  
  QWidget * widget = 0;
  if (!withConstraints)
    {
    QDoubleSpinBox * spinBox = new QDoubleSpinBox;
    spinBox->setSingleStep(step);
    spinBox->setValue(value);
    spinBox->setRange(min, max);
    widget = spinBox;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(DoubleWithoutConstaints, _name, _label, spinBox);
    }
  else
    {
    ctkDoubleSlider * slider = new ctkDoubleSlider;
    slider->setOrientation(Qt::Horizontal);
    slider->setSingleStep(step);
    slider->setTickInterval(step);
    slider->setRange(min, max);
    slider->setValue(value);
    widget = slider;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(DoubleWithConstaints, _name, _label, slider);
    }
  return widget; 
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(String, QLineEdit, text, setText, String);

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
WIDGET_VALUE_WRAPPER(Point, qMRMLNodeSelector, currentNodeId, setCurrentNode, String);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createPointTagWidget(const ModuleParameter& moduleParameter)
{
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeTypes(QStringList("vtkMRMLFiducialListNode"));
  //TODO - title + " FiducialList"
  //TODO - tparameter->SetNewNodeEnabled(1);
  //TODO - tparameter->SetNoneEnabled(noneEnabled);
  //TODO - tparameter->SetNewNodeName((title+" output").c_str());

  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Point, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Region, qMRMLNodeSelector, currentNodeId, setCurrentNode, String);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createRegionTagWidget(const ModuleParameter& moduleParameter)
{
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeTypes(QStringList("vtkMRMLROIListNode"));
  //TODO - title + " RegionList"
  //TODO - tparameter->SetNewNodeEnabled(1);
  //TODO - tparameter->SetNoneEnabled(noneEnabled);
  
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Region, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Image, qMRMLNodeSelector, currentNodeId, setCurrentNode, String);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createImageTagWidget(const ModuleParameter& moduleParameter)
{
  QString type = QString::fromStdString(moduleParameter.GetType());
  QString nodeType = Self::nodeTypeFromMap("vtkMRMLScalarVolumeNode",
                                           Self::GeometryTypeAttributeToNodeType, type);
  
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

  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeTypes(QStringList(nodeType));
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  // Specify factory attributes
  if (type == "label")
    {
    widget->addAttribute(nodeType, "LabelMap",QString("1"));
    }

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Image, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Geometry, qMRMLNodeSelector, currentNodeId, setCurrentNode, String);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createGeometryTagWidget(const ModuleParameter& moduleParameter)
{
  QString type = QString::fromStdString(moduleParameter.GetType());
  QString nodeType = Self::nodeTypeFromMap("vtkMRMLModelNode",
                                           Self::GeometryTypeAttributeToNodeType, type);
                                           
  bool multiple = (moduleParameter.GetMultiple() == "true");
  bool aggregate = (moduleParameter.GetAggregate() == "true");
  bool showHidden = (multiple && aggregate);

  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (channel != "input" && channel != "output")
    {
    qWarning() << "GeometryTag - Unknown channel:" << channel;
    return 0; 
    }

  if (showHidden)
    {
    nodeType = "vtkMRMLModelHierarchyNode";
    }

  // TODO - title + " Model"
  // TODO - SetNoneEnabled(noneEnabled)
  
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setShowHidden(showHidden);
  widget->setNodeTypes(QStringList(nodeType));
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Geometry, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Table, qMRMLNodeSelector, currentNodeId, setCurrentNode, String);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createTableTagWidget(const ModuleParameter& moduleParameter)
{
  QString type = QString::fromStdString(moduleParameter.GetType());
  QString nodeType = Self::nodeTypeFromMap("", Self::TableTypeAttributeToNodeType, type);
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
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeTypes(QStringList(nodeType));
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Table, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Transform, qMRMLNodeSelector, currentNodeId, setCurrentNode, String);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createTransformTagWidget(const ModuleParameter& moduleParameter)
{
  QString type = QString::fromStdString(moduleParameter.GetType());
  QString nodeType = Self::nodeTypeFromMap("vtkMRMLTransformNode",
                                           Self::TransformTypeAttributeToNodeType, type);

  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (channel != "input" && channel != "output")
    {
    qWarning() << "TransformTag - Unknown channel:" << channel;
    return 0; 
    }
    
  // TODO - title + " Transform"
  // TODO - SetNoneEnabled(noneEnabled);

  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeTypes(QStringList(nodeType));
  widget->setMRMLScene(this->CLIModuleWidget->mrmlScene());
  QObject::connect(this->CLIModuleWidget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Transform, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Directory, ctkDirectoryButton, directory, setDirectory, String);

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
WIDGET_VALUE_WRAPPER(File, QLineEdit, text, setText, String);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createFileTagWidget(const ModuleParameter& moduleParameter)
{
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  
  QLineEdit* widget = new QLineEdit(QString("Input %1").arg(_name));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(File, _name, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
namespace{
class ButtonGroupWidgetWrapper: public QWidget
{
public:
  ButtonGroupWidgetWrapper(QWidget* _parent, QButtonGroup* buttonGroup):
    QWidget(_parent), ButtonGroup(buttonGroup){}
  QString checkedValue()
    {
    Q_ASSERT(this->ButtonGroup);
    QAbstractButton* button = this->ButtonGroup->checkedButton();
    Q_ASSERT(button);
    return button->text(); 
    }
  void setCheckedValue(const QString& value)
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
  QButtonGroup* ButtonGroup;
};
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Enumeration, ButtonGroupWidgetWrapper, checkedValue, setCheckedValue, String);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelperPrivate::createEnumerationTagWidget(const ModuleParameter& moduleParameter)
{
  QString defaultValue = QString::fromStdString(moduleParameter.GetDefault());
  
  // iterate over each element in this parameter
  ElementConstIterator sBeginIt = moduleParameter.GetElements().begin();
  ElementConstIterator sEndIt = moduleParameter.GetElements().end();

  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString _name = QString::fromStdString(moduleParameter.GetName());
  QWidget * widget = new QWidget;
  QButtonGroup* buttonGroup = new QButtonGroup(widget); 
  qCTKFlowLayout * _layout = new qCTKFlowLayout;
  widget->setLayout(_layout);

  for (ElementConstIterator sIt = sBeginIt; sIt != sEndIt; ++sIt)
    {
    QString value = QString::fromStdString(*sIt); 
    QRadioButton * radio = new QRadioButton(value);
    _layout->addWidget(radio);
    radio->setChecked(defaultValue == value);
    // Add radio button to button group 
    buttonGroup->addButton(radio); 
    }
  INSTANCIATE_WIDGET_VALUE_WRAPPER(Enumeration, _name, _label,
    new ButtonGroupWidgetWrapper(widget, buttonGroup));
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
{
  CTK_INIT_PRIVATE(qSlicerCLIModuleUIHelper);
  CTK_D(qSlicerCLIModuleUIHelper);

  Q_ASSERT(cliModuleWidget);
  d->CLIModuleWidget = cliModuleWidget; 
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUIHelper::createTagWidget(const ModuleParameter& moduleParameter)
{
  CTK_D(qSlicerCLIModuleUIHelper);
  
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
  CTK_D(qSlicerCLIModuleUIHelper);
  Q_ASSERT(commandLineModuleNode);

  commandLineModuleNode->SetDisableModifiedEvent(true);
  
  foreach(WidgetValueWrapper* widgetValueWrapper, d->WidgetValueWrappers)
    {
    QVariant::Type type = widgetValueWrapper->value().type();
    if (type == QVariant::Bool)
      {
      commandLineModuleNode->SetParameterAsBool(widgetValueWrapper->name().toStdString(),
                                                widgetValueWrapper->value().toBool());
      }
    else if (type == QVariant::Int)
      {
      commandLineModuleNode->SetParameterAsInt(widgetValueWrapper->name().toStdString(),
                                               widgetValueWrapper->value().toInt());
      }
    else if (type == QVariant::Double)
      {
      commandLineModuleNode->SetParameterAsDouble(widgetValueWrapper->name().toStdString(),
                                                  widgetValueWrapper->value().toDouble());
      }
    else if (type == QVariant::String)
      {
      commandLineModuleNode->SetParameterAsString(
        widgetValueWrapper->name().toStdString(),
        widgetValueWrapper->value().toString().toStdString());
      }
    else
      {
      qDebug() << "Unknown widget value type:" << type;
      }
    }

  commandLineModuleNode->SetDisableModifiedEvent(false);

  // notify observer(s)
  commandLineModuleNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleUIHelper::updateUi(vtkMRMLCommandLineModuleNode* commandLineModuleNode)
{
  CTK_D(qSlicerCLIModuleUIHelper);

  if (!commandLineModuleNode)
    {
    return;
    }

  foreach(WidgetValueWrapper* valueWrapper, d->WidgetValueWrappers)
    {
    QString value = QString::fromStdString(
      commandLineModuleNode->GetParameterAsString(valueWrapper->name().toStdString()));
    valueWrapper->setValue(value);
    }
}
