#include <stdint.h>
#include "../../core/kprintf.h"

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

#define RX_BUF_SIZE 8192
#define TX_BUF_SIZE 4096
#define NUM_TX_DESC 4

typedef struct {
    uint8_t mac_addr[6];
    uint8_t* rx_buffer;
    uint8_t* tx_buffer[NUM_TX_DESC];
    uint32_t tx_current;
    uint32_t rx_current;
    uint16_t io_base;
} rtl8139_dev_t;

static rtl8139_dev_t rtl8139_device;

void rtl8139_init(void) {
    kprintf("Initializing RTL8139 network adapter...\n");
    
    // TODO: Detect PCI device
    // TODO: Enable bus mastering
    // TODO: Reset device
    
    // Allocate receive buffer
    rtl8139_device.rx_buffer = (uint8_t*)kmalloc(RX_BUF_SIZE);
    if (!rtl8139_device.rx_buffer) {
        kprintf("Failed to allocate RX buffer\n");
        return;
    }
    
    // Allocate transmit buffers
    for (int i = 0; i < NUM_TX_DESC; i++) {
        rtl8139_device.tx_buffer[i] = (uint8_t*)kmalloc(TX_BUF_SIZE);
        if (!rtl8139_device.tx_buffer[i]) {
            kprintf("Failed to allocate TX buffer %d\n", i);
            return;
        }
    }
    
    rtl8139_device.tx_current = 0;
    rtl8139_device.rx_current = 0;
    
    kprintf("RTL8139 initialized\n");
}

int rtl8139_send_packet(const void* data, size_t length) {
    if (length > TX_BUF_SIZE) {
        return -1;
    }
    
    uint32_t tx_desc = rtl8139_device.tx_current;
    
    // Copy data to transmit buffer
    memcpy(rtl8139_device.tx_buffer[tx_desc], data, length);
    
    // TODO: Write to transmit registers
    // TODO: Update transmit status
    
    rtl8139_device.tx_current = (tx_desc + 1) % NUM_TX_DESC;
    
    return 0;
}

int rtl8139_receive_packet(void* buffer, size_t max_length) {
    // TODO: Check receive status
    // TODO: Copy data from receive buffer
    // TODO: Update receive pointer
    
    return 0;
}