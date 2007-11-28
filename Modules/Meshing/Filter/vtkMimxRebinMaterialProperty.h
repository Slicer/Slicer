/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxRebinMaterialProperty.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
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


// .NAME vtkMimxRebinMaterialProperty - Allows the user to reduce the number of
//       discrete values in the grid material property field
//
// .SECTION Description
// vtkMimxRebinMaterialProperty allows the user to change the number of
// discrete values in the "Material Property" Field data. This can be
// useful for analysis in Finite Element modeling. This filter offers
// three forms of operation:
//     1) User specified histogram bins - the cell data is assigned to
//     to one of the histogram bins. Clamping is used for those
//     values that are outside of this range. This requires that the
//     user specifiy both the number of histogram bins and the Array
//     containing the histogram values. 
//
//  2) User specified minimum and maximum. This operates similar to
//     the previously described method, but the bins will be equally 
//     spaced. The user must also specify the number of histogram bins.
// 
//  3) Automatic rebinning. In this case the min and max are computed 
//     from the data and equally spaced bins are placed between these
//     values.
//
// This algorithm makes a deep copy of the input allowing the FieldData
// to be modified.

#ifndef __vtkMimxRebinMaterialProperty_h
#define __vtkMimxRebinMaterialProperty_h

#include "vtkFilter.h"

#include <vtkUnstructuredGridAlgorithm.h>
//#include <vtkTable.h>

//class vtkTable;
class vtkFieldData;

class VTK_MIMXFILTER_EXPORT vtkMimxRebinMaterialProperty : public vtkUnstructuredGridAlgorithm 
{
public:
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMimxRebinMaterialProperty,vtkUnstructuredGridAlgorithm);
  static vtkMimxRebinMaterialProperty* New();

  // Description:
  // Get/Set the number of discrete material property values to be used
  // This is the number of discrete values that will exist within
  // "Material_Property" the FieldData.
  vtkSetMacro(NumberOfHistogramBins, int);
  vtkGetMacro(NumberOfHistogramBins, int);
  
  // Description:
  // Set/Get the lower bound for the discrete material property values
  vtkSetMacro(BinLowerBound,double);
  vtkGetMacro(BinLowerBound,double);
  
  // Description:
  // Set/Get the upper bound for the discrete material property values
  vtkSetMacro(BinUpperBound,double);
  vtkGetMacro(BinUpperBound,double);
  
  // Description:
  // Set/Get the boolean used to control if the algorithm will automatically
  // calculate the maximum value based on the existing material properties. 
  // If this is set to false the  the user should provide the BinLowerBound.
  vtkSetMacro(ComputeMaxBin,bool);
  vtkGetMacro(ComputeMaxBin,bool);
  
  // Description:
  // Set/Get the boolean used to control if the algorithm will automatically
  // calculate the minimum value based on the existing material properties. 
  // If this is set to false the  the user should provide the BinLowerBound.
  vtkSetMacro(ComputeMinBin,bool);
  vtkGetMacro(ComputeMinBin,bool);
  
  // Description:
  // Get/Set the boolean used to control if the user will provide the
  // material properties or if they will be calculated automatically.
  vtkSetMacro(GeneratePropertyBins,bool);
  vtkGetMacro(GeneratePropertyBins,bool);
  
  // Description:
  // Set/Get the bins to be used for the rebinning of material properties.
  //vtkSetMacro(PropertyTable, vtkTable*);
  //vtkGetMacro(PropertyTable, vtkTable*);
  vtkSetMacro(PropertyTable, vtkFieldData*);
  vtkGetMacro(PropertyTable, vtkFieldData*);
  
protected:
  vtkMimxRebinMaterialProperty();
  ~vtkMimxRebinMaterialProperty();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);
  
  
  // Determine the histogram bins for the Mesh
  void ComputeHistogramBins( );

  int NumberOfHistogramBins;
  double BinLowerBound;
  double BinUpperBound;
  bool ComputeMaxBin;
  bool ComputeMinBin;
  bool GeneratePropertyBins;

  //vtkTable* PropertyTable;
  vtkFieldData *PropertyTable;

private:
  vtkMimxRebinMaterialProperty( const vtkMimxRebinMaterialProperty& ); // Not implemented.
  void operator = ( const vtkMimxRebinMaterialProperty& ); // Not implemented.
};

#endif // vtkRebinMaterialProperty_h
