////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019-2021 Vladislav Trifochkin
//
// This file is part of [io-lib](https://github.com/semenovf/io-lib) library.
//
// Changelog:
//      2021.09.13 Initial version
////////////////////////////////////////////////////////////////////////////////
#include "network_discoverer.hpp"
#include <iostream>
#include <string>

#if QT5_CORE_ENABLED
#   include <QCoreApplication>
#   include <QTimer>
#endif

namespace discovery = pfs::net::discovery;

// static void usage (std::string const & program, std::string const & error_string)
// {
//     std::cerr << error_string
//         << "\nRun `" << program << " client | server`"
//         << std::endl;
// }

#if QT5_CORE_ENABLED
//     class radiocast_timer_qt5: public radiocast_timer
//     {
//         QTimer _timer;
//
//     public:
//         void start (std::chrono::milliseconds interval) override
//         {
//             _timer.setSingleShot(false);
//             _timer.setInterval(interval);
//             _timer.start();
//
//             QObject::connect(& _timer, & QTimer::timeout, [this] {
//                 this->timed_out();
//             });
//         }
//
//         void stop () override
//         {
//             _timer.stop();
//         }
//     };
#endif

int main (int argc, char * argv[])
{
    std::string program{argv[0]};

//     if (argc < 2) {
//         usage(program, "Too few arguments");
//         return EXIT_FAILURE;
//     }

#if QT5_CORE_ENABLED
    QCoreApplication app(argc, argv);
//     radiocast_timer_qt5 rtimer;
#endif

    auto discoverer = discovery::make_discoverer<discovery::backend_enum::qt5>(/*rtimer*/);

    {
//         network_discoverer::options options;
//         options.listener_addr4 = "*";
//         options.listener_port = 42222;
//         options.listener_interface = "*";
//         options.peer_addr4 = "227.1.1.255";
//         discoverer.set_options(std::move(options));
//
//         discoverer.incoming_data_received.connect([] (std::string const & request) {
//             std::cout << "Listener: request: " << request << std::endl;
//         });
//
//         auto rc = discoverer.start();
//
//         if (!rc) {
//             std::cerr << "Start network discovering failure: "
//                 << rc.error << std::endl;
//             return EXIT_FAILURE;
//         }
    }

#if QT5_CORE_ENABLED
//     QObject::connect(& timer, & QTimer::timeout, [& discoverer] {
//         discoverer.radiocast();
//     });

    return app.exec();
#endif
}

