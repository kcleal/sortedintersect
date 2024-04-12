from setuptools.extension import Extension
from setuptools import setup, find_packages

ext_modules = list()

ext_modules.append(Extension("sortedintersect.sintersect",
                             ["sortedintersect/sintersect.pyx"],
                             language="c++",
                             extra_compile_args=["-std=c++17"]
                             ))


setup(version='0.4.1',
      name='sortedintersect',
      description="Interval intersection for sorted query and target intervals",
      author="Kez Cleal",
      author_email="clealk@cardiff.ac.uk",
      packages=find_packages(),
      setup_requires=['cython'],
      install_requires=['cython'],
      ext_modules=ext_modules,
)