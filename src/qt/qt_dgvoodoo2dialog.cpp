/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          DgVoodoo2 configuration UI module.
 *
 *
 *
 * Authors: [Your Name]
 *
 *          Copyright 2025 [Your Name]
 */
#include "qt_dgvoodoo2dialog.hpp"
#include "ui_qt_dgvoodoo2dialog.h"
#include "qt_models_common.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

extern "C" {
#include <86box/86box.h>
#include <86box/config.h>
#include <86box/device.h>
#include <86box/dgvoodoo2.h>
#include <86box/device/pci_dgvoodoo.h>
#include <86box/machine.h>
}

DgVoodoo2Dialog::DgVoodoo2Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DgVoodoo2Dialog)
{
    ui->setupUi(this);

    // Initialize GPU Vendors
    auto model = ui->comboBoxGPUVendor->model();
    Models::AddEntry(model, tr("NVIDIA"), DGVOODOO2_GPU_VENDOR_NVIDIA);
    Models::AddEntry(model, tr("ATI/AMD"), DGVOODOO2_GPU_VENDOR_ATI);
    Models::AddEntry(model, tr("Matrox"), DGVOODOO2_GPU_VENDOR_MATROX);

    // Set current GPU vendor and model based on config
    ui->comboBoxGPUVendor->setCurrentIndex(dgvoodoo2_gpu_vendor);
    populateGPUModels(dgvoodoo2_gpu_vendor);
    
    // Initialize wrapper DLL checkboxes
    ui->checkBoxDDraw->setChecked(dgvoodoo2_ddraw_enabled);
    ui->checkBoxD3DImm->setChecked(dgvoodoo2_d3dimm_enabled);
    ui->checkBoxD3D8->setChecked(dgvoodoo2_d3d8_enabled);
    ui->checkBoxD3D9->setChecked(dgvoodoo2_d3d9_enabled);
    
    // Path to dgvoodoo2 directory
    ui->lineEditPath->setText(QString::fromUtf8(dgvoodoo2_path));
    
    // Set bus type
    ui->radioButtonAGP->setChecked(dgvoodoo2_use_agp);
    ui->radioButtonPCI->setChecked(!dgvoodoo2_use_agp);
    
    // Check if machine has AGP support
    bool has_agp = machine_has_bus(machine, MACHINE_BUS_AGP) > 0;
    ui->radioButtonAGP->setEnabled(has_agp);
    if (!has_agp && dgvoodoo2_use_agp) {
        ui->radioButtonPCI->setChecked(true);
        ui->radioButtonAGP->setChecked(false);
        ui->labelWarning->setText(tr("Note: Current machine doesn't support AGP, using PCI instead."));
    } else {
        ui->labelWarning->setText("");
    }
    
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    
    // Connect browse button signal
    connect(ui->pushButtonBrowse, &QPushButton::clicked, this, &DgVoodoo2Dialog::onBrowseClicked);
    
    // Connect GPU vendor combobox to update model list
    connect(ui->comboBoxGPUVendor, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DgVoodoo2Dialog::onGPUVendorChanged);
}

DgVoodoo2Dialog::~DgVoodoo2Dialog()
{
    delete ui;
}

void DgVoodoo2Dialog::populateGPUModels(int vendorIndex)
{
    ui->comboBoxGPUModel->clear();
    auto model = ui->comboBoxGPUModel->model();
    
    if (vendorIndex == DGVOODOO2_GPU_VENDOR_NVIDIA) {
        // NVIDIA GPU models
        Models::AddEntry(model, tr("GeForce4 TI 4800"), DGVOODOO2_GPU_NVIDIA_GEFORCE4_TI4800);
        Models::AddEntry(model, tr("GeForce FX 5700 Ultra"), DGVOODOO2_GPU_NVIDIA_GEFORCE_FX5700ULTRA);
        Models::AddEntry(model, tr("GeForce 9800 GT"), DGVOODOO2_GPU_NVIDIA_GEFORCE_9800GT);
    } else if (vendorIndex == DGVOODOO2_GPU_VENDOR_ATI) {
        // ATI/AMD GPU models
        Models::AddEntry(model, tr("Radeon 8500"), DGVOODOO2_GPU_ATI_RADEON_8500);
    } else if (vendorIndex == DGVOODOO2_GPU_VENDOR_MATROX) {
        // Matrox GPU models
        Models::AddEntry(model, tr("Parhelia-512"), DGVOODOO2_GPU_MATROX_PARHELIA512);
    }
    
    // Set the current model based on config
    for (int i = 0; i < ui->comboBoxGPUModel->count(); i++) {
        if (ui->comboBoxGPUModel->itemData(i).toInt() == dgvoodoo2_gpu_model) {
            ui->comboBoxGPUModel->setCurrentIndex(i);
            break;
        }
    }
}

void DgVoodoo2Dialog::onGPUVendorChanged(int index)
{
    populateGPUModels(ui->comboBoxGPUVendor->currentData().toInt());
}

void DgVoodoo2Dialog::onBrowseClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select DgVoodoo2 Directory"),
                                                   ui->lineEditPath->text(),
                                                   QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->lineEditPath->setText(dir);
        
        // Check if this path contains the necessary DLLs
        QDir dgvDir(dir);
        bool hasDDraw = dgvDir.exists("DDraw.dll");
        bool hasD3DImm = dgvDir.exists("D3DImm.dll");
        bool hasD3D8 = dgvDir.exists("D3D8.dll");
        bool hasD3D9 = dgvDir.exists("D3D9.dll");
        
        if (!(hasDDraw || hasD3DImm || hasD3D8 || hasD3D9)) {
            QMessageBox::warning(this, tr("Missing DLLs"),
                                tr("The selected directory doesn't contain any DgVoodoo2 wrapper DLLs (DDraw.dll, D3DImm.dll, D3D8.dll, D3D9.dll).\n\nPlease verify that you selected the correct DgVoodoo2 directory."));
        } else {
            // Update checkboxes based on what's available
            ui->checkBoxDDraw->setEnabled(hasDDraw);
            ui->checkBoxDDraw->setChecked(hasDDraw && dgvoodoo2_ddraw_enabled);
            
            ui->checkBoxD3DImm->setEnabled(hasD3DImm);
            ui->checkBoxD3DImm->setChecked(hasD3DImm && dgvoodoo2_d3dimm_enabled);
            
            ui->checkBoxD3D8->setEnabled(hasD3D8);
            ui->checkBoxD3D8->setChecked(hasD3D8 && dgvoodoo2_d3d8_enabled);
            
            ui->checkBoxD3D9->setEnabled(hasD3D9);
            ui->checkBoxD3D9->setChecked(hasD3D9 && dgvoodoo2_d3d9_enabled);
        }
    }
}

void DgVoodoo2Dialog::accept()
{
    // Validate path if DgVoodoo2 is enabled
    if (ui->lineEditPath->text().isEmpty()) {
        QMessageBox::warning(this, tr("Missing Path"),
                           tr("Please specify the path to the DgVoodoo2 directory containing the wrapper DLLs."));
        return;
    }
    
    save();
    QDialog::accept();
}

void DgVoodoo2Dialog::save()
{
    dgvoodoo2_gpu_vendor = ui->comboBoxGPUVendor->currentData().toInt();
    dgvoodoo2_gpu_model = ui->comboBoxGPUModel->currentData().toInt();
    dgvoodoo2_ddraw_enabled = ui->checkBoxDDraw->isChecked();
    dgvoodoo2_d3dimm_enabled = ui->checkBoxD3DImm->isChecked();
    dgvoodoo2_d3d8_enabled = ui->checkBoxD3D8->isChecked();
    dgvoodoo2_d3d9_enabled = ui->checkBoxD3D9->isChecked();
    dgvoodoo2_use_agp = ui->radioButtonAGP->isChecked();
    
    // If AGP is selected but not supported, force PCI
    if (dgvoodoo2_use_agp && !(machine_has_bus(machine, MACHINE_BUS_AGP) > 0)) {
        dgvoodoo2_use_agp = 0;
    }
    
    strncpy(dgvoodoo2_path, ui->lineEditPath->text().toUtf8().constData(), sizeof(dgvoodoo2_path) - 1);
    dgvoodoo2_path[sizeof(dgvoodoo2_path) - 1] = '\0';
    
    dgvoodoo2_save();
}