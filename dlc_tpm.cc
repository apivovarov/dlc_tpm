// g++ --std=c++17 -O3 -I./cpr/include -I./cpr/build/_deps/curl-src/include
// -I./cpr/build/cpr_generated_includes dlc_tpm.cc -o dlc_tpm -L./cpr/build/lib
// -lcpr -lpthread LD_LIBRARY_PATH=./cpr/build/lib ./dlc_tpm

#include <cpr/cpr.h>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <time.h>
#include <vector>

using std::cout;
using std::endl;
using std::string;
using std::vector;

std::string endpoint_url = "http://localhost:8080/invocations";
int th_num = 1;
int warmup_sec = 2;
int win_sec = 10;
void test_ping(std::string body_str);
void tpm_runner(int thread_id, std::string body, time_t ss, int *my_cnt);

void readArgs(int argc, char **argv) {
  vector<string> args(argv + 1, argv + argc);

  // Loop over command-line args
  for (auto i = args.begin(); i != args.end(); ++i) {
    if (*i == "-h" || *i == "--help") {
      cout << "Syntax: dlc_tpm --url <url> --threads <n> --win_sec <n> "
              "--warmup_sec <n>"
           << endl;
      exit(1);
    } else if (*i == "--url") {
      endpoint_url = *++i;
    } else if (*i == "--threads") {
      th_num = stoi(*++i);
    } else if (*i == "--warmup_sec") {
      warmup_sec = stoi(*++i);
    } else if (*i == "--win_sec") {
      win_sec = stoi(*++i);
    }
  }
  cout << "Endpoint: " << endpoint_url << "\n";
  cout << "Warmup sec: " << warmup_sec << endl;
  cout << "Measuring window sec: " << win_sec << endl;
}

int main(int argc, char **argv) {
  readArgs(argc, argv);
  std::ifstream t("test.json");
  std::stringstream buffer;
  buffer << t.rdbuf();
  t.close();
  std::string body_str = buffer.str();

  test_ping(body_str);

  time_t ss = time(NULL);
  int all_cnt[th_num];
  vector<std::thread> th_vector;
  for (int i = 0; i < th_num; i++) {
    // std::thread t(tpm_runner, i, body_str);
    th_vector.emplace_back(tpm_runner, i, body_str, ss, &(all_cnt[i]));
  }
  for (auto &t : th_vector) {
    t.join();
  }

  std::cout << "All Done" << endl;
  int total_cnt = 0;
  for (int i = 0; i < th_num; i++) {
    total_cnt += all_cnt[i];
  }
  float tpm = total_cnt * 60 / win_sec;
  cout << "Total N of Transactions: " << total_cnt << endl;
  cout << "Measuring window sec: " << win_sec << endl;
  printf("TPM: %.2f\n", tpm);
  return 0;
}

void test_ping(std::string body_str) {
  cpr::Url url{endpoint_url};
  cpr::Body body{body_str};
  cpr::Header header{{"Content-Type", "application/json"}};
  cpr::Response resp = cpr::Post(url, header, body);
  std::cout << resp.status_code << endl;
  if (resp.status_code != 200) {
    cout << "Bad response. Exit" << endl;
    exit(400);
  } else {
    cout << "Test ping: OK" << endl;
  }
}

void tpm_runner(int thread_id, std::string body_str, time_t ss, int *my_cnt) {
  cpr::Url url{endpoint_url};
  cpr::Body body{body_str};
  cpr::Header header{{"Content-Type", "application/json"}};
  int cnt = 0;
  time_t start_time = ss + warmup_sec;
  time_t stop_time = start_time + win_sec;
  time_t break_time = stop_time + warmup_sec;
  while (true) {
    cpr::Response resp = cpr::Post(url, header, body);
    time_t t0 = time(NULL);
    if (t0 >= start_time && t0 < stop_time) {
      cnt++;
    } else if (t0 >= break_time) {
      break;
    }
    std::cout << thread_id << ": " << resp.status_code << endl;
    // std::cout << resp.text << "\n";
  }
  std::cout << thread_id << ": "
            << "Done, cnt: " << cnt << endl;
  *my_cnt = cnt;
}
