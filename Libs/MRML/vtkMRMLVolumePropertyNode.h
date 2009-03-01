// .NAME vtkMRMLVolumePropertyNode - MRML node to represent volume rendering information
// .SECTION Description
// This node is especially used to store visualization parameter sets for volume rendering


#ifndef __vtkMRMLVolumePropertyNode_h
#define __vtkMRMLVolumePropertyNode_h

#include "vtkMRML.h"
#include "vtkMRMLDisplayNode.h"

#include "vtkVolumeProperty.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"

#include <string>
#include <vtksys/stl/vector>

#define COUNT_CROPPING_REGION_PLANES 6

class VTK_MRML_EXPORT vtkMRMLVolumePropertyNode : public vtkMRMLStorableNode
{
public:
    //--------------------------------------------------------------------------
    // OWN methods
    //--------------------------------------------------------------------------
    
    //BTX

    // Description:
    // Get a string representation of all points in the vtkPiecewiseFunction. 
    //format: <numberOfPoints> <XValue1> <OpacityValue1> ...<XValueN> <OpacityValueN> 
    std::string GetPiecewiseFunctionString(vtkPiecewiseFunction* function);

    // Description:
    // Get a string representation of all points in the vtkColorTransferFunction. 
    //format: <numberOfPoints> <XValue1> <RValue1> <GValue1><BValue1> ...<XValueN> <RValueN> <GValueN><BValueN>
    std::string GetColorTransferFunctionString(vtkColorTransferFunction* function);

    // Description:
    // Put parameters described in a String into an existing vtkPiecewiseFunction, use together with GetPiecewiseFunctionString
    void GetPiecewiseFunctionFromString(std::string str,vtkPiecewiseFunction* result);

    // Description:
    // Put parameters described in a String into an existing vtkColorTransferFunction, use together with getColorTransferFunctionString
    void GetColorTransferFunctionFromString(std::string str, vtkColorTransferFunction* result);

    //ETX

    // Description:
    // Create a new vtkMRMLVolumePropertyNode
    static vtkMRMLVolumePropertyNode *New();
    vtkTypeMacro(vtkMRMLVolumePropertyNode,vtkMRMLStorableNode);
    void PrintSelf(ostream& os, vtkIndent indent);

    vtkGetObjectMacro(VolumeProperty,vtkVolumeProperty);

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
    // Finds the storage node and read the data
    virtual void UpdateScene(vtkMRMLScene *scene){
       Superclass::UpdateScene(scene);
    };

    //Description;
    //Copy only the paramterset (like Volume Propertys, Piecewiesefunctions etc. as deep copy,but no references etc.)
    void CopyParameterSet(vtkMRMLNode *node);

    // Description:
    // Get node XML tag name (like Volume, Model)
    virtual const char* GetNodeTagName() {return "VolumeProperty";};

    // Description:
    // 

    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData);

    // Description:
    // transform utility functions
    virtual bool CanApplyNonLinearTransforms() { return false; }
    virtual void ApplyTransform(vtkMatrix4x4* transformMatrix) {};
    virtual void ApplyTransform(vtkAbstractTransform* transform) {};

    // Description:
    // Create default storage node or NULL if does not have one
    virtual vtkMRMLStorageNode* CreateDefaultStorageNode();


protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkMRMLVolumePropertyNode(void);
    // Description:
    // Use ->Delete() to delete object
    ~vtkMRMLVolumePropertyNode(void);

    // Description:
    // Main parameters for visualization
    vtkVolumeProperty* VolumeProperty;

private:
    // Description:
    // Caution: Not implemented
    vtkMRMLVolumePropertyNode(const vtkMRMLVolumePropertyNode&);//Not implemented
    void operator=(const vtkMRMLVolumePropertyNode&);// Not implmented

};

#endif
