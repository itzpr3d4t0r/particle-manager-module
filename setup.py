from setuptools import setup, Extension

module = Extension(
    "particle_manager",
    sources=["src/particle_manager.c"],
)

setup(
    name="particle_manager",
    version="0.1-alpha",
    description="Particle Manager Module",
    ext_modules=[module],
)
