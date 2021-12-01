import os
import subprocess
import shutil
import glob

from pathlib import Path

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

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
            subprocess.check_output(["cmake", "--version"])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: "
                + ", ".join(e.name for e in self.extensions)
            )

        cwd = Path().absolute()

        # these dirs will be created in build_py, so if you don't have
        # any python sources to bundle, the dirs will be missing
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        print("EXT DIR IS" + extdir)

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",

        ]

        build_args = ["--config", "Release", "--", "-j2"]

#        env = os.environ.copy()

        self.announce("Running CMake prepare", level=3)
        subprocess.check_call(["cmake", cwd] + cmake_args, cwd=self.build_temp)

        self.announce("Building extensions")
        cmake_cmd = ["cmake", "--build", "."] + build_args
        subprocess.check_call(cmake_cmd, cwd=(self.build_temp))
        dir1 = self.build_temp + "/anytime/pyjass.py" # copy our swig script into the lib so it gets installed together 
        dir2 = extdir + "/pyjass.cpython*" # remove the default dummy.so that causes the program not work 
        shutil.move(dir1, extdir)
        for f in glob.glob(dir2):
            os.remove(f)

setup(
    name='pyjass',
    version='0.1',
    author='Andrew Trotman',
    include_dirs =[''],
    author_email='andrew@cs.otago.ac.nz',
    download_url='',
#    packages=[''],
#    package_dir={'': '.'},
#   package_data={'': ['_example.so']}, #replace me with your package data
   classifiers=[
       'Programming Language :: Python :: 3',
       'Operating System :: MacOS :: MacOS X',
       'Operating System :: POSIX :: Linux', # WSL will be treated as Linux so it's not a problem
   ],
   python_requires='>=3.0',
   ext_modules=[CMakeExtension("dummy")], # force python to generate a dummy.so/pyc bindings - cmake script takes care of that
   cmdclass=dict(build_ext=CMakeBuild)
)
