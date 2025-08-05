/**
 *  Hak Cipta Dilindungi (c) 2025 UnOS Team of ConoCS
 *
 *  Lisensi: GPL v3.0
 *  Semua kode dalam file ini dan modul kernel terkait harus bersifat
 *  FREE dan OPEN SOURCE, sesuai dengan lisensi tersebut.
 *
 *  Nama File:
 *      acpi.c
 *
 *  Ringkasan:
 *      Modul yang digunakan untuk memanggil sebuah ACPI di awal boot ketika
 *      sistem operasi UnOS mulai dinyalakan. Beberapa ACPI tersebut seperti
 *      BGRT
 *      FADT
 *      MADT
 *      HPET (belum di sertakan)
 *
 *  Penulis:
 *      Rasya, 02-Aug-2025
 *
 *  Penafian:
 *      Penulis bertanggung jawab apabila modul ini menyebabkan
 *      kerusakan, kehilangan, atau ketidakstabilan pada kernel.
 *
 *  Histori Revisi:
 *      05-Aug-2025 : Rasya : Penambahan DOXYGEN pada modul
 */

#include <unoskrnl.h>

GLOBAL BOOT_INFO *bootInfo;



/**
 * @brief init_acpi untuk meng-initialize ACPI
 *
 * init_acpi akan melakukan parse MADT, FADT, BGRT dan dipersiapkan
 * untuk kernel
 *
 * 
 * 
 * @return VOID
 */
VOID 
init_acpi(

) {
    ACPI_MADT *Madt = (ACPI_MADT*)bootInfo->AcpiBootInform->ACPIMADT;
    if (Madt == NULL) {
        serial_print("ACPI MADT not found!\n");
        return;
    }

    serial_print("ACPI MADT found!\n");
    CHARA8 sig[5];
    memcpy(sig, &Madt->Header.Signature, 4);
    sig[4] = '\0';
    serial_printf("MADT Signature: %s\n", sig);
    serial_printf("MADT Length: %u\n", Madt->Header.Length);
    serial_printf("Local APIC Address: 0x%X\n", Madt->LocalApicAddress);
    serial_printf("MADT Flags: 0x%X\n", Madt->Flags);
    UEFIParseMADT(Madt);

    ACPI_FADT *Fadt = (ACPI_FADT*)bootInfo->AcpiBootInform->ACPIFADT;
    if (Fadt == NULL) {
        serial_printf("ACPI FADT not found!\n");
    } else {
        serial_print("ACPI FADT Found\n");

        CHARA8 Sig [5];
        memcpy(Sig, &Fadt->Header.Signature, 4);
        sig[4] = '\0';
        serial_printf("FADT Signature: %s\n", sig);
        serial_printf("FADT Length: %u\n", Fadt->Header.Length);
        serial_printf("PM1a_CNT_BLK: 0x%X\n", Fadt->Pm1aCntBlk);

        AcpiParseFADT(Fadt);
    }
}


/**
 * @brief apic_send_eoi untuk mengirimkan sinyal EOI ke interrupt
 *          hardware
 *
 * mengirimkan sinyal
 */
VOID
 apic_send_eoi(

 ) {
    volatile uint32_t* lapic_eoi = (uint32_t*)(0xFEE000B0);
    *lapic_eoi = 0;
}