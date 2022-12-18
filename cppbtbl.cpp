#include <iostream>
#include <string>
#include <sstream>
#include <set>
#include <vector>
#include <sys/eventfd.h>
#include <bits/stdc++.h>
#include <getopt.h>
#include <poll.h>
#include <sdbus-c++/sdbus-c++.h>

// battery check interval (if a device is connected)
#define CHECK_INTERVAL      15 * 1000

#define UPOWER_IFACE        "org.freedesktop.UPower"
#define UPOWER_PATH         "/org/freedesktop/UPower"
#define UPOWER_DEVICE_IFACE "org.freedesktop.UPower.Device"

#define PROPERTIES_IFACE    "org.freedesktop.DBus.Properties"

const int icon_length = 5;
const char *icons[5] = { "", "", "", "", "" };

std::set<sdbus::ObjectPath> watch_list;

enum OutputFormat {
    format_waybar,
    format_icon_only,
    format_icon_device_name,
    format_raw,
    
    format_invalid
};
OutputFormat out_format = format_raw;

const char *get_icon(int percentage) {
    int icon_idx = (percentage * icon_length / 100) + 0.5;
    return icons[icon_idx];
}

void replace_all(std::string &s, const std::string &search, const std::string &replace) {
    for( size_t pos = 0; ; pos += replace.length() ) {
        // Locate the substring to replace
        pos = s.find( search, pos );
        if( pos == std::string::npos ) break;
        // Replace by erasing and inserting
        s.erase( pos, search.length() );
        s.insert( pos, replace );
    }
}

void _get_battery_infos() {
    if (watch_list.empty()) return;

    int least_percentage = 100;
    std::string least_device_name;
    std::stringstream tooltip;
    for (auto &device : watch_list) {
        auto device_obj = sdbus::createProxy(UPOWER_IFACE, device);
        std::string device_name;
        try {
            device_name = device_obj->getProperty("Model")
                                        .onInterface(UPOWER_DEVICE_IFACE)
                                        .get<std::string>();
        } catch (const sdbus::Error& e) {
            device_name = device_obj->getProperty("Serial")
                            .onInterface(UPOWER_DEVICE_IFACE)
                            .get<std::string>();
        }

        double percentage = device_obj->getProperty("Percentage")
                                .onInterface(UPOWER_DEVICE_IFACE)
                                .get<double>();
        tooltip << device_name << ": " << percentage << "%\n";

        if (percentage < least_percentage) {
            least_percentage = percentage;
            least_device_name = std::move(device_name);
        }
    }

    std::string tooltip_str = tooltip.str();
    tooltip_str.erase(tooltip_str.length() - 1);

    switch (out_format) {
        case format_waybar:
            replace_all(tooltip_str, "\n", "\\n");
            replace_all(tooltip_str, "\"", "\\\"");
            // lazy af solution, but should work
            std::cout << "{\"percentage\":" << least_percentage << ",\"tooltip\":\"" << tooltip_str << "\"}" << std::endl;
            break;
        case format_icon_device_name:
            std::cout << get_icon(least_percentage) << ": " << least_device_name << std::endl;
            break;
        case format_icon_only:
            std::cout << get_icon(least_percentage) << std::endl;
            break;
        case format_raw:
            std::cout << tooltip_str << std::endl;
            break;
    }
}

void _device_added(sdbus::ObjectPath &path) {
    auto device_object = sdbus::createProxy(UPOWER_IFACE, path);

    std::string native_path = device_object
                                ->getProperty("NativePath")
                                    .onInterface(UPOWER_DEVICE_IFACE)
                                    .get<std::string>();
    if (native_path.rfind("/org/bluez") != 0) {
        // non-bluetooth device, we can ignore
        return;
    }

    // std::cerr << "[DEBUG] Added to watchlist: " << path << std::endl;
    
    watch_list.insert(std::move(path));
}

void _device_removed(sdbus::ObjectPath &path) {
    watch_list.erase(path);
    
    if (watch_list.empty()) {
        std::cout << std::endl;
    }
}

void _device_added_signal(sdbus::Signal &signal) {
    sdbus::ObjectPath device_path;
    signal >> device_path;

    // std::cerr << "[DEBUG] Received device_added signal!" << std::endl;
    _device_added(device_path);
}

void _device_removed_signal(sdbus::Signal &signal) {
    sdbus::ObjectPath device_path;
    signal >> device_path;

    // std::cerr << "[DEBUG] Received device_removed signal!" << std::endl;
    _device_removed(device_path);
}

void help(char *name) {
    std::cout
        << "Usage: " << name << " -f [format] [-e]\n"
        << "-f/--format [format]    valid options: waybar, icononly, icon+devicename, raw (default: raw)\n"
        << "-h/--help               show this help screen\n"
        << "-e/--dont-follow        output info and exit" << std::endl;
}

OutputFormat optarg_to_format() {
    if (strcmp(optarg, "waybar") == 0) return format_waybar;
    if (strcmp(optarg, "icononly") == 0) return format_icon_only;
    if (strcmp(optarg, "icon+devicename") == 0) return format_icon_device_name;
    if (strcmp(optarg, "raw") == 0) return format_raw;

    return format_invalid;
}

int main(int argc, char *argv[]) {
    static struct option long_options[] = {
        {"help",        no_argument,       0, 'h'},
        {"format",      required_argument, 0, 'f'},
        {"dont-follow", no_argument,       0, 'e'}
    };
    bool dont_follow = false;
    int opt;
    while ((opt = getopt_long(argc, argv, "hef:", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                help(argv[0]);
                return 0;
            case 'f':
                out_format = optarg_to_format();
                if (out_format == format_invalid) {
                    std::cerr << "Invalid format!\n"
                        << "Valid values are: waybar, icononly, icon+devicename, raw." << std::endl;

                    return 1;
                }
                break;
            case 'e':
                dont_follow = true;
                break;
            default:
                return 1;
        }
    }

    // enumerate currently connected devices, and if applicable, add them to the watch_list
    auto proxy = sdbus::createProxy(UPOWER_IFACE, UPOWER_PATH);
    auto method = proxy->createMethodCall(UPOWER_IFACE, "EnumerateDevices");
    auto reply = proxy->callMethod(method);
    
    std::vector<sdbus::ObjectPath> res;
    reply >> res;


    for (auto &obj : res) {
        _device_added(obj);
    }

    if (dont_follow) {
        _get_battery_infos();
        return 0;
    }

    // signal handlers
    proxy->registerSignalHandler(UPOWER_IFACE, "DeviceAdded", &_device_added_signal);
    proxy->registerSignalHandler(UPOWER_IFACE, "DeviceRemoved", &_device_removed_signal);
    proxy->finishRegistration();
    
    auto connection = &proxy->getConnection();

    // `poll` event loop + timer for polling connected devices' battery
    while (true)
    {
        auto processed = connection->processPendingRequest();
        if (processed)
            continue; // Process next one

        int timeout = -1;
        if (!watch_list.empty()) {
            timeout = CHECK_INTERVAL;
            _get_battery_infos();
        }

        auto pollData = connection->getEventLoopPollData();
        struct pollfd fds[] = {
            {pollData.fd, pollData.events, 0}
        };

        auto r = poll(fds, 1, timeout);

        if (r < 0 && errno == EINTR)
            continue;

    }
}