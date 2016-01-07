file(REMOVE_RECURSE
  "../../SuperaCore/lib/libSuperaCore.rootmap"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/SuperaCore.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
