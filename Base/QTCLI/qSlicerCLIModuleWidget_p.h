#ifndef __qSlicerCLIModuleWidget_p_h
#define __qSlicerCLIModuleWidget_p_h

#include "qSlicerCLIModuleWidget.h"
#include "ui_qSlicerCLIModule.h"

// SlicerQT includes
#include "qSlicerCLIModuleWidget.h"
#include "qSlicerWidget.h"

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QHash>

// ModuleDescriptionParser includes
#include <ModuleDescription.h>

// STL includes
#include <vector>

class QFormLayout;
class QBoxLayout; 

//-----------------------------------------------------------------------------
class qSlicerCLIModuleWidgetPrivate: public qSlicerWidget,
                                     public qCTKPrivate<qSlicerCLIModuleWidget>,
                                     public Ui_qSlicerCLIModule
{
  Q_OBJECT
  QCTK_DECLARE_PUBLIC(qSlicerCLIModuleWidget);
  
public:
  typedef qSlicerCLIModuleWidgetPrivate Self;
  qSlicerCLIModuleWidgetPrivate()
    {
//     this->Logic = 0;
    this->ProcessInformation = 0;
    this->Name = "NA"; 
    }

  typedef std::vector<ModuleParameterGroup>::const_iterator ParameterGroupConstIterator;
  typedef std::vector<ModuleParameterGroup>::iterator       ParameterGroupIterator;

  typedef std::vector<ModuleParameter>::const_iterator ParameterConstIterator;
  typedef std::vector<ModuleParameter>::iterator       ParameterIterator;

  typedef std::vector<std::string>::const_iterator ElementConstIterator;
  typedef std::vector<std::string>::iterator       ElementIterator;


  // Description:
  // Calling this method will loop trough the structure resulting
  // from the XML parsing and generate the corresponding UI.
  void setupUi(qSlicerWidget* widget);
  
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

public slots:
  void onApplyButtonPressed();
  void onCancelButtonPressed();
  void onDefaultButtonPressed();

public:

  // Map used to store the different relation
  //  parameter type -> MRML node type
  static bool MapInitialized; 
  static QHash<QString, QString> ImageTypeAttributeToNodeType;
  static QHash<QString, QString> GeometryTypeAttributeToNodeType;
  static QHash<QString, QString> TableTypeAttributeToNodeType;
  static QHash<QString, QString> TransformTypeAttributeToNodeType;  

  QString           Name;
  QString           Title;
  QString           Category;
  QString           Contributor;

  std::vector<ModuleParameterGroup> ParameterGroups;
  ModuleProcessInformation*         ProcessInformation;

//   qSlicerCLIModuleLogic * Logic;
};


#endif
