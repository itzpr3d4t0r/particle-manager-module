from pygame import Surface


class PyParticle:
    img_sequences = [
        [Surface((s, s)) for s in range(2, 20, 1)],
        [Surface((s, s)) for s in range(1, 5, 1)],
        [Surface((s, s)) for s in range(50, 101, 10)],
    ]
    for seq in img_sequences:
        for i, img in enumerate(seq):
            img.fill(-1)

    def __init__(self, x, y, vx, vy, accx, accy, grav_x=0, grav_y=0):
        self.x = x
        self.y = y
        self.vx = vx
        self.vy = vy
        self.acc_x = accx
        self.acc_y = accy
        self.grav_x = grav_x
        self.grav_y = grav_y
        self.seq_ix = 0
        self.time = 0
        self.update_speed = 1
        self.images = self.img_sequences[0]

    def update(self, dt):
        self.acc_x += self.grav_x
        self.acc_y += self.grav_y
        self.vx += self.acc_x * dt
        self.vy += self.acc_y * dt
        self.x += self.vx * dt
        self.y += self.vy * dt
