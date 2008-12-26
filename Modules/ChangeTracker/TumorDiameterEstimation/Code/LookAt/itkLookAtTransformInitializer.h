#ifndef __itkLookAtTransformInitializer_h
#define __itkLookAtTransformInitializer_h

#include "itkObject.h"
#include "itkObjectFactory.h"

#include <iostream>

namespace itk
{

/** \class LookAtTransformInitializer
 * \brief LookAtTransformInitializer is a helper class intended to
 * initialize the transform to look at a specified point in an image.
 *
 * This class allows the user to specify a transform to center an object.
 * The object is specified via a center point, a direction to "look",
 * and an "up" vector. This process basically orients a centered plane in
 * 3-D image. The "up" vector specifies the direction to be considered as
 * the y-axis after the transformation. A warning is raised if the up vector
 * is the same as the direction vector.
 *
 * This class can be used to specify a transform suitable for extracting an
 * oblique section (see: itkObliqueSectionImageFilter).
 *
 * See the OpenGL gluLookAt(..) function for more details:
 *  http://pyopengl.sourceforge.net/documentation/manual/gluLookAt.3G.html
 *  http://developer.apple.com/documentation/Darwin/Reference/ManPages/man3/gluLookAt.3.html
 *
 * \author Dan Mueller, Queensland University of Technology, dan.muel[at]gmail.com
 *
 * \ingroup Transforms
 * \sa itkObliqueSectionImageFilter
 */
template < class TTransform, class TImage >
class ITK_EXPORT LookAtTransformInitializer : public Object
{
public:
  /** Standard class typedefs. */
  typedef LookAtTransformInitializer Self;
  typedef Object                     Superclass;
  typedef SmartPointer<Self>         Pointer;
  typedef SmartPointer<const Self>   ConstPointer;
  
  /** New macro for creation of through a Smart Pointer. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( LookAtTransformInitializer, Object );

  /** Type of the transform to initialize */
  typedef TTransform                      TransformType;
  typedef typename TransformType::Pointer TransformPointer;

  /** Image type to use in the initialization of the transform */
  typedef TImage                          ImageType;
  typedef typename TImage::Pointer        ImagePointer;
  typedef typename TImage::ConstPointer   ImageConstPointer;

  /** Dimension of parameters. */
  itkStaticConstMacro(SpaceDimension, unsigned int, 
                      TransformType::SpaceDimension);

  /** Point and index types. */
  typedef typename TransformType::InputPointType PointType;
  typedef ContinuousIndex< typename PointType::ValueType, SpaceDimension >
    ContinuousIndexType;
  
  /** Vector type. */
  typedef typename TransformType::InputVectorType  VectorType;

  /** Size type. */
  typedef typename ImageType::SizeType SizeType;

  /** Set the transform to be initialized. */
  itkSetObjectMacro( Transform,   TransformType );

  /** Set the image on which the transform is to be applied. */
  itkSetConstObjectMacro( Image,  ImageType );

  /** Initialize the transform using data from the images */
  virtual void InitializeTransform();
  
  /** Set the plane using a center, direction to "look", and
   *  intended plane size. The "up" vector defaults to the y-axis [0,1,0]. */
  virtual void SetPlane( PointType center,
                         VectorType direction,
                         SizeType size )
  {
    m_Center = center;
    m_Direction = direction;
    m_Size = size;
    this->Modified( );
  }

  /** Set the plane using a center, direction to "look",
   *  "up" vector, and intended plane size. */
  virtual void SetPlane( PointType  center,
                         VectorType direction,
                         VectorType up,
                         SizeType size )
  {
    m_Center = center;
    m_Direction = direction;
    m_Up = up;
    m_Size = size;
    this->Modified( );
  }

protected:
  LookAtTransformInitializer();
  ~LookAtTransformInitializer(){};

  void PrintSelf(std::ostream &os, Indent indent) const;

private:
  LookAtTransformInitializer(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  TransformPointer    m_Transform;
  ImageConstPointer   m_Image;
  
  PointType  m_Center;
  VectorType m_Direction;
  VectorType m_Up;
  SizeType   m_Size;

};

}  // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLookAtTransformInitializer.txx"
#endif

#endif
