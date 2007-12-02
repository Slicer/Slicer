/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxRebinMaterialProperty.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.2 $

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
#include <vtkCellLinks.h>
#include <vtkFieldData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkAbstractArray.h>
#include <vtkUnstructuredGridAlgorithm.h>
//#include <vtkTable.h>
#include <vtkFieldData.h>

#include "vtkMimxRebinMaterialProperty.h"

vtkCxxRevisionMacro(vtkMimxRebinMaterialProperty,"$Revision: 1.2 $");
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
  // the following line should fix leak, but vtkCellLinks::DeepCopy does a memcopy of 
  // pointers to allocate memory, meaning that they get double-freed by the destructor
  //out->GetCellLinks()->Delete(); // need to do this because BuildLinks will create a new vtkCellLinks
  out->BuildLinks( );
  
  /* Get the Grid Field Data - Material Property */
  vtkFieldData *gridFieldData = in->GetFieldData();
  //vtkDoubleArray *materialPropertyArray = NULL;
  //vtkAbstractArray *tmpArray = gridFieldData->GetAbstractArray("Material_Properties");
  //vtkAbstractArray *tmpArray = vtkAbstractArray::SafeDownCast( gridFieldData->GetArray("Material_Properties") );
  //if (tmpArray->IsA("vtkDoubleArray"))
  //  {
  //  materialPropertyArray = vtkDoubleArray::SafeDownCast(tmpArray);
  //  }
  vtkDoubleArray *materialPropertyArray = vtkDoubleArray::New();
  materialPropertyArray->SetNumberOfComponents(1);
  int fieldDataSize = gridFieldData->GetArray("Material_Properties")->GetNumberOfTuples();
  materialPropertyArray->SetNumberOfTuples( fieldDataSize );
  gridFieldData->GetArray("Material_Properties")->GetData(0, fieldDataSize-1, 0, 0, materialPropertyArray);
  if ( materialPropertyArray == NULL)
  {
    vtkErrorMacro(<< "vtkRebinMaterialProperty::RequestData - Grid does not contain 'Material_Properties' array");
    return 0;
  }
  
  if ( this->NumberOfHistogramBins == 0 )
  {
    vtkErrorMacro(<< "vtkRebinMaterialProperty::RequestData - Invalid number of histogram bins specified");
    return 0;
  } 
  
  /* Determine the Histogram Bins - 1) From user or 2) from the data */
  if ( ! this->GeneratePropertyBins )
    {
    if ( this->PropertyTable->GetArray("Histogram") == NULL )
    //if ( this->PropertyTable->GetColumnByName("Histogram") == NULL )
      {
      vtkErrorMacro(<< "vtkRebinMaterialProperty::RequestData - Defined Property Table must have field data named 'Histogram'");
      return 0;
      }
    this->NumberOfHistogramBins = this->PropertyTable->GetArray("Histogram")->GetNumberOfTuples();
    //this->NumberOfHistogramBins = this->PropertyTable->GetColumnByName("Histogram")->GetNumberOfTuples();
    }
  else
    {
    if ( this->ComputeMaxBin ) this->BinUpperBound = materialPropertyArray->GetValue(0);
    if ( this->ComputeMinBin ) this->BinLowerBound = materialPropertyArray->GetValue(0);
    for (int i=0;i<materialPropertyArray->GetNumberOfTuples();i++)
      {
      double tmpValue = materialPropertyArray->GetValue(i);
      if ( this->ComputeMaxBin && (this->BinUpperBound < tmpValue) )
        {
        this->BinUpperBound = tmpValue;
        }
      if ( this->ComputeMinBin && (this->BinLowerBound > tmpValue) )
        {
        this->BinLowerBound = tmpValue;
        }
      }
    ComputeHistogramBins( ); 
    }
    
  //vtkDoubleArray* histogramArray = NULL;
  //vtkAbstractArray *tableArray =  vtkAbstractArray::SafeDownCast( this->PropertyTable->GetArray("Histogram") ); 
  //vtkAbstractArray *tableArray = this->PropertyTable->GetColumnByName("Histogram");
  //if (tableArray->IsA("vtkDoubleArray"))
  //  {
  //  histogramArray = vtkDoubleArray::SafeDownCast(tableArray);
  //  if ( ! this->GeneratePropertyBins )
  //    {
  //    this->BinUpperBound = histogramArray->GetValue( this->NumberOfHistogramBins-1 );
  //    this->BinLowerBound = histogramArray->GetValue( 0 );
  //    }
  //  }
  vtkDoubleArray *histogramArray = vtkDoubleArray::New();
  histogramArray->SetNumberOfComponents(1);
  this->NumberOfHistogramBins = this->PropertyTable->GetArray("Histogram")->GetNumberOfTuples();
  histogramArray->SetNumberOfTuples( this->NumberOfHistogramBins );
  
  this->PropertyTable->GetArray("Histogram")->GetData(0, this->NumberOfHistogramBins-1, 0, 0, histogramArray);
  if (histogramArray->IsA("vtkDoubleArray"))
    {
    if ( ! this->GeneratePropertyBins )
      {
      this->BinUpperBound = histogramArray->GetValue( this->NumberOfHistogramBins-1 );
      this->BinLowerBound = histogramArray->GetValue( 0 );
      }
    }

  /* Rebin the Material Properties for the Grid */
  vtkDoubleArray *binPropertyArray = vtkDoubleArray::New();
  binPropertyArray->SetNumberOfValues( materialPropertyArray->GetNumberOfTuples() );
  binPropertyArray->SetName( "Material_Properties" );
  
  for (int i=0;i<binPropertyArray->GetNumberOfTuples();i++)
    {
    double currentMaterialProperty = materialPropertyArray->GetValue(i);
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
             ( currentMaterialProperty < histogramArray->GetValue(k+1)) )
          {
          binPropertyArray->SetValue( i, histogramArray->GetValue(k) );
          break;
          } 
        }
      }
    }
  histogramArray->Delete( );  
  materialPropertyArray->Delete( );
  
  gridFieldData->RemoveArray( "Material_Properties" );
  gridFieldData->AddArray( binPropertyArray );
  out->SetFieldData( gridFieldData );
  //binPropertyArray->Delete( );

  return 1;
}

void vtkMimxRebinMaterialProperty::ComputeHistogramBins(  ) 
{
  //Calculating and setting the bin ranges and bin values
  // this->PropertyTable = vtkTable::New();
  this->PropertyTable = vtkFieldData::New();
  
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
    // this->PropertyTable->AddColumn( histogramArray );
    this->PropertyTable->AddArray( histogramArray );
  }
}

