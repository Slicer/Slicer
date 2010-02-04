/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCLIModuleWidget_p_h
#define __qSlicerCLIModuleWidget_p_h

#include "qSlicerCLIModuleWidget.h"
#include "ui_qSlicerCLIModule.h"

/// SlicerQT includes
#include "qSlicerCLIModuleWidget.h"
#include "qSlicerWidget.h"

/// qCTK includes
#include <qCTKPimpl.h>

/// QT includes
#include <QHash>
#include <QList>

/// ModuleDescriptionParser includes
#include <ModuleDescription.h>

/// STL includes
#include <vector>

class vtkSlicerCLIModuleLogic; 
class QFormLayout;
class QBoxLayout;
class vtkMRMLCommandLineModuleNode; 

//-----------------------------------------------------------------------------
class WidgetValueWrapper
{
public:
  WidgetValueWrapper(const QString& _label):Label(_label){}
  virtual QVariant value() = 0;
  QString label(){ return this->Label; }
  QString Label; 
};

//-----------------------------------------------------------------------------
#define WIDGET_VALUE_WRAPPER(_NAME, _WIDGET, _VALUE_GETTER)           \
namespace{                                                            \
class _NAME##WidgetValueWrapper: public WidgetValueWrapper            \
{                                                                     \
public:                                                               \
  _NAME##WidgetValueWrapper(const QString& _label, _WIDGET * widget): \
    WidgetValueWrapper(_label), Widget(widget){}                      \
  virtual QVariant value()                                            \
    {                                                                 \
    QVariant _value(this->Widget->_VALUE_GETTER());                   \
    return _value;                                                    \
    }                                                                 \
  _WIDGET* Widget;                                                    \
};                                                                    \
}

//-----------------------------------------------------------------------------
#define INSTANCIATE_WIDGET_VALUE_WRAPPER(_NAME, _LABEL, _WIDGET_INSTANCE)  \
this->WidgetValueWrappers.push_back(new _NAME##WidgetValueWrapper(_LABEL, _WIDGET_INSTANCE));

//-----------------------------------------------------------------------------
class qSlicerCLIModuleWidgetPrivate: public QObject,
                                     public qCTKPrivate<qSlicerCLIModuleWidget>,
                                     public Ui_qSlicerCLIModule
{
  Q_OBJECT
  QCTK_DECLARE_PUBLIC(qSlicerCLIModuleWidget);
  
public:
  typedef qSlicerCLIModuleWidgetPrivate Self;
  qSlicerCLIModuleWidgetPrivate()
    {
    this->ProcessInformation = 0;
    this->Name = "NA";
    this->CommandLineModuleNode = 0; 
    }
  
  /// 
  /// Convenient function to cast qSlicerModuleLogic into qSlicerCLIModuleLogic
  vtkSlicerCLIModuleLogic* logic()const;

  typedef std::vector<ModuleParameterGroup>::const_iterator ParameterGroupConstIterator;
  typedef std::vector<ModuleParameterGroup>::iterator       ParameterGroupIterator;

  typedef std::vector<ModuleParameter>::const_iterator ParameterConstIterator;
  typedef std::vector<ModuleParameter>::iterator       ParameterIterator;

  typedef std::vector<std::string>::const_iterator ElementConstIterator;
  typedef std::vector<std::string>::iterator       ElementIterator;


  /// 
  /// Calling this method will loop trough the structure resulting
  /// from the XML parsing and generate the corresponding UI.
  void setupUi(qSlicerWidget* widget);
  
  /// 
  /// Initiliaze the maps containing the mapping
  ///   parameter type -> MRML node type (classname)
  static void initializeMaps();

  /// 
  /// Convenient method allowing to retrieve the node type associated
  /// with the parameter type
  static QString nodeTypeFromMap(const QString& defaultValue,
                                 const QHash<QString, QString>& map,
                                 const QString& attribute);
                                 
  /// 
  void addParameterGroups();
  void addParameterGroup(QBoxLayout* layout,
                         const ModuleParameterGroup& parameterGroup);

  /// 
  void addParameters(QFormLayout* layout, const ModuleParameterGroup& parameterGroup);
  void addParameter(QFormLayout* layout, const ModuleParameter& moduleParameter);

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
  /// Update MRMLCommandLineModuleNode properties using value entered using the UI
  void updateMRMLCommandLineModuleNode();

public slots:
  void onApplyButtonPressed();
  void onCancelButtonPressed();
  void onDefaultButtonPressed();

  void updateCommandButtonState();

public:

  /// Map used to store the different relation
  ///  parameter type -> MRML node type
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

  QList<WidgetValueWrapper*> WidgetValueWrappers; 
  
  vtkMRMLCommandLineModuleNode* CommandLineModuleNode; 
///   qSlicerCLIModuleLogic * Logic;
};


#endif
