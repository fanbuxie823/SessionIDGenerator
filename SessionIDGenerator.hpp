#ifndef SESSION_ID_GENERATOR_HPP_
#define SESSION_ID_GENERATOR_HPP_
#include <atomic>
#include <mutex>
#include <deque>

#ifdef DEBUG
#include <bitset>
#endif

//template<typename T>
template<std::unsigned_integral T>
class SessionIDGenerator {
 private:
  std::deque<std::atomic<T>> sess_buckets;//每分配一个sid将会将对应位置零
  std::mutex b_m_;
  uint max_session_{64};
  uint bucket_num{0};
  static constexpr int bucket_size = sizeof(T) * 8;

  void UpdateSessionBucket() {
	sess_buckets.clear();
	bucket_num = (max_session_ / bucket_size) + ((max_session_ % bucket_size) ? 1 : 0);
	for (int i = 0; i < bucket_num; ++i) {
	  sess_buckets.emplace_back(0);
	  sess_buckets[i] -= 1;
	}
	auto &bucket = sess_buckets[bucket_num - 1];
	auto high_bit_last_bucket = bucket_num * bucket_size - max_session_;
	bucket = bucket >> high_bit_last_bucket;
  }

 public:
  SessionIDGenerator() {
	UpdateSessionBucket();
  }

  explicit SessionIDGenerator(int session_num) {
	max_session_ = session_num;
	UpdateSessionBucket();
  }

  uint GetOneSession() {
	std::lock_guard<std::mutex> lock(b_m_);
	uint sid = 0, index = 0;
	for (int i = 0; i < bucket_num; ++i) {
	  auto &bucket = sess_buckets[i];
	  if (bucket == 0)
		continue;
	  //取出最低1位
	  ulong low_bit = (bucket & (bucket - 1)) ^ bucket;
	  //把相应位置0
	  bucket ^= low_bit;
	  //将这个最低位转成索引
	  while (low_bit != 0) {
		low_bit >>= 1;
		++index;
	  }
	  sid = i * bucket_size + index;
	  break;
	}
	return sid;
  }

  void ReturnOneSession(uint sid) {
	if (sid > max_session_)
	  return;
	T b_sid = 1;
	--sid;//sid实际上是索引位+1得到的，所以先减一
	auto index = sid / bucket_size;
	b_sid = (b_sid << (sid - index * bucket_size));
	auto &bucket = sess_buckets[sid / bucket_size];
	bucket |= b_sid; //把相应位置1
  }

  uint ResetMaxSession(int max_session) {
	max_session_ = max_session;
	UpdateSessionBucket();
	return max_session_;
  }

  uint GetMaxSession() const {
	return max_session_;
  }

#ifdef DEBUG
  void Dump() const {
	std::cout << "Dump ========================================================= \n";
	for (int i = 0; i < bucket_num; ++i) {
	  auto &bucket = sess_buckets[i];
	  std::bitset<bucket_size> bucket_bit(bucket);
	  std::cout << "index:" << i << "\nbucket_bit:" << bucket_bit << '\n';
	}
	std::cout << "============================================================== \n";
  }
#endif
};

#endif //SESSIONIDGENERATOR_HPP_
