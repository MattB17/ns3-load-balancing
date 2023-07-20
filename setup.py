from setuptools import setup, find_packages
from os import path
from io import open

# Get current directory.
current_dire = path.abspath(path.dirname(__file__))

# The setup.
setup(
  name="LoadBalancingAnalysis",
  version="0.1.0",
  description="Load Balancing Frameworks and Experiments in NS3",
  author="Matt Buckley",
  packages=find_packages(exclude=['contrib', 'docs', 'tests'])
)