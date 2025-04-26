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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#define HAVE_STDARG_H
#include <86box/86box.h>
#include <86box/device.h>
#include <86box/plat.h>
#include <86box/path.h>
#include <86box/config.h>
#include <86box/dgvoodoo2.h>
#include <86box/device/pci_dgvoodoo.h>

/* Global variables for DgVoodoo2 configuration */
int dgvoodoo2_enabled = 0;
int dgvoodoo2_gpu_vendor = DGVOODOO2_GPU_VENDOR_NVIDIA;
int dgvoodoo2_gpu_model = DGVOODOO2_GPU_NVIDIA_GEFORCE4_TI4800;
int dgvoodoo2_ddraw_enabled = 1;
int dgvoodoo2_d3dimm_enabled = 1;
int dgvoodoo2_d3d8_enabled = 1;
int dgvoodoo2_d3d9_enabled = 1;
int dgvoodoo2_use_agp = 1;  /* Default to AGP if supported */
char dgvoodoo2_path[512] = "";

static const char *dgvoodoo2_files[] = {
    "DDraw.dll",
    "D3DImm.dll",
    "D3D8.dll",
    "D3D9.dll"
};

/* Card name lookup tables */
static const char *dgvoodoo2_nvidia_names[] = {
    "NVIDIA GeForce4 Ti 4800",
    "NVIDIA GeForce FX 5700 Ultra",
    "NVIDIA GeForce 9800 GT"
};

static const char *dgvoodoo2_ati_names[] = {
    "ATI Radeon 8500"
};

static const char *dgvoodoo2_matrox_names[] = {
    "Matrox Parhelia-512"
};

void
dgvoodoo2_load_config(void)
{
    dgvoodoo2_enabled = config_get_int("DgVoodoo2", "enabled", 0);
    dgvoodoo2_gpu_vendor = config_get_int("DgVoodoo2", "gpu_vendor", DGVOODOO2_GPU_VENDOR_NVIDIA);
    dgvoodoo2_gpu_model = config_get_int("DgVoodoo2", "gpu_model", DGVOODOO2_GPU_NVIDIA_GEFORCE4_TI4800);
    dgvoodoo2_ddraw_enabled = config_get_int("DgVoodoo2", "ddraw_enabled", 1);
    dgvoodoo2_d3dimm_enabled = config_get_int("DgVoodoo2", "d3dimm_enabled", 1);
    dgvoodoo2_d3d8_enabled = config_get_int("DgVoodoo2", "d3d8_enabled", 1);
    dgvoodoo2_d3d9_enabled = config_get_int("DgVoodoo2", "d3d9_enabled", 1);
    dgvoodoo2_use_agp = config_get_int("DgVoodoo2", "use_agp", 1);
    config_get_string("DgVoodoo2", "path", "", dgvoodoo2_path, sizeof(dgvoodoo2_path));
}

void
dgvoodoo2_save(void)
{
    config_set_int("DgVoodoo2", "enabled", dgvoodoo2_enabled);
    config_set_int("DgVoodoo2", "gpu_vendor", dgvoodoo2_gpu_vendor);
    config_set_int("DgVoodoo2", "gpu_model", dgvoodoo2_gpu_model);
    config_set_int("DgVoodoo2", "ddraw_enabled", dgvoodoo2_ddraw_enabled);
    config_set_int("DgVoodoo2", "d3dimm_enabled", dgvoodoo2_d3dimm_enabled);
    config_set_int("DgVoodoo2", "d3d8_enabled", dgvoodoo2_d3d8_enabled);
    config_set_int("DgVoodoo2", "d3d9_enabled", dgvoodoo2_d3d9_enabled);
    config_set_int("DgVoodoo2", "use_agp", dgvoodoo2_use_agp);
    config_set_string("DgVoodoo2", "path", dgvoodoo2_path);
}

const char*
dgvoodoo2_get_card_name(int vendor, int model)
{
    switch (vendor) {
        case DGVOODOO2_GPU_VENDOR_NVIDIA:
            if (model < (sizeof(dgvoodoo2_nvidia_names) / sizeof(char*)))
                return dgvoodoo2_nvidia_names[model];
            break;
        case DGVOODOO2_GPU_VENDOR_ATI:
            if (model < (sizeof(dgvoodoo2_ati_names) / sizeof(char*)))
                return dgvoodoo2_ati_names[model];
            break;
        case DGVOODOO2_GPU_VENDOR_MATROX:
            if (model < (sizeof(dgvoodoo2_matrox_names) / sizeof(char*)))
                return dgvoodoo2_matrox_names[model];
            break;
    }
    
    return "Unknown GPU";
}

static bool
dgvoodoo2_copy_dll(const char *filename)
{
    char src_path[1024];
    char dest_path[1024];
    
    /* Build source path */
    memset(src_path, 0, sizeof(src_path));
    snprintf(src_path, sizeof(src_path) - 1, "%s/%s", dgvoodoo2_path, filename);
    
    /* Build destination path (current directory) */
    memset(dest_path, 0, sizeof(dest_path));
    path_get_filename(dest_path, filename);
    
    /* Check if source file exists */
    if (plat_path_exists(src_path)) {
        /* Copy the file to the executable directory */
        return plat_copy_file(src_path, dest_path);
    }
    
    return false;
}

void
dgvoodoo2_cleanup_dll(void)
{
    char filepath[512];
    
    /* Remove all DgVoodoo2 DLLs from the current directory */
    for (int i = 0; i < sizeof(dgvoodoo2_files) / sizeof(dgvoodoo2_files[0]); i++) {
        memset(filepath, 0, sizeof(filepath));
        path_get_filename(filepath, dgvoodoo2_files[i]);
        
        if (plat_path_exists(filepath))
            plat_remove_file(filepath);
    }
}

void
dgvoodoo2_init(void)
{
    /* If already called, don't copy DLLs again */
    static int initialized = 0;
    
    if (initialized)
        return;
    
    /* Mark as initialized */
    initialized = 1;
    
    /* Clean up any existing DLLs */
    dgvoodoo2_cleanup_dll();
    
    /* Copy DLLs if enabled */
    if (dgvoodoo2_enabled && dgvoodoo2_path[0]) {
        if (dgvoodoo2_ddraw_enabled)
            dgvoodoo2_copy_dll(dgvoodoo2_files[0]);
        
        if (dgvoodoo2_d3dimm_enabled)
            dgvoodoo2_copy_dll(dgvoodoo2_files[1]);
        
        if (dgvoodoo2_d3d8_enabled)
            dgvoodoo2_copy_dll(dgvoodoo2_files[2]);
        
        if (dgvoodoo2_d3d9_enabled)
            dgvoodoo2_copy_dll(dgvoodoo2_files[3]);
    }
}

void
dgvoodoo2_close(void)
{
    /* Clean up DLLs */
    dgvoodoo2_cleanup_dll();
}

/*
 * Unified DgVoodoo2 module initialization
 */
void
dgvoodoo2_module_init(void)
{
    /* Load DgVoodoo2 configuration */
    dgvoodoo2_load_config();
    
    /* Initialize device specifics */
    if (dgvoodoo2_enabled) {
        dgvoodoo2_device_init();
        dgvoodoo2_device_register();
    }
}