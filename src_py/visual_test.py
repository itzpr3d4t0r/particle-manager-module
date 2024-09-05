from random import random

import pygame
from itz_particle_manager import ParticleManager, SPAWN_POINT

from src_py.pyparticle import PyParticle

pygame.init()

imgs = [
    [pygame.Surface((s, s)) for s in range(10, 20, 1)],
    [pygame.Surface((s, s)) for s in range(1, 5, 1)],
    [pygame.Surface((s, s)) for s in range(50, 101, 10)],
]
for seq in imgs:
    for i, img in enumerate(seq):
        img.fill(-1)

PARTICLE_NUM = 30000

PM = ParticleManager()
PM.add_group(
    1,
    SPAWN_POINT,
    PARTICLE_NUM,  # number of particles
    (500, 500),  # spawn pos
    imgs,  # image sequence
    (-1, 1, True),  # x velocity info
    (-1, 1, True),  # y velocity info
)

particles = [
    PyParticle(500, 500, random() * 2 - 1, random() * 2 - 1, 0, 0)
    for _ in range(PARTICLE_NUM)
]

screen = pygame.display.set_mode((1000, 1000))

use_pm = True

clock = pygame.time.Clock()
font = pygame.font.SysFont("Arial", 20, True)

while True:
    dt = clock.tick_busy_loop(1000) * 60 / 1000

    screen.fill(0)

    if use_pm:
        PM.update(dt)
        for seq, flag in PM.groups:
            screen.fblits(seq, flag)
    else:
        for p in particles:
            p.update(dt)
        screen.fblits([(p.images[0], (p.x, p.y)) for p in particles])

    screen.blit(font.render(f"fps: {int(clock.get_fps())}", True, "red"))
    screen.blit(font.render(f"particles: {PM.num_particles}", True, "red"), (0, 30))

    pygame.display.flip()

    for event in pygame.event.get([pygame.QUIT, pygame.KEYUP, pygame.MOUSEBUTTONUP]):
        if event.type == pygame.QUIT:
            pygame.quit()
            quit()
        elif event.type == pygame.KEYUP:
            if event.key == pygame.K_1:
                use_pm = not use_pm
        elif event.type == pygame.MOUSEBUTTONUP:
            PM.add_group(
                1,
                SPAWN_POINT,
                100,
                pygame.mouse.get_pos(),
                imgs,
                (-1, 1, True),
                (-1, 1, True),
            )
