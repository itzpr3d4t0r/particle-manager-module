import unittest

import pygame
from itz_particle_manager import ParticleManager, SPAWN_POINT


class TestParticleManager(unittest.TestCase):
    surface_sequences = [
        [pygame.Surface((s, s)) for s in range(10, 1, -1)],
    ]

    def test_ParticleManager_init(self):
        """Ensure that the ParticleManager class is initialized correctly."""
        pm = ParticleManager()
        self.assertEqual(pm.num_particles, 0)
        self.assertEqual(pm.num_groups, 0)
        self.assertEqual(pm.groups, [])

    def test_num_groups_del(self):
        """Ensure that the num_groups attribute cannot be deleted."""
        pm = ParticleManager()
        with self.assertRaises(AttributeError):
            del pm.num_groups

    def test_num_particles_del(self):
        """Ensure that the num_particles attribute cannot be deleted."""
        pm = ParticleManager()
        with self.assertRaises(AttributeError):
            del pm.num_particles

    def test_groups_del(self):
        """Ensure that the groups attribute cannot be deleted."""
        pm = ParticleManager()
        with self.assertRaises(AttributeError):
            del pm.groups

    def test_add_group(self):
        """Ensure that the add_group method works as expected."""
        combinations = [
            (1, SPAWN_POINT, 1, (0, 0), self.surface_sequences, (1,), (1,)),
            (1, SPAWN_POINT, 10, (0, 0), self.surface_sequences, (1,), (1,)),
            (1, SPAWN_POINT, 100, (0, 0), self.surface_sequences, (1,), (1,)),
            (1, SPAWN_POINT, 1000, (0, 0), self.surface_sequences, (1,), (1,)),
        ]

        for (
            flag,
            spawn_mode,
            num_particles,
            spawn_pos,
            surface_sequences,
            vx,
            vy,
        ) in combinations:
            pm = ParticleManager()
            pm.add_group(flag, spawn_mode, num_particles, spawn_pos, surface_sequences)
            self.assertEqual(pm.num_groups, 1)
            self.assertEqual(len(pm.groups), 1)
            self.assertEqual(pm.num_particles, num_particles)

        pm = ParticleManager()
        for args in combinations:
            pm.add_group(*args)

        self.assertEqual(pm.num_groups, len(combinations))
        self.assertEqual(len(pm.groups), len(combinations))
        self.assertEqual(
            pm.num_particles,
            sum([num_particles for f, sm, num_particles, *_ in combinations]),
        )

    def test_update(self):
        """Ensure that the update method works as expected."""
        pm = ParticleManager()
        pm.add_group(0, SPAWN_POINT, 10, (0, 0), self.surface_sequences)
        pm.update(0.1)
        self.assertTrue(all(isinstance(group, tuple) for group in pm.groups))


if __name__ == "__main__":
    unittest.main()
