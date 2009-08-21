/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile:$
  Date:      $Date:$
  Version:   $Revision:$

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkIntensityCurves.h"

#include "vtkDoubleArray.h"
#include "vtkMRMLTimeSeriesBundleNode.h"
#include "vtkMRMLScalarVolumeNode.h"

vtkStandardNewMacro(vtkIntensityCurves);
vtkCxxRevisionMacro(vtkIntensityCurves, "$Revision: $");

//---------------------------------------------------------------------------
vtkIntensityCurves::vtkIntensityCurves()
{
  this->BundleNode = NULL;
  this->MaskNode   = NULL;
  this->PreviousBundleNode = NULL;
  this->PreviousMaskNode   = NULL;
  
  this->IntensityCurve.clear();
  this->PreviousUpdateTime = 0;
}


//---------------------------------------------------------------------------
vtkIntensityCurves::~vtkIntensityCurves()
{
}


//---------------------------------------------------------------------------
void vtkIntensityCurves::PrintSelf(ostream& os, vtkIndent indent)
{
}


//---------------------------------------------------------------------------
int vtkIntensityCurves::Update()
{

  if (!this->BundleNode || !this->MaskNode)
    {
    std::cerr << "bundle or mask node has not been set." << std::endl;
    return 0;
    }

  std::cerr << "intensity update: " << this->GetMTime() << std::endl;
  std::cerr << "Bundle update: " << this->BundleNode->GetMTime() << std::endl;
  std::cerr << "Mask update: " << this->MaskNode->GetMTime() << std::endl;

  // if BundleNode or MaskNode have been changed from the previous update
  if (this->BundleNode != this->PreviousBundleNode ||
      this->MaskNode != this->PreviousMaskNode)
    {
    // set PreviousUpdate time to zero to force intensity curve generation
    this->PreviousUpdateTime = 0;
    }

  if (this->PreviousUpdateTime > this->BundleNode->GetMTime() &&
      this->PreviousUpdateTime > this->MaskNode->GetMTime())
    {
    // if the intensity curve is newer than the 4D bundle and the mask
    // do nothing
    std::cerr << "intensity curve is up to date." << std::endl;
    return 1;
    }
  
  // Generate intensity curves
  this->PreviousBundleNode = this->BundleNode;
  this->PreviousMaskNode   = this->MaskNode;

  GenerateIntensityCurve();

  this->Modified();
  this->PreviousUpdateTime = this->GetMTime();
}


//---------------------------------------------------------------------------
vtkIntArray* vtkIntensityCurves::GetLabelList()
{
  vtkIntArray* array = vtkIntArray::New();

  IntensityCurveMapType::iterator iter;
  for (iter = this->IntensityCurve.begin(); iter != this->IntensityCurve.end(); iter ++)
    {
    int label = iter->first;
    array->InsertNextValue(label);
    }

  return array;
}


//---------------------------------------------------------------------------
vtkMRMLDoubleArrayNode* vtkIntensityCurves::GetCurve(int label)
{
  IntensityCurveMapType::iterator iter;

  iter = this->IntensityCurve.find(label);
  if (iter != this->IntensityCurve.end())
    {
    return iter->second;
    }
  else
    {
    return NULL;
    }

}


//---------------------------------------------------------------------------
int vtkIntensityCurves::OutputDataInCSV(ostream& os, int label)
{

  Update();

  vtkMRMLDoubleArrayNode* anode = this->GetCurve(label);
  if (anode)
    {
    vtkDoubleArray* data = anode->GetArray();
    if (data)
      {
      int nData = data->GetNumberOfTuples();
      for (int i = 0; i < nData; i ++)
        {
        double* xy = data->GetTuple(i);
        // Write the data
        //      t        ,      mean     ,      std
        //   -----------------------------------------
        os << xy[0] << ", " << xy[1] << ", " << xy[2] << std::endl;
        }
      }
    return 1;
    }
  else
    {
    return 0;
    }

}


//---------------------------------------------------------------------------
void vtkIntensityCurves::GenerateIntensityCurve()
{

  this->IntensityCurve.clear();

  if (this->BundleNode && this->MaskNode)
    {
    vtkImageData*  mask = this->MaskNode->GetImageData();

    IndexTableMapType  indexTableMap;
    GenerateIndexMap(mask, indexTableMap);

    IndexTableMapType::iterator iter;

    vtkDoubleArray* array;
    
    for (iter = indexTableMap.begin(); iter != indexTableMap.end(); iter ++)
      {
      int label = iter->first;
      //this->IntensityCurve[label] = vtkDoubleArray::New();
      //this->IntensityCurve[label]->SetNumberOfComponents( static_cast<vtkIdType>(3) );
      array  = vtkDoubleArray::New();
      array->SetNumberOfComponents( static_cast<vtkIdType>(3) );

      vtkMRMLDoubleArrayNode* anode = vtkMRMLDoubleArrayNode::New();
      this->GetMRMLScene()->AddNode(anode);
      this->IntensityCurve[label] = anode;
      anode->SetArray(array);
      anode->Delete();
      }
    
    int nFrames = this->BundleNode->GetNumberOfFrames();
    for (int i = 0; i < nFrames; i ++)
      {
      //std::cerr << "processing frame = " << i << std::endl;
      vtkMRMLScalarVolumeNode* inode
        = vtkMRMLScalarVolumeNode::SafeDownCast(this->BundleNode->GetFrameNode(i));
      if (inode)
        {
        IndexTableMapType::iterator iter2;
        for (iter2 = indexTableMap.begin(); iter2 != indexTableMap.end(); iter2 ++)
          {
          int label = iter2->first;
          //std::cerr << "    processing label = " << label << std::endl;
          IndexTableType& indexTable = iter2->second;
          double meanvalue = GetMeanIntensity(inode->GetImageData(), indexTable);
          double sdvalue   = GetSDIntensity(inode->GetImageData(), meanvalue, indexTable);
          //std::cerr << "mean = " << meanvalue << ", sd = " << sdvalue << std::endl;
          
          // get time stamp
          vtkMRMLTimeSeriesBundleNode::TimeStamp ts;
          this->BundleNode->GetTimeStamp(i, &ts);

          double xy[3];
          //xy[0] = (double)i * this->Interval;
          xy[0] = (double)ts.second + (double)ts.nanosecond / 1000000000.0;
          xy[1] = meanvalue;
          xy[2] = sdvalue;
          this->IntensityCurve[label]->GetArray()->InsertNextTuple(xy);
          }
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkIntensityCurves::GenerateIndexMap(vtkImageData* mask, IndexTableMapType& indexTableMap)
{
  int maskDim[3];

  mask->GetDimensions(maskDim);
  int x = maskDim[0];
  int y = maskDim[1];
  int z = maskDim[2];

  indexTableMap.clear();
  for (int i = 0; i < x; i ++)
    {
    for (int j = 0; j < y; j ++)
      {
      for (int k = 0; k < z; k ++)
        {
        int label = mask->GetScalarComponentAsDouble(i, j, k, 0);
        if (label != 0)
          {
          CoordType index;
          index.x = i;
          index.y = j;
          index.z = k;
          indexTableMap[label].push_back(index);
          }
        }
      }
    }
}


//---------------------------------------------------------------------------
double vtkIntensityCurves::GetMeanIntensity(vtkImageData* image, IndexTableType& indexTable)
{
  double sum = 0.0;
  
  IndexTableType::iterator iter;
  for (iter = indexTable.begin(); iter != indexTable.end(); iter ++)
    {
    sum += image->GetScalarComponentAsDouble(iter->x, iter->y, iter->z, 0);
    }

  double mean = sum / (double)indexTable.size();

  return mean;
}


double vtkIntensityCurves::GetSDIntensity(vtkImageData* image, double mean, IndexTableType& indexTable)
{
  double s = 0.0;
  double n = (double) indexTable.size();

  IndexTableType::iterator iter;
  for (iter = indexTable.begin(); iter != indexTable.end(); iter ++)
    {
    double i = image->GetScalarComponentAsDouble(iter->x, iter->y, iter->z, 0);
    s += (i - mean)*(i - mean);
    }
  double sigma = sqrt(s/n);
  return sigma;
}



