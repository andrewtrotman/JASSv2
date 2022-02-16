import os
import subprocess
import shutil
import glob
import platform
import sys
import re

from pathlib import Path

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from packaging import version # following PEP 440 standards on performing version comparison 

#version_file = open(os.path.join(mypackage_root_dir, 'VERSION'))

class CMakeExtension(Extension):
    def __init__(self, name):
        Extension.__init__(self, name, sources=[])

class CMakeBuild(build_ext):
    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)
        super().run()

    def build_cmake(self, ext):
        try:
            temp = re.findall("\d+\.\d+", str(subprocess.check_output(["swig", "-version"])))
            if version.parse(temp[0]) < version.parse('4.0'):
              raise RuntimeError(
                  "Swig 4.0 or higher is required in order to build pyjass."
              )                  
        except OSError:
            raise RuntimeError(
                "Swig must be installed in order to build pyjass. Please run apt-get install swig or brew install swig"
            )        
        try:
            temp = re.findall("\d+\.\d+", str(subprocess.check_output(["cmake", "--version"])))
            if version.parse(temp[0]) < version.parse('3.7'):
              raise RuntimeError(
                  "CMake 3.8 or higher is required in order to build pyjass"
              )  
                          
        except OSError:
            raise RuntimeError(
                "CMake must be installed in order to build pyjass. Please run apt-get install cmake or brew install cmake"
                + ", ".join(e.name for e in self.extensions)
            )

        cwd = Path().absolute()

        # these dirs will be created in build_py, so if you don't have
        # any python sources to bundle, the dirs will be missing
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = [
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir
        ]

        build_args = ["--config", "Release", "--", "-j2"]

        self.announce("Running CMake prepare", level=3)
        subprocess.check_call(["cmake", cwd] + cmake_args, cwd=self.build_temp)

        self.announce("Building extensions")
        cmake_cmd = ["cmake", "--build", "."] + build_args
        subprocess.check_call(cmake_cmd, cwd=(self.build_temp))

        dir1 = self.build_temp + "/anytime/pyjass.py" # copy our swig script into the lib so it gets installed together
        # This is a workaround for MacOS X
        # clang (Xcode) expects some sort of file but in Linux it gets compiled anyway. We just copy the _so file to dummy and install it
        if platform.system() == "Darwin":
            fakeLib = extdir + "/dummy.cpython-" + str(sys.version_info[0]) + str(sys.version_info[1]) + "-darwin.so" #create a Darwin Version to handle pythons' cmake
            shutil.copy((extdir + "/_pyjass.so"), fakeLib)        
        shutil.copy(dir1, extdir)

setup(
    cmdclass=dict(build_ext=CMakeBuild),
    name='pyjass',
    version='0.2a7',
    author='Andrew Trotman',
    description='The JASS experimental Score-at-a-Time search engine.',
    long_description='Please see https://github.com/andrewtrotman/JASSv2 ',
    include_dirs =[''],
    author_email='andrew@cs.otago.ac.nz',
    download_url='',
    keywords='search engine, search tool, jass, SaaT',
    project_urls={  # Optional
    'Bug Reports': 'https://github.com/andrewtrotman/JASSv2/issues',
    'Source': 'https://github.com/andrewtrotman/JASSv2',
    },
#    packages=[''],
#    package_dir={'': '.'},
#   package_data={'': ['_example.so']}, #replace me with your package data
   classifiers=[
       'Programming Language :: Python :: 3',
       'Operating System :: MacOS :: MacOS X',
       'Operating System :: POSIX :: Linux', # WSL will be treated as Linux so it's not a problem
   ],
   python_requires='>=3.4',
   install_requires=['wheel>0.35','packaging>20','cmake>3.7.2'],
   ext_modules=[CMakeExtension("dummy")] # force python to generate a dummy.so/pyc bindings - cmake script takes care of that
)
