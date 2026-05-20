#include <jni.h>
#include <string>
#include <cmath>
#include <algorithm>

// Không gian lưu trữ cấu hình trong bộ nhớ của Mod
namespace Config {
    float SMOOTH_FACTOR    = 0.15f;   // Giá trị mặc định nếu mất mạng
    float DEAD_ZONE_RAD    = 0.002f;  // Giá trị mặc định
    float AIM_SILENT_RATIO = 0.90f;   // Giá trị mặc định
}

// ==========================================
// THUẬT TOÁN PHÂN TÍCH CHUỖI JSON ĐỘC LẬP
// ==========================================
float ParseJsonFloat(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return -1.0f;
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return -1.0f;
    
    size_t start = json.find_first_of("0123456789.-", pos);
    size_t end = json.find_first_not_of("0123456789.-", start);
    
    if (start != std::string::npos) {
        std::string valStr = json.substr(start, end - start);
        try {
            return std::stof(valStr);
        } catch (...) {
            return -1.0f;
        }
    }
    return -1.0f;
}

// ==========================================
// HÀM KẾT NỐI MẠNG QUA ANDROID JNI
// ==========================================
void FetchConfigFromGitHub(JNIEnv* env) {
    // Đường dẫn tĩnh tới file chứa thông số Service Mod của riêng bạn
    std::string urlStr = "https://github.io";

    // Khởi tạo đối tượng java.net.URL
    jclass urlClass = env->FindClass("java/net/URL");
    jmethodID urlInit = env->GetMethodID(urlClass, "<init>", "(Ljava/lang/String;)V");
    jstring urlJStr = env->NewStringUTF(urlStr.c_str());
    jobject urlObj = env->NewObject(urlClass, urlInit, urlJStr);

    // Mở kết nối URLConnection
    jmethodID openConn = env->GetMethodID(urlClass, "openConnection", "()Ljava/net/URLConnection;");
    jobject connObj = env->CallObjectMethod(urlObj, openConn);

    // Lấy InputStream từ kết nối mạng
    jclass connClass = env->FindClass("java/net/URLConnection");
    jmethodID getInputStream = env->GetMethodID(connClass, "getInputStream", "()Ljava/io/InputStream;");
    jobject streamObj = env->CallObjectMethod(connObj, getInputStream);

    // Sử dụng java.util.Scanner để đọc toàn bộ dữ liệu văn bản từ luồng mạng
    jclass scannerClass = env->FindClass("java/util/Scanner");
    jmethodID scannerInit = env->GetMethodID(scannerClass, "<init>", "(Ljava/io/InputStream;)V");
    jobject scannerObj = env->NewObject(scannerClass, scannerInit, streamObj);

    // Đặt Delimiter đọc từ đầu đến cuối chuỗi (\A)
    jmethodID useDelimiter = env->GetMethodID(scannerClass, "useDelimiter", "(Ljava/lang/String;)Ljava/util/Scanner;");
    jstring delim = env->NewStringUTF("\\A");
    env->CallObjectMethod(scannerObj, useDelimiter, delim);

    // Chuyển đổi toàn bộ kết quả phản hồi thành chuỗi ký tự Java String
    jmethodID nextMethod = env->GetMethodID(scannerClass, "next", "()Ljava/lang/String;");
    jstring resultJStr = (jstring)env->CallObjectMethod(scannerObj, nextMethod);

    if (resultJStr != nullptr) {
        const char* rawJson = env->GetStringUTFChars(resultJStr, nullptr);
        std::string jsonStr(rawJson);
        env->ReleaseStringUTFChars(resultJStr, rawJson);

        // Tiến hành bóc tách dữ liệu từ chuỗi JSON thô vừa tải về
        float smooth = ParseJsonFloat(jsonStr, "smooth_factor");
        float deadzone = ParseJsonFloat(jsonStr, "dead_zone_rad");
        float silent = ParseJsonFloat(jsonStr, "silent_ratio");

        // Nếu thông số hợp lệ (lớn hơn 0), cập nhật trực tiếp vào cấu hình Mod
        if (smooth > 0) Config::SMOOTH_FACTOR = smooth;
        if (deadzone > 0) Config::DEAD_ZONE_RAD = deadzone;
        if (silent > 0) Config::AIM_SILENT_RATIO = silent;
    }
}

// ==========================================
// CỔNG GIAO TIẾP JNI NATIVE CHUẨN KHI KHỞI CHẠY
// ==========================================
extern "C" JNIEXPORT void JNICALL
Java_com_android_systemui_NativeEngine_Init(JNIEnv* env, jobject thiz, jlong base, jlong sOff, jlong uOff) {
    
    // Tự động chạy ngầm tải cấu hình từ Github về nạp vào biến tĩnh trước khi xử lý Hook game
    FetchConfigFromGitHub(env);

    // Tiếp tục thực hiện logic nạp Offset và kích hoạt Hook...
}
