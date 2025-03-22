#ifndef FILE_H_
#define FILE_H_
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdlib>
#include <memory>
#include <string>

#ifdef _WIN32
// https://stackoverflow.com/a/42906151
#include <direct.h>
#include <stdio.h>
#include <windows.h>
#define mkdir(dir, mode) _mkdir(dir)
#define unlink(file) _unlink(file)
#define rmdir(dir) _rmdir(dir)
#else
#include <unistd.h>
#endif

#include <cstdint>
#include <cstdio>
#include <string>
#include "utils.h"

namespace file {
class File {
 public:
  File(const std::string& filename, const std::string& mode) {
    fd_.reset(
        static_cast<FileImpl*>(std::fopen(filename.c_str(), mode.c_str())));
    SPIEL_CHECK_TRUE(fd_);
  }

  // File is move only.
  File(File&& other) = default;
  File& operator=(File&& other) = default;
  File(const File&) = delete;
  File& operator=(const File&) = delete;

  // Flush and Close.
  ~File() {
    if (fd_) {
      Flush();
      Close();
    }
  }
  // Flush the buffer to disk.
  bool Flush() { return !std::fflush(fd_.get()); }
  // Offset of the current point in the file.
  std::int64_t Tell() { return std::ftell(fd_.get()); }
  // Move the current point.
  bool Seek(std::int64_t offset) {
    return !std::fseek(fd_.get(), offset, SEEK_SET);
  }

  // Read count bytes.
  std::string Read(std::int64_t count) {
    std::string out(count, '\0');
    int read = std::fread(&out[0], sizeof(char), count, fd_.get());
    out.resize(read);
    return out;
  }

  // Read the entire file.
  std::string ReadContents() {
    Seek(0);
    return Read(Length());
  }

  // Write to the file.
  bool Write(const std::string& str) {
    return std::fwrite(str.data(), sizeof(char), str.size(), fd_.get()) ==
           str.size();
  }

  // Length of the entire file.
  std::int64_t Length() {
    std::int64_t current = std::ftell(fd_.get());
    std::fseek(fd_.get(), 0, SEEK_END);
    std::int64_t length = std::ftell(fd_.get());
    std::fseek(fd_.get(), current, SEEK_SET);
    return length;
  }

 private:
  // Close the file. Use the destructor instead.
  bool Close() { return !std::fclose(fd_.release()); }

  class FileImpl : public std::FILE {};
  std::unique_ptr<FileImpl> fd_;
};

// Reads the file at filename to a string. Dies if this doesn't succeed.
std::string ReadContentsFromFile(const std::string& filename,
                                 const std::string& mode) {
  File f(filename, mode);
  return f.ReadContents();
}

// Write the string contents to the file. Dies if it doesn't succeed.
void WriteContentsToFile(const std::string& filename, const std::string& mode,
                         const std::string& contents) {
  File f(filename, mode);
  f.Write(contents);
}
// Does the file/directory exist?
bool Exists(const std::string& path) {
  struct stat info;
  return stat(path.c_str(), &info) == 0;
}
// Is it a directory?
bool IsDirectory(const std::string& path) {
  struct stat info;
  return stat(path.c_str(), &info) == 0 && info.st_mode & S_IFDIR;
}
// Make a directory.
bool Mkdir(const std::string& path, int mode = 0755) {
  return mkdir(path.c_str(), mode) == 0;
}
// Mkdir recursively.
bool Mkdirs(const std::string& path, int mode = 0755) {
  struct stat info;
  size_t pos = 0;
  while (pos != std::string::npos) {
    pos = path.find_first_of("\\/", pos + 1);
    std::string sub_path = path.substr(0, pos);
    if (stat(sub_path.c_str(), &info) == 0) {
      if (info.st_mode & S_IFDIR) {
        continue;  // directory already exists
      } else {
        return false;  // is a file?
      }
    }
    if (!Mkdir(sub_path, mode)) {
      return false;  // permission error?
    }
  }
  return true;
}
// Remove/delete the file/directory.
bool Remove(const std::string& path) {
  if (IsDirectory(path)) {
    return rmdir(path.c_str()) == 0;
  } else {
    return unlink(path.c_str()) == 0;
  }
}

// Get the canonical file path.
std::string RealPath(const std::string& path) {
#ifdef _WIN32
  char real_path[MAX_PATH];
  if (_fullpath(real_path, path.c_str(), MAX_PATH) == nullptr)
#else
  char real_path[PATH_MAX];
  if (realpath(path.c_str(), real_path) == nullptr)
  // If there was an error return an empty path
#endif
  {
    return "";
  }

  return std::string(real_path);
}

std::string GetEnv(const std::string& key, const std::string& default_value) {
  char* val = std::getenv(key.c_str());
  return ((val != nullptr) ? std::string(val) : default_value);
}
std::string GetTmpDir() {
  return GetEnv("TMPDIR", "/tmp");
}
}  // namespace file

#endif /* FILE_H_ */
