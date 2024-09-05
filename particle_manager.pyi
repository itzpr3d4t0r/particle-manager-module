from typing import Sequence, Union, Tuple, List

import pygame

Coord = Union[Sequence[float], Sequence[int]]

"""
This flag makes ParticleManager.add_group() expect the following args (ordered and positional):

number,
pos,
images,
vx = (min=0, max=0, random=False),
vy = (min=0, max=0, random=False),
accx = (min=0, max=0, random=False),
accy = (min=0, max=0, random=False),
update_speed = (min=1, max=1, random=False),
start_time = (min=1, max=1, random=False),
gravity

"""
SPAWN_POINT: int = 0

class ParticleManager:
    num_particles: int
    num_groups: int
    groups: List[Tuple[List[Tuple[pygame.Surface, Coord]], int]]

    def __init__(self) -> None: ...
    def add_group(self, blend_flag: int, spawn_type: int, *args) -> None: ...
    def update(self, dt: float) -> None: ...

def rand_point_in_circle(x: float, y: float, r: float) -> Tuple[float, float]: ...
