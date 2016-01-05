//
// cint script to generate libraries
// Declaire namespace & classes you defined
// #pragma statement: order matters! Google it ;)
//

#ifdef __CINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ namespace larcaffe+;
#pragma link C++ namespace larcaffe::supera+;
#pragma link C++ class larcaffe::supera::logger+;
#pragma link C++ class larcaffe::supera::supera_base+;
#pragma link C++ class larcaffe::supera::converter_base+;
#pragma link C++ class larcaffe::supera::lmdb_converter+;
//ADD_NEW_CLASS ... do not change this line
#endif




