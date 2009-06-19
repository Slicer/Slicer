/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRML4DBundleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkIntensityCurves_h
#define __vtkIntensityCurves_h


#include "vtkObject.h"
#include "vtkFourDAnalysisWin32Header.h"

#include "vtkMRML4DBundleNode.h"

class VTK_FourDAnalysis_EXPORT vtkIntensityCurves : public vtkObject
{
 protected:
  //BTX
  typedef struct {
    int x;
    int y;
    int z;
  } CoordType;
  typedef std::vector<CoordType>         IndexTableType;
  typedef std::map<int, IndexTableType>  IndexTableMapType;  // IndexTableMapType[label]
  typedef std::map<int, vtkDoubleArray*> IntensityCurveMapType;
  //ETX

 public:

  static vtkIntensityCurves *New();
  vtkTypeRevisionMacro(vtkIntensityCurves, vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetObjectMacro ( BundleNode, vtkMRML4DBundleNode );
  vtkSetObjectMacro ( BundleNode, vtkMRML4DBundleNode );
  vtkGetObjectMacro ( MaskNode,   vtkMRMLScalarVolumeNode );
  vtkSetObjectMacro ( MaskNode,   vtkMRMLScalarVolumeNode );

  void            SetInterval(double interval) { this->Interval = interval; };
  int             Update();
  vtkIntArray*    GetLabelList();
  vtkDoubleArray* GetCurve(int label);
  int             OutputDataInCSV(ostream& os, int label);

 protected:
  vtkIntensityCurves();
  virtual ~vtkIntensityCurves();

  //void   GenerateIntensityCurveMap();
  void   GenerateIntensityCurve();
  void   GenerateIndexMap(vtkImageData* mask, IndexTableMapType& indexTableMap);
  double GetMeanIntensity(vtkImageData* image, IndexTableType& indexTable);
  double GetSDIntensity(vtkImageData* image, double mean, IndexTableType& indexTable);

 private:

  vtkMRML4DBundleNode*     BundleNode;
  vtkMRMLScalarVolumeNode* MaskNode;

  vtkMRML4DBundleNode*     PreviousBundleNode;
  vtkMRMLScalarVolumeNode* PreviousMaskNode;

  IntensityCurveMapType IntensityCurve;  // IntensityCurveMean[label]
  long                  PreviousUpdateTime;
  double                Interval;        // interval (s)

};


#endif //__vtkIntensityCurves_h
