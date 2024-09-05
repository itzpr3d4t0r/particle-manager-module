project(
    'itz_particle_manager',
    'c',
    version : '0.1',
    default_options : [
        'optimization=3',
    ],
)

# This code was copied from the meson.build file of the Pygame project, I do not own this code.

itz_pm = 'itz_particle_manager' # This is the name of the module

if host_machine.system() == 'windows'
    plat = 'win'
elif host_machine.system() == 'darwin'
    plat = 'mac'
elif host_machine.system() == 'linux'
    plat = 'linux'
elif host_machine.system() == 'android'
    plat = 'android'
    error(
        'The meson buildconfig of itz_particle_manager does not support android for now.',
        'However it may be added in the future',
    )
elif host_machine.system() == 'emscripten'
    plat = 'emscripten'
    error(
        'The meson buildconfig of itz_particle_manager does not support emscripten for now. ',
        'However it may be added in the future',
    )
else
    # here it one of: cygwin, dragonfly, freebsd, gnu, haiku, netbsd, openbsd, sunos
    plat = 'unix'
    warning(
        'itz_particle_manager does not actively support building on your platform:',
        host_machine.system(),
    )
endif

cc = meson.get_compiler('c')

fs = import('fs')

py = import('python').find_installation(pure : false)
py_dep = py.dependency()

if not cc.has_header('Python.h', dependencies : py_dep)
    error(
        'Cannot use `Python.h`. Perhaps you need to install python-dev|python-devel',
    )
endif

curr_dir = py.get_install_dir()

include_dirs = []
lib_dirs = []
if plat == 'win' and host_machine.cpu_family().startswith('x86')
    arch_suffix = 'x' + host_machine.cpu_family().substring(-2)
    base_dir = meson.current_source_dir()
    prebuilt_dir = base_dir / 'prebuilt-' + arch_suffix

    # download prebuilts (uses legacy builconfig code)
    if not fs.is_dir(prebuilt_dir)
        run_command(
            [
                find_program('python3', 'python'),
                'download_win_prebuilt.py',
            ],
            check : true,
        )
    endif

    sdl_ver = '2.30.6'

    dlls = []

    # SDL2
    sdl_dir = prebuilt_dir / 'SDL2-@0@'.format(sdl_ver)
    sdl_lib_dir = sdl_dir / 'lib' / arch_suffix
    include_dirs += fs.relative_to(sdl_dir / 'include', base_dir)
    lib_dirs += sdl_lib_dir
    dlls += sdl_lib_dir / 'SDL2.dll'

    # put dlls in root of install
    install_data(dlls, install_dir : curr_dir)
else
    bases = ['/usr/local', '/usr', '/opt/homebrew', '/opt/local']
    foreach inc_dir : bases
        foreach sub_inc : [
            '',
            '/SDL2',
        ]
            full_inc = inc_dir / 'include' + sub_inc
            if fs.exists(full_inc)
                include_dirs += full_inc
            endif
        endforeach
    endforeach

    foreach lib_dir : bases
        foreach sub_lib : ['lib', 'lib64']
            full_lib = lib_dir / sub_lib
            if fs.exists(full_lib)
                lib_dirs += full_lib
            endif
        endforeach
    endforeach
endif

sdl_dep = dependency('sdl2', required : false)
if not sdl_dep.found()
    sdl_dep = declare_dependency(
        include_directories : include_dirs,
        dependencies : cc.find_library('SDL2', dirs : lib_dirs),
    )
endif

itzpm_base_deps = [sdl_dep, py_dep]

summary(
    {
        'SDL2' : sdl_dep.found(),
    },
    section : 'Dependencies',
)

simd_avx2 = false
simd_avx2_flags = []
if host_machine.cpu_family().startswith('x86')
    flag = (cc.get_argument_syntax() == 'msvc') ? '/arch:AVX2' : '-mavx2'
    if cc.has_argument(flag)
        simd_avx2_flags += flag
        simd_avx2 = true
    endif
endif

simd_sse2_neon = false
simd_sse2_neon_flags = []
if host_machine.cpu_family() == 'arm'
    # first check if compiler supports the flag, and use it then. Needed only
    # on 32-bit armv7.
    flag = '-mfpu=neon'
    if cc.has_argument(flag) and host_machine.cpu() == 'armv7l'
        simd_sse2_neon_flags += [flag, '-march=armv7-a']
        simd_sse2_neon = true
    endif
elif host_machine.cpu_family() == 'aarch64'
    # no explicit flag needed in this case
    simd_sse2_neon = true
endif

if simd_sse2_neon
    add_global_arguments('-DPG_ENABLE_ARM_NEON=1', language: 'c')
endif

summary(
    {
        'AVX2': simd_avx2,
        'NEON': simd_sse2_neon,
    },
    section: 'SIMD',
)

# List of source files
src_files = [
    'src/module.c',
    'src/particle_group.c',
    'src/simd_particles_avx2.c',
    'src/simd_particles_sse2.c',
    'src/f_vec.c',
]

py.extension_module(
    itz_pm,
    src_files,
    dependencies : itzpm_base_deps,
    include_directories : include_dirs,
    c_args : simd_avx2_flags + simd_sse2_neon_flags,
    install : true,
)