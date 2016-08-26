./tools/Debug/unicodedata_to_utype ../external/Unicode/UnicodeData_v6_2.txt  ../external/Unicode/PropList_v6_2.txt  > unicode_utype_v6_2.cpp
g++ -c unicode_utype_v6_2.cpp -o unicode_utype_v6_2.o
dmd cmp_d_to_utype.d unicode_utype_v6_2.o
./cmp_d_to_utype > ers

