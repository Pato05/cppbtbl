# cppbtbl (**C++** **b**lue**t**ooth **b**attery **l**evel)

A wrapper around UPower's DBus API that returns connected bluetooth devices' battery (and that can also be used as a waybar module!)

## Installing

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

## Other status bars

Two extra formats are offered to make cppbtbl usable in this case too. Those formats are `icononly` and `icon+devicename`. Please note that the default icons provided are part of the FontAwesome font, and (as of right now) the only way of changing them is by editing the source code.

A better way to handle this, would be to make a script that reads `cppbtbl`'s output and re-writes it in whatever way is best for you. For example:

```bash
#!/usr/bin/bash
DEVICES=()
PERCENTAGES=()
timeout=
while true; do
    eval "read $timeout line"
    if [ "$?" -ne "0" ]; then
        [ -z "$timeout" ] && break
        timeout=
        # timeout, flush devices info
        ...
        continue
    fi
    if [ -z "$line" ]; then
        timeout=
        # clear ARRAYs, all devices have been disconnected
        DEVICES=(); PERCENTAGES=()
        echo ''
        continue
    fi

    IFS=":"; read -a split <<< "${line//: /:}"; unset IFS
    DEVICES[${#DEVICES}]="${split[0]}"
    PERCENTAGES[${#PERCENTAGES}]="${split[1]}"
    # time out read comamnd after 1 second (we supposed that if no other data is available within one second, we need to flush)
    timeout="-t 1"
done < <(cppbtbl -f raw)
```

Please note that this is just an example, and it could likely be executed better.



Or a better way would be to just modify the source code to your likings.




### A thank-you goes to [@Justasic](https://github.com/Justasic) for helping me a lot in making this.
