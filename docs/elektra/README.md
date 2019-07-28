# LCDproc with Elektra

## Setting up Elektra

-   Checkout the latest version of Elektra's `master` branch (or PR #2805, if that hasn't been merged
    yet).
-   Compile Elektra
    ```sh
    # inside Elektra source directory
    mkdir build && cd build
    cmake .. -DPLUGINS="c;cache;dump;gopts;hexnumber;ini;list;mmapstorage;network;ni;noresolver;path;quickdump;range;reference;resolver;resolver_fm_hpu_b;spec;specload;type;validation" -DBUILD_TESTING=OFF -DENABLE_TESTING=OFF -DINSTALL_TESTING=OFF
    ```
    This builds Elektra with the minimal set of plugins required for LCDproc and with testing disabled to speed up the build process.
-   Install Elektra
    ```sh
    # inside Elektra build directory
    sudo make install
    ```

## Setting up LCDproc

The basic instructions from `INSTALL.md` are still valid. The quickest way to get started is:

```sh
# inside LCDproc source directory
sh ./autogen.sh
./configure
make
```

You may need to configure you're `PKG_CONFIG_PATH` such that `pkgconfig` is able to find Elektra.
For example this may work `PKG_CONFIG_PATH=/usr/local/lib/pkgconfig ./configure`, instead of
`./configure`.

**Note:** Not all drivers have been updated yet. These drivers should work: bayrad, CFontz,
CFontzPacket, curses, CwLnx, glk, lb216, lcdm001, MtxOrb, pyramid, text, linux_input, xosd, hd44780

Once you built the server and/or the clients, you need to configure Elektra, so that it knows about
LCDprocs configuration. This process is called mounting (since it is similar to mounting a new file
system, but for configurations).

We will demonstrate this process for running the `lcdvc` client directly from the source/build
directory, but it is similar for the other parts of LCDproc and for running installed versions.

1. **Mount the specification:** Compiling `lcdvc` produces the file
   `clients/lcdvc/elektragen.spec.eqd`. It contains the processed specificaiton for `lcdvc`. To mount it
   we use:
   `sh # inside LCDproc source directory sudo kdb mount -R noresolver "/usr/local/share/lcdproc/lcdvc_spec.eqd" "spec/sw/lcdproc/lcdvc/#0/current" specload "app=/usr/bin/cat" "app/args=#0" "app/args/#0=$PWD/clients/lcdvc/elektragen.spec.eqd"`
2. **Spec-mounting:** Once the specification is mounted, Elektra knows about it. However, Elektra
   doesn't know to treat it as a specification for application yet. For that wee need to execute
   another command:
   `sh sudo kdb spec-mount "/sw/lcdproc/lcdvc/#0/current"`

Now everything is setup and and we can use `lcdvc`.

### Running installed versions of LCDproc

TODO

### Other parts of LCDproc

The setup for other parts of LCDproc is the same, you just need to replace some paths.
This table helps you with that:

|           |                                             |                                      |                                            |
| --------- | ------------------------------------------- | ------------------------------------ | ------------------------------------------ |
| `lcdvc`   | `/usr/local/share/lcdproc/lcdvc_spec.eqd`   | `spec/sw/lcdproc/lcdvc/#0/current`   | `$PWD/clients/lcdvc/elektragen.spec.eqd`   |
| `lcdexec` | `/usr/local/share/lcdproc/lcdexec_spec.eqd` | `spec/sw/lcdproc/lcdexec/#0/current` | `$PWD/clients/lcdexec/elektragen.spec.eqd` |
| `lcdproc` | `/usr/local/share/lcdproc/lcdproc_spec.eqd` | `spec/sw/lcdproc/lcdproc/#0/current` | `$PWD/clients/lcdproc/elektragen.spec.eqd` |
| `LCDd`    | `/usr/local/share/lcdproc/lcdd_spec.eqd`    | `spec/sw/lcdproc/lcdd/#0/current`    | `$PWD/server/elektragen.spec.eqd`          |
