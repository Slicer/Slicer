#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Print diagnostic messages only if the environment variable is set
__attribute__((constructor)) static void stub_init(void)
{
  const char* debug = getenv("SLICER_DEBUG_KRB5_GSSAPI_STUB");
  if (debug && debug[0] != '\0')
  {
    fprintf(stderr,
            "[krb5-gssapi-stub] Loaded stub libgssapi_krb5.so.2\n"
            "[krb5-gssapi-stub] Stubbed symbol exported: GSS_C_NT_HOSTBASED_SERVICE (NULL)\n");
  }
}

// Minimal type definition for the GSSAPI OID struct
struct gss_OID_desc_struct {
    uint32_t length;
    void* elements;
};

typedef struct gss_OID_desc_struct gss_OID_desc;
typedef struct gss_OID_desc_struct* gss_OID;

// Stubbed symbol export: always NULL
__attribute__((visibility("default")))
const gss_OID_desc * const GSS_C_NT_HOSTBASED_SERVICE = 0;
