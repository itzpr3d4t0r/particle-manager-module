from particle_manager import ParticleManager, GROUP_POINT
from pygame import Surface
from timeit import repeat
from statistics import fmean
from pyparticle import PyParticle

images = [Surface((size, size)) for size in range(10, 0, -1)]
p = ParticleManager()

for _ in range(10):
    p.add_group(GROUP_POINT, 1, (500, 500), images, 100, (1,), (1,))
    print(p)

p.add_group(GROUP_POINT, 1, (500, 500), images, 100, (1,), (1,))

# print(p)

# particles = [PyParticle((500, 500), (1, 1)) for _ in range(10)]
#
# G = globals()
#
# p.add_group(GROUP_POINT, 100, (500, 500), images, 100, (1,), (1,))
#
# print(fmean(repeat("p.add_group(GROUP_POINT, 1000, (500, 500), images, 100, (-1, 1, True), (-1, 1, True))", repeat=1, number=1, globals=G)))
# print(fmean(repeat("[PyParticle((500, 500), (1, 1)) for _ in range(1000)]", repeat=1, number=1, globals=G)))
#
# print()
#
# for _ in range(5):
#     print(fmean(repeat("p.update(.1)", repeat=10000, number=1, globals=G)))
#
# print()
#
# for _ in range(5):
#     print(fmean(
#         repeat("for p in particles: p.update(.1)", repeat=10000, number=1, globals=G)))

# print(fmean(repeat("p.groups", repeat=1, number=1, globals=G)))
# print(
#     fmean(
#         repeat(
#             "[(p.images[p.img_ix], (p.x, p.y)) for p in particles]",
#             repeat=1,
#             number=1,
#             globals=G,
#         )
#     )
# )
