from particle_manager import ParticleManager, GROUP_POINT
from pygame import Surface
import pygame

from pyparticle import PyParticle

pygame.init()

images = [Surface((size, size)) for size in range(10, 0, -1)]
for surf in images:
    surf.fill((255, 255, 255))

PM = ParticleManager()
PM.add_group(
    GROUP_POINT,
    10000,  # number of particles
    (500, 500),  # spawn pos
    images,  # image sequence
    100,  # radius
    (-1, 1, True),  # x velocity info
    (-1, 1, True),  # y velocity info
)

particles = [PyParticle((500, 500), (0.1, 0.1)) for _ in range(10000)]

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
        screen.fblits([(p.images[p.img_ix], (p.x, p.y)) for p in particles])

    screen.blit(font.render(f"fps: {int(clock.get_fps())}", True, "white"))
    screen.blit(font.render(f"particles: {PM.num_particles}", True, "white"), (0, 30))

    pygame.display.flip()

    for event in pygame.event.get([pygame.QUIT, pygame.KEYUP]):
        if event.type == pygame.QUIT:
            pygame.quit()
            quit()
        elif event.type == pygame.KEYUP:
            if event.key == pygame.K_1:
                use_pm = not use_pm
