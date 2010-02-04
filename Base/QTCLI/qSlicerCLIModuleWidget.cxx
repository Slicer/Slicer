/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#include "qSlicerCLIModuleWidget.h"
#include "qSlicerCLIModuleWidget_p.h"
#include "vtkSlicerCLIModuleLogic.h"

// SlicerQT includes
#include "qSlicerWidget.h"

// qMRML includes
#include <qMRMLNodeSelector.h>
#include <qMRMLNodeFactory.h>

// qCTK includes
#include <qCTKCollapsibleButton.h>
#include <qCTKDoubleSlider.h>
#include <qCTKFlowLayout.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLCommandLineModuleNode.h>

// QT includes
#include <QGroupBox>
#include <QLabel>
#include <QFormLayout>
#include <QDebug>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QRadioButton>

// ITK includes
#include <itkNumericTraits.h>

//-----------------------------------------------------------------------------
bool qSlicerCLIModuleWidgetPrivate::MapInitialized = false;
QHash<QString, QString> qSlicerCLIModuleWidgetPrivate::ImageTypeAttributeToNodeType;
QHash<QString, QString> qSlicerCLIModuleWidgetPrivate::GeometryTypeAttributeToNodeType;
QHash<QString, QString> qSlicerCLIModuleWidgetPrivate::TableTypeAttributeToNodeType;
QHash<QString, QString> qSlicerCLIModuleWidgetPrivate::TransformTypeAttributeToNodeType;

//-----------------------------------------------------------------------------
qSlicerCLIModuleWidget::qSlicerCLIModuleWidget(
  ModuleDescription* desc, QWidget* _parent):Superclass(_parent)
{
  Q_ASSERT(desc);
  QCTK_INIT_PRIVATE(qSlicerCLIModuleWidget);
  QCTK_D(qSlicerCLIModuleWidget);

  // Set properties
  d->Title = QString::fromStdString(desc->GetTitle());
  d->Contributor = QString::fromStdString(desc->GetContributor());
  d->Category = QString::fromStdString(desc->GetCategory());

  d->ProcessInformation = desc->GetProcessInformation();
  d->ParameterGroups = desc->GetParameterGroups();
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidget::setup()
{
  QCTK_D(qSlicerCLIModuleWidget);
  d->setupUi(this);
//   d->Logic = new qSlicerCLIModuleLogic(this);
//   d->Logic->initialize(this->appLogic());
}

//-----------------------------------------------------------------------------
// qSlicerCLIModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
vtkSlicerCLIModuleLogic* qSlicerCLIModuleWidgetPrivate::logic()const
{
  QCTK_P(const qSlicerCLIModuleWidget);
  // Since the logic doesn't have the Q_OJBECT macro, qobject_cast isn't available
  return vtkSlicerCLIModuleLogic::SafeDownCast(p->logic());
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  this->Ui_qSlicerCLIModule::setupUi(widget);

  this->MainCollapsibleWidget->setText(this->Title);

  this->addParameterGroups();

  // Connect buttons
  this->connect(this->ApplyPushButton,
                SIGNAL(pressed()),
                SLOT(onApplyButtonPressed()));
                
  this->connect(this->CancelPushButton,
                SIGNAL(pressed()),
                SLOT(onCancelButtonPressed()));
                
  this->connect(this->DefaultPushButton,
                SIGNAL(pressed()),
                SLOT(onDefaultButtonPressed()));

  this->connect(this->MRMLCommandLineModuleNodeSelector,
                SIGNAL(itemAdded(int)),
                SLOT(updateCommandButtonState()));

  this->connect(this->MRMLCommandLineModuleNodeSelector,
                SIGNAL(itemRemoved(int)),
                SLOT(updateCommandButtonState()));
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::onApplyButtonPressed()
{
  qDebug() << "qSlicerCLIModuleWidgetPrivate::onApplyButtonPressed";
  this->updateMRMLCommandLineModuleNode();

  // QTCLI shouldn't depend on QtCore, otherwise there is a loop in the dependency
  //this->logic()->SetTemporaryDirectory(
  //  qSlicerCoreApplication::application()->tempDirectory().toLatin1());
  //((vtkSlicerApplication*)this->GetApplication())->GetTemporaryDirectory() );
// 
//     // Lazy evaluation of module target
//     this->Logic->LazyEvaluateModuleTarget(this->ModuleDescriptionObject);
// 
//     // make sure the entry point is set on the node
//     this->GetCommandLineModuleNode()->GetModuleDescription()
//       .SetTarget( this->ModuleDescriptionObject.GetTarget() );
//     
//     // apply
//     this->Logic->Apply();
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::onCancelButtonPressed()
{
  qDebug() << "qSlicerCLIModuleWidgetPrivate::onCancelButtonPressed";
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::onDefaultButtonPressed()
{
  qDebug() << "qSlicerCLIModuleWidgetPrivate::onDefaultButtonPressed";
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::updateCommandButtonState()
{
  bool enable = this->MRMLCommandLineModuleNodeSelector->count(); 
  this->ApplyPushButton->setEnabled(enable);
  this->CancelPushButton->setEnabled(enable);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::addParameterGroups()
{
  // iterate over each parameter group
  for (ParameterGroupConstIterator pgIt = this->ParameterGroups.begin();
       pgIt != this->ParameterGroups.end(); ++pgIt)
    {
    this->addParameterGroup(this->VerticalLayout, *pgIt);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::addParameterGroup(QBoxLayout* _layout,
                                                     const ModuleParameterGroup& parameterGroup)
{
  Q_ASSERT(_layout);

  qCTKCollapsibleButton * collapsibleWidget = new qCTKCollapsibleButton();
  collapsibleWidget->setText(QString::fromStdString(parameterGroup.GetLabel()));
  collapsibleWidget->setCollapsed(parameterGroup.GetAdvanced() == "true");

  // Create a vertical layout and add parameter to it
  QFormLayout *vbox = new QFormLayout;
  this->addParameters(vbox, parameterGroup);
  //vbox->addStretch(1);
  vbox->setVerticalSpacing(1);
  collapsibleWidget->setLayout(vbox);
  
  _layout->addWidget(collapsibleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::addParameters(QFormLayout* _layout,
                                                const ModuleParameterGroup& parameterGroup)
{
  Q_ASSERT(_layout);
  // iterate over each parameter in this group
  ParameterConstIterator pBeginIt = parameterGroup.GetParameters().begin();
  ParameterConstIterator pEndIt = parameterGroup.GetParameters().end();

  for (ParameterConstIterator pIt = pBeginIt; pIt != pEndIt; ++pIt)
    {
    this->addParameter(_layout, *pIt);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::addParameter(QFormLayout* _layout,
                                               const ModuleParameter& moduleParameter)
{
  Q_ASSERT(_layout);

  if (moduleParameter.GetHidden() == "true")
    {
    return;
    }

  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  QString description = QString::fromStdString(moduleParameter.GetDescription());
  
  // Parameters with flags can support the None node because they are optional
  int noneEnabled = 0;
  if (moduleParameter.GetLongFlag() != "" || moduleParameter.GetFlag() != "")
    {
    noneEnabled = 1;
    }

  QWidget * widget = 0;
  bool defaultWidget = false;
  
  if (moduleParameter.GetTag() == "integer")
    {
    widget = this->createIntegerTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "boolean")
    {
    widget = this->createBooleanTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "float")
    {
    widget = this->createFloatTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "double")
    {
    widget = this->createDoubleTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "string" ||
           moduleParameter.GetTag() == "integer-vector" ||
           moduleParameter.GetTag() == "float-vector" ||
           moduleParameter.GetTag() == "double-vector" ||
           moduleParameter.GetTag() == "string-vector")
    {
    widget = this->createStringTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "point")
    {
    widget = this->createPointTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "region")
    {
    widget = this->createRegionTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "image")
    {
    widget = this->createImageTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "geometry")
    {
    widget = this->createGeometryTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "table")
    {
    widget = this->createTableTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "transform")
    {
    widget = this->createTransformTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "directory")
    {
    widget = this->createDirectoryTagWidget(moduleParameter);
    }
  else if (moduleParameter.GetTag() == "file")
    {
    widget = this->createFileTagWidget(moduleParameter);
    }
  else if(moduleParameter.GetTag() == "string-enumeration" ||
          moduleParameter.GetTag() == "integer-enumeration" ||
          moduleParameter.GetTag() == "float-enumeration" ||
          moduleParameter.GetTag() == "double-enumeration")
    {
    widget = this->createEnumerationTagWidget(moduleParameter);
    }
  else
    {
    defaultWidget = true;
    QLabel * labelWidget = new QLabel(_label); 
    labelWidget->setToolTip(description);
    _layout->addWidget(labelWidget);
    }

  Q_ASSERT(!defaultWidget && widget);
  if (widget)
    {
    widget->setToolTip(description);
    _layout->addRow(new QLabel(_label), widget);
    }
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(IntegerWithoutConstraints, QSpinBox, value);
WIDGET_VALUE_WRAPPER(IntegerWithConstraints, QSlider, value);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createIntegerTagWidget(const ModuleParameter& moduleParameter)
{
  int value = QString::fromStdString(moduleParameter.GetDefault()).toInt();
  int step = 1; 
  int min = itk::NumericTraits<int>::NonpositiveMin();
  int max = itk::NumericTraits<int>::max();
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  
  QWidget * widget = 0; 
  if (!withConstraints)
    {
    QSpinBox * spinBox = new QSpinBox;
    spinBox->setSingleStep(step);
    spinBox->setValue(value);
    spinBox->setRange(min, max);
    widget = spinBox;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(IntegerWithoutConstraints, _label, spinBox);
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
    INSTANCIATE_WIDGET_VALUE_WRAPPER(IntegerWithConstraints, _label, slider);
    }
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Boolean, QCheckBox, isChecked);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createBooleanTagWidget(const ModuleParameter& moduleParameter)
{
  QString valueAsStr = QString::fromStdString(moduleParameter.GetDefault());
  QCheckBox * widget = new QCheckBox;
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  widget->setChecked(valueAsStr == "true");
  INSTANCIATE_WIDGET_VALUE_WRAPPER(Boolean, _label, widget);
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(FloatWithoutConstaints, QDoubleSpinBox, value);
WIDGET_VALUE_WRAPPER(FloatWithConstaints, qCTKDoubleSlider, value);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createFloatTagWidget(const ModuleParameter& moduleParameter)
{
  float value = QString::fromStdString(moduleParameter.GetDefault()).toFloat();
  float step = 0.1;
  int min = itk::NumericTraits<float>::NonpositiveMin();
  int max = itk::NumericTraits<float>::max();
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  
  QWidget * widget = 0;
  if (!withConstraints)
    {
    QDoubleSpinBox * spinBox = new QDoubleSpinBox;
    spinBox->setSingleStep(step);
    spinBox->setValue(value);
    spinBox->setRange(min, max);
    widget = spinBox;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(FloatWithoutConstaints, _label, spinBox);
    }
  else
    {
    qCTKDoubleSlider * slider = new qCTKDoubleSlider;
    slider->setOrientation(Qt::Horizontal);
    slider->setTickInterval(step);
    slider->setSingleStep(step);
    slider->setRange(min, max);
    slider->setValue(value);
    widget = slider;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(FloatWithConstaints, _label, slider);
    }
  return widget; 
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(DoubleWithoutConstaints, QDoubleSpinBox, value);
WIDGET_VALUE_WRAPPER(DoubleWithConstaints, qCTKDoubleSlider, value);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createDoubleTagWidget(const ModuleParameter& moduleParameter)
{
  double value = QString::fromStdString(moduleParameter.GetDefault()).toDouble();
  double step = 0.1;
  double min = itk::NumericTraits<double>::NonpositiveMin();
  double max = itk::NumericTraits<double>::max();
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  
  QWidget * widget = 0;
  if (!withConstraints)
    {
    QDoubleSpinBox * spinBox = new QDoubleSpinBox;
    spinBox->setSingleStep(step);
    spinBox->setValue(value);
    spinBox->setRange(min, max);
    widget = spinBox;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(DoubleWithoutConstaints, _label, spinBox);
    }
  else
    {
    qCTKDoubleSlider * slider = new qCTKDoubleSlider;
    slider->setOrientation(Qt::Horizontal);
    slider->setSingleStep(step);
    slider->setTickInterval(step);
    slider->setRange(min, max);
    slider->setValue(value);
    widget = slider;
    INSTANCIATE_WIDGET_VALUE_WRAPPER(DoubleWithConstaints, _label, slider);
    }
  return widget; 
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(String, QLineEdit, text);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createStringTagWidget(const ModuleParameter& moduleParameter)
{
  QString valueAsStr = QString::fromStdString(moduleParameter.GetDefault());
  QLineEdit * widget = new QLineEdit;
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  widget->setText(valueAsStr);
  INSTANCIATE_WIDGET_VALUE_WRAPPER(String, _label, widget);
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Point, qMRMLNodeSelector, currentNodeId);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createPointTagWidget(const ModuleParameter& moduleParameter)
{
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeTypes(QStringList("vtkMRMLFiducialListNode"));
  //TODO - title + " FiducialList"
  //TODO - tparameter->SetNewNodeEnabled(1);
  //TODO - tparameter->SetNoneEnabled(noneEnabled);
  //TODO - tparameter->SetNewNodeName((title+" output").c_str());

  QCTK_P(qSlicerCLIModuleWidget);
  widget->setMRMLScene(p->mrmlScene());
  QObject::connect(p, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                  widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Point, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Region, qMRMLNodeSelector, currentNodeId);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createRegionTagWidget(const ModuleParameter& moduleParameter)
{
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeTypes(QStringList("vtkMRMLROIListNode"));
  //TODO - title + " RegionList"
  //TODO - tparameter->SetNewNodeEnabled(1);
  //TODO - tparameter->SetNoneEnabled(noneEnabled);
  QCTK_P(qSlicerCLIModuleWidget);
  widget->setMRMLScene(p->mrmlScene());
  QObject::connect(p, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Region, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Image, qMRMLNodeSelector, currentNodeId);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createImageTagWidget(const ModuleParameter& moduleParameter)
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

  QCTK_P(qSlicerCLIModuleWidget);
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeTypes(QStringList(nodeType));
  widget->setMRMLScene(p->mrmlScene());
  QObject::connect(p, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  // Specify factory attributes
  if (type == "label")
    {
    widget->factory()->addAttribute("LabelMap","1");
    }

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Image, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Geometry, qMRMLNodeSelector, currentNodeId);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createGeometryTagWidget(const ModuleParameter& moduleParameter)
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
  
  QCTK_P(qSlicerCLIModuleWidget);
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setShowHidden(showHidden);
  widget->setNodeTypes(QStringList(nodeType));
  widget->setMRMLScene(p->mrmlScene());
  QObject::connect(p, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Geometry, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Table, qMRMLNodeSelector, currentNodeId);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createTableTagWidget(const ModuleParameter& moduleParameter)
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

  QCTK_P(qSlicerCLIModuleWidget);
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeTypes(QStringList(nodeType));
  widget->setMRMLScene(p->mrmlScene());
  QObject::connect(p, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Table, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Transform, qMRMLNodeSelector, currentNodeId);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createTransformTagWidget(const ModuleParameter& moduleParameter)
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

  QCTK_P(qSlicerCLIModuleWidget);
  QString _label = QString::fromStdString(moduleParameter.GetLabel());
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeTypes(QStringList(nodeType));
  widget->setMRMLScene(p->mrmlScene());
  QObject::connect(p, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  INSTANCIATE_WIDGET_VALUE_WRAPPER(Transform, _label, widget);
  
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createDirectoryTagWidget(const ModuleParameter& moduleParameter)
{
  Q_UNUSED(moduleParameter);
  QPushButton* widget = new QPushButton("Select directory ...");
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createFileTagWidget(const ModuleParameter& moduleParameter)
{
  Q_UNUSED(moduleParameter);
  QPushButton* widget = new QPushButton("Select file ...");
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
  QButtonGroup* ButtonGroup;
};
}

//-----------------------------------------------------------------------------
WIDGET_VALUE_WRAPPER(Enumeration, ButtonGroupWidgetWrapper, checkedValue);

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleWidgetPrivate::createEnumerationTagWidget(const ModuleParameter& moduleParameter)
{
  QString defaultValue = QString::fromStdString(moduleParameter.GetDefault());
  
  // iterate over each element in this parameter
  ElementConstIterator sBeginIt = moduleParameter.GetElements().begin();
  ElementConstIterator sEndIt = moduleParameter.GetElements().end();

  QString _label = QString::fromStdString(moduleParameter.GetLabel());
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
  INSTANCIATE_WIDGET_VALUE_WRAPPER(Enumeration, _label, new ButtonGroupWidgetWrapper(widget, buttonGroup));
  return widget;
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::initializeMaps()
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
QString qSlicerCLIModuleWidgetPrivate::nodeTypeFromMap(const QString& defaultValue,
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

//---------------------------------------------------------------------------
void qSlicerCLIModuleWidgetPrivate::updateMRMLCommandLineModuleNode()
{ 
  QCTK_P(qSlicerCLIModuleWidget);
  Q_ASSERT(p->mrmlScene());
   
  vtkMRMLCommandLineModuleNode* node = vtkMRMLCommandLineModuleNode::SafeDownCast(
    this->MRMLCommandLineModuleNodeSelector->currentNode());
  Q_ASSERT(node);
  
  // save node parameters for Undo
  p->mrmlScene()->SaveStateForUndo(node);

  foreach(WidgetValueWrapper* widgetValueWrapper, this->WidgetValueWrappers)
    {
    QVariant::Type type = widgetValueWrapper->value().type();
    if (type == QVariant::Bool)
      {
      node->SetParameterAsBool(widgetValueWrapper->label().toStdString(),
                               widgetValueWrapper->value().toBool());
      }
    else if (type == QVariant::Double)
      {
      node->SetParameterAsDouble(widgetValueWrapper->label().toStdString(),
                                 widgetValueWrapper->value().toDouble());
      }
    else if (type == QVariant::String)
      {
      node->SetParameterAsString(widgetValueWrapper->label().toStdString(),
                                 widgetValueWrapper->value().toString().toStdString());
      }
    else
      {
      qDebug() << "Uknown widget value type:" << type;
      }
    }
}
