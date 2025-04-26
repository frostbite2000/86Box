/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          DgVoodoo2 PCI/AGP device emulation.
 *
 *
 *
 * Authors: [Your Name]
 *
 *          Copyright 2025 [Your Name]
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#define HAVE_STDARG_H
#include <86box/86box.h>
#include <86box/device.h>
#include <86box/io.h>
#include <86box/mem.h>
#include <86box/pci.h>
#include <86box/rom.h>
#include <86box/plat.h>
#include <86box/video.h>
#include <86box/machine.h>
#include <86box/device/pci_dgvoodoo.h>
#include <86box/dgvoodoo2.h>

/* Current GPU device configuration */
static uint16_t dgvoodoo2_current_vendor_id = 0x10DE;  // Default: NVIDIA
static uint16_t dgvoodoo2_current_device_id = 0x0301;  // Default: GeForce4 Ti 4600
static uint8_t  dgvoodoo2_current_revision = 0xA1;
static uint8_t  dgvoodoo2_current_pci_class = 0x03;    // Display controller
static uint8_t  dgvoodoo2_current_pci_subclass = 0x00; // VGA-compatible controller
static uint8_t  dgvoodoo2_current_prog_if = 0x00;
static char     dgvoodoo2_current_name[128];
static int      dgvoodoo2_is_agp = 1;                  // Default use AGP
static int      dgvoodoo2_memory_size = 128;           // Default 128MB
static char     dgvoodoo2_bios_fn[128] = {0};          // BIOS ROM file

/* Base memory and I/O ranges */
static uint32_t dgvoodoo2_memory_base = 0xE0000000;   // 3.5GB mark
static uint32_t dgvoodoo2_mmio_base = 0xFD000000;     // 4GB - 48MB mark
static uint32_t dgvoodoo2_io_base = 0x3000;           // Default I/O base

/* PCI Device state */
typedef struct dgvoodoo2_t {
    uint8_t card_bus;
    uint8_t card_device;
    uint8_t pci_regs[256];
    uint8_t int_line;
    
    uint32_t memory_size;
    uint32_t memory_mapping;
    uint32_t mmio_mapping;
    
    rom_t bios_rom;
    
    uint8_t *vram;
} dgvoodoo2_t;

static void *dgvoodoo2_init(const device_t *info);
static void dgvoodoo2_close(void *priv);

/* GPU-specific initialization functions */
static void *geforce4_ti_init(const device_t *info)
{
    dgvoodoo2_set_gpu(DGVOODOO2_GPU_VENDOR_NVIDIA, DGVOODOO2_GPU_NVIDIA_GEFORCE4_TI4800);
    return dgvoodoo2_init(info);
}

static void *geforcefx_5700_init(const device_t *info)
{
    dgvoodoo2_set_gpu(DGVOODOO2_GPU_VENDOR_NVIDIA, DGVOODOO2_GPU_NVIDIA_GEFORCE_FX5700ULTRA);
    return dgvoodoo2_init(info);
}

static void *geforce_9800gt_init(const device_t *info)
{
    dgvoodoo2_set_gpu(DGVOODOO2_GPU_VENDOR_NVIDIA, DGVOODOO2_GPU_NVIDIA_GEFORCE_9800GT);
    return dgvoodoo2_init(info);
}

static void *radeon_8500_init(const device_t *info)
{
    dgvoodoo2_set_gpu(DGVOODOO2_GPU_VENDOR_ATI, DGVOODOO2_GPU_ATI_RADEON_8500);
    return dgvoodoo2_init(info);
}

static void *matrox_parhelia_init(const device_t *info)
{
    dgvoodoo2_set_gpu(DGVOODOO2_GPU_VENDOR_MATROX, DGVOODOO2_GPU_MATROX_PARHELIA512);
    return dgvoodoo2_init(info);
}

/* Define the actual video devices */
const device_t dgvoodoo2_nvidia_geforce4_ti4800 = {
    .name = "NVIDIA GeForce4 Ti 4800 (DgVoodoo2)",
    .internal_name = "nvidia_geforce4_ti4800_dgvoodoo2",
    .flags = DEVICE_PCI,
    .local = 0,
    .init = geforce4_ti_init,
    .close = dgvoodoo2_close,
    .reset = NULL,
    { .available = dgvoodoo2_available },
    .speed_changed = NULL,
    .force_redraw = NULL,
    .config = NULL
};

const device_t dgvoodoo2_nvidia_geforce_fx5700ultra = {
    .name = "NVIDIA GeForce FX 5700 Ultra (DgVoodoo2)",
    .internal_name = "nvidia_geforcefx_5700ultra_dgvoodoo2",
    .flags = DEVICE_PCI,
    .local = 0,
    .init = geforcefx_5700_init,
    .close = dgvoodoo2_close,
    .reset = NULL,
    { .available = dgvoodoo2_available },
    .speed_changed = NULL,
    .force_redraw = NULL,
    .config = NULL
};

const device_t dgvoodoo2_nvidia_geforce_9800gt = {
    .name = "NVIDIA GeForce 9800 GT (DgVoodoo2)",
    .internal_name = "nvidia_geforce_9800gt_dgvoodoo2",
    .flags = DEVICE_PCI,
    .local = 0,
    .init = geforce_9800gt_init,
    .close = dgvoodoo2_close,
    .reset = NULL,
    { .available = dgvoodoo2_available },
    .speed_changed = NULL,
    .force_redraw = NULL,
    .config = NULL
};

const device_t dgvoodoo2_ati_radeon_8500 = {
    .name = "ATI Radeon 8500 (DgVoodoo2)",
    .internal_name = "ati_radeon_8500_dgvoodoo2",
    .flags = DEVICE_PCI,
    .local = 0,
    .init = radeon_8500_init,
    .close = dgvoodoo2_close,
    .reset = NULL,
    { .available = dgvoodoo2_available },
    .speed_changed = NULL,
    .force_redraw = NULL,
    .config = NULL
};

const device_t dgvoodoo2_matrox_parhelia512 = {
    .name = "Matrox Parhelia-512 (DgVoodoo2)",
    .internal_name = "matrox_parhelia512_dgvoodoo2",
    .flags = DEVICE_PCI,
    .local = 0,
    .init = matrox_parhelia_init,
    .close = dgvoodoo2_close,
    .reset = NULL,
    { .available = dgvoodoo2_available },
    .speed_changed = NULL,
    .force_redraw = NULL,
    .config = NULL
};

/* PCI/AGP access functions */
static uint8_t
dgvoodoo2_pci_read(int func, int addr, void *priv)
{
    dgvoodoo2_t *dev = (dgvoodoo2_t *)priv;
    
    if (func > 0)
        return 0xff;
    
    return dev->pci_regs[addr];
}

static void
dgvoodoo2_pci_write(int func, int addr, uint8_t val, void *priv)
{
    dgvoodoo2_t *dev = (dgvoodoo2_t *)priv;
    
    if (func > 0)
        return;
    
    /* Immediately return on read-only registers */
    if ((addr < 4) || ((addr >= 8) && (addr < 0xc)) || ((addr >= 0x10) && (addr < 0x14)))
        return;
    
    switch (addr) {
        case 0x04: /* Command register */
            dev->pci_regs[addr] = val & 0x03; /* Only preserve memory and I/O space enable bits */
            break;
        
        case 0x10: case 0x11: case 0x12: case 0x13: /* Memory base */
            if (val)
                dev->pci_regs[addr] = val;
            break;
        
        case 0x30: case 0x31: case 0x32: case 0x33: /* BIOS ROM address */
            if (dev->bios_rom.rom != NULL) {
                if (addr == 0x30) {
                    dev->pci_regs[addr] = val & 0x01; /* Only preserve memory enable bit */
                } else if (addr == 0x31) {
                    dev->pci_regs[addr] = val & 0xfc; /* Preserve bits 31:14 */
                } else {
                    dev->pci_regs[addr] = val;
                }
            }
            break;
            
        case 0x3c: /* Interrupt line */
            dev->pci_regs[addr] = val;
            dev->int_line = val;
            break;
            
        default:
            dev->pci_regs[addr] = val;
            break;
    }
}

/* Configure GPU model based on vendor and model IDs */
void
dgvoodoo2_set_gpu(int vendor, int model)
{
    /* Set vendor ID */
    switch (vendor) {
        case DGVOODOO2_GPU_VENDOR_NVIDIA:
            dgvoodoo2_current_vendor_id = 0x10DE; /* NVIDIA */
            break;
        
        case DGVOODOO2_GPU_VENDOR_ATI:
            dgvoodoo2_current_vendor_id = 0x1002; /* ATI */
            break;
            
        case DGVOODOO2_GPU_VENDOR_MATROX:
            dgvoodoo2_current_vendor_id = 0x102B; /* Matrox */
            break;
            
        default:
            dgvoodoo2_current_vendor_id = 0x10DE; /* Default: NVIDIA */
            break;
    }
    
    /* Set device ID, memory size, and other properties based on the selected model */
    switch (vendor) {
        case DGVOODOO2_GPU_VENDOR_NVIDIA:
            switch (model) {
                case DGVOODOO2_GPU_NVIDIA_GEFORCE4_TI4800:
                    dgvoodoo2_current_device_id = 0x0301; /* GeForce4 Ti */
                    dgvoodoo2_current_revision = 0xA1;
                    dgvoodoo2_memory_size = 128;
                    strcpy(dgvoodoo2_bios_fn, "roms/video/nvidia/NV28.rom");
                    strcpy(dgvoodoo2_current_name, "NVIDIA GeForce4 Ti 4800");
                    break;
                    
                case DGVOODOO2_GPU_NVIDIA_GEFORCE_FX5700ULTRA:
                    dgvoodoo2_current_device_id = 0x0341; /* GeForce FX 5700 Ultra */
                    dgvoodoo2_current_revision = 0xA2;
                    dgvoodoo2_memory_size = 256;
                    strcpy(dgvoodoo2_bios_fn, "roms/video/nvidia/NV36.rom");
                    strcpy(dgvoodoo2_current_name, "NVIDIA GeForce FX 5700 Ultra");
                    break;
                    
                case DGVOODOO2_GPU_NVIDIA_GEFORCE_9800GT:
                    dgvoodoo2_current_device_id = 0x0614; /* GeForce 9800 GT */
                    dgvoodoo2_current_revision = 0xA3;
                    dgvoodoo2_memory_size = 512;
                    strcpy(dgvoodoo2_bios_fn, "roms/video/nvidia/G92.rom");
                    strcpy(dgvoodoo2_current_name, "NVIDIA GeForce 9800 GT");
                    break;
                    
                default:
                    dgvoodoo2_current_device_id = 0x0301;
                    dgvoodoo2_current_revision = 0xA1;
                    dgvoodoo2_memory_size = 128;
                    strcpy(dgvoodoo2_bios_fn, "roms/video/nvidia/NV28.rom");
                    strcpy(dgvoodoo2_current_name, "NVIDIA GeForce4 Ti 4800");
                    break;
            }
            break;
            
        case DGVOODOO2_GPU_VENDOR_ATI:
            dgvoodoo2_current_device_id = 0x514C; /* Radeon 8500 */
            dgvoodoo2_current_revision = 0x00;
            dgvoodoo2_memory_size = 128;
            strcpy(dgvoodoo2_bios_fn, "roms/video/ati/r200.bin");
            strcpy(dgvoodoo2_current_name, "ATI Radeon 8500");
            break;
            
        case DGVOODOO2_GPU_VENDOR_MATROX:
            dgvoodoo2_current_device_id = 0x0D80; /* Parhelia-512 */
            dgvoodoo2_current_revision = 0x00;
            dgvoodoo2_memory_size = 256;
            strcpy(dgvoodoo2_bios_fn, "roms/video/matrox/parhelia.bin");
            strcpy(dgvoodoo2_current_name, "Matrox Parhelia-512");
            break;
            
        default:
            dgvoodoo2_current_device_id = 0x0301; /* Default: GeForce4 Ti */
            dgvoodoo2_current_revision = 0xA1;
            dgvoodoo2_memory_size = 128;
            strcpy(dgvoodoo2_bios_fn, "roms/video/nvidia/NV28.rom");
            strcpy(dgvoodoo2_current_name, "NVIDIA GeForce4 Ti 4800");
            break;
    }
    
    /* Updated dgvoodoo2 config to match PCI device */
    dgvoodoo2_gpu_vendor = vendor;
    dgvoodoo2_gpu_model = model;
}

/* Memory mapped device functions */
static uint8_t
dgvoodoo2_read(uint32_t addr, void *priv)
{
    /* Memory reads from framebuffer/MMIO space */
    return 0xff;
}

static uint16_t
dgvoodoo2_readw(uint32_t addr, void *priv)
{
    return dgvoodoo2_read(addr, priv) | (dgvoodoo2_read(addr + 1, priv) << 8);
}

static uint32_t
dgvoodoo2_readl(uint32_t addr, void *priv)
{
    return dgvoodoo2_readw(addr, priv) | (dgvoodoo2_readw(addr + 2, priv) << 16);
}

static void
dgvoodoo2_write(uint32_t addr, uint8_t val, void *priv)
{
    /* Memory writes to framebuffer/MMIO space */
}

static void
dgvoodoo2_writew(uint32_t addr, uint16_t val, void *priv)
{
    dgvoodoo2_write(addr, val & 0xff, priv);
    dgvoodoo2_write(addr + 1, val >> 8, priv);
}

static void
dgvoodoo2_writel(uint32_t addr, uint32_t val, void *priv)
{
    dgvoodoo2_writew(addr, val & 0xffff, priv);
    dgvoodoo2_writew(addr + 2, val >> 16, priv);
}

/* Main device init/close functions */
static void *
dgvoodoo2_init(const device_t *info)
{
    dgvoodoo2_t *dev;
    
    /* Allocate device structure */
    dev = (dgvoodoo2_t *)malloc(sizeof(dgvoodoo2_t));
    memset(dev, 0, sizeof(dgvoodoo2_t));
    
    /* Set up PCI registers */
    dev->pci_regs[0x00] = dgvoodoo2_current_vendor_id & 0xff;         /* Vendor ID LSB */
    dev->pci_regs[0x01] = (dgvoodoo2_current_vendor_id >> 8) & 0xff;  /* Vendor ID MSB */
    dev->pci_regs[0x02] = dgvoodoo2_current_device_id & 0xff;         /* Device ID LSB */
    dev->pci_regs[0x03] = (dgvoodoo2_current_device_id >> 8) & 0xff;  /* Device ID MSB */
    
    dev->pci_regs[0x04] = 0x03;  /* Command: memory and I/O access enabled */
    dev->pci_regs[0x05] = 0x00;
    dev->pci_regs[0x06] = 0x80;  /* Status: has capability list */
    dev->pci_regs[0x07] = 0x02;  /* Fast back-to-back transactions supported */
    
    dev->pci_regs[0x08] = dgvoodoo2_current_revision;  /* Revision ID */
    dev->pci_regs[0x09] = 0x00;  /* Programming interface */
    dev->pci_regs[0x0a] = dgvoodoo2_current_pci_subclass;  /* Subclass */
    dev->pci_regs[0x0b] = dgvoodoo2_current_pci_class;     /* Class code */
    
    dev->pci_regs[0x0c] = 0x00;  /* Cache line size */
    dev->pci_regs[0x0d] = 0x00;  /* Latency timer */
    dev->pci_regs[0x0e] = 0x00;  /* Header type */
    dev->pci_regs[0x0f] = 0x00;  /* BIST */
    
    /* Memory BARs */
    dev->pci_regs[0x10] = 0x00;
    dev->pci_regs[0x11] = 0x00;
    dev->pci_regs[0x12] = 0x00;
    dev->pci_regs[0x13] = 0xf0;  /* Frame buffer memory: bottom 16 MB */
    
    dev->pci_regs[0x14] = 0x00;
    dev->pci_regs[0x15] = 0x00;
    dev->pci_regs[0x16] = 0xfd;
    dev->pci_regs[0x17] = 0xf0;  /* MMIO space: 16MB at 0xFD000000 */
    
    /* Other registers */
    dev->pci_regs[0x2c] = dgvoodoo2_current_vendor_id & 0xff;         /* Subsystem Vendor ID LSB */
    dev->pci_regs[0x2d] = (dgvoodoo2_current_vendor_id >> 8) & 0xff;  /* Subsystem Vendor ID MSB */
    dev->pci_regs[0x2e] = dgvoodoo2_current_device_id & 0xff;         /* Subsystem ID LSB */
    dev->pci_regs[0x2f] = (dgvoodoo2_current_device_id >> 8) & 0xff;  /* Subsystem ID MSB */
    
    /* Expansion ROM address */
    if (strlen(dgvoodoo2_bios_fn) > 0) {
        dev->pci_regs[0x30] = 0x00;
        dev->pci_regs[0x31] = 0x00;
        dev->pci_regs[0x32] = 0x00;
        dev->pci_regs[0x33] = 0xf0;
    } else {
        dev->pci_regs[0x30] = 0x00;
        dev->pci_regs[0x31] = 0x00;
        dev->pci_regs[0x32] = 0x00;
        dev->pci_regs[0x33] = 0x00;
    }
    
    /* Interrupt line */
    dev->pci_regs[0x3c] = 0x01;  /* INT A */
    dev->pci_regs[0x3d] = 0x01;  /* Interrupt pin 1 */
    dev->pci_regs[0x3e] = 0x00;  /* Min grant */
    dev->pci_regs[0x3f] = 0x00;  /* Max latency */
    
    /* Register PCI device */
    if (dgvoodoo2_use_agp && machine_has_bus(machine, MACHINE_BUS_AGP)) {
        /* AGP device */
        pci_add_card(PCI_ADD_AGP, dgvoodoo2_pci_read, dgvoodoo2_pci_write, dev, &dev->card_bus, &dev->card_device);
    } else {
        /* PCI device */
        pci_add_card(PCI_ADD_VIDEO, dgvoodoo2_pci_read, dgvoodoo2_pci_write, dev, &dev->card_bus, &dev->card_device);
    }
    
    /* Map framebuffer memory */
    mem_mapping_add(&dev->memory_mapping,
                   dgvoodoo2_memory_base, dgvoodoo2_memory_size * 1024 * 1024,
                   dgvoodoo2_read, dgvoodoo2_readw, dgvoodoo2_readl,
                   dgvoodoo2_write, dgvoodoo2_writew, dgvoodoo2_writel,
                   NULL, MEM_MAPPING_EXTERNAL,
                   dev);
    
    /* Map MMIO space */
    mem_mapping_add(&dev->mmio_mapping,
                   dgvoodoo2_mmio_base, 16 * 1024 * 1024,
                   dgvoodoo2_read, dgvoodoo2_readw, dgvoodoo2_readl,
                   dgvoodoo2_write, dgvoodoo2_writew, dgvoodoo2_writel,
                   NULL, MEM_MAPPING_EXTERNAL,
                   dev);
    
    /* Load VBIOS if available */
    if (strlen(dgvoodoo2_bios_fn) > 0) {
        rom_init(&dev->bios_rom, dgvoodoo2_bios_fn, 0xc0000, 0x8000, 0x7fff, 0, MEM_MAPPING_EXTERNAL);
        mem_mapping_enable(&dev->bios_rom.mapping);
    }
    
    /* Enable DgVoodoo2 wrapper by copying DLLs */
    dgvoodoo2_init();
    
    return dev;
}

static void
dgvoodoo2_close(void *priv)
{
    dgvoodoo2_t *dev = (dgvoodoo2_t *)priv;
    
    if (!dev)
        return;
    
    /* Unmap memory regions */
    mem_mapping_remove(&dev->memory_mapping);
    mem_mapping_remove(&dev->mmio_mapping);
    
    /* Free ROM if loaded */
    if (dev->bios_rom.rom != NULL) {
        free(dev->bios_rom.rom);
        dev->bios_rom.rom = NULL;
    }
    
    /* Clean up DLLs */
    dgvoodoo2_cleanup_dll();
    
    /* Free device structure */
    free(dev);
}

/* Add video capability to devices */
int
dgvoodoo2_available(void)
{
    return dgvoodoo2_enabled;
}

/* Device detection function */
void
dgvoodoo2_device_init(void)
{
    /* Load configuration */
    dgvoodoo2_load_config();
    
    /* Set GPU model parameters */
    dgvoodoo2_set_gpu(dgvoodoo2_gpu_vendor, dgvoodoo2_gpu_model);
}

/* Register DgVoodoo2 as a selectable video option */
void
dgvoodoo2_device_register(void)
{
    /* Add each DgVoodoo2 device to the list of available video devices */
    device_add_option(&dgvoodoo2_nvidia_geforce4_ti4800, "video");
    device_add_option(&dgvoodoo2_nvidia_geforce_fx5700ultra, "video");
    device_add_option(&dgvoodoo2_nvidia_geforce_9800gt, "video");
    device_add_option(&dgvoodoo2_ati_radeon_8500, "video");
    device_add_option(&dgvoodoo2_matrox_parhelia512, "video");
}