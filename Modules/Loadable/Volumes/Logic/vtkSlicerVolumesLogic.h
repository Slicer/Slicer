/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumesLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerVolumesLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing properties of volumes


#ifndef __vtkSlicerVolumesLogic_h
#define __vtkSlicerVolumesLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLVolumeNode.h"

// STD includes
#include <cstdlib>
#include <list>

#include "vtkSlicerVolumesModuleLogicExport.h"

class vtkMRMLLabelMapVolumeNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLScalarVolumeDisplayNode;
class vtkMRMLVolumeHeaderlessStorageNode;
class vtkStringArray;

struct ArchetypeVolumeNodeSet
{
  ArchetypeVolumeNodeSet(vtkMRMLScene * scene):Scene(scene), LabelMap(false){}
  ArchetypeVolumeNodeSet(const ArchetypeVolumeNodeSet& set) {
    Node = set.Node;
    DisplayNode = set.DisplayNode;
    StorageNode = set.StorageNode;
    Scene = set.Scene;
    LabelMap = set.LabelMap;
  }
  vtkSmartPointer<vtkMRMLVolumeNode> Node;
  vtkSmartPointer<vtkMRMLVolumeDisplayNode> DisplayNode;
  vtkSmartPointer<vtkMRMLStorageNode> StorageNode;
  vtkSmartPointer<vtkMRMLScene> Scene;
  bool LabelMap;  // is this node set for labelmaps?
};

class VTK_SLICER_VOLUMES_MODULE_LOGIC_EXPORT vtkSlicerVolumesLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerVolumesLogic *New();
  vtkTypeMacro(vtkSlicerVolumesLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  typedef vtkSlicerVolumesLogic Self;

  /// Loading options, bitfield
  enum LoadingOptions {
    LabelMap = 1,
    CenterImage = 2,
    SingleFile = 4,
    AutoWindowLevel = 8,
    DiscardOrientation = 16
  };

  /// Factory function to create a volume node, display node, and
  /// storage node, configure the in the specified scene, and
  /// initialize the storage node with the "options".
  typedef ArchetypeVolumeNodeSet (*ArchetypeVolumeNodeSetFactory)(std::string& volumeName, vtkMRMLScene* scene, int options);

  virtual void SetColorLogic(vtkMRMLColorLogic* colorLogic);
  vtkMRMLColorLogic* GetColorLogic()const;

  /// Examine the file name to see if the extension is one of the supported
  /// freesurfer volume formats. Used to assign the proper colour node to label maps.
  int IsFreeSurferVolume(const char* filename);

  /// The currently active mrml volume node
  void SetActiveVolumeNode(vtkMRMLVolumeNode *ActiveVolumeNode);
  vtkMRMLVolumeNode* GetActiveVolumeNode()const;


  /// Register a factory method that can create and configure a node
  /// set (ArchetypeVolumeNodeSet) containing a volume node, display
  /// node, and storage node. The nodes are configured within the
  /// factory method with default settings and are added to the scene
  /// and cross-referenced appropriately. Node types must be
  /// registered with the scene beforehand the factory is
  /// called. Factories are tested in the order they are registered.
  void RegisterArchetypeVolumeNodeSetFactory(ArchetypeVolumeNodeSetFactory factory);

  /// Register a factory method that can create and configure a node
  /// set (ArchetypeVolumeNodeSet) containing a volume node, display
  /// node, and storage node. The nodes are configured within the
  /// factory method with default settings and are added to the scene
  /// and cross-referenced appropriately. Node types must be
  /// registered with the scene beforehand the factory is called.
  /// This version inserts the factory at the head of the list, and
  /// hence the factory will be tested first, rather than pushing onto
  /// the back of the list of factories.
  void PreRegisterArchetypeVolumeNodeSetFactory(ArchetypeVolumeNodeSetFactory factory);

  /// Overloaded function of AddArchetypeVolume to provide more
  /// loading options, where variable loadingOptions is bit-coded as following:
  /// bit 0: label map
  /// bit 1: centered
  /// bit 2: loading single file
  /// bit 3: calculate window level automatically
  /// bit 4: discard image orientation
  /// higher bits are reserved for future use
  vtkMRMLVolumeNode* AddArchetypeVolume (const char* filename, const char* volname, int loadingOptions)
    {
    return (this->AddArchetypeVolume( filename, volname, loadingOptions, nullptr));
    }
  vtkMRMLVolumeNode* AddArchetypeVolume (const char* filename, const char* volname, int loadingOptions, vtkStringArray *fileList);
  vtkMRMLVolumeNode* AddArchetypeVolume (const char *filename, const char* volname)
    {
    return this->AddArchetypeVolume( filename, volname, 0, nullptr);
    }

  /// Load a scalar volume function directly, bypassing checks of all factories done in AddArchetypeVolume.
  /// \sa AddArchetypeVolume(const NodeSetFactoryRegistry& volumeRegistry, const char* filename, const char* volname, int loadingOptions, vtkStringArray *fileList)
  vtkMRMLScalarVolumeNode* AddArchetypeScalarVolume(const char* filename, const char* volname, int loadingOptions, vtkStringArray *fileList);

  /// Write volume's image data to a specified file
  int SaveArchetypeVolume (const char* filename, vtkMRMLVolumeNode *volumeNode);

  /// Create a label map volume to match the given \a volumeNode and add it to the current scene
  /// \sa GetMRMLScene()
  vtkMRMLLabelMapVolumeNode *CreateAndAddLabelVolume(vtkMRMLVolumeNode *volumeNode,
                                                   const char *name);

  /// Create a label map volume to match the given \a volumeNode and add it to the \a scene
  vtkMRMLLabelMapVolumeNode *CreateAndAddLabelVolume(vtkMRMLScene *scene,
                                                   vtkMRMLVolumeNode *volumeNode,
                                                   const char *name);
  /// \deprecated
  /// Create a label map volume to match the given \a volumeNode and add it to
  /// the current scene.
  /// \sa CreateAndAddLabelVolume
  vtkMRMLLabelMapVolumeNode *CreateLabelVolume(vtkMRMLVolumeNode *volumeNode,
                                             const char *name);
  /// \deprecated
  /// Create a label map volume to match the given \a volumeNode and add it to the \a scene
  /// \sa CreateAndAddLabelVolume
  vtkMRMLLabelMapVolumeNode *CreateLabelVolume(vtkMRMLScene *scene,
                                             vtkMRMLVolumeNode *volumeNode,
                                             const char *name);

  /// \deprecated
  /// Fill in a label map volume to match the given template volume node.
  /// \sa FillLabelVolumeFromTemplate(vtkMRMLScene*, vtkMRMLScalarVolumeNode*, vtkMRMLVolumeNode*)
  /// \sa GetMRMLScene()
  vtkMRMLLabelMapVolumeNode *FillLabelVolumeFromTemplate(vtkMRMLLabelMapVolumeNode *labelNode,
                                                       vtkMRMLVolumeNode *templateNode);

  /// \deprecated
  /// Fill in a label map volume to match the given template volume node, under
  /// the assumption that the given label map node is already added to the scene.
  /// A display node will be added to it if the label node doesn't already have
  /// one, and the image data associated with the label node will be allocated
  /// according to the template volumeNode.
  vtkMRMLLabelMapVolumeNode *FillLabelVolumeFromTemplate(vtkMRMLScene *scene,
                                                       vtkMRMLLabelMapVolumeNode *labelNode,
                                                       vtkMRMLVolumeNode *templateNode);

  /// Set a label map volume to match the given input volume node, under
  /// the assumption that the given label map node is already added to the scene.
  /// A display node will be added to it if the label node doesn't already have
  /// one, and the image data associated with the label node will be allocated
  /// according to the template volumeNode.
  vtkMRMLLabelMapVolumeNode *CreateLabelVolumeFromVolume(vtkMRMLScene *scene,
                                                       vtkMRMLLabelMapVolumeNode *outputVolume,
                                                       vtkMRMLVolumeNode *inputVolume);

  /// Set a scalar volume to match the given input volume node, under
  /// the assumption that the given label map node is already added to the scene.
  /// A display node will be added to it if the label node doesn't already have
  /// one, and the image data associated with the label node will be allocated
  /// according to the template volumeNode.
  vtkMRMLScalarVolumeNode *CreateScalarVolumeFromVolume(vtkMRMLScene *scene,
    vtkMRMLScalarVolumeNode *outputVolume,
    vtkMRMLVolumeNode *inputVolume);

  /// Clear the image data of a volume node to contain all zeros
  static void ClearVolumeImageData(vtkMRMLVolumeNode *volumeNode);

  /// Return a string listing any warnings about the spatial validity of
  /// the labelmap with respect to the volume.  An empty string indicates
  /// that the two volumes are identical samplings of the same spatial
  /// region and that the second volume input is a label map.
  /// \sa CompareVolumeGeometry
  std::string CheckForLabelVolumeValidity(vtkMRMLScalarVolumeNode *volumeNode,
                                          vtkMRMLLabelMapVolumeNode *labelNode);

  /// Generate a string listing any warnings about the spatial validity of
  /// the second volume with respect to the first volume.  An empty string
  /// indicates that the two volumes are identical samplings of the same
  /// spatial region.
  /// Checks include:
  ///  Valid image data.
  ///  Same dimensions.
  ///  Same spacing.
  ///  Same origin.
  ///  Same IJKtoRAS.
  /// \sa CheckForLabelVolumeValidity, ResampleVolumeToReferenceVolume
  std::string CompareVolumeGeometry(vtkMRMLScalarVolumeNode *volumeNode1,
                                    vtkMRMLScalarVolumeNode *volumeNode2);


  /// Create a deep copy of a \a volumeNode and add it to the current scene.
  /// If cloneImageData is false then the volume node is created without image data.
  /// \sa GetMRMLScene()
  vtkMRMLScalarVolumeNode *CloneVolume(vtkMRMLVolumeNode *volumeNode, const char *name);

  /// Create a empty copy of a \a volumeNode without imageData and add it to the current scene
  /// \sa GetMRMLScene()
  static vtkMRMLScalarVolumeNode *CloneVolumeWithoutImageData(vtkMRMLScene *scene,
                                                              vtkMRMLVolumeNode *volumeNode,
                                                              const char *name);

  /// Create a deep copy of a \a volumeNode and add it to the \a scene
  /// Only works for vtkMRMLScalarVolumeNode.
  /// The method is kept as is for background compatibility only, internally it calls CloneVolumeGeneric.
  /// \sa CloneVolumeGeneric
  static vtkMRMLScalarVolumeNode *CloneVolume(vtkMRMLScene *scene,
                                              vtkMRMLVolumeNode *volumeNode,
                                              const char *name,
                                              bool cloneImageData=true);
  /// Create a deep copy of a \a volumeNode and add it to the \a scene
  static vtkMRMLVolumeNode *CloneVolumeGeneric(vtkMRMLScene *scene,
    vtkMRMLVolumeNode *volumeNode,
    const char *name,
    bool cloneImageData = true);

  /// Computes matrix we need to register
  /// V1Node to V2Node given the "register.dat" matrix from tkregister2 (FreeSurfer)
  void TranslateFreeSurferRegistrationMatrixIntoSlicerRASToRASMatrix(vtkMRMLVolumeNode *V1Node,
                             vtkMRMLVolumeNode *V2Node,
                             vtkMatrix4x4 *FSRegistrationMatrix,
                             vtkMatrix4x4 *ResultsMatrix);

  /// Convenience method to compute a volume's Vox2RAS-tkreg Matrix
  void ComputeTkRegVox2RASMatrix ( vtkMRMLVolumeNode *VNode,
                                   vtkMatrix4x4 *M );

  /// Center the volume on the origin (0,0,0)
  /// \sa GetVolumeCenteredOrigin()
  void CenterVolume(vtkMRMLVolumeNode *volumeNode);

  /// Compute the origin of the volume in order for the volume to be centered.
  /// \sa CenterVolume()
  void GetVolumeCenteredOrigin(vtkMRMLVolumeNode *volumeNode, double* origin);

  ///  Convenience method to resample input volume using reference volume info
  /// \sa CompareVolumeGeometry
  static vtkMRMLScalarVolumeNode* ResampleVolumeToReferenceVolume(vtkMRMLVolumeNode *inputVolumeNode,
                                                           vtkMRMLVolumeNode *referenceVolumeNode);

  /// Getting the epsilon value to use when determining if the
  /// elements of the IJK to RAS matrices of two volumes match.
  /// Defaults to 10 to the minus 6.
  vtkGetMacro(CompareVolumeGeometryEpsilon, double);
  /// Setting the epsilon value and associated precision to use when determining
  /// if the elements of the IJK to RAS matrices of two volumes match and how to
  /// print out the mismatched elements.
  void SetCompareVolumeGeometryEpsilon(double epsilon);

  /// Get the precision with which to print out volume geometry mismatches,
  /// value is set when setting the compare volume geometry epsilon.
  /// \sa SetCompareVolumeGeometryEpsilon
  vtkGetMacro(CompareVolumeGeometryPrecision, int);

protected:
  vtkSlicerVolumesLogic();
  ~vtkSlicerVolumesLogic() override;
  vtkSlicerVolumesLogic(const vtkSlicerVolumesLogic&);
  void operator=(const vtkSlicerVolumesLogic&);

  void ProcessMRMLNodesEvents(vtkObject * caller,
                                  unsigned long event,
                                  void * callData) override;


  void InitializeStorageNode(vtkMRMLStorageNode * storageNode,
                             const char * filename,
                             vtkStringArray *fileList,
                             vtkMRMLScene * mrmlScene = nullptr);

  void SetAndObserveColorToDisplayNode(vtkMRMLDisplayNode* displayNode,
                                       int labelmap, const char* filename);

  typedef std::list<ArchetypeVolumeNodeSetFactory> NodeSetFactoryRegistry;

  /// Convenience function allowing to try to load a volume using a given
  /// list of \a NodeSetFactoryRegistry
  vtkMRMLVolumeNode* AddArchetypeVolume(
      const NodeSetFactoryRegistry& volumeRegistry,
      const char* filename, const char* volname, int loadingOptions,
      vtkStringArray *fileList);

protected:
  vtkSmartPointer<vtkMRMLVolumeNode> ActiveVolumeNode;

  vtkSmartPointer<vtkMRMLColorLogic> ColorLogic;

  NodeSetFactoryRegistry VolumeRegistry;

  /// Allowable difference in comparing volume geometry double values.
  /// Defaults to 1 to the power of 10 to the minus 6
  double CompareVolumeGeometryEpsilon;

  /// Error print out precision, paried with CompareVolumeGeometryEpsilon.
  /// defaults to 6
  int CompareVolumeGeometryPrecision;
};

#endif
