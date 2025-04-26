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
#ifndef QT_DGVOODOO2DIALOG_HPP
#define QT_DGVOODOO2DIALOG_HPP

#include <QDialog>
#include <QComboBox>

namespace Ui {
class DgVoodoo2Dialog;
}

class DgVoodoo2Dialog : public QDialog {
    Q_OBJECT

public:
    explicit DgVoodoo2Dialog(QWidget *parent = nullptr);
    ~DgVoodoo2Dialog();

    void save();

protected:
    void accept() override;

private slots:
    void onGPUVendorChanged(int index);
    void onBrowseClicked();

private:
    Ui::DgVoodoo2Dialog *ui;
    void populateGPUModels(int vendorIndex);
};

#endif // QT_DGVOODOO2DIALOG_HPP