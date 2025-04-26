/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          DgVoodoo2 PCI device definitions.
 *
 *
 *
 * Authors: [Your Name]
 *
 *          Copyright 2025 [Your Name]
 */

#ifndef DEVICE_PCI_DGVOODOO_H
#define DEVICE_PCI_DGVOODOO_H

/* DgVoodoo2 PCI/AGP device */
extern const device_t dgvoodoo2_device;

/* Individual GPU models */
extern const device_t dgvoodoo2_nvidia_geforce4_ti4800;
extern const device_t dgvoodoo2_nvidia_geforce_fx5700ultra;
extern const device_t dgvoodoo2_nvidia_geforce_9800gt;
extern const device_t dgvoodoo2_ati_radeon_8500;
extern const device_t dgvoodoo2_matrox_parhelia512;

/* Initialize the device */
extern void dgvoodoo2_device_init(void);

/* Register devices as video options */
extern void dgvoodoo2_device_register(void);

/* Set GPU model for emulation */
extern void dgvoodoo2_set_gpu(int vendor, int model);

/* Device availability check */
extern int dgvoodoo2_available(void);

/* Clean up DLLs function */
extern void dgvoodoo2_cleanup_dll(void);

#endif /* DEVICE_PCI_DGVOODOO_H */