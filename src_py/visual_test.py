from random import random, randint

import pygame
from itz_particle_manager import ParticleManager, EMIT_POINT, Emitter, ParticleEffect

from src_py.pyparticle import PyParticle

pygame.init()


def rand_between(low, high):
    return low + random() * (high - low)


color = (randint(0, 255), randint(0, 255), randint(0, 255))
imgs = tuple(pygame.Surface((s, s)) for s in range(5, 0, -1))
for img in imgs:
    img.fill(color)

PARTICLE_NUM_ON_CLICK = 10000
idx = 0
BLEND_MODE = pygame.BLEND_ADD

PM = ParticleManager()
particles = []

emitter = Emitter(
    emit_shape=EMIT_POINT,
    emit_number=PARTICLE_NUM_ON_CLICK,
    animation=imgs,
    particle_lifetime=(60, 180),
    speed_x=(-2, 2),
    speed_y=(-2, 2),
    blend_mode=BLEND_MODE,
)

effect = ParticleEffect((emitter,))

screen = pygame.display.set_mode((1000, 1000))

use_pm = True

clock = pygame.time.Clock()
font = pygame.font.SysFont("Arial", 20, True)
pm_mode_txt = font.render(
    "Using: ParticleManager (press 1 to change mode)", True, "green"
)
list_mode_txt = font.render("Using: List (press 1 to change mode)", True, "green")
fps_text = font.render("fps: 0", True, "red")

t = 0
timer = 0

while True:
    dt = clock.tick_busy_loop(10000) * 60 / 1000

    screen.fill(0)

    if use_pm:
        PM.update(dt)
        PM.draw(screen)
    else:
        particles = [p for p in particles if p.update(dt)]
        screen.fblits(
            [(p.images[int(p.time)], (p.x, p.y)) for p in particles], BLEND_MODE
        )

    t += dt
    if t > 5:
        t = 0
        fps_text = font.render(f"fps: {int(clock.get_fps())}", True, "red")

    timer += dt
    if timer >= 5:
        timer = 0

        if use_pm:
            PM.spawn_effect(effect, (500, 500))
        else:
            particles.extend(
                [
                    PyParticle(
                        500,
                        500,
                        rand_between(-1, 1),
                        rand_between(-1, 1),
                        update_speed=rand_between(0.02, 0.04),
                        imgs=imgs,
                    )
                    for _ in range(PARTICLE_NUM_ON_CLICK)
                ]
            )

    screen.blit(fps_text)
    screen.blit(
        font.render(
            f"particles: {PM.num_particles if use_pm else len(particles)}", True, "red"
        ),
        (0, 30),
    )
    screen.blit(pm_mode_txt if use_pm else list_mode_txt, (0, 55))

    pygame.display.flip()

    for event in pygame.event.get([pygame.QUIT, pygame.KEYUP, pygame.MOUSEBUTTONUP]):
        if event.type == pygame.QUIT:
            pygame.quit()
            quit()
        elif event.type == pygame.KEYUP:
            if event.key == pygame.K_1:
                use_pm = not use_pm
