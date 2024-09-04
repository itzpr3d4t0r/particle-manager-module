from typing import Sequence, Union, List, Tuple

import pygame

Coord = Union[Sequence[float], Sequence[int]]

"""
This flag makes ParticleManager.add_group() expect the following args (ordered and positional):

number,
pos,
images,
vx=(min=0, max=0, randomize_x=False),
vy=(min=0, max=0, randomize_y=False),
gravity=(x, y)

"""
SPAWN_POINT: int = 0

class ParticleManager:
    num_particles: int
    num_groups: int
    # groups: List[Tuple[List[Tuple[pygame.Surface, Coord]], int]]
    def __init__(self) -> None: ...
    # def add_group(self, blend_flag: int, spawn_type: int, *args) -> None: ...
    # def remove_group(self, group_index: int) -> None: ...
    # def update(self, dt: float) -> None: ...

def rand_point_in_circle(x: float, y: float, r: float) -> Tuple[float, float]: ...
