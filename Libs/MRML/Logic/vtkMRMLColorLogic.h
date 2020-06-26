/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkMRMLColorLogic_h
#define __vtkMRMLColorLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"
#include "vtkMRMLLogicExport.h"

// MRML includes
class vtkMRMLColorNode;
class vtkMRMLColorTableNode;
class vtkMRMLProceduralColorNode;
class vtkMRMLPETProceduralColorNode;
class vtkMRMLdGEMRICProceduralColorNode;
class vtkMRMLColorTableNode;

// STD includes
#include <cstdlib>
#include <vector>

/// \brief MRML logic class for color manipulation.
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the colors.
class VTK_MRML_LOGIC_EXPORT vtkMRMLColorLogic : public vtkMRMLAbstractLogic
{
public:

  /// The Usual vtk class functions
  static vtkMRMLColorLogic *New();
  vtkTypeMacro(vtkMRMLColorLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// \brief Add default color nodes.
  ///
  /// The default color nodes are singleton and are not included in the
  /// the saved scene.
  ///
  /// This function enables the vtkMRMLScene::BatchProcessState.
  ///
  /// The type of default nodes along with their properties are listed
  /// in the table below:
  ///
  /// | Family                    | Category                 | Type                           | Node name                      | Singleton Tag                         | Node ID                                                     |
  /// | ------------------------- | ------------------------ | ------------------------------ | ------------------------------ | ------------------------------------- | ----------------------------------------------------------- |
  /// | ColorTable                | Discrete                 | Labels                         | Labels                         | Labels                                | vtkMRMLColorTableNodeLabels                                 |
  /// | ColorTable                | Discrete                 | FullRainbow                    | FullRainbow                    | FullRainbow                           | vtkMRMLColorTableNodeFullRainbow                            |
  /// | ColorTable                | Discrete                 | Grey                           | Grey                           | Grey                                  | vtkMRMLColorTableNodeGrey                                   |
  /// | ColorTable                | Discrete                 | Iron                           | Iron                           | Iron                                  | vtkMRMLColorTableNodeIron                                   |
  /// | ColorTable                | Discrete                 | Rainbow                        | Rainbow                        | Rainbow                               | vtkMRMLColorTableNodeRainbow                                |
  /// | ColorTable                | Discrete                 | Ocean                          | Ocean                          | Ocean                                 | vtkMRMLColorTableNodeOcean                                  |
  /// | ColorTable                | Discrete                 | Desert                         | Desert                         | Desert                                | vtkMRMLColorTableNodeDesert                                 |
  /// | ColorTable                | Discrete                 | InvertedGrey                   | InvertedGrey                   | InvertedGrey                          | vtkMRMLColorTableNodeInvertedGrey                           |
  /// | ColorTable                | Discrete                 | ReverseRainbow                 | ReverseRainbow                 | ReverseRainbow                        | vtkMRMLColorTableNodeReverseRainbow                         |
  /// | ColorTable                | Discrete                 | fMRI                           | fMRI                           | fMRI                                  | vtkMRMLColorTableNodefMRI                                   |
  /// | ColorTable                | Discrete                 | fMRIPA                         | fMRIPA                         | fMRIPA                                | vtkMRMLColorTableNodefMRIPA                                 |
  /// | ColorTable                | Discrete                 | Random                         | Random                         | Random                                | vtkMRMLColorTableNodeRandom                                 |
  /// | ColorTable                | Discrete                 | Red                            | Red                            | Red                                   | vtkMRMLColorTableNodeRed                                    |
  /// | ColorTable                | Discrete                 | Green                          | Green                          | Green                                 | vtkMRMLColorTableNodeGreen                                  |
  /// | ColorTable                | Discrete                 | Blue                           | Blue                           | Blue                                  | vtkMRMLColorTableNodeBlue                                   |
  /// | ColorTable                | Discrete                 | Yellow                         | Yellow                         | Yellow                                | vtkMRMLColorTableNodeYellow                                 |
  /// | ColorTable                | Discrete                 | Cyan                           | Cyan                           | Cyan                                  | vtkMRMLColorTableNodeCyan                                   |
  /// | ColorTable                | Discrete                 | Magenta                        | Magenta                        | Magenta                               | vtkMRMLColorTableNodeMagenta                                |
  /// | ColorTable                | Discrete                 | Warm1                          | Warm1                          | Warm1                                 | vtkMRMLColorTableNodeWarm1                                  |
  /// | ColorTable                | Discrete                 | Warm2                          | Warm2                          | Warm2                                 | vtkMRMLColorTableNodeWarm2                                  |
  /// | ColorTable                | Discrete                 | Warm3                          | Warm3                          | Warm3                                 | vtkMRMLColorTableNodeWarm3                                  |
  /// | ColorTable                | Discrete                 | Cool1                          | Cool1                          | Cool1                                 | vtkMRMLColorTableNodeCool1                                  |
  /// | ColorTable                | Discrete                 | Cool2                          | Cool2                          | Cool2                                 | vtkMRMLColorTableNodeCool2                                  |
  /// | ColorTable                | Discrete                 | Cool3                          | Cool3                          | Cool3                                 | vtkMRMLColorTableNodeCool3                                  |
  /// | ColorTable                | Shade                    | WarmShade1                     | WarmShade1                     | WarmShade1                            | vtkMRMLColorTableNodeWarmShade1                             |
  /// | ColorTable                | Shade                    | WarmShade2                     | WarmShade2                     | WarmShade2                            | vtkMRMLColorTableNodeWarmShade2                             |
  /// | ColorTable                | Shade                    | WarmShade3                     | WarmShade3                     | WarmShade3                            | vtkMRMLColorTableNodeWarmShade3                             |
  /// | ColorTable                | Shade                    | CoolShade1                     | CoolShade1                     | CoolShade1                            | vtkMRMLColorTableNodeCoolShade1                             |
  /// | ColorTable                | Shade                    | CoolShade2                     | CoolShade2                     | CoolShade2                            | vtkMRMLColorTableNodeCoolShade2                             |
  /// | ColorTable                | Shade                    | CoolShade3                     | CoolShade3                     | CoolShade3                            | vtkMRMLColorTableNodeCoolShade3                             |
  /// | ColorTable                | Tint                     | WarmTint1                      | WarmTint1                      | WarmTint1                             | vtkMRMLColorTableNodeWarmTint1                              |
  /// | ColorTable                | Tint                     | WarmTint2                      | WarmTint2                      | WarmTint2                             | vtkMRMLColorTableNodeWarmTint2                              |
  /// | ColorTable                | Tint                     | WarmTint3                      | WarmTint3                      | WarmTint3                             | vtkMRMLColorTableNodeWarmTint3                              |
  /// | ColorTable                | Tint                     | CoolTint1                      | CoolTint1                      | CoolTint1                             | vtkMRMLColorTableNodeCoolTint1                              |
  /// | ColorTable                | Tint                     | CoolTint2                      | CoolTint2                      | CoolTint2                             | vtkMRMLColorTableNodeCoolTint2                              |
  /// | ColorTable                | Tint                     | CoolTint3                      | CoolTint3                      | CoolTint3                             | vtkMRMLColorTableNodeCoolTint3                              |
  /// | ProceduralColor           | Discrete                 | RandomIntegers                 | RandomIntegers                 | RandomIntegers                        | vtkMRMLProceduralColorNodeRandomIntegers                    |
  /// | ProceduralColor           | Continuous               | RedGreenBlue                   | RedGreenBlue                   | RedGreenBlue                          | vtkMRMLProceduralColorNodeRedGreenBlue                      |
  /// | PETProceduralColor        | PET                      | PET-Heat                       | PET-Heat                       | PET-Heat                              | vtkMRMLPETProceduralColorNodePET-Heat                       |
  /// | PETProceduralColor        | PET                      | PET-Rainbow                    | PET-Rainbow                    | PET-Rainbow                           | vtkMRMLPETProceduralColorNodePET-Rainbow                    |
  /// | PETProceduralColor        | PET                      | PET-MaximumIntensityProjection | PET-MaximumIntensityProjection | PET-MaximumIntensityProjection        | vtkMRMLPETProceduralColorNodePET-MaximumIntensityProjection |
  /// | dGEMRICProceduralColor    | Cartilage MRI            | dGEMRIC-1.5T                   | dGEMRIC-1.5T                   | dGEMRIC-1.5T                          | vtkMRMLdGEMRICProceduralColorNodedGEMRIC-1.5T               |
  /// | dGEMRICProceduralColor    | Cartilage MRI            | dGEMRIC-3T                     | dGEMRIC-3T                     | dGEMRIC-3T                            | vtkMRMLdGEMRICProceduralColorNodedGEMRIC-3T                 |
  /// | ColorTable                | Default Labels from File | File                           | LightPaleChartColors           | FileLightPaleChartColors.txt          | vtkMRMLColorTableNodeFileLightPaleChartColors.txt           |
  /// | ColorTable                | Default Labels from File | File                           | ColdToHotRainbow               | FileColdToHotRainbow.txt              | vtkMRMLColorTableNodeFileColdToHotRainbow.txt               |
  /// | ColorTable                | Default Labels from File | File                           | Viridis                        | FileViridis.txt                       | vtkMRMLColorTableNodeFileViridis.txt                        |
  /// | ColorTable                | Default Labels from File | File                           | Magma                          | FileMagma.txt                         | vtkMRMLColorTableNodeFileMagma.txt                          |
  /// | ColorTable                | Default Labels from File | File                           | DivergingBlueRed               | FileDivergingBlueRed.txt              | vtkMRMLColorTableNodeFileDivergingBlueRed.txt               |
  /// | ColorTable                | Default Labels from File | File                           | HotToColdRainbow               | FileHotToColdRainbow.txt              | vtkMRMLColorTableNodeFileHotToColdRainbow.txt               |
  /// | ColorTable                | Default Labels from File | File                           | DarkBrightChartColors          | FileDarkBrightChartColors.txt         | vtkMRMLColorTableNodeFileDarkBrightChartColors.txt          |
  /// | ColorTable                | Default Labels from File | File                           | MediumChartColors              | FileMediumChartColors.txt             | vtkMRMLColorTableNodeFileMediumChartColors.txt              |
  /// | ColorTable                | Default Labels from File | File                           | Slicer3_2010_Label_Colors      | FileSlicer3_2010_Label_Colors.txt     | vtkMRMLColorTableNodeFileSlicer3_2010_Label_Colors.txt      |
  /// | ColorTable                | Default Labels from File | File                           | Slicer3_2010_Brain_Labels      | FileSlicer3_2010_Brain_Labels.txt     | vtkMRMLColorTableNodeFileSlicer3_2010_Brain_Labels.txt      |
  /// | ColorTable                | Default Labels from File | File                           | Inferno                        | FileInferno.txt                       | vtkMRMLColorTableNodeFileInferno.txt                        |
  /// | ColorTable                | Default Labels from File | File                           | PelvisColor                    | FilePelvisColor.txt                   | vtkMRMLColorTableNodeFilePelvisColor.txt                    |
  /// | ColorTable                | Default Labels from File | File                           | SPL-BrainAtlas-ColorFile       | FileSPL-BrainAtlas-ColorFile.txt      | vtkMRMLColorTableNodeFileSPL-BrainAtlas-ColorFile.txt       |
  /// | ColorTable                | Default Labels from File | File                           | AbdomenColors                  | FileAbdomenColors.txt                 | vtkMRMLColorTableNodeFileAbdomenColors.txt                  |
  /// | ColorTable                | None                     | File                           | GenericColors                  | FileGenericColors.txt                 | vtkMRMLColorTableNodeFileGenericColors.txt                  |
  /// | ColorTable                | Default Labels from File | File                           | 64Color-Nonsemantic            | File64Color-Nonsemantic.txt           | vtkMRMLColorTableNodeFile64Color-Nonsemantic.txt            |
  /// | ColorTable                | Default Labels from File | File                           | Plasma                         | FilePlasma.txt                        | vtkMRMLColorTableNodeFilePlasma.txt                         |
  /// | ColorTable                | Default Labels from File | File                           | Cividis                        | FileCividis.txt                       | vtkMRMLColorTableNodeFileCividis.txt                        |
  /// | ColorTable                | Default Labels from File | File                           | SPL-BrainAtlas-2009-ColorFile  | FileSPL-BrainAtlas-2009-ColorFile.txt | vtkMRMLColorTableNodeFileSPL-BrainAtlas-2009-ColorFile.txt  |
  /// | ColorTable                | Default Labels from File | File                           | SPL-BrainAtlas-2012-ColorFile  | FileSPL-BrainAtlas-2012-ColorFile.txt | vtkMRMLColorTableNodeFileSPL-BrainAtlas-2012-ColorFile.txt  |
  /// | ColorTable                | None                     | File                           | GenericAnatomyColors           | FileGenericAnatomyColors.txt          | vtkMRMLColorTableNodeFileGenericAnatomyColors.txt           |
  ///
  /// \note The table has been generated using Libs/MRML/Core/Documentation/generate_default_color_node_property_table.py
  ///
  /// \sa vtkMRMLNode::GetSingletonTag(), vtkMRMLScene::Commit()
  /// \sa RemoveDefaultColorNodes()
  ///
  /// \sa AddLabelsNode()
  /// \sa AddDefaultTableNodes()
  /// \sa AddDefaultProceduralNodes()
  /// \sa AddPETNodes()
  /// \sa AddDGEMRICNodes()
  /// \sa AddDefaultFileNodes()
  /// \sa AddUserFileNodes()
  virtual void AddDefaultColorNodes();

  /// \brief Remove default color nodes.
  ///
  /// \sa AddDefaultColorNodes()
  virtual void RemoveDefaultColorNodes();

  /// Return the default color table node id for a given type
  static const char * GetColorTableNodeID(int type);

  /// Return the default dGEMRIC color node id for a given type
  static const char * GetdGEMRICColorNodeID(int type);

  /// Return the default PET color node id for a given type
  static const char * GetPETColorNodeID(int type);

  /// \brief Return a default color node id for a procedural color node.
  ///
  /// \warning You are responsible to delete the returned string.
  static const char * GetProceduralColorNodeID(const char *name);

  /// \brief Return a default color node id for a file based node,
  /// based on the file name.
  ///
  /// \warning You are responsible to delete the returned string.
  static const char * GetFileColorNodeID(const char *fileName);
  static std::string  GetFileColorNodeSingletonTag(const char * fileName);

  /// Return a default color node id for a volume
  virtual const char * GetDefaultVolumeColorNodeID();

  /// Return a default color node id for a label map
  virtual const char * GetDefaultLabelMapColorNodeID();

  /// Return a default color node id for the editor
  virtual const char * GetDefaultEditorColorNodeID();

  /// Return a default color node id for a model
  virtual const char * GetDefaultModelColorNodeID();

  /// Return a default color node id for a chart
  virtual const char * GetDefaultChartColorNodeID();

  /// Return a default color node id for a plot
  virtual const char * GetDefaultPlotColorNodeID();

  /// Add a file to the input list Files, checking first for null, duplicates
  void AddColorFile(const char *fileName, std::vector<std::string> *Files);

  /// Load in a color file, creating a storage node. Returns a pointer to the
  /// created node on success, 0 on failure (no file, invalid color file). The
  /// name of the created color node is \a nodeName if specified or
  /// the fileName otherwise. Try first to load it as a color table
  /// node, then if that fails, as a procedural color node. It calls
  /// CreateFileNode or CreateProceduralFileNode which are also used
  /// for the built in color nodes, so it has to unset some flags: set
  /// the category to File, turn save with scene on on the node and
  /// it's storage node, turn off hide from editors, remove the
  /// singleton tag.
  /// \sa CreateFileNode, CreateProceduralFileNode
  vtkMRMLColorNode* LoadColorFile(const char *fileName, const char *nodeName = nullptr);

  /// Get/Set the user defined paths where to look for extra colour files
  vtkGetStringMacro(UserColorFilePaths);
  vtkSetStringMacro(UserColorFilePaths);

  /// Returns a vtkMRMLColorTableNode copy (type = vtkMRMLColorTableNode::User)
  /// of the \a color node. The node is not added to the scene and you are
  /// responsible for deleting it.
  static vtkMRMLColorTableNode* CopyNode(vtkMRMLColorNode* colorNode, const char* copyName);

  /// Returns a vtkMRMLProceduralColorNode copy (type = vtkMRMLColorTableNode::User)
  /// of the \a color node. The node is not added to the scene and you are
  /// responsible for deleting it. If there is no color transfer function on the
  /// input node, for example if it's a color table node, it will return a
  /// procedural node with a blank color transfer function.
  static vtkMRMLProceduralColorNode* CopyProceduralNode(vtkMRMLColorNode* colorNode, const char* copyName);

protected:
  vtkMRMLColorLogic();
  ~vtkMRMLColorLogic() override;
  // disable copy constructor and operator
  vtkMRMLColorLogic(const vtkMRMLColorLogic&);
  void operator=(const vtkMRMLColorLogic&);

  /// Reimplemented to listen to specific scene events
  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  /// Called when the scene fires vtkMRMLScene::NewSceneEvent.
  /// We add the default LUTs.
  virtual void OnMRMLSceneNewEvent();

  vtkMRMLColorTableNode* CreateLabelsNode();
  vtkMRMLColorTableNode* CreateDefaultTableNode(int type);
  vtkMRMLProceduralColorNode* CreateRandomNode();
  vtkMRMLProceduralColorNode* CreateRedGreenBlueNode();
  vtkMRMLPETProceduralColorNode* CreatePETColorNode(int type);
  vtkMRMLdGEMRICProceduralColorNode* CreatedGEMRICColorNode(int type);
  vtkMRMLColorTableNode* CreateDefaultFileNode(const std::string& colorname);
  vtkMRMLColorTableNode* CreateUserFileNode(const std::string& colorname);
  vtkMRMLColorTableNode* CreateFileNode(const char* fileName);
  vtkMRMLProceduralColorNode* CreateProceduralFileNode(const char* fileName);

  void AddLabelsNode();
  void AddDefaultTableNode(int i);
  void AddDefaultProceduralNodes();
  void AddPETNode(int type);
  void AddDGEMRICNode(int type);
  void AddDefaultFileNode(int i);
  void AddUserFileNode(int i);

  void AddDefaultTableNodes();
  void AddPETNodes();
  void AddDGEMRICNodes();
  void AddDefaultFileNodes();
  void AddUserFileNodes();

  virtual std::vector<std::string> FindDefaultColorFiles();
  virtual std::vector<std::string> FindUserColorFiles();

  /// Return the ID of a node that doesn't belong to a scene.
  /// It is the concatenation of the node class name and its type.
  static const char * GetColorNodeID(vtkMRMLColorNode* colorNode);

  /// a vector holding discovered default colour files, found in the
  /// Resources/ColorFiles directory, white space separated with:
  /// int name r g b a
  /// with rgba in the range 0-255
  std::vector<std::string> ColorFiles;

  /// a vector holding discovered user defined colour files, found in the
  /// UserColorFilesPath directories.
  std::vector<std::string> UserColorFiles;
  /// a string holding delimiter separated (; on win32, : else) paths where to
  /// look for extra colour files, set from the return value of
  /// vtkMRMLApplication::GetColorFilePaths
  char *UserColorFilePaths;

  static std::string TempColorNodeID;

  std::string RemoveLeadAndTrailSpaces(std::string);
};

#endif
