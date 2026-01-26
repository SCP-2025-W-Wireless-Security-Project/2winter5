#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"
#include <time.h>

void usage() {
    printf("sudo ./sniff <interface>\n");
}

time_t last_alert_time = 0;

void send_alert(const char* level, const char* message, const char* source_mac, const char* target_mac) {
    time_t now = time(NULL);
    if (now - last_alert_time < 2) { 
        return;
    }
    last_alert_time = now;

    char command[1024];
    snprintf(command, sizeof(command),
        "curl -X POST http://localhost:3000/api/logs "
        "-H 'Content-Type: application/json' "
        "-d '{\"level\": \"%s\", \"message\": \"%s\", \"source_mac\": \"%s\", \"details\": \"Target: %s, Reason: Deauth(7)\"}' "
        "> /dev/null 2>&1 &",
        level, message, source_mac, target_mac
    );
    system(command);
    printf("[!] Alert Sent: %s -> %s\n", source_mac, target_mac);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        usage();
        return -1;
    }

    char* dev = argv[1];
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "couldn't open device %s | %s\n", dev, errbuf);
        return -1;
    }

    printf("[*] Sniffer running on %s...\n", dev);

    while (1) {
        struct pcap_pkthdr* header;
        const u_char* packet;
        int res = pcap_next_ex(handle, &header, &packet);
        if (res == 0) continue; 
        if (res == -1 || res == -2) break; 

        radiotap_header_t* radiotap = (radiotap_header_t*)packet; 
        if (header->len < sizeof(radiotap_header_t) || header->len < radiotap->len) continue; 
        
        int offset = radiotap->len; 
        const u_char* payload = packet + offset; 
        int payload_len = header->len - offset; 

        if (payload_len < sizeof(dot11_header_t)) continue; 
        dot11_header_t* dot11 = (dot11_header_t*)payload; 

        if (dot11->frame_control[0] == 0xC0) {
            char source_mac[18];
            char target_mac[18];

            snprintf(source_mac, sizeof(source_mac), "%02X:%02X:%02X:%02X:%02X:%02X",
                dot11->transmitter_address[0], dot11->transmitter_address[1], dot11->transmitter_address[2],
                dot11->transmitter_address[3], dot11->transmitter_address[4], dot11->transmitter_address[5]);

            snprintf(target_mac, sizeof(target_mac), "%02X:%02X:%02X:%02X:%02X:%02X",
                dot11->receiver_address[0], dot11->receiver_address[1], dot11->receiver_address[2],
                dot11->receiver_address[3], dot11->receiver_address[4], dot11->receiver_address[5]);

            printf("[DETECTED] Deauth: %s -> %s\n", source_mac, target_mac);
            
            send_alert("CRITICAL", "Deauth Attack Detected", source_mac, target_mac);

            continue;
        }

        if (dot11->frame_control[0] != 0x80) continue; 

        printf("BSSID: %02X:%02X:%02X:%02X:%02X:%02X\t", 
            dot11->bssid[0], dot11->bssid[1], dot11->bssid[2],
            dot11->bssid[3], dot11->bssid[4], dot11->bssid[5]
        );
        
        int fixed_param_len = sizeof(beacon_fixed_params_t);
        if (payload_len < sizeof(dot11_header_t) + fixed_param_len) continue;

        const u_char* tag_ptr = payload + sizeof(dot11_header_t) + fixed_param_len;
        int tag_len = payload_len - (sizeof(dot11_header_t) + fixed_param_len);

        while (tag_len >= 2) {
            uint8_t tag_num = tag_ptr[0];
            uint8_t tag_value_len = tag_ptr[1];

            if (tag_len < 2 + tag_value_len) break;

            if (tag_num == 0) {
                printf("SSID: ");
                for (int i = 0; i < tag_value_len; i++) {
                    printf("%c", tag_ptr[2 + i]);
                }
                printf("\n");
                break;
            }

            tag_ptr += 2 + tag_value_len;
            tag_len -= 2 + tag_value_len;
        }
    }

    pcap_close(handle);

    return 0;
}
