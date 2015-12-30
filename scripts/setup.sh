# external libs
export PROTOBUF_HOME=/uboone/app/users/tmw/projects/supera/protobuf
export LMDB_HOME=/uboone/app/users/tmw/projects/supera/lmdb/libraries/liblmdb
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PROTOBUF_HOME}/lib:${LMDB_HOME}
export PATH=${PATH}:${PROTOBUF_HOME}/bin

# setup uboone
ubooneme
source /uboone/app/users/tmw/projects/supera/larsoft/localProducts_larsoft_v04_31_00_e9_prof/setup
mrbsetenv



