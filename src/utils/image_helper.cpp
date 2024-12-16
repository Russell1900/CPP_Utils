#include "utils/image_helper.h"
#include "spdlog/spdlog.h"
#include "utils/path_helper.h"
#include <stdexcept>

using namespace std;

namespace image_helper {
// class ImageFilter
ImageFilter::ImageFilter(const vector<string> &exts) : exts_(exts) {}

vector<string> ImageFilter::operator()(const vector<string> &filepaths) {
  vector<string> res;
  for (const string &file : filepaths) {
    if (is_image(file)) {
      res.emplace_back(file);
    }
  }
  return res;
}

bool ImageFilter::is_image(const string &path) {
  for (const string &ext : exts_) {
    if (::path_helper::endswith(path, ext)) {
      return true;
    }
  }
  return false;
}

// class BasicImage
BasicImage::BasicImage() {}
BasicImage::~BasicImage() {
  if (image_data_)
    delete image_data_;
}

void BasicImage::image_path(const string &p) { image_path_ = p; }
string BasicImage::image_path() const { return image_path_; }

void BasicImage::image_name(const string &name) { image_name_ = name; }
string BasicImage::image_name() const { return image_name_; }

void BasicImage::image_data(const char *data) { image_data_ = data; }
const char *BasicImage::image_data() const { return image_data_; }

void BasicImage::len(int l) { len_ = l; }
int BasicImage::len() const { return len_; }

// class ImageQueue
ImageQueue::ImageQueue(const string &image_folder, bool recursive,
                       ImageFilter *&&img_filter, size_t capacity)
    : ConcurrentQueue<BasicImage *>(capacity), image_foler_(image_folder),
      recursive_(recursive), img_filter_(img_filter), started_(false),
      joined_(false), ii_(0) {
  // load image paths
  ::path_helper::list_files(image_foler_, image_files_, recursive_);
  if (img_filter_) {
    image_files_ = (*img_filter_)(image_files_);
  }
  total_num_ = image_files_.size();
}

ImageQueue::~ImageQueue() {
  if (img_filter_) {
    delete img_filter_;
  }
  if (started_ && !joined_) {
    join();
  }
}

void ImageQueue::read_image_() {
  for (int i = 0; i < total_num_; ++i) {
    BasicImage *img = new BasicImage();
    int idx = i % image_files_.size();
    img->image_path(image_files_[idx]);
    img->image_name(::path_helper::filename(image_files_[idx]));
    ::path_helper::read_file(image_files_[idx], img->image_data_, img->len_);
    push(img);
  }
  for (; ii_ < thread_num_; ++ii_)
    push(nullptr);
}

bool ImageQueue::start(size_t thread_num) {
  if (!started_) {
    spdlog::error("image queue already running");
    return false;
  }
  spdlog::info("image queue started");
  thread_num_ = thread_num;
  started_ = true;
  worker_ = thread(&ImageQueue::read_image_, this);
  return true;
}

void ImageQueue::join(bool force) {
  if (force) {
    for (; ii_ < thread_num_; ++ii_)
      push(nullptr);
  }
  worker_.join();
  joined_ = false;
  spdlog::info("image queue joined");
}

void ImageQueue::release_image(BasicImage *&img_ptr) {
  delete img_ptr;
  img_ptr = nullptr;
}

BasicImage *ImageQueue::pop() { return ConcurrentQueue<BasicImage *>::pop(); }

// class ImagePool
ImagePool::ImagePool(const string &dir_path, bool recursive, int use_num,
                     int total_num, ImageFilter *&&img_filter)
    : image_foler_(dir_path), recursive_(recursive), use_num_(use_num),
      total_num_(total_num), img_filter_(img_filter), i_(0), ii_(0) {}

ImagePool::~ImagePool() {
  if (img_filter_) {
    delete img_filter_;
  }
  for (auto ite = cache_.begin(); ite != cache_.end(); ++ite) {
    delete ite->second;
  }
}

void ImagePool::insert_(const string &image_path) {
  if (cache_.find(image_path) == cache_.end()) {
    unique_lock<mutex> lck(lck_);
    if (cache_.find(image_path) == cache_.end()) {
      cache_.insert(pair<string, BasicImage *>(image_path, nullptr));
      BasicImage *img = new BasicImage();
      img->image_path(image_path);
      img->image_name(::path_helper::filename(image_path));
      ::path_helper::read_file(img->image_path_, img->image_data_, img->len_);
      cache_.at(image_path) = img;
    }
  }
}

bool ImagePool::start(size_t thread_num) {
  i_ = 0;
  ii_ = 0;
  thread_num_ = thread_num;
  ::path_helper::list_files(image_foler_, image_files_, recursive_);
  if (img_filter_) {
    image_files_ = (*img_filter_)(image_files_);
  }

  int len = image_files_.size();
  if (use_num_ < 0 || use_num_ > len) {
    use_num_ = len;
  }
  if (total_num_ < 0) {
    total_num_ = len;
  }
  int m = len - use_num_;
  if (m > 0) {
    for (int i = 0; i < m; ++i) {
      image_files_.pop_back();
    }
  }
  for (int i = 0; i < image_files_.size(); ++i) {
    insert_(image_files_[i]);
  }
  spdlog::info("image pool loaded, size: {}", cache_.size());
  return true;
}

void ImagePool::join(bool force) {
  if (force) {
    i_ = total_num_;
  }
}

BasicImage *ImagePool::pop() {
  int i = i_++;
  if (i < total_num_) {
    BasicImage *node = cache_.at(image_files_.at(i % image_files_.size()));
    BasicImage *p_img = new BasicImage();
    p_img->image_data(node->image_data());
    p_img->image_name(node->image_name() + "____" + to_string(i));
    p_img->len(node->len());
    return p_img;
  }
  if (ii_++ < thread_num_) {
    return nullptr;
  }
  throw runtime_error("ImagePool has already ended");
}

void ImagePool::release_image(BasicImage *&img_ptr) {
  img_ptr->image_data(nullptr);
  delete img_ptr;
  img_ptr = nullptr;
}

// class ImageProducerFactory
ImageProducer *ImageProducerFactory::make(const YAML::Node &config) {
  ImageProducer *ip;
  ImageFilter *img_filter = nullptr;
  if (config["Type"].IsDefined()) {
    if (config["Exts"].IsDefined()) {
      assert(config["Exts"].IsSequence());
      vector<string> exts;
      for (const auto &ext : config["Exts"]) {
        exts.push_back(ext.as<string>());
      }
      img_filter = new ImageFilter(exts);
    }
    if (config["Type"].as<string>() == "Queue") {
      ip =
          new ImageQueue(config["Path"].as<string>(),
                         config["Recursive"].as<bool>(), std::move(img_filter));
    } else if (config["Type"].as<string>() == "Cache") {
      ip = new ImagePool(config["Path"].as<string>(),
                         config["Recursive"].as<bool>(),
                         config["UseNum"].as<int>(),
                         config["TotalNum"].as<int>(), std::move(img_filter));
    } else {
      spdlog::critical("Type field should be one of Queue or Cache");
    }
  } else {
    spdlog::critical("need specify field Type in ImageProducer");
  }
  return ip;
}
} // namespace image_helper
