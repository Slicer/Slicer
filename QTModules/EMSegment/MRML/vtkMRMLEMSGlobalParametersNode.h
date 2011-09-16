#ifndef __vtkMRMLEMSGlobalParametersNode_h
#define __vtkMRMLEMSGlobalParametersNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkEMSegment.h"
#include <vector>

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSGlobalParametersNode : 
  public vtkMRMLNode
{
public:
  static vtkMRMLEMSGlobalParametersNode *New();
  vtkTypeMacro(vtkMRMLEMSGlobalParametersNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  // virtual void UpdateReferences();

  // Description:
  // Update the stored reference to another node in the scene
  // virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes(const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "EMSGlobalParameters";}

  //
  // general parmeters
  //
  vtkSetVectorMacro(SegmentationBoundaryMin, int, 3);
  vtkGetVectorMacro(SegmentationBoundaryMin, int, 3);

  vtkSetVectorMacro(SegmentationBoundaryMax, int, 3);
  vtkGetVectorMacro(SegmentationBoundaryMax, int, 3);

  virtual void AddTargetInputChannel();
  virtual void RemoveNthTargetInputChannel(int n);
  virtual void MoveNthTargetInputChannel(int fromIndex, int toIndex);
  vtkGetMacro(NumberOfTargetInputChannels, int);

  //
  // registration parameters
  //
  vtkSetMacro(RegistrationAffineType, int);
  vtkGetMacro(RegistrationAffineType, int);

  vtkSetMacro(RegistrationDeformableType, int);
  vtkGetMacro(RegistrationDeformableType, int);

  vtkSetMacro(RegistrationInterpolationType, int);
  vtkGetMacro(RegistrationInterpolationType, int);

  vtkSetMacro(RegistrationPackageType, int);
  vtkGetMacro(RegistrationPackageType, int);

  void SetRegistrationAtlasVolumeKey(const char* key) {this->SetRegistrationAtlasVolumeKey(0,key);}
  void SetRegistrationAtlasVolumeKey(vtkIdType inputID, const char* key) ;

  const char* GetRegistrationAtlasVolumeKey() {return this->GetRegistrationAtlasVolumeKey(0);}
  const char* GetRegistrationAtlasVolumeKey(vtkIdType inputID);

  vtkSetStringMacro(RegistrationTargetVolumeKey);
  vtkGetStringMacro(RegistrationTargetVolumeKey);
  
  //
  // what to save...
  //
  vtkSetMacro(SaveIntermediateResults, int);
  vtkGetMacro(SaveIntermediateResults, int);

  vtkSetMacro(SaveSurfaceModels, int);
  vtkGetMacro(SaveSurfaceModels, int);

  //
  // how to run
  //
  vtkSetMacro(MultithreadingEnabled, int);
  vtkGetMacro(MultithreadingEnabled, int);

  vtkSetMacro(UpdateIntermediateData, int);
  vtkGetMacro(UpdateIntermediateData, int);

  vtkGetStringMacro(WorkingDirectory);
  vtkSetStringMacro(WorkingDirectory);

  //
  // intensity normalization parameters
  //

  vtkSetMacro(EnableTargetToTargetRegistration, int);
  vtkGetMacro(EnableTargetToTargetRegistration, int);

  vtkSetMacro(EnableSubParcellation, int);
  vtkGetMacro(EnableSubParcellation, int);

  vtkSetMacro(MinimumIslandSize, int);
  vtkGetMacro(MinimumIslandSize, int);

  vtkSetMacro(Island2DFlag, int);
  vtkGetMacro(Island2DFlag, int);

  vtkSetMacro(AMFSmoothing, int);
  vtkGetMacro(AMFSmoothing, int);

  vtkSetStringMacro(Colormap);
  vtkGetStringMacro(Colormap);

  virtual const char* GetNthTargetInputChannelName(vtkIdType n);
  virtual void SetNthTargetInputChannelName(vtkIdType n, const char* targetName);

  vtkGetMacro(TemplateSaveAfterSegmentation, int);
  vtkSetMacro(TemplateSaveAfterSegmentation, int);

  vtkGetStringMacro(TemplateFileName);
  vtkSetStringMacro(TemplateFileName);

  vtkGetStringMacro(TaskTclFileName);
  vtkSetStringMacro(TaskTclFileName);

  vtkGetStringMacro(TaskPreProcessingSetting);
  vtkSetStringMacro(TaskPreProcessingSetting);

  static const char* GetDefaultTaskTclFileName() { return "GenericTask.tcl"; }


protected:
  vtkMRMLEMSGlobalParametersNode();
  ~vtkMRMLEMSGlobalParametersNode();
  vtkMRMLEMSGlobalParametersNode(const vtkMRMLEMSGlobalParametersNode&);
  void operator=(const vtkMRMLEMSGlobalParametersNode&);

  // 
  // Preprocessing 
  // 
  int                                 RegistrationAffineType;
  int                                 RegistrationDeformableType;
  int                                 RegistrationInterpolationType;
  int                                 RegistrationPackageType;
  char*                               TaskTclFileName;
  char*                               TaskPreProcessingSetting; 
  int                                 EnableTargetToTargetRegistration;

  // --- Kilian Should be looked at if still is used 
  //BTX
  vtkstd::vector<std::string>         RegistrationAtlasVolumeKey;
  //ETX
  char*                               RegistrationTargetVolumeKey;
   // ---

  //
  // EM 
  //  
  char*                               WorkingDirectory;
  
  int                                 SaveIntermediateResults;
  int                                 SaveSurfaceModels;

  int                                 MultithreadingEnabled;
  int                                 UpdateIntermediateData;
  
  int                                 SegmentationBoundaryMin[3];
  int                                 SegmentationBoundaryMax[3];

  int                                 NumberOfTargetInputChannels;

  char*                               Colormap;

  //BTX
  std::vector<std::string>   InputChannelNames; 
  void SetNumberOfTargetInputChannels(vtkIdType num);
  //ETX

  // 
  // Postprocessing
  //
  int                                 MinimumIslandSize;
  int                                 Island2DFlag;

  int                                 TemplateSaveAfterSegmentation;
  char*                            TemplateFileName;

  int                                 EnableSubParcellation;

  int                                AMFSmoothing;

};

#endif
