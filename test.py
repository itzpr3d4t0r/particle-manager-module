from particle_manager import ParticleManager, GROUP_POINT
from pygame import Surface

images = [Surface((size, size)) for size in range(10, 0, -1)]
p = ParticleManager()

for _ in range(23):
    p.add_group(GROUP_POINT, 5, (500, 500), images, 100, (1,), (1,))
    print(p)