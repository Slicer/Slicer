#ifndef __vtkMRMLEMSIntensityNormalizationParametersNode_h
#define __vtkMRMLEMSIntensityNormalizationParametersNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkEMSegment.h"

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSIntensityNormalizationParametersNode : 
  public vtkMRMLNode
{
public:
  static vtkMRMLEMSIntensityNormalizationParametersNode *New();
  vtkTypeMacro(vtkMRMLEMSIntensityNormalizationParametersNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  virtual const char* GetNodeTagName()
    {return "EMSIntensityNormalizationParameters";}

  //
  // parameters for normalization filter
  vtkGetMacro(NormValue, double);
  vtkSetMacro(NormValue, double);

  vtkGetMacro(NormType, int);
  vtkSetMacro(NormType, int);

  vtkGetMacro(InitialHistogramSmoothingWidth, int);
  vtkSetMacro(InitialHistogramSmoothingWidth, int);

  vtkGetMacro(MaxHistogramSmoothingWidth, int);
  vtkSetMacro(MaxHistogramSmoothingWidth, int);
  
  vtkGetMacro(RelativeMaxVoxelNum, float);
  vtkSetMacro(RelativeMaxVoxelNum, float);

  vtkGetMacro(PrintInfo, int);
  vtkSetMacro(PrintInfo, int);

  vtkGetMacro(Enabled, int);
  vtkSetMacro(Enabled, int);
  vtkBooleanMacro(Enabled, int);

  //
  // default settings
  virtual void SetToDefaultT1SPGR();
  virtual void SetToDefaultT2();
  // sometimes T2 image histograms have a large "hump" at the end, use
  // this setting for those cases
  virtual void SetToDefaultT2_2();

protected:
  double           NormValue;
  int              NormType;
  int              InitialHistogramSmoothingWidth;
  int              MaxHistogramSmoothingWidth;
  float            RelativeMaxVoxelNum;
  int              PrintInfo;
  int              Enabled;

private:
  vtkMRMLEMSIntensityNormalizationParametersNode();
  ~vtkMRMLEMSIntensityNormalizationParametersNode();
  vtkMRMLEMSIntensityNormalizationParametersNode(const vtkMRMLEMSIntensityNormalizationParametersNode&);
  void operator=(const vtkMRMLEMSIntensityNormalizationParametersNode&);
};

#endif
