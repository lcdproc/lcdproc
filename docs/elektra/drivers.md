# Drivers

## Configuration specification

The whole server uses a single specification. This specification also includes all the drivers.
To add a new driver (e.g. `newdriver`) to the specification, you need to add at least these lines:

```ini
[newdriver]
default = ""
array = #0

[newdriver/#]
type = struct
check/type = any
default = ""
gen/struct/type = NewdriverDriverConfig
gen/struct/alloc = 0
description = Configuration for a newdriver driver.

[newdriver/#/file]
type = string
default = "newdriver"
description = name of the shared library to load (without prefix and extension)
```

This declares a new array called `newdriver` which will contain all of the different configurations
for the `newdriver` driver. It then specifies that each of the array elements is a struct of type
`NewdriverDriverConfig` (currently the standard is to call the struct `[DriverName]DriverConfig`).
Lastly we define that each array element has the subkey `file`. The driver itself can probably ignore
this key, but it is needed nonetheless. This key is used by the driver loading mechanism to determine
which `.so` file to load.

Now you can add any configuration parameters your driver needs. For example if `newdriver` has a
contrast parameter, we could add:

```ini
[newdriver/#/contrast]
type = unsigned_short
check/type = unsigned_short
default = 500
check/range = 0-1000
description = contrast value
```

**Note:** It is recommended to use a struct with `gen/struct/alloc = 0`, but there is no technical
need for it. It just makes loading the config easier and the code a bit more readable. You may also
use `gen/struct/alloc = 1` or not use structs at all, if you have a reason.

After you added the specification for your driver, you need to update the list of known drivers in the
server part of the specification. The key `server/drivers` is an array of the active drivers. The
specification contains restrictions, so that only known drivers are allowed.

```ini
[server/drivers]
default = ""
array = #0 ; TODO (elektra): max drivers
check/reference = single
check/reference/restrict = #_51
check/reference/restrict/#0 = @/bayrad/#
check/reference/restrict/#1 = @/cfontz/#
; ...
check/reference/restrict/#_51 = @/yard2lcd/#
description = "Tells the server to load a driver.
The given value is a reference the configuration of the driver, e.g. @/curses/#0"
```

To add your driver to the list of allowed drivers, just increment the value of `check/reference/restrict` 
and add a new entry to `check/reference/restrict/#` of the form `@/[DriverName]/#`. This tells Elektra
to only allow references to existing keys inside the `[DriverName]` array. The `@` just makes says the
reference is relative to the parent keys. Similar to what `~` does in the shell. For the `newdriver` driver
we would get:

```ini
[server/drivers]
default = ""
array = #0 ; TODO (elektra): max drivers
check/reference = single
check/reference/restrict = #_52
check/reference/restrict/#0 = @/bayrad/#
check/reference/restrict/#1 = @/cfontz/#
; ...
check/reference/restrict/#_51 = @/yard2lcd/#
check/reference/restrict/#_52 = @/newdriver/#
description = "Tells the server to load a driver.
The given value is a reference the configuration of the driver, e.g. @/curses/#0"
```

**Note:** To make it easier to maintain this list of driver, please keep the drivers in alphabetical order. So in the above example `newdriver` should be somewhere in the omitted part of the array.

## Elektrifying drivers

The process of elektrifying a driver is rather simple. As before, each driver must have an `init`
function, but now this function receives an `Elektra *` handle in addition to the `Driver *` handle.

The `Driver` struct was modified as well. It no longer provides the `config_get_*` function pointers,
since they are not needed anymore. Instead use the `elektraGet*` functions to access configuration.
To distinguish between multiple configurations for the same driver, we store the `index` in the
`Driver` struct.

**Note:** Please load the whole configuration during `init` and don't store the `Elektra *` handle,
it may become invalid after you return from `init`.

There are various ways to access the configuration using Elektra. The easiest one is to use
`elektraFillStructV` to read the whole configuration into a struct, from which you can then copy
values into the driver specific data structures.

For example for the `curses` driver we use:

```c
CursesDriverConfig config;
elektraFillStructV(elektra, &config, ELEKTRA_TAG_CURSES, drvthis->index);
```

Since we use a stack allocated struct with `elektraFillStructV`, we don't have to do any memory
management. There is one caveat however. If our config struct contains any strings, they may become
invalid, so we need to `strcpy` or `strdup` them, if we want to store them. For more information on
the pointer lifespan see the documentation of `elektraGetString`.

Not all kinds of specifications are compatible with the struct mechanism of Elektra's
code-generation, especially not with the `elektraFillStruct` version. If you have a more complicated
case, for example certain kinds of arrays, you will want to use the more basic `elektraGetV`.

For example `linux_input` reads its keymap like this:

```c
kdb_long_long_t size = elektraSizeV(elektra, ELEKTRA_TAG_LINUX_INPUT_KEYS, drvthis->index);

for (kdb_long_long_t i = 0; i < size; ++i) {
    struct keycode * key = malloc(sizeof(struct keycode));
    key->code = elektraGetV(elektra, ELEKTRA_TAG_LINUX_INPUT_KEYS_CODE, drvthis->index, i);
    key->button = strdup(elektraGetV(elektra, ELEKTRA_TAG_LINUX_INPUT_KEYS_BUTTON, drvthis->index, i));
    LL_AddNode(p->buttonmap, key);
}
```