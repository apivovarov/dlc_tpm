// g++ --std=c++17 -O3 -I./cpr/include -I./cpr/build/_deps/curl-src/include
// -I./cpr/build/cpr_generated_includes dlc_tpm.cc -o dlc_tpm -L./cpr/build/lib
// -lcpr -lpthread LD_LIBRARY_PATH=./cpr/build/lib ./dlc_tpm

#include <cpr/cpr.h>
#include <time.h>

#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using std::cout;
using std::endl;
using std::string;
using std::vector;

std::string bin_file = "";
std::string json_file = "test.json";
std::string content_type = "application/json";
std::string endpoint_url = "http://localhost:8080/invocations";
int th_num = 1;
int warmup_time = 2;
int test_time = 10;
void test_ping(std::string body_str);
void tpm_runner(int thread_id, const std::string& body, time_t ss, int* my_cnt);

void readArgs(int argc, char** argv) {
  vector<string> args(argv + 1, argv + argc);

  // Loop over command-line args
  for (auto i = args.begin(); i != args.end(); ++i) {
    if (*i == "-h" || *i == "--help") {
      cout << "Syntax: dlc_tpm --json_file <file> or --bin_file <file>"
              " --url <url> "
              "--threads <n> "
              "--test_time <n> "
              "--warmup_time <n>"
           << endl;
      exit(1);
    } else if (*i == "--bin_file") {
      bin_file = *++i;
    } else if (*i == "--json_file") {
      json_file = *++i;
    } else if (*i == "--url") {
      endpoint_url = *++i;
    } else if (*i == "--threads") {
      th_num = stoi(*++i);
    } else if (*i == "--warmup_time") {
      warmup_time = stoi(*++i);
    } else if (*i == "--test_time") {
      test_time = stoi(*++i);
    }
  }
  if (bin_file == "") {
    cout << "Json file: " << json_file << endl;
  } else {
    cout << "Bin file: " << bin_file << endl;
    content_type = "application/x-image";
  }
  cout << "Content Type: " << content_type << endl;
  cout << "Endpoint: " << endpoint_url << endl;
  cout << "Number of threads: " << th_num << endl;
  cout << "Warmup time sec: " << warmup_time << endl;
  cout << "Test time sec: " << test_time << endl;
  cout << "==================================================" << endl;
}

std::string read_bin_file(const std::string& filepath) {
  std::ifstream is(filepath, std::ifstream::binary);
  if (!is) {
    std::cout << "\033[31m";
    cout << "Error: Can not open file " << filepath << "\033[0m" << endl;
    exit(3);
  }
  is.seekg(0, is.end);
  int length = is.tellg();
  is.seekg(0, is.beg);
  std::vector<char> buffer;
  buffer.resize(length);
  is.read(&buffer[0], length);
  is.close();
  return std::string(buffer.begin(), buffer.end());
}

std::string read_json_file(const std::string& filepath) {
  std::ifstream is(filepath);
  if (!is) {
    std::cout << "\033[31m";
    cout << "Error: Can not open file " << filepath << "\033[0m" << endl;
    exit(3);
  }
  std::stringstream buffer;
  buffer << is.rdbuf();
  is.close();
  return buffer.str();
}

int main(int argc, char** argv) {
  readArgs(argc, argv);

  const std::string body_str = bin_file == "" ? read_json_file(json_file) : read_bin_file(bin_file);

  test_ping(body_str);

  time_t ss = time(NULL);
  int all_cnt[th_num];
  vector<std::thread> th_vector;
  for (int i = 0; i < th_num; i++) {
    th_vector.emplace_back(tpm_runner, i, std::ref(body_str), ss, &(all_cnt[i]));
  }
  for (auto& t : th_vector) {
    t.join();
  }

  std::cout << "All Done" << endl;
  int total_cnt = 0;
  for (int i = 0; i < th_num; i++) {
    total_cnt += all_cnt[i];
  }
  float tpm = total_cnt * 60 / test_time;
  cout << "==================================================" << endl;
  cout << "Endpoint: " << endpoint_url << endl;
  cout << "Number of threads: " << th_num << endl;
  cout << "Test time sec: " << test_time << endl;
  cout << "Total N of Transactions: " << total_cnt << endl;
  printf("TPM: %.2f\n", tpm);
  cout << "==================================================" << endl;
  return 0;
}

void test_ping(std::string body_str) {
  cpr::Url url{endpoint_url};
  cpr::Body body{body_str};
  cpr::Header header{{"Content-Type", content_type}};
  cpr::Response resp = cpr::Post(url, header, body);
  std::cout << resp.status_code << endl;
  if (resp.status_code != 200) {
    std::cout << "\033[31m";
    cout << "Error: Bad response, code: " << resp.status_code << "\033[0m" << endl;
    exit(400);
  } else {
    cout << "Test ping: OK" << endl;
  }
}

void tpm_runner(int thread_id, const std::string& body_str, time_t ss, int* my_cnt) {
  cpr::Url url{endpoint_url};
  cpr::Body body{body_str};
  cpr::Header header{{"Content-Type", content_type}};
  int cnt = 0;
  time_t start_time = ss + warmup_time;
  time_t stop_time = start_time + test_time;
  time_t break_time = stop_time + warmup_time;
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
