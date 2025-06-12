## Description
Creates a command line interface shell that can display the elapsed time since power on in either seconds or hours, minutes and seconds as well as control two on board LEDs. Utilises the Zephyr Logging API to log info messages, warnings, errors and debug messages.

## Folder Structure
- cli
    - src
        - main.c
    - sysbuild
        - mcuboot.conf
    - CMakeLists.txt
    - prj.conf
    - README.md
    - sample.yaml
    - sysbuild.conf

## References
https://docs.zephyrproject.org/latest/index.html

## Instructions
Use help to see all commands available.
Commands:
- time
    - Displays time elapsed since power on in seconds.
- time f
    - Displays time elapsed since power on in hours, minutes and seconds.
- led s xx
    - Sets led 1 and 2 to their respective values in the bitmask (LED2, LED1).
    - Usage: led s 01 will set led 1 on and led 2 off.
- led t xx
    - Toggles led 1 and/or 2 depending on the bitmask.
    - Usage: led t 01 will toggle led 1, but leave led 2 untouched.
