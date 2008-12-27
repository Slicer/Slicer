/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxRebinMaterialProperty.cxx,v $
Language:  C++
Date:      $Date: 2008/07/23 20:33:39 $
Version:   $Revision: 1.5 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx
 
Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkUnstructuredGrid.h>
#include <vtkFieldData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkAbstractArray.h>
#include <vtkUnstructuredGridAlgorithm.h>
#include <vtkTable.h>
#include "vtkCellData.h"

#include "vtkMimxRebinMaterialProperty.h"

vtkCxxRevisionMacro(vtkMimxRebinMaterialProperty,"$Revision: 1.5 $");
vtkStandardNewMacro(vtkMimxRebinMaterialProperty);


void vtkMimxRebinMaterialProperty::PrintSelf(ostream& os, vtkIndent indent )
{
  const char onStr[] = "On";
  const char offStr[] = "Off";

  this->Superclass::PrintSelf( os, indent );

  os << indent << "NumberOfHistogramBins:   "
     << this->NumberOfHistogramBins << endl;
  os << indent << "BinLowerBound: "
     << this->BinLowerBound << endl;
  os << indent << "BinUpperBound: "
     << this->BinUpperBound << endl;
  os << indent << "ComputeMaxBin: "
     << (this->ComputeMaxBin ? onStr : offStr) << endl;
  os << indent << "ComputeMinBin: "
     << (this->ComputeMinBin ? onStr : offStr) << endl;
  os << indent << "GeneratePropertyBins: " 
     << (this->GeneratePropertyBins ? onStr : offStr) << endl;
  os << indent << "PropertyTable: "; 
  this->PropertyTable->PrintSelf(os, indent); 
  os << endl;
}

vtkMimxRebinMaterialProperty::vtkMimxRebinMaterialProperty()
{
  this->NumberOfHistogramBins = 0; // Default is On
  this->BinLowerBound = 0.0;
  this->BinUpperBound = 0.0;
  this->ComputeMaxBin = true;
  this->ComputeMinBin = true;
  this->GeneratePropertyBins = true;
  this->PropertyTable = NULL;
  this->SetNumberOfInputPorts( 1 );
  this->ElementSetName = NULL;
}

int vtkMimxRebinMaterialProperty::FillInputPortInformation(
  int,
  vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
  return 1;
}

vtkMimxRebinMaterialProperty::~vtkMimxRebinMaterialProperty()
{
  // Nothing yet.
}

int vtkMimxRebinMaterialProperty::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkUnstructuredGrid *in = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUnstructuredGrid* out = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  

  out->DeepCopy( in );
  
  /* Get the Grid Field Data - Material Property */
//  vtkFieldData *gridFieldData = in->GetFieldData();

  char str[256];
  strcpy(str, this->ElementSetName);
  strcat(str, "_Image_Based_Material_Property");

  vtkDoubleArray *materialPropertyArray = vtkDoubleArray::SafeDownCast(
    in->GetCellData()->GetArray(str));
  /*NULL;*/
  //vtkAbstractArray *tmpArray = gridFieldData->GetAbstractArray("Material_Properties");
  //if (tmpArray->IsA("vtkDoubleArray"))
  //  {
  //  materialPropertyArray = vtkDoubleArray::SafeDownCast(tmpArray);
  //  }

  if(!this->ElementSetName)
    {
    vtkErrorMacro(<<"Set the Element Set Name for which material properties need to be binned");
    }
  if ( materialPropertyArray == NULL)
    {
    vtkErrorMacro(<< "vtkRebinMaterialProperty::RequestData - Grid does not contain Material_Properties data");
    return 0;
    }
  
  if ( this->NumberOfHistogramBins <= 0 )
    {
    vtkErrorMacro(<< "vtkRebinMaterialProperty::RequestData - Invalid number of histogram bins specified");
    return 0;
    } 
  
  /* Determine the Histogram Bins - 1) From user or 2) from the data */
  if ( ! this->GeneratePropertyBins )
    {
    if ( this->PropertyTable->GetColumnByName("Histogram") == NULL )
      {
      vtkErrorMacro(<< "vtkRebinMaterialProperty::RequestData - Defined Property Table must have field data named 'Histogram'");
      return 0;
      }
    this->NumberOfHistogramBins = this->PropertyTable->GetColumnByName("Histogram")->GetNumberOfTuples();
    }
  else
    {
    int i;
    if ( this->ComputeMaxBin )
      {
      for (i=0; i<materialPropertyArray->GetNumberOfTuples(); i++)
        {
        if(materialPropertyArray->GetValue(i) >= 0.0)
          {
          this->BinUpperBound = materialPropertyArray->GetValue(i);
          break;
          }
        }
      }
    //
    if ( this->ComputeMinBin )
      {
      for (i=0; i<materialPropertyArray->GetNumberOfTuples(); i++)
        {
        if(materialPropertyArray->GetValue(i) >= 0.0)
          {
          this->BinLowerBound = materialPropertyArray->GetValue(i);
          break;
          }
        }
      }

    for (i=0;i<materialPropertyArray->GetNumberOfTuples();i++)
      {
      double tmpValue = materialPropertyArray->GetValue(i);
      if(tmpValue >= 0.0)
        {
        if ( this->ComputeMaxBin && (this->BinUpperBound < tmpValue) )
          {
          this->BinUpperBound = tmpValue;
          }
        if ( this->ComputeMinBin && (this->BinLowerBound > tmpValue) )
          {
          this->BinLowerBound = tmpValue;
          }
        }
      }
    ComputeHistogramBins( ); 
    }
    
  vtkDoubleArray* histogramArray = NULL;
  vtkAbstractArray *tableArray = this->PropertyTable->GetColumnByName("Histogram");
  if (tableArray->IsA("vtkDoubleArray"))
    {
    histogramArray = vtkDoubleArray::SafeDownCast(tableArray);
    if ( ! this->GeneratePropertyBins )
      {
      this->BinUpperBound = histogramArray->GetValue( this->NumberOfHistogramBins-1 );
      this->BinLowerBound = histogramArray->GetValue( 0 );
      }
    }

  /* Rebin the Material Properties for the Grid */
  strcat(str,"_ReBin");

  vtkDoubleArray *RebinArray = vtkDoubleArray::SafeDownCast(
    out->GetCellData()->GetArray(str));

  if(RebinArray)        out->GetCellData()->RemoveArray(str);

  vtkDoubleArray *binPropertyArray = vtkDoubleArray::New();
  binPropertyArray->SetNumberOfValues( materialPropertyArray->GetNumberOfTuples() );
  binPropertyArray->SetName( str );
  
  for (int i=0;i<binPropertyArray->GetNumberOfTuples();i++)
    {
    double currentMaterialProperty = materialPropertyArray->GetValue(i);
    if(currentMaterialProperty >= 0.0)
      {
      if ( currentMaterialProperty <= this->BinLowerBound )
        {
        // Clamp Material Properties to lower bound
        binPropertyArray->SetValue( i, this->BinLowerBound );
        }
      else if ( currentMaterialProperty >= this->BinUpperBound )
        {
        // Clamp Material Properties to upper bound
        binPropertyArray->SetValue( i, this->BinUpperBound );
        }
      else
        {
        // Assign Material Properties based on the histogram
        for ( int k = 0; k < this->NumberOfHistogramBins-1; k++ )
          {
          if ( ( currentMaterialProperty >= histogramArray->GetValue(k) ) &&
               ( currentMaterialProperty <= histogramArray->GetValue(k+1)) )
            {
            if(fabs(currentMaterialProperty - histogramArray->GetValue(k)) > 
               fabs(currentMaterialProperty - histogramArray->GetValue(k+1)))
              binPropertyArray->SetValue( i, histogramArray->GetValue(k+1) );
            else
              binPropertyArray->SetValue( i, histogramArray->GetValue(k));
            break;
            } 
          }
        }
      }
    else
      {
      binPropertyArray->SetValue( i, -9999 );
      }
    }
  // out->GetCellData()->AddArray(materialPropertyArray) ; 
  out->GetCellData()->AddArray(binPropertyArray);
  binPropertyArray->Delete();

  return 1;
}

void vtkMimxRebinMaterialProperty::ComputeHistogramBins(  ) 
{
  //Calculating and setting the bin ranges and bin values
  this->PropertyTable = vtkTable::New();
  
  if ( this->NumberOfHistogramBins > 1 )
    {
    vtkDoubleArray *histogramArray = vtkDoubleArray::New();
    histogramArray->SetName("Histogram");
    
    double binSize = ( this->BinUpperBound - this->BinLowerBound ) /
      static_cast<double>( this->NumberOfHistogramBins - 1);
    for ( int i = 0; i < this->NumberOfHistogramBins; i++ )
      {
      histogramArray->InsertValue( i, this->BinLowerBound + 
                                   static_cast<double> ( i )* binSize );
      } 
    this->PropertyTable->AddColumn( histogramArray );
    }
}
