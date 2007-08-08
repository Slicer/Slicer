/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxRegisterVersorRigidFilter.h,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.3 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mimxRegisterVersorRigidFilter_h
#define __mimxegisterVersorRigidFilter_h

#include "vtkCommon.h"

#include "itkObject.h"
#include "itkImage.h"
#include "itkImageToImageFilter.h"
#include "itkIOCommon.h"
#include <itkExtractImageFilter.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <itkVersorRigid3DTransformOptimizer.h>
#include <itkImageRegistrationMethod.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkVersorRigid3DTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <itkTimeProbesCollectorBase.h>
#include <itkTransformFactory.h>

#include <map>
#include <string>

namespace itk {

    /** \class RegisterVersorRigidFilter
     * \brief Rigid registration convience class between
     * a 3D anatomical images.
     *
     * The Versor Rigid registration is used along with the
     * Mattes mutual information registration. The output
     * of the filter is the resulting registration.
     *
     */

class VTK_MIMXCOMMON_EXPORT mimxRegisterVersorRigidFilter : public itk::Object
{
public:
    /** Standard class typedefs. */
    typedef mimxRegisterVersorRigidFilter Self;
    typedef itk::Object        Superclass;
    typedef SmartPointer<Self>        Pointer;
    typedef SmartPointer<const Self>  ConstPointer;

    /** Fixed Image typedefs. */
    typedef itk::Image<signed short,3>      FixedImageType;
    typedef FixedImageType::Pointer         FixedImagePointer;
    typedef FixedImageType::ConstPointer    FixedImageConstPointer;
    typedef FixedImageType::RegionType      FixedImageRegionType;
    typedef FixedImageType::SizeType        FixedImageSizeType;
    typedef FixedImageType::SpacingType     FixedImageSpacingType;
    typedef FixedImageType::PointType       FixedImagePointType;
    typedef FixedImageType::PixelType       FixedImagePixelType;
    typedef FixedImageType::DirectionType   FixedImageDirectionType;
    typedef FixedImageType::IndexType       FixedImageIndexType;

    /** Moving Image typedefs. */
    typedef itk::Image<signed short,3>       MovingImageType;
    typedef MovingImageType::Pointer     MovingImagePointer;
    typedef MovingImageType::ConstPointer    MovingImageConstPointer;
    typedef MovingImageType::RegionType     MovingImageRegionType;
    typedef MovingImageType::SizeType     MovingImageSizeType;
    typedef MovingImageType::SpacingType     MovingImageSpacingType;
    typedef MovingImageType::PointType     MovingImagePointType;
    typedef MovingImageType::PixelType     MovingImagePixelType;
    typedef MovingImageType::DirectionType   MovingImageDirectionType;
    typedef MovingImageType::IndexType     MovingImageIndexType;
    

    /** Output Transform typedefs. */
    typedef itk::VersorRigid3DTransform< double >     TransformType;
    typedef itk::VersorRigid3DTransformOptimizer      OptimizerType;
    typedef itk::MattesMutualInformationImageToImageMetric<
        FixedImageType,
        MovingImageType >        MetricType;

    typedef itk:: LinearInterpolateImageFunction<
        MovingImageType,
        double          >         InterpolatorType;

    typedef itk::ImageRegistrationMethod<
        FixedImageType,
        MovingImageType >        RegistrationType;

    typedef itk::CenteredTransformInitializer< TransformType,
            FixedImageType,
            MovingImageType
                >  TransformInitializerType;
    typedef TransformType::Pointer  TransformTypePointer;
    typedef TransformType::VersorType  VersorType;
    typedef VersorType::VectorType     VectorType;
    typedef MetricType::Pointer        MetricTypePointer;
    typedef OptimizerType::Pointer      OptimizerTypePointer;
    typedef OptimizerType::ParametersType OptimizerParameterType;
    typedef OptimizerType::ScalesType  OptimizerScalesType;
    typedef InterpolatorType::Pointer   InterpolatorTypePointer;
    typedef RegistrationType::Pointer   RegistrationTypePointer;
    typedef TransformInitializerType::Pointer TransformInitializerTypePointer;

#if 0
    / ** ImageDimension constants * /
      itkStaticConstMacro(InputImageDimension, unsigned int,
      TInputImage::ImageDimension);
      itkStaticConstMacro(OutputImageDimension, unsigned int,
      TOutputImage::ImageDimension);

    / ** The dimensions of the input image must equal those of the
    output image. * /
    itkConceptMacro(SameDimension,
    (Concept::SameDimension<itkGetStaticConstMacro(InputImageDimension),itkGetStaticConstMacro(OutputImageDimension)>));

    / ** The dimension of the input image must be 4. * /
    itkConceptMacro(DimensionShouldBe4,
    (Concept::SameDimension<itkGetStaticConstMacro(InputImageDimension),4>));
     */
#endif
    /** Standard New method. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(RegisterVersorRigidFilter, itk::Object);

    /* SetInput and GetOutput Macros */
    itkSetObjectMacro (FixedImage,  FixedImageType);
    itkSetObjectMacro (MovingImage, MovingImageType);
    itkGetObjectMacro (Output,      TransformType);

    itkSetMacro (NumberOfSpatialSamples, int);
    itkSetMacro (NumberOfIterations,     int);
    itkSetMacro (TranslationScale,       float);
    itkSetMacro (MaximumStepLength,      float);
    itkSetMacro (MinimumStepLength,      float);
    itkSetMacro (RelaxationFactor,       float);


    void Update();

protected:
    mimxRegisterVersorRigidFilter();
    ~mimxRegisterVersorRigidFilter() {};


private:
    mimxRegisterVersorRigidFilter(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented

    // Input and Output Image
    FixedImagePointer         m_FixedImage;
    MovingImagePointer        m_MovingImage;
    TransformTypePointer      m_Output;


    // Registration Parameters
    float m_TranslationScale;
    float m_MaximumStepLength;
    float m_MinimumStepLength;
    float m_RelaxationFactor;
    int   m_NumberOfSpatialSamples;
    int   m_NumberOfIterations;

} ; // end of class

} // end namespace itk


#endif


