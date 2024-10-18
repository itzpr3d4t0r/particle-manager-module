from typing import Sequence, Union, Tuple, List, overload

import pygame

Coord = Union[Sequence[float], Sequence[int]]
FloatOrRange = Union[float, Sequence[float]]

AcceptableImageSequences = Union[
    pygame.Surface,  # particles all have the same one surface
    Sequence[pygame.Surface],  # particles all share the same animation
    Sequence[Sequence[pygame.Surface]],  # particles all have a random animation
]

SPAWN_POINT: int = 0

class Emitter:
    def __init__(self, emit_type: int, /, **kwargs) -> None: ...
    @overload
    def __init__(
        self,
        emit_type: int,
        emit_number: int = 1,
        looping: bool = False,
        emit_interval: float = 0,
        emit_time: float = 0,
        images: AcceptableImageSequences = None,
        particle_lifetime: FloatOrRange = 100,
        speed_x: FloatOrRange = 0,
        speed_y: FloatOrRange = 0,
        acceleration_x: FloatOrRange = 0,
        acceleration_y: FloatOrRange = 0,
        angle: FloatOrRange = 0,
        align_speed_to_angle: bool = False,
        align_acceleration_to_angle: bool = False,
    ) -> None: ...

class ParticleEffect:
    def __init__(self, emitters: List[Emitter]) -> None: ...

class ParticleManager:
    num_particles: int
    num_groups: int
    groups: List[Tuple[List[Tuple[pygame.Surface, Coord]], int]]

    def __init__(self) -> None: ...
    def add_group(self, blend_flag: int, spawn_type: int, *args) -> None: ...
    def update(self, dt: float) -> None: ...
    def draw(self, surface: pygame.Surface) -> None: ...
