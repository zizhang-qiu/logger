#include <chrono>
#include <ctime>
#include <iomanip>
#include "file.h"

class Logger {
 public:
  virtual ~Logger() = default;
  virtual void Print(const std::string& str) = 0;

  template <typename T, typename... Args>
  void Print(const std::string& format, T value, Args... args) {
    std::ostringstream oss;
    StrFormat(oss, format, value, args...);
    Print(oss.str());
  }

 private:
  template <typename T, typename... Args>
  void StrFormat(std::ostringstream& oss, const std::string& format, T value,
                 Args... args) {
    size_t pos = format.find("{}");
    if (pos != std::string::npos) {
      oss << format.substr(0, pos) << value;
      StrFormat(oss, format.substr(pos + 2), args...);
    } else {
      oss << format;
    }
  }

  void StrFormat(std::ostringstream& oss, const std::string& format) {
    oss << format;
  }
};

class FileLogger : public Logger {
 public:
  FileLogger(const std::string& path, const std::string& name,
             const std::string& mode = "w")
      : fd_(path + "/log-" + name + ".txt", mode) {
    Print("{} started", name);
  }

  using Logger::Print;
  void Print(const std::string& str) override {
    std::string time = GetCurrentTime();
    fd_.Write("[" + time + "] " + str + "\n");
    fd_.Flush();
  }

  ~FileLogger() override { Print("Closing the log."); }

 private:
  file::File fd_;

  std::string GetCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << now_ms.count();
    return oss.str();
  }
};

class NoopLogger : public Logger {
 public:
  using Logger::Print;
  void Print(const std::string& str) override {}
};