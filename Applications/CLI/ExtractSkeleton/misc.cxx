/*
 * misc.cc -- miscellaneous utitlity functions
 * 
 * author:  msturm 
 * created: 27 Mar 1997
 * changes: mastyner
 */


#include "misc.h"
#include "string.h"

void* ipAllocateData(const int size, const size_t elemsize)
{
  void *data = NULL;

  if (!(data = malloc(size * elemsize))) {
    fprintf(stderr,"Error: ipAllocateData [%s, line %d]: memory allocation failed:",
            __FILE__, __LINE__);
    perror("");
    exit(errno);
  }
  
  memset(data, 0, size * elemsize);

  return(data);
}


size_t ipGetDataSize(const ipDataType type)
{
  size_t retval;

  switch (type) {
  case IP_BYTE:
    retval = sizeof(char);
    break;
  case IP_SHORT:
    retval = sizeof(short);
    break;
  case IP_INT:
    retval = sizeof(int);
    break;
  case IP_FLOAT:
    retval = sizeof(float);
    break;
  case IP_DOUBLE:
    retval = sizeof(double);
    break;
  default:
#ifdef DEBUG_VSKEL
    fprintf(stderr,"Warning: ipGetDataSize [%s, line %d]: unsuported data type: %d\n",
            __FILE__, __LINE__, type);
#endif

    retval = 0;
    break;
  }

  return retval;
  
}
