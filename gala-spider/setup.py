# coding: utf-8

import sys
from setuptools import setup, find_packages

NAME = "spider"
VERSION = "1.0.0"

# To install the library, run the following
#
# python setup.py install
#
# prerequisite: setuptools
# http://pypi.python.org/pypi/setuptools

REQUIRES = ["connexion"]

setup(
    name=NAME,
    version=VERSION,
    description="Topo Graph Engine Service",
    author_email="zhengxian@huawei.com",
    url="",
    keywords=["Swagger", "Topo Graph Engine Service"],
    install_requires=REQUIRES,
    packages=find_packages(),
    package_data={'': ['swagger/swagger.yaml']},
    include_package_data=True,
    entry_points={
        'console_scripts': ['spider=spider.__main__:main']},
    long_description="""\
    Topo Graph Engine Service
    """
)

