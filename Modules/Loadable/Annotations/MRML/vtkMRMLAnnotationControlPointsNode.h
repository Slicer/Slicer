// .NAME vtkMRMLAnnotationControlPointsNode - MRML node to represent a fiber bundle from tractography in DTI data.
// .SECTION Description
// Annotation nodes contains control points, internally represented as vtkPolyData.
// A Annotation node contains many control points  and forms the smallest logical unit of tractography 
// that MRML will manage/read/write. Each control point has accompanying data.  
// Visualization parameters for these nodes are controlled by the vtkMRMLAnnotationPointDisplayNode class.
//

#ifndef __vtkMRMLAnnotationControlPointsNode_h
#define __vtkMRMLAnnotationControlPointsNode_h

#include "vtkMRMLAnnotationNode.h"

class vtkMRMLAnnotationPointDisplayNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationControlPointsNode : public vtkMRMLAnnotationNode
{
public:
  static vtkMRMLAnnotationControlPointsNode *New();
  vtkTypeMacro(vtkMRMLAnnotationControlPointsNode,vtkMRMLAnnotationNode);

  // void PrintSelf(ostream& os, vtkIndent indent);
  // Description:
  // Just prints short summary 
  virtual void PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag = 1);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "AnnotationControlPoints";};

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Write this node's information to a string for passing to a CLI, write
  /// out the prefix before each datum. Only implemented for RAS, LPS coordinate systems.
  virtual void WriteCLI(std::ostringstream& ss, std::string prefix, int coordinateSystem = 0);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node) {Superclass::Copy(node);}

  void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );


  enum
    {
      ControlPointModifiedEvent = 19010,
    };

  virtual void Modified() 
    {
    Superclass::Modified();

    if (!this->GetDisableModifiedEvent())
      {
      this->InvokeEvent(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent);
      }
    }

  /// 
  /// Invokes any modified events that are 'pending', meaning they were generated
  /// while the DisableModifiedEvent flag was nonzero.
  /// Returns the old flag state.
  virtual int InvokePendingModifiedEvent ()
    {
    if ( this->GetModifiedEventPending() )
      {
      this->InvokeEvent(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent);
      }
    return Superclass::InvokePendingModifiedEvent();
    }

  // Description:
  // get associated display node or NULL if not set
  vtkMRMLAnnotationPointDisplayNode* GetAnnotationPointDisplayNode();

  // Description:
  // Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();  

  int  AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag);

  int  SetControlPoint(int id, double newControl[3],int selectedFlag, int visibleFlag);

  int  SetControlPointWorldCoordinates(int id, double newControl[3], int selectedFlag, int visibleFlag);

  int  SetControlPoint(int id, double newControl[3]);

  int  SetControlPointWorldCoordinates(int id, double newControl[3]);

  void DeleteControlPoint(int id);
  /// Return the RAS coordinates of point ID.
  /// Try to limit calling this function because it is performance critical.
  /// Also, when queried again it resets all former pointers. Copying is therefore necessary.
  double* GetControlPointCoordinates(vtkIdType id);

  void GetControlPointWorldCoordinates(vtkIdType id, double *point);

  int GetNumberOfControlPoints();

  enum 
  {
    CP_SELECTED =  vtkMRMLAnnotationNode::NUM_TEXT_ATTRIBUTE_TYPES,
    CP_VISIBLE,
    NUM_CP_ATTRIBUTE_TYPES
  };

  const char *GetAttributeTypesEnumAsString(int val);


  // Description:
  // Initializes all variables associated with annotations
  virtual void ResetAnnotations();

  // Description:
  // add display node if not already present
  void CreateAnnotationPointDisplayNode();

  /// flags to determine how the next fiducial added to the list is labelled
  enum NumberingSchemes
  {
      SchemeMin = 0,
      UseID = SchemeMin,
      UseIndex,
      UsePrevious,
      SchemeMax = UsePrevious,
  };


  /// Flag determining how to number the next added fiducial
  virtual void SetNumberingScheme(int numberingScheme);
  vtkGetMacro(NumberingScheme, int);

  /// Return a string representing the numbering scheme, set it from a string
  const char* GetNumberingSchemeAsString();
  const char* GetNumberingSchemeAsString(int g);
  void SetNumberingSchemeFromString(const char *schemeString);

  virtual void Initialize(vtkMRMLScene* mrmlScene);

protected:
  vtkMRMLAnnotationControlPointsNode(); 
  ~vtkMRMLAnnotationControlPointsNode() { };
  vtkMRMLAnnotationControlPointsNode(const vtkMRMLAnnotationControlPointsNode&);
  void operator=(const vtkMRMLAnnotationControlPointsNode&);

  // Description:
  // Create Poly data with substructures necessary for this class 
  void CreatePolyData();

  // Description:
  // Initializes control pointes as well as attributes
  void ResetControlPoints(); 

  // Description:
  // Initializes all attributes 
  void ResetControlPointsAttributesAll(); 

  /// How the next annotation will be numbered in it's Text field
  int NumberingScheme;
};

#endif
