import os
import re
import sys
import sysconfig
import platform
import subprocess
import shutil

from pathlib import Path

from distutils.version import LooseVersion
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
from setuptools.command.test import test as TestCommand

class CMakeExtension(Extension):
    def __init__(self, name):
        Extension.__init__(self, name, sources=[])


class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: " +
                ", ".join(e.name for e in self.extensions))

        build_directory = os.path.abspath(self.build_temp)
        build_directory2 = os.path.abspath(os.path.dirname(__file__))
        print("NUILD DIR 2" + build_directory2)

        cmake_args = [
            # '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + build_directory
            # '-DPYTHON_EXECUTABLE=' + sys.executable
        ]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

       # cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg] # include the args to make the build 

        # Assuming Makefiles
        # build_args += ['--', '-j2']

        self.build_args = build_args

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(
            env.get('CXXFLAGS', ''),
            self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        # CMakeLists.txt is in the same directory as this setup.py file
        cmake_list_dir = os.path.abspath(os.path.dirname(__file__))
       # print(self.build_temp)
        print('-'*10, 'Running CMake prepare', '-'*40)
        subprocess.check_call(['./build.sh'])
        # subprocess.check_call(['make'])


        # print('-'*10, 'Building extensions', '-'*40)
        # cmake_cmd = ['make', '', '.'] + self.build_args
        # subprocess.check_call(cmake_cmd)

        # Move from build temp to final position
    #     for ext in self.extensions: 
    #         print(ext)
    #         self.move_output(ext)

    # def move_output(self, ext):
    #     build_temp = Path(os.path.abspath(os.path.dirname(__file__))).resolve()
    #     print(build_temp)
    #     print(type(ext))
    #     dest_path = Path(self.get_ext_fullpath(ext.name)).resolve()
    #     print(dest_path)
    #     source_path = build_temp / self.get_ext_filename(ext.name)
    #     dest_directory = dest_path.parents[0]
    #     dest_directory.mkdir(parents=True, exist_ok=True)
    #     self.copy_file(source_path, dest_path)
        
        
ext_modules = [
  CMakeExtension('_pyjass.so')
]

setup(
    name='PACKAGENAME',
    version='PACKAGEVERSION',  # specified elsewhere
    author='',
    author_email='',
    download_url='',
    packages=['output'],
   # package_dir={'': '.'},
   package_data={'': ['_example.so']}, #replace me with your package data
   classifiers=[
       'Programming Language :: Python :: 3',
       'Operating System :: MacOS :: MacOS X',
       'Operating System :: POSIX :: Linux', # WSL will be treated as Linux so it's not a problem
   ],
   python_requires='>=3.6',
   ext_modules=ext_modules,
   cmdclass=dict(build_ext=CMakeBuild)

)
