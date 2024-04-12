from setuptools.extension import Extension
from setuptools import setup, find_packages

ext_modules = list()

ext_modules.append(Extension("sortedintersect.sintersect",
                             ["sortedintersect/sintersect.pyx"],
                             language="c++",
                             ))


setup(version='0.4.0',
      name='sortedintersect',
      description="Interval intersection for sorted query and target intervals",
      long_description=open('README.md').read(),
      author="Kez Cleal",
      author_email="clealk@cardiff.ac.uk",
      packages=find_packages(),
      setup_requires=['cython'],
      install_requires=['cython'],
      ext_modules=ext_modules,
)