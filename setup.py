from setuptools import setup, Extension

module = Extension(
    "itz_particle_manager",
    sources=["src/module.c"],
)

setup(
    name="itz_particle_manager",
    version="0.1-alpha",
    description="ItzPr4d4t0r's Particle Manager module",
    ext_modules=[module],
)
