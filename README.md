Đề tài **“Thiết kế hệ thống khóa từ RFID tích hợp theo dõi hành vi mở cửa”** về cơ
bản em đã hoàn thành các yêu cầu đặt ra:
- Thiết lập ý tưởng đề tài: Hệ thống đã được thiết kế với sự tích hợp của vi điều khiển STM32F103C8T6 và các module ngoại vi như ESP32 DEVKIT V1, RC522 và I2C LCD. Các linh kiện tương thích với nhau cho độ phản hồi nhanh và dễ dàng giao tiếp, với nhiều mã nguồn mở trên internet.
- Thiết kế hệ thống phần cứng: Hệ thống phản hồi tương đối ổn định (chỉ có trường hợp nhiễu LCD tần suất thấp do đặt gần nguồn Adapter), đáp ứng tốt các yêu cầu về bảo mật và chức năng.
- Truyền dữ liệu lên Google Sheets: dữ liệu về hành vi mở cửa được truyền tải lên Google Sheets thông qua ESP32 và giao thức HTTP, giúp người quản lý dễ dàng theo dõi và giám sát từ xa.
