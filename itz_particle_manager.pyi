from typing import Sequence, Union, Tuple, overload

import pygame

Coord = Union[Sequence[float], Sequence[int]]
FloatOrRange = Union[float, Sequence[float]]

EMIT_POINT: int = 0

class Emitter:
    @overload
    def __init__(
        self,
        emit_shape: int,
        emit_number: int,
        animation: Tuple[pygame.Surface, ...],
        particle_lifetime: FloatOrRange,
        speed_x: FloatOrRange = 0,
        speed_y: FloatOrRange = 0,
        acceleration_x: FloatOrRange = 0,
        acceleration_y: FloatOrRange = 0,
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
