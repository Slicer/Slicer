#include "qSlicerCLIModule.h"
#include "ui_qSlicerCLIModule.h"

// SlicerQT includes
#include "qSlicerCLIModuleLogic.h"

// CTK includes
#include "qCTKCollapsibleWidget2.h"

// Libs/ModuleDescriptionParser includes
#include "ModuleDescription.h"
#include "ModuleDescriptionParser.h"

// QT includes
#include <QGroupBox>
#include <QLabel>
#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerCLIModule::qInternal: public Ui::qSlicerCLIModule
{
  qInternal()
    {
    this->Logic = 0;
    this->ProcessInformation = 0;
    }

  typedef std::vector<ModuleParameterGroup>::const_iterator ParameterGroupConstIterator;
  typedef std::vector<ModuleParameterGroup>::iterator       ParameterGroupIterator;

  typedef std::vector<ModuleParameter>::const_iterator ParameterConstIterator;
  typedef std::vector<ModuleParameter>::iterator       ParameterIterator;

  // Description:
  void addParameterGroups(QWidget * parent);
  void addParameterGroup(QBoxLayout* layout,
                          const ModuleParameterGroup& parameterGroup);

  // Description:
  void addParameters(QBoxLayout* layout, const ModuleParameterGroup& parameterGroup);
  void addParameter(QBoxLayout* layout, const ModuleParameter& moduleParameter);

  // Description:
  void addIntegerTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addBooleanTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addFloatTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addDoubleTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addStringTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addPointTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addRegionTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addImageTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addGeometryTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addTableTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addTransformTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addDirectoryTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addFileTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addEnumerationTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);
  void addDefaultTagWidget(QBoxLayout* layout, const ModuleParameter& moduleParameter);

  QString           Title;
  QString           Acknowledgement;
  QString           Help;
  QString           Category;
  QString           Contributor;

  std::vector<ModuleParameterGroup> ParameterGroups;
  ModuleProcessInformation*         ProcessInformation;

  qSlicerCLIModuleLogic * Logic;
};

//-----------------------------------------------------------------------------
qSlicerCxxInternalConstructor1Macro(qSlicerCLIModule, QWidget*);
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

  // TODO

  this->Internal->addParameterGroups(this);
}

//-----------------------------------------------------------------------------
// Internal methods

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addParameterGroups(QWidget * parent)
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

  qCTKCollapsibleWidget2 * collapsibleWidget = new qCTKCollapsibleWidget2();
  collapsibleWidget->setTitle(QString::fromStdString(parameterGroup.GetLabel()));
  collapsibleWidget->setCollapsed(parameterGroup.GetAdvanced() == "true");

  // Create a vertical layout and add parameter to it
  QVBoxLayout *vbox = new QVBoxLayout;
  this->addParameters(vbox, parameterGroup);
  vbox->addStretch(1);
  collapsibleWidget->setLayout(vbox);

  layout->addWidget(collapsibleWidget);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addParameters(QBoxLayout* layout,
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
void qSlicerCLIModule::qInternal::addParameter(QBoxLayout* layout,
                                               const ModuleParameter& moduleParameter)
{
  Q_ASSERT(layout);

  // Parameters with flags can support the None node because they are optional
  int noneEnabled = 0;
  if (moduleParameter.GetLongFlag() != "" || moduleParameter.GetFlag() != "")
    {
    noneEnabled = 1;
    }

  if (moduleParameter.GetTag() == "integer")
    {
    this->addIntegerTagWidget(layout, moduleParameter);
    }
  else if (moduleParameter.GetTag() == "boolean")
    {
    this->addBooleanTagWidget(layout, moduleParameter);
    }
  else if (moduleParameter.GetTag() == "float")
    {
    this->addFloatTagWidget(layout, moduleParameter);
    }
  else if (moduleParameter.GetTag() == "double")
    {
    this->addDoubleTagWidget(layout, moduleParameter);
    }
  else if (moduleParameter.GetTag() == "string" ||
           moduleParameter.GetTag() == "integer-vector" ||
           moduleParameter.GetTag() == "float-vector" ||
           moduleParameter.GetTag() == "double-vector" ||
           moduleParameter.GetTag() == "string-vector")
    {
    this->addStringTagWidget(layout, moduleParameter);
    }
  else if (moduleParameter.GetTag() == "point")
    {
    this->addPointTagWidget(layout, moduleParameter);
    }
  else if (moduleParameter.GetTag() == "region")
    {
    this->addRegionTagWidget(layout, moduleParameter);
    }
  else if (moduleParameter.GetTag() == "image")
    {
    this->addImageTagWidget(layout, moduleParameter);
    }
  else if (moduleParameter.GetTag() == "geometry")
    {
    this->addGeometryTagWidget(layout, moduleParameter);
    }
  else if (moduleParameter.GetTag() == "table")
    {
    this->addTableTagWidget(layout, moduleParameter);
    }
  else if (moduleParameter.GetTag() == "transform")
    {
    this->addTransformTagWidget(layout, moduleParameter);
    }
  else if (moduleParameter.GetTag() == "directory")
    {
    this->addDirectoryTagWidget(layout, moduleParameter);
    }
  else if (moduleParameter.GetTag() == "file")
    {
    this->addFileTagWidget(layout, moduleParameter);
    }
  else if(moduleParameter.GetTag() == "string-enumeration" ||
          moduleParameter.GetTag() == "integer-enumeration" ||
          moduleParameter.GetTag() == "float-enumeration" ||
          moduleParameter.GetTag() == "double-enumeration")
    {
    this->addEnumerationTagWidget(layout, moduleParameter);
    }
  else
    {
    this->addDefaultTagWidget(layout, moduleParameter);
    }

  // TODO Ballon help ?
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addIntegerTagWidget(QBoxLayout* layout,
                                                      const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addBooleanTagWidget(QBoxLayout* layout,
                                                      const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addFloatTagWidget(QBoxLayout* layout,
                                                    const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addDoubleTagWidget(QBoxLayout* layout,
                                                     const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addStringTagWidget(QBoxLayout* layout,
                                                     const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addPointTagWidget(QBoxLayout* layout,
                                                    const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addRegionTagWidget(QBoxLayout* layout,                                                                                                              const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addImageTagWidget(QBoxLayout* layout,
                                                    const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addGeometryTagWidget(QBoxLayout* layout,
                                                       const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addTableTagWidget(QBoxLayout* layout,
                                                    const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addTransformTagWidget(QBoxLayout* layout,
                                                        const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addDirectoryTagWidget(QBoxLayout* layout,
                                                        const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addFileTagWidget(QBoxLayout* layout,
                                                   const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addEnumerationTagWidget(QBoxLayout* layout,
                                                          const ModuleParameter& moduleParameter)
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::qInternal::addDefaultTagWidget(QBoxLayout* layout,
                                                      const ModuleParameter& moduleParameter)
{
  QLabel * label = new QLabel(QString::fromStdString(moduleParameter.GetLabel()));
  qDebug() << "label:" << QString::fromStdString(moduleParameter.GetLabel());
  layout->addWidget(label);
}
