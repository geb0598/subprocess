#include <thread>

int main() {
    std::this_thread::sleep_for(std::chrono::seconds(10));
}