// .NAME vtkMRMLVolumeRenderingNode - MRML node to represent volume rendering information
// .SECTION Description
// This node is especially used to store visualization parameter sets for volume rendering


#ifndef __vtkMRMLVolumeRenderingNode_h
#define __vtkMRMLVolumeRenderingNode_h

#include "vtkMRML.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkVolumeRenderingModule.h"
#include "vtkVolumeProperty.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"

#include <string>
#include "vtkVolumeMapper.h"

#define COUNT_CROPPING_REGION_PLANES 6

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkMRMLVolumeRenderingNode : public vtkMRMLNode
{
public:
    //--------------------------------------------------------------------------
    // OWN methods
    //--------------------------------------------------------------------------
    
    //BTX
    // Description:
    // Get a string representation of all points in the vtkPiecewiseFunction. 
    //format: <numberOfPoints> <XValue1> <OpacityValue1> ...<XValueN> <OpacityValueN> 
    std::string getPiecewiseFunctionString(vtkPiecewiseFunction* function);
    // Description:
    // Get a string representation of all points in the vtkColorTransferFunction. 
    //format: <numberOfPoints> <XValue1> <RValue1> <GValue1><BValue1> ...<XValueN> <RValueN> <GValueN><BValueN>
    std::string getColorTransferFunctionString(vtkColorTransferFunction* function);
    // Description:
    // Put parameters described in a String into an existing vtkPiecewiseFunction, use together with getPiecewiseFunctionString
    void GetPiecewiseFunctionFromString(std::string str,vtkPiecewiseFunction* result);
    // Description:
    // Put parameters described in a String into an existing vtkColorTransferFunction, use together with getColorTransferFunctionString
    void GetColorTransferFunction(std::string str, vtkColorTransferFunction* result);
    //ETX
    // Description:
    // Create a new vtkMRMLVolumeRenderingNode
    static vtkMRMLVolumeRenderingNode *New();
    vtkTypeMacro(vtkMRMLVolumeRenderingNode,vtkMRMLNode);
    void PrintSelf(ostream& os, vtkIndent indent);
    //BTX
    //Description:
    //constant for hardware accelerated 3D texture mapping
    static const int Texture=0;
    //Description:
    //constant for software accelerated 3D raycast mapping
    static const int RayCast=1;
    //ETX
    // Description:
    // Set/Get mapper of this node. Use constants Texture and Raycast
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
    vtkGetObjectMacro(VolumeProperty,vtkVolumeProperty);
    // Description:
    // Set/get the vtkVolumeProperty of the MRML Node. The volume property holds all important visualization informations
    void SetVolumeProperty(vtkVolumeProperty *ar)
    {
        this->VolumeProperty=ar;
    }
    //BTX
    // Description:
    // Add a reference to another MRML node. Use only for vtkMRMLScalarVolumeNodes
    void AddReference(std::string id);
    // Description:
    // Check of this node as a reference to the specified vtkMRMLScalarVolumeNode
    bool HasReference(std::string id);
    // Description
    // If there is a reference to the specified vtkMRMLScalaraVolumeNode remove it
    void RemoveReference(std::string id);
    //ETX    


    // Description:
    // Set/Get if the Node saves data of a label map or a grayscale data set.
    // 0: No labelmap; 1: labelmap
    vtkSetMacro(IsLabelMap,int);
    vtkGetMacro(IsLabelMap,int);
    vtkBooleanMacro(IsLabelMap,int);

    vtkSetMacro(CroppingEnabled,int);
    vtkGetMacro(CroppingEnabled,int);
    vtkBooleanMacro(CroppingEnabled,int);

    vtkSetVector6Macro(CroppingRegionPlanes,double);
    vtkGetVectorMacro(CroppingRegionPlanes,double,6);

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

    //Description;
    //Copy only the paramterset (like Volume Propertys, Piecewiesefunctions etc. as deep copy,but no references etc.)
    void CopyParameterset(vtkMRMLNode *node);

    // Description:
    // Get node XML tag name (like Volume, Model)
    virtual const char* GetNodeTagName() {return "VolumeRendering";};

    // Description:
    // 
    virtual void UpdateScene(vtkMRMLScene *scene);
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData);

protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkMRMLVolumeRenderingNode(void);
    // Description:
    // Use ->Delete() to delete object
    ~vtkMRMLVolumeRenderingNode(void);

    // Description:
    // Main parameters for visualization
    vtkVolumeProperty* VolumeProperty;
    // Description:
    // 0 means hardware accelerated 3D texture Mapper, 1 fixed raycastMapper 
    int Mapper;
    // Description:
    //1 Yes it is a LabelMap,0 no it is not a Label Map
    int IsLabelMap;

    // Description:
    // Cropping planes from the mapper
    double CroppingRegionPlanes[COUNT_CROPPING_REGION_PLANES];

    // Description:
    // Is cropping enabled?
    int CroppingEnabled;
    //BTX
    // Description:
    // References to vtkMRMLScalarVolumeNodes
    vtksys_stl::vector<std::string> References;
    //ETX

private:
    // Description:
    // Caution: Not implemented
    vtkMRMLVolumeRenderingNode(const vtkMRMLVolumeRenderingNode&);//Not implemented
    void operator=(const vtkMRMLVolumeRenderingNode&);// Not implmented

};

#endif
