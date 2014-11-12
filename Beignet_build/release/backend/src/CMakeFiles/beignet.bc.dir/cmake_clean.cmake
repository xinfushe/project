FILE(REMOVE_RECURSE
  "ocl_vector.h"
  "usr/local/lib/beignet/ocl_stdlib.h"
  "CMakeFiles/beignet.bc"
  "beignet.bc"
  "ocl_barrier.ll.bc"
  "ocl_memset.ll.bc"
  "ocl_memcpy.ll.bc"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/beignet.bc.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
