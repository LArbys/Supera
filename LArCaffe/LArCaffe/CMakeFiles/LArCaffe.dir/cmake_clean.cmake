file(REMOVE_RECURSE
  "../lib/libLArCaffe.rootmap"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/LArCaffe.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
