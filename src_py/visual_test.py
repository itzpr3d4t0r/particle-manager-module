from random import random

import pygame
from itz_particle_manager import ParticleManager, SPAWN_POINT

from src_py.pyparticle import PyParticle

pygame.init()


def rand_between(low, high):
    return low + random() * (high - low)


imgs = [
    [pygame.Surface((s, s)) for s in range(5, 1, -1)],
]
for seq in imgs:
    for i, img in enumerate(seq):
        img.fill(-1)

PARTICLE_NUM = 30000
PARTICLE_NUM_ON_CLICK = 1000

PM = ParticleManager()
PM.add_group(
    1,
    SPAWN_POINT,
    PARTICLE_NUM,  # number of particles
    (500, 500),  # spawn pos
    imgs,  # image sequence
    (-1, 1),  # x velocity info
    (-1, 1),  # y velocity info
    0,  # x acceleration info
    0,  # y acceleration info
    (0.025, 0.8),  # update speed info
)

particles = [
    PyParticle(
        500,
        500,
        rand_between(-1, 1),
        rand_between(-1, 1),
        0,
        0,
        update_speed=rand_between(0.025, 0.8),
    )
    for _ in range(PARTICLE_NUM)
]

screen = pygame.display.set_mode((1000, 1000))

use_pm = True

clock = pygame.time.Clock()
font = pygame.font.SysFont("Arial", 20, True)
pm_mode_txt = font.render("PM mode (press 1 to change mode)", True, "green")
list_mode_txt = font.render("List mode  (press 1 to change mode)", True, "green")
click_to_spawn = font.render("Click any mouse button to spawn particles", True, "white")

t = 0
fps_text = font.render("fps: 0", True, "red")

while True:
    dt = clock.tick_busy_loop(10000) * 60 / 1000

    screen.fill(0)

    if use_pm:
        PM.update(dt)
        PM.draw(screen)
        screen.blit(
            font.render(f"num groups: {PM.num_groups}", True, "red"),
            (180, 30),
        )
    else:
        particles = [p for p in particles if p.update(dt)]
        screen.fblits([(p.images[int(p.time)], (p.x, p.y)) for p in particles])

    t += dt
    if t > 5:
        t = 0
        fps_text = font.render(f"fps: {int(clock.get_fps())}", True, "red")

    screen.blit(fps_text)
    screen.blit(
        font.render(
            f"particles: {PM.num_particles if use_pm else len(particles)}", True, "red"
        ),
        (0, 30),
    )
    screen.blit(pm_mode_txt if use_pm else list_mode_txt, (0, 55))
    screen.blit(click_to_spawn, (0, 80))

    pygame.display.flip()

    for event in pygame.event.get([pygame.QUIT, pygame.KEYUP, pygame.MOUSEBUTTONUP]):
        if event.type == pygame.QUIT:
            pygame.quit()
            quit()
        elif event.type == pygame.KEYUP:
            if event.key == pygame.K_1:
                use_pm = not use_pm
        elif event.type == pygame.MOUSEBUTTONUP:
            if use_pm:
                PM.add_group(
                    1,
                    SPAWN_POINT,
                    PARTICLE_NUM_ON_CLICK,
                    pygame.mouse.get_pos(),
                    imgs,
                    (-5, 5),  # x velocity info
                    (-5, 5),  # y velocity info
                    0,  # x acceleration info
                    0,  # y acceleration info
                    (0.025, 0.8),  # update speed info
                )
            else:
                particles.extend(
                    [
                        PyParticle(
                            *pygame.mouse.get_pos(),
                            rand_between(-5, 5),
                            rand_between(-5, 5),
                            0,
                            0,
                            update_speed=rand_between(0.025, 0.8),
                        )
                        for _ in range(PARTICLE_NUM_ON_CLICK)
                    ]
                )
