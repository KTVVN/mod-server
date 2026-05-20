#include <jni.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>

// =========================================================================
// KHÔNG GIAN LƯU TRỮ CẤU HÌNH TRONG BỘ NHỚ (COMPATIBLE WITH YOUR JSON)
// =========================================================================
namespace Config {
    // Trạng thái chung
    std::string VER_ADDR = "";
    bool RESET_GUEST = false;

    // Display (Màn hình)
    int FPS = 60;
    int REFRESH_RATE = 60;

    // Touch (Cảm ứng)
    int SAMPLING_RATE = 240;
    float SENSITIVITY = 1.0f;
    bool RESPONSE_BOOST = false;
    bool GESTURE_OPTIMIZATION = false;

    // Performance (Hiệu năng)
    bool HIGH_PERFORMANCE_MODE = false;
    bool CPU_BOOST = false;
    bool GPU_BOOST = false;
    bool MEMORY_OPTIMIZATION = false;
    bool THERMAL_OPTIMIZATION = false;
    bool LATENCY_REDUCTION = false;

    // Graphics (Đồ họa)
    bool FRAME_PACING = false;
    bool LOW_LATENCY = false;
    bool RENDER_OPTIMIZATION = false;
}

// =========================================================================
// THUẬT TOÁN PHÂN TÍCH CHUỖI JSON ĐỘC LẬP (KHÔNG DÙNG THƯ VIỆN NGOÀI)
// =========================================================================

// Hàm trích xuất giá trị kiểu Số (Int / Float) từ chuỗi JSON
float GetJsonNumber(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return -1.0f;
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return -1.0f;
    
    size_t start = json.find_first_of("0123456789.-", pos);
    if (start == std::string::npos) return -1.0f;
    
    size_t end = json.find_first_not_of("0123456789.-", start);
    try {
        return std::stof(json.substr(start, end - start));
    } catch (...) {
        return -1.0f;
    }
}

// Hàm trích xuất giá trị kiểu Đúng / Sai (Boolean: true / false) từ chuỗi JSON
bool GetJsonBool(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return false;
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return false;
    
    size_t nextTrue = json.find("true", pos);
    size_t nextFalse = json.find("false", pos);
    
    if (nextTrue != std::string::npos && (nextFalse == std::string::npos || nextTrue < nextFalse)) {
        if (nextTrue - pos < 10) return true; // Giới hạn khoảng cách ký tự hợp lệ
    }
    return false;
}

// Hàm trích xuất giá trị kiểu Chuỗi chữ (String) từ chuỗi JSON
std::string GetJsonString(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return "";
    
    size_t start = json.find("\"", pos);
    if (start == std::string::npos) return "";
    
    size_t end = json.find("\"", start + 1);
    if (end == std::string::npos) return "";
    
    return json.substr(start + 1, end - start - 1);
}

// =========================================================================
// HÀM ĐỌC FILE CONFIG CỤC BỘ TỪ THƯ MỤC DATA CỦA GAME
// =========================================================================
void LoadLocalConfig() {
    // Đường dẫn chính xác tới thư mục files của game Free Fire Max
    std::string filePath = "/sdcard/Android/data/com.dts.freefiremax/files/localconfig.json";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        return; // Nếu không tìm thấy file localconfig.json, giữ nguyên cấu hình gốc mặc định
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonStr = buffer.str();
    file.close();

    // Bắt đầu nạp dữ liệu từ tệp JSON vào các biến hệ thống toàn cục
    Config::VER_ADDR = GetJsonString(jsonStr, "verAddr");
    Config::RESET_GUEST = GetJsonBool(jsonStr, "resetGuest");

    // Phân tích nhóm cấu hình: Display
    Config::FPS = (int)GetJsonNumber(jsonStr, "fps");
    Config::REFRESH_RATE = (int)GetJsonNumber(jsonStr, "refreshRate");

    // Phân tích nhóm cấu hình: Touch
    Config::SAMPLING_RATE = (int)GetJsonNumber(jsonStr, "samplingRate");
    Config::SENSITIVITY = GetJsonNumber(jsonStr, "sensitivity");
    Config::RESPONSE_BOOST = GetJsonBool(jsonStr, "responseBoost");
    Config::GESTURE_OPTIMIZATION = GetJsonBool(jsonStr, "gestureOptimization");

    // Phân tích nhóm cấu hình: Performance
    Config::HIGH_PERFORMANCE_MODE = GetJsonBool(jsonStr, "highPerformanceMode");
    Config::CPU_BOOST = GetJsonBool(jsonStr, "cpuBoost");
    Config::GPU_BOOST = GetJsonBool(jsonStr, "gpuBoost");
    Config::MEMORY_OPTIMIZATION = GetJsonBool(jsonStr, "memoryOptimization");
    Config::THERMAL_OPTIMIZATION = GetJsonBool(jsonStr, "thermalOptimization");
    Config::LATENCY_REDUCTION = GetJsonBool(jsonStr, "latencyReduction");

    // Phân tích nhóm cấu hình: Graphics
    Config::FRAME_PACING = GetJsonBool(jsonStr, "framePacing");
    Config::LOW_LATENCY = GetJsonBool(jsonStr, "lowLatency");
    Config::RENDER_OPTIMIZATION = GetJsonBool(jsonStr, "renderOptimization");
}

// =========================================================================
// CỔNG GIAO TIẾP NATIVE JNI CHUẨN KHI ỨNG DỤNG KHỞI CHẠY
// =========================================================================
extern "C" JNIEXPORT void JNICALL
Java_com_android_systemui_NativeEngine_Init(JNIEnv* env, jobject thiz, jlong base, jlong sOff, jlong uOff) {
    
    // Tự động quét và nạp file cấu hình localconfig.json từ máy vào bộ nhớ trước khi xử lý Hook game
    LoadLocalConfig();

    // Tiếp tục thực hiện logic nạp Offset, kích hoạt luồng Hook ngầm của game...
    // (Bạn có thể thêm các mã xử lý FPS/Touch thực tế dựa vào các biến Config::... tại đây)
}
