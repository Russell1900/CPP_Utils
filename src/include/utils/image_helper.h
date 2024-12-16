#pragma once
#include "utils/concurrent_queue.hpp"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace image_helper {

class ImageFilter {
protected:
  std::vector<std::string> exts_;

public:
  ImageFilter(const std::vector<std::string> &exts);
  std::vector<std::string>
  operator()(const std::vector<std::string> &filepaths);
  bool is_image(const std::string &path);
};

class BasicImage {
protected:
  std::string image_name_;
  std::string image_path_;
  const char *image_data_;
  int len_;

  BasicImage();
  void image_path(const std::string &p);
  void image_name(const std::string &name);
  void image_data(const char *data);
  void len(int l);

  friend class ImageQueue;
  friend class ImagePool;

public:
  std::string image_path() const;
  std::string image_name() const;
  const char *image_data() const;
  int len() const;
  virtual ~BasicImage();
};

class ImageProducer;

class ImageProducerFactory {
public:
  static ImageProducer *make(const YAML::Node &config);
};

class ImageProducer {
public:
  virtual bool start(size_t thread_num) = 0;
  virtual void join(bool force) = 0;
  virtual void release_image(BasicImage *&img_ptr) = 0;
  virtual BasicImage *pop() = 0;
  virtual ~ImageProducer(){};
};

class ImageQueue : public ConcurrentQueue<BasicImage *>, public ImageProducer {
protected:
  std::string image_foler_;
  std::vector<std::string> image_files_;
  bool recursive_;

  ImageFilter *img_filter_;

  std::atomic<int> ii_;
  size_t thread_num_;
  int total_num_;

  std::thread worker_;
  bool started_;
  bool joined_;

  void read_image_();

public:
  ImageQueue(const std::string &image_folder, bool recursive,
             ImageFilter *&&img_filter = nullptr, size_t capacity = 20);
  virtual ~ImageQueue();
  bool start(size_t thread_num) override;
  void join(bool force = false) override;
  void release_image(BasicImage *&img_ptr) override;
  BasicImage *pop() override;
};

class ImagePool : public ImageProducer {
protected:
  std::string image_foler_;
  bool recursive_;
  ImageFilter *img_filter_;

  std::vector<std::string> image_files_;
  int use_num_;
  int total_num_;

  std::unordered_map<std::string, BasicImage *> cache_;
  std::mutex lck_;

  std::atomic<size_t> i_;
  std::atomic<size_t> ii_;
  size_t thread_num_;

  void insert_(const std::string &image_path);

public:
  ImagePool(const std::string &dir_path, bool recursive, int use_num,
            int total_num, ImageFilter *&&img_filter = nullptr);
  virtual ~ImagePool();

  bool start(size_t thread_num) override;
  void join(bool force = false) override;
  void release_image(BasicImage *&img_ptr) override;
  BasicImage *pop() override;
};
} // namespace image_helper
