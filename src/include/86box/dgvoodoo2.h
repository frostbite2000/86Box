/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          DgVoodoo2 wrapper for DirectDraw/Direct3D.
 *
 *
 *
 * Authors: [Your Name]
 *
 *          Copyright 2025 [Your Name]
 */

#ifndef DGVOODOO2_H
#define DGVOODOO2_H

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPU Vendor IDs */
#define DGVOODOO2_GPU_VENDOR_NVIDIA 0
#define DGVOODOO2_GPU_VENDOR_ATI 1
#define DGVOODOO2_GPU_VENDOR_MATROX 2

/* NVIDIA GPU Models */
#define DGVOODOO2_GPU_NVIDIA_GEFORCE4_TI4800 0
#define DGVOODOO2_GPU_NVIDIA_GEFORCE_FX5700ULTRA 1
#define DGVOODOO2_GPU_NVIDIA_GEFORCE_9800GT 2

/* ATI GPU Models */
#define DGVOODOO2_GPU_ATI_RADEON_8500 0

/* Matrox GPU Models */
#define DGVOODOO2_GPU_MATROX_PARHELIA512 0

/* Global variables for DgVoodoo2 configuration */
extern int dgvoodoo2_enabled;
extern int dgvoodoo2_gpu_vendor;
extern int dgvoodoo2_gpu_model;
extern int dgvoodoo2_ddraw_enabled;
extern int dgvoodoo2_d3dimm_enabled;
extern int dgvoodoo2_d3d8_enabled;
extern int dgvoodoo2_d3d9_enabled;
extern int dgvoodoo2_use_agp;  /* Whether to use AGP (1) or PCI (0) */
extern char dgvoodoo2_path[512];

/* Initialize DgVoodoo2 wrapper */
extern void dgvoodoo2_init(void);

/* Close DgVoodoo2 wrapper */
extern void dgvoodoo2_close(void);

/* Save/Load DgVoodoo2 configuration */
extern void dgvoodoo2_save(void);
extern void dgvoodoo2_load_config(void);

extern void dgvoodoo2_module_init(void);

/* Get device information */
extern const char* dgvoodoo2_get_card_name(int vendor, int model);

#ifdef __cplusplus
}
#endif

#endif /* DGVOODOO2_H */