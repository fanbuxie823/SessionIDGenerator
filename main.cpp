#include <iostream>
#include <thread>
#include <vector>
#include "SessionIDGenerator.hpp"

std::atomic<bool> g_func_running{false};

static void func(SessionIDGenerator<ulong> *sg, int index) {
  std::vector<uint> sids;
  bool no_free_id = false;
  while (g_func_running) {
	if (!no_free_id) {
	  auto sid = sg->GetOneSession();
	  if (sid != 0) {
		std::cout << "[" << index << "] Get one sid:" << sid << std::endl;
		sids.emplace_back(sid);
	  } else {
		no_free_id = true;
//		std::cout << "[" << index << "] No free sid" << std::endl;
	  }
	} else {
	  if (sids.empty()){
		no_free_id = false;
		continue;
	  }
	  uint sid = sids.back();
	  sids.pop_back();
//		std::cout << "[" << index << "] Try return one sid:" << sid << std::endl;
	  sg->ReturnOneSession(sid);
	  std::cout << "[" << index << "] Sid:" << sid << " returned" << std::endl;
	}
	std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
}

int main() {
  SessionIDGenerator<ulong> sg(120);

//  std::vector<uint> sids;
//  while (true){
//	auto sid = sg.GetOneSession();
//	if (sid == 0)
//	  break;
//	sids.push_back(sid);
//	sg.Dump();
//  }
//  for (auto sid:sids) {
//	sg.ReturnOneSession(sid);
//	sg.Dump();
//  }

  std::vector<std::thread> t_v;
  g_func_running = true;
  for (int i = 0; i < 9; ++i) {
	t_v.emplace_back(func, &sg, i);
  }
  std::this_thread::sleep_for(std::chrono::seconds(5));
  g_func_running = false;
  for (auto &t : t_v) {
	t.join();
  }
  return 0;
}
