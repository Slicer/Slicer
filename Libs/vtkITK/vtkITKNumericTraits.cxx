
namespace itk
{

#if defined(VTK_TYPE_USE___INT64)
const __int64 NumericTraits<__int64>::ZeroValue() = 0;
const __int64 NumericTraits<__int64>::OneValue() = 1;

const unsigned __int64 NumericTraits<unsigned __int64>::ZeroValue() = 0ui64;
const unsigned __int64 NumericTraits<unsigned __int64>::OneValue() = 1ui64;
#endif

};
