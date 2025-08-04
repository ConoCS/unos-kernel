#include <unoskrnl.h>

static inline void outl(uint16_t port, uint32_t val) {
    asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint32_t pci_config_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    return (1U << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
}

uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = pci_config_address(bus, device, function, offset);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

uint32_t pci_read_bar(uint8_t bus, uint8_t device, uint8_t function, uint8_t bar_index) {
    if (bar_index > 5) return 0;
    return pci_read(bus, device, function, 0x10 + (bar_index * 4));
}

void pci_scan() {
    for(uint16_t bus = 0; bus < 256; bus++) {
        for(uint8_t device = 0; device < 32; device++) {
            for(uint8_t function = 0; function < 8; function++) {
                uint32_t vendor_device = pci_read(bus, device, function, 0x00);
                if((vendor_device & 0xFFFF) == 0xFFFF) continue;

                uint16_t vendor = vendor_device & 0xFFFF;
                uint16_t device_id = vendor_device >> 16;

                serial_printf("PCI Device Found: Bus %d, Device %d, Func %d: Vendor=%X, Device=%X\n", bus, device, function, vendor, device_id);

                uint32_t class_info = pci_read(bus, device, function, 0x08);
                uint8_t class_code = (class_info >> 24) & 0xFF;
                uint8_t subclass = (class_info >> 16) & 0xFF;
                uint8_t prog_if = (class_info >> 8) & 0xFF;

                if (class_code == 0x01 && subclass == 0x06) {
                    serial_printf("-> This is an AHCI Controller\n");

                    uint32_t bar5 = pci_read_bar(bus, device, function, 5);
                    serial_printf("BAR5 (ABAR) Address: %X", bar5);
                    ahci_init((void*)(bar5 & ~0x0F));
                }
            }
        }
    }
}