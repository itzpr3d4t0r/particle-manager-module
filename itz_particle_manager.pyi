from typing import Sequence, Union, Tuple, List, overload

import pygame

Coord = Union[Sequence[float], Sequence[int]]
FloatOrRange = Union[float, Sequence[float]]

EMIT_POINT: int = 0

class Emitter:
    def __init__(self, emit_type: int, /, **kwargs) -> None: ...
    @overload
    def __init__(
        self,
        emit_shape: int,
        emit_number: int = 1,
        looping: bool = False,
        emit_interval: float = 0,
        emit_time: float = 0,
        animation: Tuple[pygame.Surface, ...] = None,
        particle_lifetime: FloatOrRange = 60,
        speed_x: FloatOrRange = 0,
        speed_y: FloatOrRange = 0,
        acceleration_x: FloatOrRange = 0,
        acceleration_y: FloatOrRange = 0,
        angle: FloatOrRange = 0,
        align_speed_to_angle: bool = False,
        align_acceleration_to_angle: bool = False,
        blend_mode: int = pygame.BLEND_ADD,
    ) -> None: ...

class ParticleEffect:
    def __init__(self, emitters: Tuple[Emitter]) -> None: ...

class ParticleManager:
    @property
    def num_particles(self) -> int: ...
    def __init__(self) -> None: ...
    def spawn_effect(
        self, effect: ParticleEffect, position: Sequence[float]
    ) -> None: ...
    def update(self, dt: float) -> None: ...
    def draw(self, surf: pygame.Surface) -> None: ...
