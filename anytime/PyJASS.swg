%module pyjass
%{
#include "JASS_anytime_api.h"
%}
%include "stdint.i"
%include "std_map.i"
%include "std_string.i"
%include "std_vector.i"
%template(vector_str) std::vector<std::string>;
%include "JASS_anytime_api.h"
%include "JASS_anytime_result.h"
%include "JASS_anytime_thread_result.h"
