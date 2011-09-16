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

  virtual double GetLogMean(int index);
  virtual void SetLogMean(int index, double value);

  virtual double GetLogCovariance(int row, int column); 
  virtual void SetLogCovariance(int row, int column, double value);

  virtual double GetLogMeanCorrection(int index);
  virtual void SetLogMeanCorrection(int index, double value);

  virtual double GetLogCovarianceCorrection(int row, int column);
  virtual void SetLogCovarianceCorrection(int row, int column, double value);

  //BTX
  // It makes a copy of it 
  vtkstd::vector<vtkstd::vector<double> >  GetLogCovarianceCorrection()
    {
      return this->LogCovarianceCorrection;
    }

  vtkstd::vector<vtkstd::vector<double> >  GetLogCovariance()
    {
      return this->LogCovariance;
    }

  enum
    {
    DistributionSpecificationManual = 0,
    DistributionSpecificationManuallySample,
    DistributionSpecificationAutoSample
    };

  //ETX
  vtkGetMacro(DistributionSpecificationMethod, int);
  vtkSetMacro(DistributionSpecificationMethod, int);

  // name of the parcellation volume in the atlas
  vtkGetStringMacro(SubParcellationVolumeName);
  vtkSetStringMacro(SubParcellationVolumeName);

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
  char*                               SubParcellationVolumeName;

  //BTX
  typedef vtkstd::vector<double>                PointType;
  typedef vtkstd::vector<PointType>             SamplePointListType;
  typedef SamplePointListType::iterator         SamplePointListIterator;
  typedef SamplePointListType::const_iterator   SamplePointListConstIterator;

  SamplePointListType                           DistributionSamplePointsRAS;
  vtkstd::vector<double>                        LogMean;
  vtkstd::vector<vtkstd::vector<double> >       LogCovariance;

  // These values define the deviation from LogMean and LogCovariance 
  // that way one can automatically compute Intensity distribution from Template and then might slight changes 
  // the gui always shows the LogMean - LogMeanCorrection 
  vtkstd::vector<double>                        LogMeanCorrection;
  vtkstd::vector<vtkstd::vector<double> >       LogCovarianceCorrection;
  //ETX

  unsigned int                        NumberOfTargetInputChannels;
};

#endif
