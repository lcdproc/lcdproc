# LCDproc with Elektra

## Setting up Elektra

-   Checkout the latest version of Elektra's `master` branch (commit `79408a3509d4e8f97369c877ff08709f7de04976`
    or later).
-   Compile Elektra
    ```sh
    # inside Elektra source directory
    mkdir build && cd build
    cmake .. -DPLUGINS="c;cache;dump;gopts;hexnumber;ini;list;mmapstorage;network;ni;noresolver;path;quickdump;range;reference;resolver;resolver_fm_hpu_b;spec;specload;type;validation;sync" -DBUILD_TESTING=OFF -DENABLE_TESTING=OFF -DINSTALL_TESTING=OFF
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

You could do this for the binaries directly in your source/build directory, but we recommend you
install LCDproc instead.

To install LCDproc run:

```sh
# inside LCDproc source directory
sudo make install
./post-install.sh
```

The script `post-install.sh` sets up Elektra such that it knows about LCDprocs specifications.

If you run `LCDd` now, you will see an error:

```
An error occurred while initializing elektra: Validation Semantic: Required key /sw/lcdproc/lcdd/#0/current/server/drivers/#0 is missing.
```

This simply means that you haven't chosen a driver yet. To choose a driver run:

```sh
kdb set '/sw/lcdproc/lcdd/#0/current/server/drivers/#0' '@/curses/#0'
```

This chooses the `curses` driver. Specifically it choses the first configuration of the `curses` driver,
which is stored below `/sw/lcdproc/lcdd/#0/current/curses/#0` (`@` stands for the parent key
`/sw/lcdproc/lcdd/#0/current`).
