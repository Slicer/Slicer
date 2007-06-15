#ifndef __vtkMRMLROIListNode_h
#define __vtkMRMLROIListNode_h

#include <string> 

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLROINode.h"
#include "vtkCollection.h"
#include "vtkMRMLTransformableNode.h"

#include <string>

// .NAME vtkMRMLROIListNode - MRML list node to represent a list of ROINODE 
// .SECTION Description

class VTK_MRML_EXPORT vtkMRMLROIListNode : public vtkMRMLNode
{
  public:
    static vtkMRMLROIListNode *New();
    vtkTypeMacro(vtkMRMLROIListNode,vtkMRMLNode);
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
    virtual const char* GetNodeTagName() {return "ROIList";};

    // Description:
    // 
    virtual void UpdateScene(vtkMRMLScene *scene);

    // Description:
    // update display node ids
    void UpdateReferences();

    // Description:
    // Get the number of ROIs in the list
    int GetNumberOfROIs();

    // Description:
    // Restrict access to the ROI, pass in a value via the list
    // so that the appropriate events can be invoked. Returns 0 on success

    // Description:
    // Get/Set for Nth ROI node Position in RAS cooridnates
    // Note: The ROI Postion is the center of the ROI 
    int SetNthROIXYZ(int n, float x, float y, float z);
    float *GetNthROIXYZ(int n);
    // Description:
    // Get/Set for Nth radius of the ROI in RAS cooridnates
    int SetNthROIRadiusXYZ(int n, float Radiusx, float Radiusy, float Radiusz);
    float *GetNthROIRadiusXYZ(int n);
    // Description:
    // Get/Set for Nth ROI node Position in IJK cooridnates
    // Note: The ROI Postion is the center of the ROI 
    int SetNthROIIJK(int n, float i, float j, float k);
    float *GetNthROIIJK(int n);
    // Description:
    // Get/Set for Nth radius of the ROI in IJK cooridnates
    int SetNthROIRadiusIJK(int n, float Radiusi, float Radiusj, float Radiusk);
    float *GetNthROIRadiusIJK(int n);

    // Description:
    // Get/Set for label text of the Nth ROI  
    int SetNthROILabelText(int n, const char *text);
    const char *GetNthROILabelText(int n);
    // Description:
    // Get/Set for selected flag of the Nth ROI  
    int SetNthROISelected(int n, int flag);
    int GetNthROISelected(int n);
    // Description:
    // Get/Set for ID of the Nth ROI  
    int SetNthROIID(int n, const char *id); 
    const char *GetNthROIID(int n);
   
    // Description:
    // Add an ROI to the list with default values
    int AddROI( );
    // Description:
    // Remove an ROI from the list 
    void RemoveROI(int i);
    // Description:
    // Remove all ROIs from the list 
    void RemoveAllROIs();

    void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

    //BTX
    // Description:
    // DisplayModifiedEvent is generated when display node parameters is changed
    // PolyDataModifiedEvent is generated when something else is changed
    enum
      {
      DisplayModifiedEvent = 21000,  
      PolyDataModifiedEvent = 21001, 
      ROIModifiedEvent = 21002,
      };
    //ETX

    // Description:
    // Get/Set for list visibility 
    void SetVisibility(int visible);
    vtkGetMacro(Visibility,int);

    // Description:
    // Get/Set for Text scale
    void SetTextScale(double scale);
    vtkGetMacro(TextScale,double);

    // Description:
    // Get/Set for ROI and Text color
    void SetColor(double r, double g, double b);
    void SetColor(double c[3]);
    vtkGetVectorMacro(Color,double,3);

    // Description:
    // Get/Set for colour for when an ROI is selected
    void SetSelectedColor(double r, double g, double b);
    void SetSelectedColor(double c[3]);
    vtkGetVectorMacro(SelectedColor,double,3);

    // Description:
    // Opacity of the ROI expressed as a number from 0 to 1
    void SetOpacity(double opacity);
    vtkGetMacro(Opacity, double);

    // Description:
    // Ambient of the ROI expressed as a number from 0 to 100
    vtkSetMacro(Ambient, double);
    vtkGetMacro(Ambient, double);

    // Description:
    // Diffuse of the ROI expressed as a number from 0 to 100
    vtkSetMacro(Diffuse, double);
    vtkGetMacro(Diffuse, double);

    // Description:
    // Specular of the ROI expressed as a number from 0 to 100
    vtkSetMacro(Specular, double);
    vtkGetMacro(Specular, double);

    // Description:
    // Power of the ROI expressed as a number from 0 to 100
    vtkSetMacro(Power, double);
    vtkGetMacro(Power, double);

    // Description:
    // Get/Set for the volume ID associated with the ROI
    vtkGetStringMacro(VolumeNodeID);
    vtkSetStringMacro(VolumeNodeID);

    // Description:
    // Set the Volume node ID for each ROI node in the list
    void SetAllVolumeNodeID();

    // Description:
    // Syncronize the ROI position and radius in IJK 
    // coordinates according RAS coordinates
    void UpdateIJK();
    
  protected:
    vtkMRMLROIListNode();
    ~vtkMRMLROIListNode();

    vtkMRMLROIListNode(const vtkMRMLROIListNode&);
    void operator=(const vtkMRMLROIListNode&);

    // Description:
    // The collection of ROI that make up this list
    vtkCollection *ROIList;

    double TextScale;
    int Visibility;
    double Color[3];
    double SelectedColor[3];

    // Description:
    // disallow access to the ROI box by outside classes, have them use
    // SetNthROI
    vtkMRMLROINode* GetNthROINode(int n);

    // Description:
    // Numbers relating to the 3D render of the ROI
    double Opacity;
    double Ambient;
    double Diffuse;
    double Specular;
    double Power;
    
    // Description:
    // The ID of the volume node that is associated with the ROI 
    char *VolumeNodeID;
};
#endif
