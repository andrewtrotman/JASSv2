./tools/Debug/unicodedata_to_utype ../external/Unicode/UnicodeData.txt  ../external/Unicode/PropList.txt  > ers.cpp 
gcc -c is_alpha.c -o is_alpha.o 
dmd is_alpha_list.d is_alpha.o 
./is_alpha_list  > ers

