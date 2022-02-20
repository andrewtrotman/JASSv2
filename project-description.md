# PyJASS  
  
The JASS experimental search engine is still in development. pyJASS is the python bindings for JASS Search Engine


## Automated Installation

There are two approaches to automatically install pyJASS

### Anaconda/Conda 

Anaconda ensures that all of JASS dependices are met (including non-pythonic (CMake, doxygen, SWIG) and Python dependices (wheels)), so that pyJASS *just works*. 
In order to install via anaconda, first clone the git repo and then run the following command

```
conda env create -f env.yml
conda activate pyjass
```

### pip
Alternatively you can install via pip. However, pip does not install non-Python dependices for you (such as CMake and SWIG) and it is your responsibility to ensure
that they are installed (it does check and inform you if any of the dependices aren't met) 

```
pip install --user pyjass
```


## How to Use
You can then start Python and import pyjass thus:  
  
```  
python3  
import pyjass  
```  

Create a JASS anytime object, load an index, and call methods on that object:  
  
```  
index = pyjass.anytime()  
index.load_index(2)  
get_document_count()  
```  
  
To search, call the search method:  
  
```  
got = index.search("Something")  
```  
  

A full example of a Python program that loads an index (from the current directory), does a search, and prints the results:  
  
```  
import pyjass  
index = pyjass.anytime()  
ok = index.load_index(2)  
print("Compressed using:", index.get_encoding_scheme_name())  
print("D-ness:", index.get_encoding_scheme_d())  
print("Documents:", index.get_document_count())  
print("")  
  
results = index.search("one")  
print("ID:", results.query_id)  
print("query:", results.query)  
print("Postings Processed:", results.postings_processed)  
print("Time (ns):", results.search_time_in_ns)  
print("Results:")  
print(results.results_list)  
```

For complete documentation, please refer to [our repo](https://github.com/andrewtrotman/JASSv2).
