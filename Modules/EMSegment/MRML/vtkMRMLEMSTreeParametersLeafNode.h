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

  // manipulate tree structure
  virtual void AddChildNode(const char* childNodeID);
  virtual void RemoveNthChildNode(int n);
  virtual void MoveNthChildNode(int fromIndex, int toIndex);

  // manipulate target input channels
  vtkGetMacro(NumberOfTargetInputChannels, unsigned int);
  virtual void SetNumberOfTargetInputChannels(unsigned int n);
  virtual void AddTargetInputChannel();
  virtual void RemoveNthTargetInputChannel(int index);
  virtual void MoveNthTargetInputChannel(int fromIndex, int toIndex);

  vtkGetMacro(IntensityLabel, int);
  vtkSetMacro(IntensityLabel, int);

  vtkGetMacro(PrintQuality, int);
  vtkSetMacro(PrintQuality, int);

  virtual double GetLogMean(int index) const;
  virtual void SetLogMean(int index, double value);

  virtual double GetLogCovariance(int row, int column) const;
  virtual void SetLogCovariance(int row, int column, double value);

  //BTX
  enum
    {
    DistributionSpecificationManual = 0,
    DistributionSpecificationManuallySample,
    DistributionSpecificationAutoSample
    };
  //ETX
  vtkGetMacro(DistributionSpecificationMethod, int);
  vtkSetMacro(DistributionSpecificationMethod, int);

  virtual int GetNumberOfSamplePoints() const;
  virtual void AddSamplePoint(double xyz[3]);
  virtual void RemoveNthSamplePoint(int n);
  virtual void ClearSamplePoints();
  virtual void GetNthSamplePoint(int n, double xyz[3]) const;

protected:
  vtkMRMLEMSTreeParametersLeafNode();
  ~vtkMRMLEMSTreeParametersLeafNode();
  vtkMRMLEMSTreeParametersLeafNode(const vtkMRMLEMSTreeParametersLeafNode&);
  void operator=(const vtkMRMLEMSTreeParametersLeafNode&);

  int                                 PrintQuality;
  int                                 IntensityLabel;
  int                                 DistributionSpecificationMethod;

  //BTX
  typedef vtkstd::vector<double>                PointType;
  typedef vtkstd::vector<PointType>             SamplePointListType;
  typedef SamplePointListType::iterator         SamplePointListIterator;
  typedef SamplePointListType::const_iterator   SamplePointListConstIterator;

  SamplePointListType                           DistributionSamplePointsRAS;
  vtkstd::vector<double>                        LogMean;
  vtkstd::vector<vtkstd::vector<double> >       LogCovariance;
  //ETX

  unsigned int                        NumberOfTargetInputChannels;
};

#endif
