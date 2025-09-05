#include <iostream>

#include <thread>
#include <chrono>

#include "hal/Socket.h"
#include "hal/ServerSocket.h"
#include "hal/Serial.h"




int main(const int argc, const char* argv[]) {

    int port_number = 10024;
    const std::string filename = argv[1];
    const int baudrate = atoi(argv[2]);

    ssr::ServerSocket ss;
    ss.bind(port_number);
    ss.listen();
    while (true) {
        std::cout << "waiting..." << std::endl;
        ssr::Socket socket = ss.accept();
        if (socket.setNonBlock(true) < 0) {
            std::cerr << "setNonBlock failed" << std::endl;
            break;
        }
        ssr::Serial s(filename.c_str(), baudrate);
        
        const size_t MAX_BUFFER = 256;
        uint8_t buffer[MAX_BUFFER];

        while (true) {
            int received_size;
            if ((received_size=s.getSizeInRxBuffer()) < 0) {
                // Error failed.
                std::cerr << "getSizeInRxBuffer failed." << std::endl;
                break;
            }
            if (received_size > 0) {
                if (received_size > MAX_BUFFER) {
                    received_size = MAX_BUFFER;
                }
                if (s.read(buffer, received_size) < 0) {
                    std::cerr << "read failed." << std::endl;
                    break;
                }

                if (socket.write(buffer, received_size) < 0) {
                    std::cerr << "send failed." << std::endl;
                    break;
                }
            }

            if ((received_size=socket.read(buffer, MAX_BUFFER)) < 0) {
                std::cerr << "recv failed." << std::endl;
                break;
            }
            if (received_size > 0) {
                if (s.write(buffer, received_size) < 0) {
                    std::cerr << "write failed." << std::endl;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

    }
    return 0;
}