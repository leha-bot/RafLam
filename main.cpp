#include "server/Server.h"

int main() {
    std::cout << "*** start working ***\n";
    try {
        ProdServer s(6379, 1);
        s.serve();
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        exit(1);
    } catch (std::invalid_argument& e) {
        std::cerr << e.what() << '\n';
    }
}
