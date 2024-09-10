from typing import Sequence, Union, Tuple, List

import pygame

Coord = Union[Sequence[float], Sequence[int]]

"""
This flag makes ParticleManager.add_group() expect the following args (ordered and positional):

number,
pos,
images,
vx = float or (min=0, max=0),
vy = float or (min=0, max=0),
accx = float or (min=0, max=0),
accy = float or (min=0, max=0),
update_speed = float or (min=1, max=1),
start_time = float or (min=1, max=1),
gravity = (x, y),

"""
SPAWN_POINT: int = 0

class ParticleManager:
    num_particles: int
    num_groups: int
    groups: List[Tuple[List[Tuple[pygame.Surface, Coord]], int]]

    def __init__(self) -> None: ...
    def add_group(self, blend_flag: int, spawn_type: int, *args) -> None: ...
    def update(self, dt: float) -> None: ...
    def draw(self, surface: pygame.Surface) -> None: ...
