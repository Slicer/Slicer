#ifndef __vtkMRMLEMSGlobalParametersNode_h
#define __vtkMRMLEMSGlobalParametersNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkEMSegment.h"

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSGlobalParametersNode : 
  public vtkMRMLNode
{
public:
  static vtkMRMLEMSGlobalParametersNode *New();
  vtkTypeMacro(vtkMRMLEMSGlobalParametersNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

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

  vtkSetMacro(NumberOfTargetInputChannels, int);
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

  vtkSetStringMacro(RegistrationAtlasVolumeKey);
  vtkGetStringMacro(RegistrationAtlasVolumeKey);

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

  vtkGetStringMacro(WorkingDirectory);
  vtkSetStringMacro(WorkingDirectory);

protected:
  vtkMRMLEMSGlobalParametersNode();
  ~vtkMRMLEMSGlobalParametersNode();
  vtkMRMLEMSGlobalParametersNode(const vtkMRMLEMSGlobalParametersNode&);
  void operator=(const vtkMRMLEMSGlobalParametersNode&);

  int                                 RegistrationAffineType;
  int                                 RegistrationDeformableType;
  int                                 RegistrationInterpolationType;

  char*                               RegistrationAtlasVolumeKey;
  char*                               RegistrationTargetVolumeKey;

  char*                               WorkingDirectory;
  
  int                                 SaveIntermediateResults;
  int                                 SaveSurfaceModels;

  int                                 MultithreadingEnabled;

  int                                 SegmentationBoundaryMin[3];
  int                                 SegmentationBoundaryMax[3];

  int                                 NumberOfTargetInputChannels;
};

#endif
