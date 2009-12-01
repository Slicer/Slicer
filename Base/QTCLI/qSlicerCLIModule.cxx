#include "qSlicerCLIModule.h"
#include "ui_qSlicerCLIModule.h"

// SlicerQT includes
#include "qSlicerCLIModuleLogic.h"

// MRML includes
#include "qMRMLNodeSelector.h"
#include "qMRMLNodeFactory.h"

// CTK includes
#include "qCTKCollapsibleButton.h"
#include "qCTKSlider.h"
#include "qCTKFlowLayout.h"

// ModuleDescriptionParser includes
#include "ModuleDescription.h"
#include "ModuleDescriptionParser.h"

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
#include "itkNumericTraits.h"

//-----------------------------------------------------------------------------
struct qSlicerCLIModule::qInternal: public Ui::qSlicerCLIModule
{
  typedef qInternal Self; 
  qInternal(qSlicerAbstractModule * backPointer)
    {
    this->Logic = 0;
    this->ProcessInformation = 0;
    this->BackPointer = backPointer; 
    }

  typedef std::vector<ModuleParameterGroup>::const_iterator ParameterGroupConstIterator;
  typedef std::vector<ModuleParameterGroup>::iterator       ParameterGroupIterator;

  typedef std::vector<ModuleParameter>::const_iterator ParameterConstIterator;
  typedef std::vector<ModuleParameter>::iterator       ParameterIterator;

  typedef std::vector<std::string>::const_iterator ElementConstIterator;
  typedef std::vector<std::string>::iterator       ElementIterator;


  // Description:
  // Initiliaze the maps containing the mapping
  //   parameter type -> MRML node type (classname)
  static void initializeMaps();

  // Description:
  // Convenient method allowing to retrieve the node type associated
  // with the parameter type
  static QString nodeTypeFromMap(const QString& defaultValue,
                                 const QHash<QString, QString>& map,
                                 const QString& attribute);
                                 
  // Description:
  void addParameterGroups();
  void addParameterGroup(QBoxLayout* layout,
                         const ModuleParameterGroup& parameterGroup);

  // Description:
  void addParameters(QFormLayout* layout, const ModuleParameterGroup& parameterGroup);
  void addParameter(QFormLayout* layout, const ModuleParameter& moduleParameter);

  // Description:
  // Create widget corresponding to the different parameters
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

  // Map used to store the different relation
  //  parameter type -> MRML node type
  static bool MapInitialized; 
  static QHash<QString, QString> ImageTypeAttributeToNodeType;
  static QHash<QString, QString> GeometryTypeAttributeToNodeType;
  static QHash<QString, QString> TableTypeAttributeToNodeType;
  static QHash<QString, QString> TransformTypeAttributeToNodeType;  
  
  QString           Title;
  QString           Acknowledgement;
  QString           Help;
  QString           Category;
  QString           Contributor;

  std::vector<ModuleParameterGroup> ParameterGroups;
  ModuleProcessInformation*         ProcessInformation;

  qSlicerCLIModuleLogic * Logic;
  qSlicerAbstractModule * BackPointer;
};

//-----------------------------------------------------------------------------
bool qSlicerCLIModule::qInternal::MapInitialized = false; 
QHash<QString, QString> qSlicerCLIModule::qInternal::ImageTypeAttributeToNodeType;
QHash<QString, QString> qSlicerCLIModule::qInternal::GeometryTypeAttributeToNodeType;
QHash<QString, QString> qSlicerCLIModule::qInternal::TableTypeAttributeToNodeType;
QHash<QString, QString> qSlicerCLIModule::qInternal::TransformTypeAttributeToNodeType;

//-----------------------------------------------------------------------------
qSlicerCxxInternalBckPtrConstructor1Macro(qSlicerCLIModule, QWidget*);
qSlicerCxxDestructorMacro(qSlicerCLIModule);

//-----------------------------------------------------------------------------
void qSlicerCLIModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setup()
{
  this->Internal->Logic = new qSlicerCLIModuleLogic(this);
  this->Internal->Logic->initialize(this->appLogic());
}

//-----------------------------------------------------------------------------
qSlicerGetInternalCxxMacro(qSlicerCLIModule, QString, title, Title);
qSlicerGetInternalCxxMacro(qSlicerCLIModule, QString, category, Category);
qSlicerGetInternalCxxMacro(qSlicerCLIModule, QString, contributor, Contributor);
qSlicerGetInternalCxxMacro(qSlicerCLIModule, QString, acknowledgementText, Acknowledgement);
qSlicerGetInternalCxxMacro(qSlicerCLIModule, QString, helpText, Help);

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setXmlModuleDescription(const char* xmlModuleDescription)
{
  //qDebug() << "xmlModuleDescription:" << xmlModuleDescription;

  // Parse module description
  ModuleDescription desc;
  ModuleDescriptionParser parser;
  if (parser.Parse(std::string(xmlModuleDescription), desc) != 0)
    {
    qWarning() << "Failed to parse xml module description";
    return;
    }

  // Set properties
  this->Internal->Title = QString::fromStdString(desc.GetTitle());
  this->Internal->Acknowledgement = QString::fromStdString(desc.GetAcknowledgements());
  this->Internal->Contributor = QString::fromStdString(desc.GetContributor());
  this->Internal->Category = QString::fromStdString(desc.GetCategory());

  this->Internal->ProcessInformation = desc.GetProcessInformation();
  this->Internal->ParameterGroups = desc.GetParameterGroups();

  QString help =
    "%1<br>"
    "For more detailed documentation see:<br>"
    "%2";

  this->Internal->Help = help.arg(
    QString::fromStdString(desc.GetDescription())).arg(
    QString::fromStdString(desc.GetDocumentationURL()));
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setupUi()
{
  this->Internal->setupUi(this);

  this->Internal->MainCollapsibleWidget->setText(this->title());

  this->Internal->addParameterGroups();

  // Connect buttons
  this->connect(this->Internal->ApplyPushButton,
                SIGNAL(pressed()),
                SLOT(onApplyButtonPressed()));
                
  this->connect(this->Internal->CancelPushButton,
                SIGNAL(pressed()),
                SLOT(onCancelButtonPressed()));
                
  this->connect(this->Internal->DefaultPushButton,
                SIGNAL(pressed()),
                SLOT(onDefaultButtonPressed()));
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::onApplyButtonPressed()
{
  qDebug() << "qSlicerCLIModule::onApplyButtonPressed";
// // Apply button was pressed
//     //std::cout << "  Apply" << std::endl;
//     this->UpdateMRML();
//     this->Logic->SetTemporaryDirectory( ((vtkSlicerApplication*)this->GetApplication())->GetTemporaryDirectory() );
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
void qSlicerCLIModule::onCancelButtonPressed()
{
  qDebug() << "qSlicerCLIModule::onCancelButtonPressed";
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::onDefaultButtonPressed()
{
  qDebug() << "qSlicerCLIModule::onDefaultButtonPressed";
}

//-----------------------------------------------------------------------------
// Internal methods

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addParameterGroups()
{
  // iterate over each parameter group
  for (ParameterGroupConstIterator pgIt = this->ParameterGroups.begin();
       pgIt != this->ParameterGroups.end(); ++pgIt)
    {
    this->addParameterGroup(this->VerticalLayout, *pgIt);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addParameterGroup(QBoxLayout* layout,
                                                     const ModuleParameterGroup& parameterGroup)
{
  Q_ASSERT(layout);

  qCTKCollapsibleButton * collapsibleWidget = new qCTKCollapsibleButton();
  collapsibleWidget->setText(QString::fromStdString(parameterGroup.GetLabel()));
  collapsibleWidget->setCollapsed(parameterGroup.GetAdvanced() == "true");

  // Create a vertical layout and add parameter to it
  QFormLayout *vbox = new QFormLayout;
  this->addParameters(vbox, parameterGroup);
  //vbox->addStretch(1);
  vbox->setVerticalSpacing(1);
  collapsibleWidget->setLayout(vbox);

  layout->addWidget(collapsibleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addParameters(QFormLayout* layout,
                                                const ModuleParameterGroup& parameterGroup)
{
  Q_ASSERT(layout);
  // iterate over each parameter in this group
  ParameterConstIterator pBeginIt = parameterGroup.GetParameters().begin();
  ParameterConstIterator pEndIt = parameterGroup.GetParameters().end();

  for (ParameterConstIterator pIt = pBeginIt; pIt != pEndIt; ++pIt)
    {
    this->addParameter(layout, *pIt);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addParameter(QFormLayout* layout,
                                               const ModuleParameter& moduleParameter)
{
  Q_ASSERT(layout);

  if (moduleParameter.GetHidden() == "true")
    {
    return;
    }

  QString label = QString::fromStdString(moduleParameter.GetLabel());
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
    QLabel * label = new QLabel(label); 
    label->setToolTip(description);
    layout->addWidget(label);
    }

  Q_ASSERT(!defaultWidget && widget);
  if (widget)
    {
    widget->setToolTip(description);
    layout->addRow(new QLabel(label), widget);
    }
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createIntegerTagWidget(const ModuleParameter& moduleParameter)
{
  int value = QString::fromStdString(moduleParameter.GetDefault()).toInt();
  int step = 1; 
  int min = itk::NumericTraits<int>::NonpositiveMin();
  int max = itk::NumericTraits<int>::max();
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();

  QWidget * widget = 0; 
  if (!withConstraints)
    {
    QSpinBox * spinBox = new QSpinBox;
    spinBox->setSingleStep(step);
    spinBox->setValue(value);
    spinBox->setRange(min, max);
    widget = spinBox;
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
    slider->setTickInterval(step);
    slider->setRange(min, max);
    slider->setValue(value);
    widget = slider; 
    }
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createBooleanTagWidget(const ModuleParameter& moduleParameter)
{
  QString valueAsStr = QString::fromStdString(moduleParameter.GetDefault());
  QCheckBox * widget = new QCheckBox;
  widget->setChecked(valueAsStr == "true");
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createFloatTagWidget(const ModuleParameter& moduleParameter)
{
  float value = QString::fromStdString(moduleParameter.GetDefault()).toFloat();
  float step = 0.1;
  int min = itk::NumericTraits<float>::NonpositiveMin();
  int max = itk::NumericTraits<float>::max();
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();

  QWidget * widget = 0;
  if (!withConstraints)
    {
    QDoubleSpinBox * spinBox = new QDoubleSpinBox;
    spinBox->setSingleStep(step);
    spinBox->setValue(value);
    spinBox->setRange(min, max);
    widget = spinBox; 
    }
  else
    {
    qCTKSlider * slider = new qCTKSlider;
    slider->setOrientation(Qt::Horizontal);
    slider->setTickInterval(step);
    slider->setRange(min, max);
    slider->setValue(value);
    widget = slider;
    }
  return widget; 
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createDoubleTagWidget(const ModuleParameter& moduleParameter)
{
  double value = QString::fromStdString(moduleParameter.GetDefault()).toDouble();
  double step = 0.1;
  double min = itk::NumericTraits<double>::NonpositiveMin();
  double max = itk::NumericTraits<double>::max();
  bool withConstraints = !QString::fromStdString(moduleParameter.GetConstraints()).isEmpty();

  QWidget * widget = 0;
  if (!withConstraints)
    {
    QDoubleSpinBox * spinBox = new QDoubleSpinBox;
    spinBox->setSingleStep(step);
    spinBox->setValue(value);
    spinBox->setRange(min, max);
    widget = spinBox; 
    }
  else
    {
    qCTKSlider * slider = new qCTKSlider;
    slider->setOrientation(Qt::Horizontal);
    slider->setTickInterval(step);
    slider->setRange(min, max);
    slider->setValue(value);
    widget = slider; 
    }
  return widget; 
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createStringTagWidget(const ModuleParameter& moduleParameter)
{
  QString valueAsStr = QString::fromStdString(moduleParameter.GetDefault());
  QLineEdit * widget = new QLineEdit;
  widget->setText(valueAsStr);
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createPointTagWidget(const ModuleParameter& moduleParameter)
{
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeType("vtkMRMLFiducialListNode");
  //TODO - title + " FiducialList"
  //TODO - tparameter->SetNewNodeEnabled(1);
  //TODO - tparameter->SetNoneEnabled(noneEnabled);
  //TODO - tparameter->SetNewNodeName((title+" output").c_str());
  widget->setMRMLScene(this->BackPointer->mrmlScene());
  QObject::connect(this->BackPointer, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createRegionTagWidget(const ModuleParameter& moduleParameter)
{
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeType("vtkMRMLROIListNode");
  //TODO - title + " RegionList"
  //TODO - tparameter->SetNewNodeEnabled(1);
  //TODO - tparameter->SetNoneEnabled(noneEnabled);
  widget->setMRMLScene(this->BackPointer->mrmlScene());
  QObject::connect(this->BackPointer, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createImageTagWidget(const ModuleParameter& moduleParameter)
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
  
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeType(nodeType);
  widget->setMRMLScene(this->BackPointer->mrmlScene());
  QObject::connect(this->BackPointer, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  // Specify factory attributes
  if (type == "label")
    {
    widget->factory()->addAttribute("LabelMap","1");
    }
  
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createGeometryTagWidget(const ModuleParameter& moduleParameter)
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
  
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setShowHidden(showHidden);
  widget->setNodeType(nodeType);
  widget->setMRMLScene(this->BackPointer->mrmlScene());
  QObject::connect(this->BackPointer, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));

  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createTableTagWidget(const ModuleParameter& moduleParameter)
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
    qWarning() << "GeometryTag - Unknown channel:" << channel;
    return 0; 
    }
    
  // TODO - title + " Table"
  // TODO - SetNoneEnabled(1)
  
  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeType(nodeType);
  widget->setMRMLScene(this->BackPointer->mrmlScene());
  QObject::connect(this->BackPointer, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));
                   
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createTransformTagWidget(const ModuleParameter& moduleParameter)
{
  QString type = QString::fromStdString(moduleParameter.GetType());
  QString nodeType = Self::nodeTypeFromMap("vtkMRMLTransformNode",
                                           Self::TransformTypeAttributeToNodeType, type);

  QString channel = QString::fromStdString(moduleParameter.GetChannel());
  if (channel != "input" && channel != "output")
    {
    qWarning() << "GeometryTag - Unknown channel:" << channel;
    return 0; 
    }
    
  // TODO - title + " Transform"
  // TODO - SetNoneEnabled(noneEnabled);

  qMRMLNodeSelector * widget = new qMRMLNodeSelector;
  widget->setNodeType(nodeType);
  widget->setMRMLScene(this->BackPointer->mrmlScene());
  QObject::connect(this->BackPointer, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));
                   
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createDirectoryTagWidget(const ModuleParameter& moduleParameter)
{
  QPushButton* widget = new QPushButton("Select directory ...");
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createFileTagWidget(const ModuleParameter& moduleParameter)
{
  QPushButton* widget = new QPushButton("Select file ...");
  return widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModule::qInternal::createEnumerationTagWidget(const ModuleParameter& moduleParameter)
{
  QString defaultValue = QString::fromStdString(moduleParameter.GetDefault());
  
  // iterate over each element in this parameter
  ElementConstIterator sBeginIt = moduleParameter.GetElements().begin();
  ElementConstIterator sEndIt = moduleParameter.GetElements().end();

  QWidget * widget = new QWidget;
  qCTKFlowLayout * layout = new qCTKFlowLayout;
  widget->setLayout(layout);

  for (ElementConstIterator sIt = sBeginIt; sIt != sEndIt; ++sIt)
    {
    QString value = QString::fromStdString(*sIt); 
    QRadioButton * radio = new QRadioButton(value);
    layout->addWidget(radio);
    radio->setChecked(defaultValue == value);
    }
  return widget;
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::initializeMaps()
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
QString qSlicerCLIModule::qInternal::nodeTypeFromMap(const QString& defaultValue,
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
