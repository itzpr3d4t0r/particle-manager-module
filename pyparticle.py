from pygame import Surface


class PyParticle:
    images = [Surface((size, size)) for size in range(10, 0, -1)]
    for surf in images:
        surf.fill((255, 255, 255))

    def __init__(self, pos, vel):
        self.x, self.y = pos
        self.vx, self.vy = vel
        self.img_ix = 0

    def update(self, dt):
        self.x += self.vx * dt
        self.y += self.vy * dt
