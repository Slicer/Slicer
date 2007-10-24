/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPolyDataTensorToColor.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPolyDataTensorToColor - extracts points whose scalar value satisfies threshold criterion
// .SECTION Description
// vtkPolyDataTensorToColor is a filter that extracts points from a dataset that 
// satisfy a threshold criterion. The criterion can take three forms:
// 1) greater than a particular value; 2) less than a particular value; or
// 3) between a particular value. The output of the filter is polygonal data.
// .SECTION See Also
// vtkThreshold

#ifndef __vtkPolyDataTensorToColor_h
#define __vtkPolyDataTensorToColor_h

#include "vtkPolyDataAlgorithm.h"
#include "vtkTeemConfigure.h"

#include "vtkTensorGlyph.h"

class VTK_TEEM_EXPORT vtkPolyDataTensorToColor : public vtkPolyDataAlgorithm
{
public:
  static vtkPolyDataTensorToColor *New();
  vtkTypeRevisionMacro(vtkPolyDataTensorToColor,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetClampMacro(ColorMode, int, vtkTensorGlyph::COLOR_BY_SCALARS, vtkTensorGlyph::COLOR_BY_EIGENVALUES);
  vtkGetMacro(ColorMode, int);
  void SetColorModeToScalars()
    {this->SetColorMode(vtkTensorGlyph::COLOR_BY_SCALARS);};
  void SetColorModeToEigenvalues()
    {this->SetColorMode(vtkTensorGlyph::COLOR_BY_EIGENVALUES);}; 
    
  // Description:
  // Turn on/off extraction of eigenvalues from tensor.
  vtkSetMacro(ExtractEigenvalues,int);
  vtkBooleanMacro(ExtractEigenvalues,int);
  vtkGetMacro(ExtractEigenvalues,int);

  // Description:
  // Turn on/off extraction of scalars for color.
  vtkSetMacro(ExtractScalar,int);
  vtkBooleanMacro(ExtractScalar,int);
  vtkGetMacro(ExtractScalar,int);

    
  // TO DO: make more of these

  // Description:
  // Output one component scalars according to scalar invariants
  void ColorGlyphsByLinearMeasure();
  void ColorGlyphsBySphericalMeasure();
  void ColorGlyphsByPlanarMeasure();
  void ColorGlyphsByMaxEigenvalue();
  void ColorGlyphsByMidEigenvalue();
  void ColorGlyphsByMinEigenvalue();
  void ColorGlyphsByRelativeAnisotropy();
  void ColorGlyphsByFractionalAnisotropy();
  void ColorGlyphsByTrace();


protected:
  vtkPolyDataTensorToColor();
  ~vtkPolyDataTensorToColor() {};

  // Usual data generation method
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  void ColorGlyphsBy(int measure);
  int ColorMode; // The coloring mode to use for the glyphs.
  int ExtractEigenvalues; // Boolean controls eigenfunction extraction
  
  int ExtractScalar;

  int ScalarInvariant;  // which function of eigenvalues to use for coloring

private:
  vtkPolyDataTensorToColor(const vtkPolyDataTensorToColor&);  // Not implemented.
  void operator=(const vtkPolyDataTensorToColor&);  // Not implemented.
};

#endif
