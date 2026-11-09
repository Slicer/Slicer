/*=========================================================================

  Program:   3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

/// \brief ITK transforms that store the parameters of model-based transforms
///
/// A model-based transform (vtkMeshDisplacementTransform or vtkScatteredBSplineTransform) is computed from
/// displacements at the points of the mesh of a model. Its file holds only its parameters, in the fixed parameters,
/// and the key of the source of its input point set (InputPointSetSourceKey), which identifies the model: the
/// transform is computed again from that model when the file is read.
/// These classes cannot compute any transformation, they are only used for reading and writing the parameters.
/// Other ITK-based applications cannot use them: a model-based transform can be exported to a displacement field for
/// that.
///
/// The Inverse... classes store the inverse of the transform, as the inverse classes of vtkITKTransformInverse.h do.
///
/// The layout of the fixed parameters is described in the "Model-based transform files" section of the developer guide
/// page of the Transforms module (Docs/developer_guide/modules/transforms.md).
///

#ifndef __vtkITKModelBasedTransform_h
#define __vtkITKModelBasedTransform_h

#include <itkTransform.h>

namespace itk
{

//----------------------------------------------------------------------------
template <typename TParametersValueType, unsigned int NDimensions>
class ModelBasedTransformParameters : public Transform<TParametersValueType, NDimensions, NDimensions>
{
public:
  /** Standard class typedefs. */
  typedef ModelBasedTransformParameters Self;
  typedef Transform<TParametersValueType, NDimensions, NDimensions> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(ModelBasedTransformParameters, Transform);

  void SetParameters(const typename Superclass::ParametersType& parameters) override
  {
    this->m_Parameters = parameters;
    this->Modified();
  }
  void SetFixedParameters(const typename Superclass::FixedParametersType& fixedParameters) override
  {
    this->m_FixedParameters = fixedParameters;
    this->Modified();
  }

  /** Define all computation methods as unimplemented to make sure they are not used */
  using Superclass::TransformPoint;
  typename Superclass::OutputPointType TransformPoint(const typename Superclass::InputPointType&) const override
  {
    itkExceptionMacro("Only storage methods are implemented for " << this->GetNameOfClass());
  }
  void ComputeJacobianWithRespectToParameters(const typename Superclass::InputPointType&, typename Superclass::JacobianType&) const override
  {
    itkExceptionMacro("Only storage methods are implemented for " << this->GetNameOfClass());
  }
};

#define vtkITKModelBasedTransformClassMacro(name)                                        \
  template <typename TParametersValueType, unsigned int NDimensions>                     \
  class name : public ModelBasedTransformParameters<TParametersValueType, NDimensions>   \
  {                                                                                      \
  public:                                                                                \
    typedef name Self;                                                                   \
    typedef ModelBasedTransformParameters<TParametersValueType, NDimensions> Superclass; \
    typedef SmartPointer<Self> Pointer;                                                  \
    typedef SmartPointer<const Self> ConstPointer;                                       \
    itkTypeMacro(name, ModelBasedTransformParameters);                                   \
    itkNewMacro(Self);                                                                   \
  };

vtkITKModelBasedTransformClassMacro(MeshDisplacementTransform);
vtkITKModelBasedTransformClassMacro(InverseMeshDisplacementTransform);
vtkITKModelBasedTransformClassMacro(ScatteredBSplineTransform);
vtkITKModelBasedTransformClassMacro(InverseScatteredBSplineTransform);

#undef vtkITKModelBasedTransformClassMacro

} // namespace itk

typedef itk::MeshDisplacementTransform<double, 3> MeshDisplacementTransformDoubleType;
typedef itk::InverseMeshDisplacementTransform<double, 3> InverseMeshDisplacementTransformDoubleType;
typedef itk::ScatteredBSplineTransform<double, 3> ScatteredBSplineTransformDoubleType;
typedef itk::InverseScatteredBSplineTransform<double, 3> InverseScatteredBSplineTransformDoubleType;

#endif
