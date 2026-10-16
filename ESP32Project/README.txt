không dùng các chân gpio sau:
GPIO19 và GPIO20: Đang được nối với khối USB Serial/JTAG nội (USB D-/D+) dùng cho nạp code và debug
GPIO0, GPIO3, GPIO45, GPIO46: Là các chân Strapping cấu hình chế độ boot/điện áp
GPIO35, GPIO36, GPIO37: Bị chiếm dụng bởi bộ nhớ Octal SPI PSRAM trên các bản module có PSRAM (như N8R8 hay N16R8)

ADC chọn chân 32-39
