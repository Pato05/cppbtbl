# cppbtbl (**C++** **b**lue**t**ooth **b**attery **l**evel)

A wrapper around UPower's DBus API that returns connected bluetooth devices' battery (and that can also be used as a waybar module!)

## Installing

### From the AUR

If you're using Arch Linux, you can install the package from the AUR under the `cppbtbl` name by using the AUR helper of your choice

### Manually

First of all, you're gonna need to install sdbus-c++, see [sdbus-cpp: Building and installing the library](https://github.com/Kistler-Group/sdbus-cpp#building-and-installing-the-library)

Clone the repo

```bash
git clone https://github.com/pato05/cppbtbl
cd cppbtbl
```

Build using CMake

```bash
cmake .
make
sudo make install
```

### Note

cppbtbl depends on sdbus-c++ which itself depends on libsystemd. Regardless of this dependency, though, you don't necessarily need systemd, as you can get libsystemd without having it: see [sdbus-cpp: Solving libsystemd dependency](https://github.com/Kistler-Group/sdbus-cpp/blob/master/docs/using-sdbus-c++.md#solving-libsystemd-dependency).

Note that if you are using systemd, you won't have any issues using cppbtbl.

## Configuration

`cppbtbl` supports a variety of flags, which can be seen by doing `cppbtbl --help`.

Each of these flags (in its long form) can be written inside a configuration file (with the format `name=value`) that is found inside either
`$XDG_CONFIG_HOME/cppbtbl/config` (if defined) or `$HOME/.config/cppbtbl/config`.

An example of this config file can be as follows:

```
format=custom
output={icon} ({percentage}%)
icons=1,2,3,4,5
```

You can also specify the `dont-follow` option here (if needed) by just typing it with no following characters.

```
format=custom
output={icon} ({percentage}%)
icons=1,2,3,4,5
dont-follow
```

## Waybar

```json
...
    "custom/bt-battery": {
        "exec": "cppbtbl -f waybar",
        "restart-interval": 5,
        "return-type": "json",
        "format": "{icon}",
        "format-icons": ["󰁺", "󰁻", "󰁼", "󰁼", "󰁽", "󰁿", "󰂀", "󰂀", "󰂂", "󰁹"]
    },
...
```

Change format and icons accordingly, and the module should work!

## Polybar

```
[module/bt-battery]
type = custom/script

exec = cppbtbl

; This is necessary, because cppbtbl will continue running
; so let's make sure polybar knows about it
tail = true
```

Now, you need to define your custom options, see [#configuration](#configuration).
Please **make sure** to use `--format=custom`.

## Other status bars

In other status bars, just like we saw in Polybar, you can use `--format custom` and defining your own `--output`-format and your `--icons`-set

For example `cppbtbl --format custom --output "{icon} ({percentage}%)" --icons 'icon1,icon2,icon3'`.

Keep in mind that by default the `cppbtbl` process will always be running and listening for connection/disconnection events, therefore your bar must be
able to handle that. In case you want to use a "polling" approach, you can use the `--dont-follow` option, which will close the program once it
returned the desired output.

Please note that the "polling" approach won't be closely as efficient as the "listening" one, because too high of an interval will cause massive CPU usage,
too low and you'll get a delay between your device being connected and it being shown in your bar.


### A thank-you goes to [@Justasic](https://github.com/Justasic) for helping me a lot in making this.
