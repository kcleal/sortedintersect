from setuptools.extension import Extension
from setuptools import setup, find_packages

ext_modules = list()

ext_modules.append(Extension("sortedintersect.intersect",
                             ["sortedintersect/intersect.pyx"],
                             # extra_compile_args=extras,
                             # define_macros=[("NPY_NO_DEPRECATED_API", "NPY_1_7_API_VERSION")],
                             language="c++",
                             ))


setup(version='0.1.0',
      name='sortedintersect',
      description="Interval intersection for sorted query and target intervals",
      long_description=open('README.rst').read(),
      author="Kez Cleal",
      author_email="clealk@cardiff.ac.uk",
      packages=find_packages(),
      setup_requires=['cython'],
      install_requires=['cython'],
      ext_modules=ext_modules,
      test_suite='nose.collector',
      tests_require='nose',
)