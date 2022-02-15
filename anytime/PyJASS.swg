%module pyjass
%{
#include "JASS_anytime_api.h"
#include <memory>
%}
%include "stdint.i"
%include "std_map.i"
%include <std_string.i>
%include <std_vector.i>
%include <std_shared_ptr.i>
namespace std {
   %template(IntVector) vector<int>;
   %template(DoubleVector) vector<double>;
   %template(StringVector) vector<string>;
   %template(ConstCharVector) vector<const char*>;
}
%shared_ptr(std::vector<std::string>)
%include "JASS_anytime_api.h"
%include "JASS_anytime_result.h"
%include "JASS_anytime_thread_result.h"

%template(string_result_map) std::map<std::string, JASS_anytime_result>;
%template(result_vector) std::vector< JASS_anytime_thread_result >;
%template(string_vector) std::vector< std::string >;
