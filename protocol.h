#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

// Radiotap Header
typedef struct {
    uint8_t version;
    uint8_t pad;
    uint16_t len;
    uint32_t present;
} __attribute__((packed)) radiotap_header_t;

// 802.11 Frame Header
typedef struct {
    uint8_t frame_control[2];       
    uint16_t duration;              
    uint8_t receiver_address[6];
    uint8_t transmitter_address[6];
    uint8_t bssid[6];
    uint16_t sequence_control;      
} __attribute__((packed)) dot11_header_t;

// Beacon Fixed Parameters
typedef struct {
    uint8_t timestamp[8];
    uint16_t beacon_interval;
    uint16_t capability_info;
} __attribute__((packed)) beacon_fixed_params_t;

// Deauthentication Body
typedef struct {
    uint16_t reason_code;
} __attribute__((packed)) deauth_body_t;

#endif
