/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkITKImageThresholdCalculator_h
#define __vtkITKImageThresholdCalculator_h

#include "vtkImageAlgorithm.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkMatrix4x4.h"

#include "vtkITK.h"
#include "itkImageIOBase.h"

class vtkStringArray;

class VTK_ITK_EXPORT vtkITKImageThresholdCalculator : public vtkImageAlgorithm
{
public:
  static vtkITKImageThresholdCalculator *New();
  vtkTypeMacro(vtkITKImageThresholdCalculator,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkGetMacro(Threshold, double);

  enum
  {
    METHOD_HUANG,
    METHOD_INTERMODES,
    METHOD_ISO_DATA,
    METHOD_KITTLER_ILLINGWORTH,
    METHOD_LI,
    METHOD_MAXIMUM_ENTROPY,
    METHOD_MOMENTS,
    METHOD_OTSU,
    METHOD_RENYI_ENTROPY,
    METHOD_SHANBHAG,
    METHOD_TRIANGLE,
    METHOD_YEN,
    NUMBER_OF_METHODS // this must be the last
  };

  //@{
  /**
   * The resizing method to use.  The default is to set the output image
   * dimensions, and allow the filter to resize the image to these new
   * dimensions.  It is also possible to resize the image by setting the
   * output image spacing or by setting a magnification factor.
   */
  vtkSetClampMacro(Method, int, 0, NUMBER_OF_METHODS-1);
  vtkGetMacro(Method, int);
  void SetMethodToHuang() { this->SetMethod(METHOD_HUANG); }
  void SetMethodToIntermodes() { this->SetMethod(METHOD_INTERMODES); }
  void SetMethodToIsoData() { this->SetMethod(METHOD_ISO_DATA); }
  void SetMethodToKittlerIllingworth() { this->SetMethod(METHOD_KITTLER_ILLINGWORTH); }
  void SetMethodToLi() { this->SetMethod(METHOD_LI); }
  void SetMethodToMaximumEntropy() { this->SetMethod(METHOD_MAXIMUM_ENTROPY); }
  void SetMethodToMoments() { this->SetMethod(METHOD_MOMENTS); }
  void SetMethodToOtsu() { this->SetMethod(METHOD_OTSU); }
  void SetMethodToRenyiEntropy() { this->SetMethod(METHOD_RENYI_ENTROPY); }
  void SetMethodToShanbhag() { this->SetMethod(METHOD_SHANBHAG); }
  void SetMethodToTriangle() { this->SetMethod(METHOD_TRIANGLE); }
  void SetMethodToYen() { this->SetMethod(METHOD_YEN); }
  static const char *GetMethodAsString(int method);
  //@}

  /// Bring vtkAlgorithm::Update methods here
  /// to avoid hiding Update override.
  using vtkAlgorithm::Update;
  /// The main interface which triggers the writer to start.
  void Update() override;

protected:
  vtkITKImageThresholdCalculator();
  ~vtkITKImageThresholdCalculator() override;

  int Method;
  double Threshold;

private:
  vtkITKImageThresholdCalculator(const vtkITKImageThresholdCalculator&) = delete;
  void operator=(const vtkITKImageThresholdCalculator&) = delete;
};

//vtkStandardNewMacro(vtkITKImageThresholdCalculator)

#endif
