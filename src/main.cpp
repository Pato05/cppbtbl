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

#include "config.h"
#include "utils.h"

// battery check interval (if a device is connected)
#define CHECK_INTERVAL      15 * 1000

#define UPOWER_IFACE        "org.freedesktop.UPower"
#define UPOWER_PATH         "/org/freedesktop/UPower"
#define UPOWER_DEVICE_IFACE "org.freedesktop.UPower.Device"

#define PROPERTIES_IFACE    "org.freedesktop.DBus.Properties"


std::set<sdbus::ObjectPath> watch_list;

// runtime options
struct ProgramOptions opts;

std::string get_icon(int percentage) {
    int icon_idx = (percentage * opts.icons.size() / 100) + 0.5;
    return opts.icons[icon_idx];
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

    switch (opts.output_format) {
        case format_waybar:
            replace_all(tooltip_str, "\n", "\\n");
            replace_all(tooltip_str, "\"", "\\\"");
            // lazy af solution, but should work
            std::cout << "{\"percentage\":" << least_percentage << ",\"tooltip\":\"" << tooltip_str << "\"}" << std::endl;
            break;
        case format_custom:
            {
                // make an std::string copy of the char*
                std::string output(opts.custom_format);
                std::string icon = get_icon(least_percentage);
                // replace all variables
                replace(output, "{icon}", icon);
                replace(output, "{percentage}", std::to_string(least_percentage));
                replace(output, "{name}", least_device_name);

                std::cout << output << std::endl;
            }
            break;
        case format_raw_default:
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


int main(int argc, char *const *argv) {
    if (int exitcode = parse_opts(argc, argv, &opts);
        exitcode != -1)
            return exitcode;

    // enumerate currently connected devices, and if applicable, add them to the watch_list
    std::unique_ptr<sdbus::IProxy> proxy = sdbus::createProxy(UPOWER_IFACE, UPOWER_PATH);
    sdbus::MethodCall method = proxy->createMethodCall(UPOWER_IFACE, "EnumerateDevices");
    auto reply = proxy->callMethod(method);
    
    std::vector<sdbus::ObjectPath> res;
    reply >> res;


    for (auto &obj : res) {
        _device_added(obj);
    }

    if (opts.dont_follow) {
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
        bool processed = connection->processPendingRequest();
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

        // if (r < 0 && errno == EINTR)
        //     continue;

    }
}