# Itz Particle Manager Module
[![Python Code Quality](https://github.com/itzpr3d4t0r/particle-manager-module/actions/workflows/black.yml/badge.svg)](https://github.com/itzpr3d4t0r/pygame_geometry/actions/workflows/black.yml)
[![C++ Code Quality](https://github.com/itzpr3d4t0r/particle-manager-module/actions/workflows/cppcheck.yml/badge.svg)](https://github.com/itzpr3d4t0r/particle-manager-module/actions/workflows/cppcheck.yml)
[![Ubuntu latest](https://github.com/itzpr3d4t0r/particle-manager-module/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/itzpr3d4t0r/particle-manager-module/actions/workflows/ubuntu.yml)
[![Windows latest](https://github.com/itzpr3d4t0r/particle-manager-module/actions/workflows/windows.yml/badge.svg)](https://github.com/itzpr3d4t0r/particle-manager-module/actions/workflows/windows.yml)
[![MacOS latest](https://github.com/itzpr3d4t0r/particle-manager-module/actions/workflows/macos.yml/badge.svg)](https://github.com/itzpr3d4t0r/particle-manager-module/actions/workflows/macos.yml)
![Commits per week](https://img.shields.io/github/commit-activity/w/itzpr3d4t0r/particle-manager-module/master)
[![Code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)

This CPython module aims to deliver a fast particle manager by avoiding having to deal with individual particles in python.

Some important notes:

- This module requires the `pygame` module to work correctly.  If you don't have it installed you can do so by running `pip install pygame-ce`.
- This module isn't ready to cover all use cases yet, but it should be enough for most simple particle systems. If you have any suggestions or issues, feel free to open an issue or a pull request.

# Usage

Creating and running a particle manager is really simple, but it does require a bit of setup:

1. Create a `ParticleManager` object. This object will be responsible for adding, updating, and drawing the particles.
2. Create a group of particles. A group is a collection of particles that share the same properties.
3. Update the particle manager. This will update all the particles in the manager.
4. Draw the particles to a surface calling `pm.draw(surface)`.

In this example, we will suppose `surface` to be the screen surface and make a particle
manager that spawns particles from a point:

```Python
import pygame
from itz_particle_manager import ParticleManager, SPAWN_POINT
from pygame import Surface

# Create a surface to draw the particles on
surface = Surface((1000, 1000))

# Create a list of progressively smaller white squares to use as images for the particles
img_sequences = [[pygame.Surface((s, s)) for s in range(5, 1, -1)]]
for sequence in img_sequences:
    for img in sequence:
        img.fill("white")

# Create a particle manager
particle_manager = ParticleManager()

# Add a group of particles to the particle manager
particle_manager.add_group(
    pygame.BLEND_ADD,  # blend mode
    SPAWN_POINT,       # spawn type (SPAWN_POINT: spawn from a point)
    100,               # number of particles
    (500, 500),        # spawn pos
    img_sequences,     # image sequences to take animations from
    (-2, 2),           # x velocity info
    (-2, 2),           # y velocity info
)

# Update the particle manager
particle_manager.update(1)  # 1 is the delta time, in a real game loop you would use the
                            # time since the last frame instead of this hardcoded value

# Draw the particles to the surface
particle_manager.draw(surface)

```


# Installation
Once you navigate to the project's directory you can:

### Windows 10 / Windows 11
1. Install Python 3.8+
2. Install [Visual Studio Community 2022](https://visualstudio.microsoft.com/vs/community/) or [Visual Studio Build Tools 2017](https://aka.ms/vs/15/release/vs_buildtools.exe) and make sure you mark `MSVC v140 - VS 2015 C++ build tools (v14.00)` with the installation
3. Run `python -m pip install setuptools -U` (If on 3.12 or greater make sure to have the `wheel` package installed)
4. Install the latest version of [git](https://gitforwindows.org/)
5. Run `git clone https://github.com/itzpr3d4t0r/particle-manager-module.git`
6. Run `py -m pip install .`
