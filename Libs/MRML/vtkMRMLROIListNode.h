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
    // Restrict access to the ROI points, pass in a value via the list
    // so that the appropriate events can be invoked. Returns 0 on success
    int SetNthROIXYZ(int n, float x, float y, float z);
    int SetNthROIDeltaXYZ(int n, float Deltax, float Deltay, float Deltaz);
    int SetNthROILabelText(int n, const char *text);
    int SetNthROISelected(int n, int flag);
    int SetNthROIID(int n, const char *id);

    // Description:
    // Get the elements of the ROI box
    // Return a three element float holding the position
    float *GetNthROIXYZ(int n);
    // Return a three element float holding the size of the ROI box
    float *GetNthROIDeltaXYZ(int n);
    // get the label text of the nth fiducial
    const char *GetNthROILabelText(int n);
    // get the selected state on the nth fiducial
    int GetNthROISelected(int n);
    // get the id of the nth fiducial
    const char *GetNthROIID(int n);

    // Description:
    // Add a ROI point to the list with default values
    int AddROI( );

    void RemoveROI(int i);
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
    // Opacity of the fiducial surface expressed as a number from 0 to 1
    void SetOpacity(double opacity);
    vtkGetMacro(Opacity, double);

    // Description:
    // Ambient of the fiducial surface expressed as a number from 0 to 100
    vtkSetMacro(Ambient, double);
    vtkGetMacro(Ambient, double);

    // Description:
    // Diffuse of the fiducial surface expressed as a number from 0 to 100
    vtkSetMacro(Diffuse, double);
    vtkGetMacro(Diffuse, double);

    // Description:
    // Specular of the fiducial surface expressed as a number from 0 to 100
    vtkSetMacro(Specular, double);
    vtkGetMacro(Specular, double);

    // Description:
    // Power of the fiducial surface expressed as a number from 0 to 100
    vtkSetMacro(Power, double);
    vtkGetMacro(Power, double);

    vtkGetStringMacro(VolumeNodeID);
    vtkSetStringMacro(VolumeNodeID);

    // Description:
    // Set the Volume node ID for each roi node in the list
    void SetAllVolumeNodeID();

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
    // Numbers relating to the 3D render of the fiducials
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
