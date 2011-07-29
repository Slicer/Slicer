
namespace itk
{

#if defined(VTK_TYPE_USE___INT64)
const __int64 NumericTraits<__int64>::Zero = 0;
const __int64 NumericTraits<__int64>::One = 1;

const unsigned __int64 NumericTraits<unsigned __int64>::Zero = 0ui64;
const unsigned __int64 NumericTraits<unsigned __int64>::One = 1ui64;
#endif

#if ITK_VERSION_MAJOR < 4
#if defined(VTK_TYPE_USE_LONG_LONG) && !defined(ITK_TYPE_USE_LONG_LONG)
const long long NumericTraits<long long>::Zero = 0;
const long long NumericTraits<long long>::One = 1;

const unsigned long long NumericTraits<unsigned long long>::Zero = 0;
const unsigned long long NumericTraits<unsigned long long>::One = 1;
#endif
#endif

};
