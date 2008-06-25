/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxRegisterBSplineFilter.h,v $
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

#ifndef __mimxRegisterBSplineFilter_h
#define __mimxRegisterBSplineFilter_h

#include "vtkCommon.h"

#include "itkObject.h"
#include "itkImage.h"
#include "itkImageToImageFilter.h"
#include "itkIOCommon.h"
#include <itkExtractImageFilter.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <itkImageRegistrationMethod.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkBSplineDeformableTransform.h>
#include <itkLBFGSBOptimizer.h>
#include <itkCenteredTransformInitializer.h>
#include <itkTimeProbesCollectorBase.h>
#include <itkTransformFactory.h>

namespace itk {

    /** \class DtiToAnatomicalBSplineRegistrationFilter
     * \brief This is a convience class to perform non linear image registration
     *  between a 3D image set and a 4D image set. The input 
     *  images must be signed short. In future revisions, the
     *  class will be modified to support vector images instead 
     *  of 4D images.
     *
     * For image registration the B-Spline Registration is used with the
     * MattesMutualInformationImageToImageMetric. A bulk transform can be
     * specified proving an initial starting point for the deformable 
     * registration. 
     */

class VTK_MIMXCOMMON_EXPORT mimxRegisterBSplineFilter : public itk::Object
{
public:
    /** Standard class typedefs. */
    typedef mimxRegisterBSplineFilter   Self;
    typedef itk::Object        Superclass;
    typedef SmartPointer<Self>        Pointer;
    typedef SmartPointer<const Self>  ConstPointer;

    /** Fixed Image typedefs. */
    typedef itk::Image<signed short,3>      FixedImageType;
    typedef FixedImageType::Pointer    FixedImagePointer;
    typedef FixedImageType::ConstPointer    FixedImageConstPointer;
    typedef FixedImageType::RegionType    FixedImageRegionType;
    typedef FixedImageType::SizeType    FixedImageSizeType;
    typedef FixedImageType::SpacingType    FixedImageSpacingType;
    typedef FixedImageType::PointType    FixedImagePointType;
    typedef FixedImageType::PixelType    FixedImagePixelType;
    typedef FixedImageType::DirectionType   FixedImageDirectionType;
    typedef FixedImageType::IndexType    FixedImageIndexType;

    /** Moving Image typedefs. */
    typedef itk::Image<signed short,3>      MovingImageType;
    typedef MovingImageType::Pointer     MovingImagePointer;
    typedef MovingImageType::ConstPointer    MovingImageConstPointer;
    typedef MovingImageType::RegionType     MovingImageRegionType;
    typedef MovingImageType::SizeType     MovingImageSizeType;
    typedef MovingImageType::SpacingType     MovingImageSpacingType;
    typedef MovingImageType::PointType     MovingImagePointType;
    typedef MovingImageType::PixelType     MovingImagePixelType;
    typedef MovingImageType::DirectionType   MovingImageDirectionType;
    typedef MovingImageType::IndexType     MovingImageIndexType;


    /** Internal typedefs. */
    static const unsigned int SpaceDimension = 3;
    static const unsigned int SplineOrder = 3;
    typedef double CoordinateRepType;
    typedef itk::BSplineDeformableTransform<
        CoordinateRepType,
        SpaceDimension,
        SplineOrder >               TransformType;
    typedef TransformType::RegionType               TransformRegionType;
    typedef TransformRegionType::SizeType              TransformSizeType;
    typedef TransformType::SpacingType               TransformSpacingType;
    typedef TransformType::OriginType               TransformOriginType;
    typedef TransformType::ParametersType            TransformParametersType;

    typedef itk::LBFGSBOptimizer                     OptimizerType;


    typedef itk::MattesMutualInformationImageToImageMetric<
        FixedImageType,
        MovingImageType >          MetricType;

    typedef itk:: LinearInterpolateImageFunction<
        MovingImageType,
        double          >        InterpolatorType;

    typedef itk::ImageRegistrationMethod<
        FixedImageType,
        MovingImageType >          RegistrationType;


    typedef TransformType::Pointer             TransformTypePointer;
    typedef MetricType::Pointer                  MetricTypePointer;
    typedef OptimizerType::Pointer               OptimizerTypePointer;
    typedef OptimizerType::ParametersType         OptimizerParameterType;
    typedef OptimizerType::ScalesType             OptimizerScalesType;
    typedef OptimizerType::BoundSelectionType        OptimizerBoundSelectionType;
    typedef OptimizerType::BoundValueType          OptimizerBoundValueType;

    typedef InterpolatorType::Pointer            InterpolatorTypePointer;
    typedef RegistrationType::Pointer            RegistrationTypePointer;





    /** Typedef of the bulk transform. */
    /*
    typedef itk::VersorRigid3DTransform< double >     BulkTransformType;
    typedef BulkTransformType::Pointer           BulkTransformPointer;
     */
    typedef Transform<CoordinateRepType,itkGetStaticConstMacro(SpaceDimension),
            itkGetStaticConstMacro(SpaceDimension)> BulkTransformType;
    typedef BulkTransformType::ConstPointer  BulkTransformPointer;
#if 0
    /*
    void SetBulkTransform( BulkTransformPointer &BulkTransform )
    {
    m_Output->SetBulkTransform( BulkTransform );
    }
     */

    /** ImageDimension constants * /
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
    itkTypeMacro(AnatomicalBSplineFilter, itk::Object);

    /* SetInput and GetOutput Macros */
    itkSetObjectMacro (FixedImage, FixedImageType);
    itkSetObjectMacro (MovingImage, MovingImageType);
    itkSetConstObjectMacro (BulkTransform, BulkTransformType);
    itkGetObjectMacro (Output, TransformType);



    itkSetMacro (SpatialSampleScale, int);
    itkSetMacro (MaximumNumberOfIterations, int);
    itkSetMacro (MaximumNumberOfEvaluations, int);
    itkSetMacro (MaximumNumberOfCorrections, int);
    itkSetMacro (BSplineHistogramBins, int);
    itkSetMacro (GridSize, int);
    itkSetMacro (GridBorderSize, int);
    itkSetMacro (BaseImage, int);
    itkSetMacro (CostFunctionConvergenceFactor, float);
    itkSetMacro (ProjectedGradientTolerance, float);


    void Update();

protected:
    mimxRegisterBSplineFilter();
    ~mimxRegisterBSplineFilter() {};


private:
    mimxRegisterBSplineFilter(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented

    /*** Input and Output Objects ***/
    FixedImagePointer         m_FixedImage;
    MovingImagePointer        m_MovingImage;
    BulkTransformPointer      m_BulkTransform;
    TransformTypePointer      m_Output;


    // Parameters for the image registration
    int   m_SpatialSampleScale;
    int   m_MaximumNumberOfIterations;
    int   m_MaximumNumberOfEvaluations;
    int   m_MaximumNumberOfCorrections;
    int   m_BSplineHistogramBins;
    int   m_BaseImage;
    int   m_GridSize;
    int   m_GridBorderSize;
    float m_CostFunctionConvergenceFactor;
    float m_ProjectedGradientTolerance;


} ; // end of class

} // end namespace itk


#endif


