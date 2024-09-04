from pygame import Surface


class PyParticle:
    img_sequences = [
        [Surface((s, s)) for s in range(10, 20, 1)],
        [Surface((s, s)) for s in range(1, 5, 1)],
        [Surface((s, s)) for s in range(50, 101, 10)],
    ]

    def __init__(self, x, y, vx, vy, accx, accy):
        self.x = x
        self.y = y
        self.vx = vx
        self.vy = vy
        self.acc_x = accx
        self.acc_y = accy
        self.seq_ix = 0
        self.time = 0
        self.update_speed = 1
        self.images = self.img_sequences[0]

    def update(self, dt):
        self.x += self.vx * dt
        self.y += self.vy * dt
