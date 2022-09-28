# dumbtex
 Latex math renderer using [stb_image_write](https://github.com/nothings/stb/blob/master/stb_image_write.h "Link to file") and [libschrift](https://github.com/tomolt/libschrift "Link to repo") libraries.

WIP. Tested only normal font included in `fonts` file. Another fonts may invoke undefined behavior.

## Building
Use `make debug` for O0 optimization and `-D DEBUG -D PROFILER` flags

Use `make profiler` for O3 optimization and `-D PROFILER` flag

Use `make prod` for O3 optimization only

## Compiler flags
`-D DEBUG` - flag for debug output

`-D PROFILER` - flag for tracing profile (use it in `chrome://tracing`)
