#ifndef __vtkMRMLVolumeRenderingDisplayNode_h
#define __vtkMRMLVolumeRenderingDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkVolumeRenderingModule.h"
#include "vtkVolumeProperty.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include <string>
//#include "vtkVolumeTextureMapper3D.h"
#include "vtkVolumeMapper.h"
// .NAME vtkMRMLVolumeRenderingDisplayNode - MRML node to represent Volume Rendering information
// .SECTION Description
class VTK_VOLUMERENDERINGMODULE_EXPORT vtkMRMLVolumeRenderingDisplayNode : public vtkMRMLDisplayNode
{
public:
    static int getPiecewiseFunctionString(vtkPiecewiseFunction* function, char* result);
    static void getColorTransferFunctionString(vtkColorTransferFunction* function, char* result);
    static void GetPiecewiseFunctionFromString(char* string,vtkPiecewiseFunction* result);
    static void GetColorTransferFunction(char* string, vtkColorTransferFunction* result);
    static vtkMRMLVolumeRenderingDisplayNode *New();
    vtkTypeMacro(vtkMRMLVolumeRenderingDisplayNode,vtkMRMLNode);
    void PrintSelf(ostream& os, vtkIndent indent);
    

    vtkGetObjectMacro(mapper,vtkVolumeMapper);
    vtkSetObjectMacro(mapper,vtkVolumeMapper);

    vtkGetObjectMacro(volumeProperty,vtkVolumeProperty);
    vtkSetObjectMacro(volumeProperty,vtkVolumeProperty);

    vtkSetMacro(pipelineInitialized,int);
    vtkGetMacro(pipelineInitialized,int);
    vtkBooleanMacro(pipelineInitialized,int);
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  // Description:
  // Read in a text file holding colours
  // Return 1 on sucess, 0 on failure
  virtual int ReadFile ();
  
  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "VolumeRendering";};

  // Description:
  // 
  virtual void UpdateScene(vtkMRMLScene *scene);

  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  // Description:
  // Return the lowest and the highest type integers (defined in enum in
  // subclass), for use in looping
  virtual int GetFirstType();
  virtual int GetLastType ();
  
  // Description:
  // return a text string describing the colour look up table type
  virtual const char * GetTypeAsString();

  //BTX
  // Description:
  // TypeModifiedEvent is generated when the type of the colour look up table changes
  enum
    {
      TypeModifiedEvent = 20002,
    };
//ETX

  // Description:
  //Own Methods
  
protected:
  vtkMRMLVolumeRenderingDisplayNode(void);
    ~vtkMRMLVolumeRenderingDisplayNode(void);
  vtkMRMLVolumeRenderingDisplayNode(const vtkMRMLVolumeRenderingDisplayNode&);
  void operator=(const vtkMRMLVolumeRenderingDisplayNode&);
  vtkVolumeProperty* volumeProperty;
  vtkVolumeMapper* mapper;
  int pipelineInitialized;//0=no,1=Yes


};

#endif
