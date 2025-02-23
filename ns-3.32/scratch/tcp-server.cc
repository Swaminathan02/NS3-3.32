#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <cmath>

#define PORT 8082
#define BUFFER_SIZE 1024
#define NUM_PACKETS 1000
#define PACKET_LOSS_RATE 1

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void server_function(int& packets_received) {
    int sockfd, newsockfd, n;
    socklen_t clilen;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) {
        error("ERROR on accept");
    }

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        n = read(newsockfd, buffer, BUFFER_SIZE - 1);
        if (n < 0) {
            error("ERROR reading from socket");
        } else if (n == 0) {
            // Client has closed the connection
            break;
        }

        packets_received++;
        std::cout << "Packet received: " << buffer << std::endl;

        n = write(newsockfd, "ACK", 3);
        if (n < 0) {
            error("ERROR writing to socket");
        }
    }

    close(newsockfd);
    close(sockfd);
}

double calculate_rto(double rtt) {
    static double srtt = 0;
    static double mdev = 0;
    static bool initialized = false;

    if (!initialized) {
        srtt = rtt;
        mdev = rtt / 2;
        initialized = true;
        return srtt + 4 * mdev;
    }

    double err = rtt - srtt;
    srtt += 0.125 * err;
    mdev += 0.25 * (std::fabs(err) - mdev);

    double rto = srtt + 4 * mdev;
    return rto;
}

double adjust_rto(double rto, int retransmissions) {
    if (retransmissions > 2 && rto > 15.0) {
        double random_value = static_cast<double>(std::rand()) / RAND_MAX; // Random value between 0 and 1
        if (random_value < 0.5) {
            rto -= random_value;
        }
    }
    return rto;
}

int main() {
    // Initialize random seed
    std::srand(std::time(0));

    int packets_received = 0;

    // Start the server in a separate thread
    std::thread server_thread(server_function, std::ref(packets_received));
    
    // Give the server some time to start
    sleep(1);

    // Client code
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    int packets_sent = 0, packets_lost = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    server = gethostbyname("localhost");
    if (server == NULL) {
        error("ERROR, no such host");
    }

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    double total_data_sent = 0.0;
    double total_data_received = 0.0;

    double rto = 1.0; // Initial RTO
    int retransmissions = 0; // Initialize retransmission count

    for (int i = 0; i < NUM_PACKETS; ++i) {
        bool packet_sent = false;
        auto packet_start_time = std::chrono::high_resolution_clock::now();
        do {
            // Send packet
            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, BUFFER_SIZE, "Packet %d", i + 1);

            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0) {
                error("ERROR writing to socket");
            }
            total_data_sent += strlen(buffer);

            // Wait for ACK or timeout
            memset(buffer, 0, BUFFER_SIZE);
            n = read(sockfd, buffer, BUFFER_SIZE - 1);
            if (n < 0) {
                error("ERROR reading from socket");
            } else if (n == 0) {
                // Server has closed the connection
                packets_lost++;
            } else {
                auto packet_end_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> rtt_duration = packet_end_time - packet_start_time;
                double rtt = rtt_duration.count();
                rto = calculate_rto(rtt);
                rto = adjust_rto(rto, retransmissions); // Apply the proposed scheme
                std::cout << "Server reply: " << buffer << std::endl;
                total_data_received += strlen(buffer);
                packet_sent = true; // Packet successfully sent
                retransmissions = 0; // Reset retransmission count
            }

            auto packet_end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> packet_elapsed = packet_end_time - packet_start_time;
            if (packet_elapsed.count() >= rto) {
                //std::cout << "Timeout occurred for Packet " << i + 1 << ", retransmitting..." << std::endl;
                packets_lost++; // Treat as lost due to timeout
                retransmissions++; // Increment retransmission count
                break;
            }
        } while (!packet_sent);
        packets_sent++;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    double throughput = (total_data_received * 8) / (elapsed.count() * 1e6); // in Mbps
    double packet_loss_ratio = (double)packets_lost / packets_sent;
    double overhead_efficiency = (total_data_sent > 0) ? (total_data_received / total_data_sent) * 100.0 : 0.0;

    // Energy consumption estimation (simplified)
    double energy_consumed = (total_data_sent + total_data_received) * 0.0001; // Arbitrary conversion factor

    std::cout << "\n--- TCP Server Performance Metrics ---\n";
    std::cout << "Total Packets Sent: " << packets_sent << "\n";
    std::cout << "Total Packets Received: " << packets_received << "\n";
    std::cout << "Total Packets Lost: " << packets_lost << "\n";
    std::cout << "Packet Loss Ratio: " << packet_loss_ratio * 100 << " %\n";
    std::cout << "Throughput: " << throughput << " Mbps\n";
    std::cout << "Total Energy Consumed: " << energy_consumed << " J\n";
    std::cout << "Overhead Efficiency: " << overhead_efficiency << " %\n";
    close(sockfd);

    // Wait for the server thread to finish
    server_thread.join();

    return 0;
}
