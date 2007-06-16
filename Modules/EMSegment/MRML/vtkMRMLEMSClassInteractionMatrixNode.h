#ifndef __vtkMRMLEMSClassInteractionMatrixNode_h
#define __vtkMRMLEMSClassInteractionMatrixNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkEMSegment.h"
#include "vtkMRMLScene.h"

#include <vector>

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSClassInteractionMatrixNode : 
  public vtkMRMLNode
{
public:
  static vtkMRMLEMSClassInteractionMatrixNode *New();
  vtkTypeMacro(vtkMRMLEMSClassInteractionMatrixNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "EMSClassInteractionMatrix";}

  vtkGetMacro(NumberOfClasses, unsigned int);
  virtual void SetNumberOfClasses(unsigned int n);
  virtual void AddClass();
  virtual void RemoveNthClass(int index);
  virtual void MoveNthClass(int fromIndex, int toIndex);

  //BTX
  enum 
    {
      DirectionWest = 0, 
      DirectionNorth = 1, 
      DirectionUp = 2, 
      DirectionEast = 3, 
      DirectionSouth = 4, 
      DirectionDown = 5
    };
  //ETX
  virtual double GetClassInteraction(int direction, int row, int column) const;
  virtual void SetClassInteraction(int direction, int row, int column, 
                                   double value);

  //virtual double GetClassInteraction(int direction, int row, int column) const;
  //virtual void SetClassInteraction(int direction, int row, int column, 
  //double value);

protected:
  vtkMRMLEMSClassInteractionMatrixNode();
  ~vtkMRMLEMSClassInteractionMatrixNode();
  vtkMRMLEMSClassInteractionMatrixNode(const vtkMRMLEMSClassInteractionMatrixNode&);
  void operator=(const vtkMRMLEMSClassInteractionMatrixNode&);

  //BTX
  vtkstd::vector<vtkstd::string>                           DirectionNames;
  vtkstd::vector<vtkstd::vector<vtkstd::vector<double> > > Matrices;
  //ETX

  unsigned int                        NumberOfClasses;
};

#endif
