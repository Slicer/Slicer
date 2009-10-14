#ifndef __vtkMRMLPETProceduralColorNode_h
#define __vtkMRMLPETProceduralColorNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLProceduralColorNode.h"

class vtkColorTransferFunction;
class vtkMRMLStorageNode;

class VTK_MRML_EXPORT vtkMRMLPETProceduralColorNode : public vtkMRMLProceduralColorNode
{
public:
  static vtkMRMLPETProceduralColorNode *New();
  vtkTypeMacro(vtkMRMLPETProceduralColorNode,vtkMRMLProceduralColorNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
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
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "PETProceduralColor";};

  // Description:
  // 
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Get/Set for Type. In SetType, set up the custom colour options for this
  // set of colours
  virtual void SetType(int type);

  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //BTX
  // Description:
  // The list of valid procedural types
  //enum
  //{
  //  
  //};
  //ETX

  //BTX
  // Description:
  // DisplayModifiedEvent is generated when display node parameters is changed
  enum
    {
      DisplayModifiedEvent = 20000,
    };
//ETX

  // Description:
  // Read in a text file with procedural definitions
  virtual int ReadFile ();

  // Description:
  // Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode()
    {
    return Superclass::CreateDefaultStorageNode();
    };

  //BTX
  // Description:
  // The list of valid types
  // PETheat to display PET overlay with red to orange to yellow to white.
  // PETrainbow to display PET overlay with cool to warm to white.
  enum
  {
    PETheat = 0,
    PETrainbow = 1,
  };
  //ETX
  // Description:
  // Return the lowest and the highest integers, for use in looping
  int GetFirstType() { return this->PETheat; };
  int GetLastType() { return this->PETrainbow; };
  
  const char *GetTypeAsIDString();
  const char *GetTypeAsString();
  void SetTypeToHeat();
  void SetTypeToRainbow();
  
protected:
  vtkMRMLPETProceduralColorNode();
  ~vtkMRMLPETProceduralColorNode();
  vtkMRMLPETProceduralColorNode(const vtkMRMLPETProceduralColorNode&);
  void operator=(const vtkMRMLPETProceduralColorNode&);
};

#endif
