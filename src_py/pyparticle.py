from pygame import Surface


class PyParticle:
    def __init__(
        self,
        x,
        y,
        vx,
        vy,
        accx=0,
        accy=0,
        grav_x=0,
        grav_y=0,
        update_speed=1,
        imgs=None,
    ):
        self.x = x
        self.y = y
        self.vx = vx
        self.vy = vy
        self.acc_x = accx
        self.acc_y = accy
        self.grav_x = grav_x
        self.grav_y = grav_y
        self.time = 0
        self.update_speed = update_speed
        self.images = imgs
        self.imgs_len = len(self.images) - 1

    def update(self, dt):
        # self.acc_x += self.grav_x
        # self.acc_y += self.grav_y
        # self.vx += self.acc_x * dt
        # self.vy += self.acc_y * dt
        self.x += self.vx * dt
        self.y += self.vy * dt

        self.time += self.update_speed * dt
        if int(self.time) >= self.imgs_len:
            self.time = self.imgs_len
            return False
        return True
