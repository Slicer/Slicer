#ifndef __vtkMRMLVolumeRenderingNode_h
#define __vtkMRMLVolumeRenderingNode_h

#include "vtkMRML.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkVolumeRenderingModule.h"
#include "vtkVolumeProperty.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"

#include <string>
//#include "vtkVolumeTextureMapper3D.h"
#include "vtkVolumeMapper.h"
// .NAME vtkMRMLVolumeRenderingNode - MRML node to represent Volume Rendering information
// .SECTION Description
class VTK_VOLUMERENDERINGMODULE_EXPORT vtkMRMLVolumeRenderingNode : public vtkMRMLNode
{
public:
    //BTX
    std::string getPiecewiseFunctionString(vtkPiecewiseFunction* function);//, char* result);
    std::string getColorTransferFunctionString(vtkColorTransferFunction* function);
    void GetPiecewiseFunctionFromString(const char* string,vtkPiecewiseFunction* result);
    void GetColorTransferFunction(const char* string, vtkColorTransferFunction* result);
    //ETX
    static vtkMRMLVolumeRenderingNode *New();
    vtkTypeMacro(vtkMRMLVolumeRenderingNode,vtkMRMLNode);
    void PrintSelf(ostream& os, vtkIndent indent);
    //BTX
    static const int Texture=0;
    static const int RayCast=1;
    //ETX
    void SetMapper(int mapper)
    {
        if(mapper!=0||mapper!=0)
        {
            vtkErrorMacro("wrong type of mapping");
            return;
        }
        this->Mapper=mapper;
    }
    int GetMapper()
    {
        return this->Mapper;
    }

    //vtkSetObjectMacro(Mapper,vtkVolumeMapper);

    vtkGetObjectMacro(VolumeProperty,vtkVolumeProperty);
    void SetVolumeProperty(vtkVolumeProperty *ar)
    {
        this->VolumeProperty=ar;
    }

    void SetOpacityOfLabel(int index, double opacity);
    double GetOpacityOfLabel(int index);
//BTX
    void AddReference(std::string id);
    bool HasReference(std::string id);
    void RemoveReference(std::string id);
//ETX    
    //Maybe later
    //void SetAutoValuesFromImageData(vtkImageData *data);



    vtkSetMacro(IsLabelMap,int);
    vtkGetMacro(IsLabelMap,int);
    vtkBooleanMacro(IsLabelMap,int);

    //vtkSetObjectMacro(VolumeProperty,vtkVolumeProperty);

   
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
  
  //Description;
  //Copy only the paramterset (like Volume Propertys, Piecewiesefunctions etc. as deep copy,but no references etc.)
  void CopyParameterset(vtkMRMLNode *node);
  
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
    //Buffer vor char return
  const char* Buffer;
  vtkMRMLVolumeRenderingNode(void);
    ~vtkMRMLVolumeRenderingNode(void);
  vtkMRMLVolumeRenderingNode(const vtkMRMLVolumeRenderingNode&);
  void operator=(const vtkMRMLVolumeRenderingNode&);
  vtkVolumeProperty* VolumeProperty;
  int Mapper;//0 means hardware accelerated 3D texture Mapper, 1 fixed raycastMapper
  int IsLabelMap;//1 Yes it is a LabelMap,0 no it is not a Label Map
  //BTX
  vtksys_stl::vector<std::string> References;

  //if LabelMap Save the opacities of the labels of the colors
  vtksys_stl::vector<double> Opacities;
  //ETX

};

#endif
