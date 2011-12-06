
#define TEST_ASSERT(condition)                            \
  if( !(condition) )                                        \
    {                                                     \
    std::cerr << "Assertion failed in " << __FILE__       \
              << " at " << __LINE__ << std::endl;                    \
    std::cerr << #condition << std::endl;                 \
    return EXIT_FAILURE;                                  \
    }

#define TEST_ASSERT_EQUALS_VEC_3(vec, x, y, z)            \
  if( (vec)[0] != (x) ||                                   \
      (vec)[1] != (y) ||                                   \
      (vec)[2] != (z) )                                     \
    {                                                     \
    std::cerr << "Assertion failed in " << __FILE__       \
              << " at " << __LINE__ << std::endl;                    \
    std::cerr << #vec << " was " << (vec) << std::endl;    \
    std::cerr << "expected: " << (x) << ", "              \
              << (y) << ", " << (z) << std::endl;                    \
    return EXIT_FAILURE;                                  \
    }
