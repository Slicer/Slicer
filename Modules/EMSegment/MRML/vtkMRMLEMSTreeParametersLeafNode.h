#ifndef __vtkMRMLEMSTreeParametersLeafNode_h
#define __vtkMRMLEMSTreeParametersLeafNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkEMSegment.h"
#include "vtkMRMLScene.h"

#include <vector>

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSTreeParametersLeafNode : 
  public vtkMRMLNode
{
public:
  static vtkMRMLEMSTreeParametersLeafNode *New();
  vtkTypeMacro(vtkMRMLEMSTreeParametersLeafNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "EMSTreeParametersLeaf";}

  vtkGetMacro(NumberOfTargetInputChannels, unsigned int);
  virtual void SetNumberOfTargetInputChannels(unsigned int n);
  virtual void AddTargetInputChannel();
  virtual void RemoveTargetInputChannel(int index);
  virtual void MoveTargetInputChannel(int fromIndex, int toIndex);

  vtkGetMacro(IntensityLabel, int);
  vtkSetMacro(IntensityLabel, int);

  vtkGetMacro(PrintQuality, int);
  vtkSetMacro(PrintQuality, int);

  virtual double GetLogMean(int index) const;
  virtual void SetLogMean(int index, double value);

  virtual double GetLogCovariance(int row, int column) const;
  virtual void SetLogCovariance(int row, int column, double value);

protected:
  vtkMRMLEMSTreeParametersLeafNode();
  ~vtkMRMLEMSTreeParametersLeafNode();
  vtkMRMLEMSTreeParametersLeafNode(const vtkMRMLEMSTreeParametersLeafNode&);
  void operator=(const vtkMRMLEMSTreeParametersLeafNode&);

  int                                 PrintQuality;
  int                                 IntensityLabel;

  //BTX
  vtkstd::vector<double>                        LogMean;
  vtkstd::vector<vtkstd::vector<double> >       LogCovariance;
  //ETX

  unsigned int                        NumberOfTargetInputChannels;
};

#endif
