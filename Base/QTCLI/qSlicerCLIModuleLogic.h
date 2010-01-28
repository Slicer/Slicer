/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCLIModuleLogic_h
#define __qSlicerCLIModuleLogic_h

// SlicerLogic includes
#include "qSlicerModuleLogic.h"

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QStringList>

// STL includes
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include "qSlicerBaseQTCLIExport.h"

class vtkMRMLScene;
class vtkMRMLCommandLineModuleNode;
class qSlicerCLIModuleLogicPrivate;

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleLogic : public qSlicerModuleLogic
{
  //Q_OBJECT

public:

  typedef qSlicerModuleLogic Superclass;
  qSlicerCLIModuleLogic();

  virtual void setup();

  typedef enum { CommandLineModule, SharedObjectModule, PythonModule } CommandLineModuleType;

  std::string constructTemporaryFileName(const std::string& tag,
                             const std::string& type,
                             const std::string& name,
                             const std::vector<std::string>& extensions,
                             CommandLineModuleType commandType) const;

  // map to keep track of MRML Ids and filenames
  typedef std::map<std::string, std::string> MRMLIDToFileNameMap;
  MRMLIDToFileNameMap NodesToReload;
  MRMLIDToFileNameMap NodesToWrite;

  //
  // map to keep track of the MRML Ids on the main scene to the MRML
  // Ids in the miniscene sent to the module
  typedef std::map<std::string, std::string> MRMLIDMap;

  void applyTask(void *clientdata);

  // Prepare
  void getModuleInputAndOutputNode(CommandLineModuleType commandType, vtkMRMLCommandLineModuleNode * node0,
    MRMLIDToFileNameMap & nodesToWrite, MRMLIDToFileNameMap & nodesToReload);

  void generateInputDatasets(CommandLineModuleType commandType, vtkMRMLScene * miniscene, MRMLIDMap& sceneToMiniSceneMap,
    const MRMLIDToFileNameMap & nodesToWrite);

  void addOutputNodeToMiniScene(const std::string& minisceneFilename, vtkMRMLScene * miniscene, MRMLIDMap& sceneToMiniSceneMap,
    const MRMLIDToFileNameMap & nodesToReload);

  void buildCommandLine(const std::string& minisceneFilename, vtkMRMLCommandLineModuleNode * node0, MRMLIDMap& sceneToMiniSceneMap, CommandLineModuleType commandType, const std::string& target, QStringList& commandLineAsString);

  void processParametersWithIndices(const std::string& minisceneFilename, vtkMRMLCommandLineModuleNode * node0,
    MRMLIDMap& sceneToMiniSceneMap, QStringList& commandLineAsString);
//   std::string generateCommand(vtkMRMLCommandLineModuleNode * node0, const QStringList& commandLineAsString );

  // Run methods
  void runFilter(vtkMRMLCommandLineModuleNode * node0,
    const QStringList& argList, CommandLineModuleType commandType);
  void runCommandLineFilter(vtkMRMLCommandLineModuleNode * node0, const QStringList& argList);
  void runSharedObjectFilter(vtkMRMLCommandLineModuleNode * node0, const QStringList& argListist);
  void runPythonFilter(vtkMRMLCommandLineModuleNode * node0, const QStringList& argListst);

  // Terminate
  void onExecutionTerminated(vtkMRMLCommandLineModuleNode * node0, const MRMLIDMap& sceneToMiniSceneMap);
  void requestloadMinisceneLoading(const std::string& minisceneFilename,
    vtkMRMLCommandLineModuleNode * node0, vtkMRMLScene * miniscene, const MRMLIDMap& sceneToMiniSceneMap);
  void cleanUp();

  // For debug
  void setDeleteTemporaryFiles(bool enable);
  bool deleteTemporaryFiles()const;

  // Communicate progress back to the node
  static void ProgressCallback(void * who);

  // Set/Get the directory to use for temporary files
  void SetTemporaryDirectory(const char *tempdir);
  const char *GetTemporaryDirectory() const;

protected:
  std::string ConstructTemporarySceneFileName(vtkMRMLScene *scene);

  static void stringListToArray(const QStringList& list, std::vector<char*>& argv);

private:
  QCTK_DECLARE_PRIVATE(qSlicerCLIModuleLogic);
};

#endif
