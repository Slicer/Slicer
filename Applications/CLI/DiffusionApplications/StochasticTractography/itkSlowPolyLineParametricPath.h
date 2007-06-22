#ifndef _itkSlowPolyLineParametricPathPath_h
#define _itkSlowPolyLineParametricPathPath_h

#include "itkPolyLineParametricPath.h"
#include "itkVectorContainer.h"
#include "itkContinuousIndex.h"
#include "itkIndex.h"
#include "itkOffset.h"
#include "itkVector.h"

namespace itk
{


/** \class SlowPolyLineParametricPath
 * \brief  Represent a path of line segments through ND Space
 *
 * This class is intended to represent parametric paths through an image, where
 * the paths are composed of line segments.  Each line segment traverses one
 * unit of input.  A classic application of this class is the representation of
 * contours in 2D images, especially when the contours only need to be
 * approximately correct.  Another use of a path is to guide the movement of an
 * iterator through an image.
 *
 * \sa EllipseParametricPath
 * \sa FourierSeriesPath
 * \sa OrthogonallyCorrectedParametricPath
 * \sa ParametricPath
 * \sa ChainCodePath
 * \sa Path
 * \sa ContinuousIndex
 * \sa Index
 * \sa Offset
 * \sa Vector
 *
 * \ingroup PathObjects
 */
template <unsigned int VDimension>
class ITK_EXPORT SlowPolyLineParametricPath : public
PolyLineParametricPath< VDimension >
{
public:
  /** Standard class typedefs. */
  typedef SlowPolyLineParametricPath      Self;
  typedef PolyLineParametricPath<VDimension>  Superclass;
  typedef SmartPointer<Self>          Pointer;
  typedef SmartPointer<const Self>    ConstPointer;
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(SlowPolyLineParametricPath, PolyLineParametricPath);
  
  /** Input type */
  typedef typename Superclass::InputType  InputType;
  
  /** Output type */
  typedef typename Superclass::OutputType OutputType;
  
  
  /** Basic data-structure types used */
  typedef typename Superclass::ContinuousIndexType    ContinuousIndexType;           
  typedef typename Superclass::IndexType                  IndexType;                     
  typedef typename Superclass::OffsetType                  OffsetType;                    
  typedef typename Superclass::PointType              PointType;                    
  typedef typename Superclass::VectorType             VectorType;                    
  typedef typename Superclass::VertexType                   VertexType;                    
  typedef typename Superclass::VertexListType VertexListType;
  typedef typename Superclass::VertexListPointer      VertexListPointer;

  /** Increment the input variable passed by reference such that the ND index of
   * the path  moves to its next vertex-connected (8-connected in 2D) neighbor. 
   * Return the Index-space offset of the path from its prior input to its new
   * input.  If the path is unable to increment, input is not changed and an
   * offset of Zero is returned. Children are not required to implement bounds
   * checking.
   *
   * This is a fairly slow, iterative algorithm that numerically converges to
   * the next index along the path, in a vertex-connected (8-connected in 2D)
   * fashion.  When possible, children of this class should overload this
   * function with something more efficient.
   *
   * WARNING:  This default implementation REQUIRES that the ND endpoint of
   * the path be either unique or coincident only with the startpoint, since it
   * uses the endpoint as a stopping condition. */
  virtual OffsetType IncrementInput(InputType & input) const;

  
  ///** Evaluate the first derivative of the ND output with respect to the 1D
  //  * input.  This is an exact, algebraic function. */
  //virtual VectorType EvaluateDerivative(const InputType & input) const;
  
  
  /** New() method for dynamic construction */
  itkNewMacro( Self );

  
protected:
  SlowPolyLineParametricPath();
  ~SlowPolyLineParametricPath(){}
  void PrintSelf(std::ostream& os, Indent indent) const;
  
private:
  SlowPolyLineParametricPath(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSlowPolyLineParametricPath.txx"
#endif

#endif
