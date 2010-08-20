/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCLIModuleWidget_p_h
#define __qSlicerCLIModuleWidget_p_h

// Qt includes
#include <QHash>
#include <QList>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerCLIModuleWidget.h"
#include "ui_qSlicerCLIModule.h"
#include "qSlicerCLIModuleWidget.h"
#include "qSlicerWidget.h"

// ModuleDescriptionParser includes
#include <ModuleDescription.h>

// STD includes
#include <vector>

class vtkSlicerCLIModuleLogic; 
class QFormLayout;
class QBoxLayout;
class vtkMRMLCommandLineModuleNode; 

//-----------------------------------------------------------------------------
class qSlicerCLIModuleUIHelper; 

//-----------------------------------------------------------------------------
class qSlicerCLIModuleWidgetPrivate: public QObject,
                                     public ctkPrivate<qSlicerCLIModuleWidget>,
                                     public Ui_qSlicerCLIModule
{
  Q_OBJECT
  CTK_DECLARE_PUBLIC(qSlicerCLIModuleWidget);
  
public:
  typedef qSlicerCLIModuleWidgetPrivate Self;
  qSlicerCLIModuleWidgetPrivate();
  
  /// 
  /// Convenient function to cast vtkSlicerLogic into vtkSlicerCLIModuleLogic
  vtkSlicerCLIModuleLogic* logic()const;

  /// 
  /// Convenient function to cast vtkMRMLNode into vtkMRMLCommandLineModuleNode
  vtkMRMLCommandLineModuleNode* commandLineModuleNode()const; 

  typedef std::vector<ModuleParameterGroup>::const_iterator ParameterGroupConstIterator;
  typedef std::vector<ModuleParameterGroup>::iterator       ParameterGroupIterator;

  typedef std::vector<ModuleParameter>::const_iterator ParameterConstIterator;
  typedef std::vector<ModuleParameter>::iterator       ParameterIterator;


  /// 
  /// Calling this method will loop trough the structure resulting
  /// from the XML parsing and generate the corresponding UI.
  virtual void setupUi(qSlicerWidget* widget);

  /// 
  void addParameterGroups();
  void addParameterGroup(QBoxLayout* layout,
                         const ModuleParameterGroup& parameterGroup);

  /// 
  void addParameters(QFormLayout* layout, const ModuleParameterGroup& parameterGroup);
  void addParameter(QFormLayout* layout, const ModuleParameter& moduleParameter);

public slots:
  void enableCommandButtonState(bool enable);

  /// Update the ui base on the command line module node
  void updateUiFromCommandLineModuleNode(vtkObject* commandLineModuleNode);
  void updateCommandLineModuleNodeFromUi(vtkObject* commandLineModuleNode);

  void setDefaultNodeValue(vtkMRMLNode* commandLineModuleNode);
  void onValueChanged(const QString& name, const QVariant& type);

public:

  QString           Name;
  QString           Title;
  QString           Category;
  QString           Contributor;

  std::vector<ModuleParameterGroup> ParameterGroups;
  ModuleProcessInformation*         ProcessInformation;
  ModuleDescription                 ModuleDescriptionObject;

  qSlicerCLIModuleUIHelper* CLIModuleUIHelper; 
  
  vtkMRMLCommandLineModuleNode* CommandLineModuleNode;
  QString                       ModuleEntryPoint;
  QString                       ModuleType;
};


#endif
