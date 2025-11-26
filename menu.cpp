#include <iostream>
#include <string>
#include <cstdlib>
#include <map>

using namespace std;

void runSnippet(const string& snippetName, const string& compilerPath) {
    string command = compilerPath + " " + snippetName + ".minilang";
    cout << "\n=== Running " << snippetName << " ===\n";
    cout << "Output:\n";
    system(command.c_str());
    cout << "===================\n";
}

void showMenu() {
    cout << R"(
╔══════════════════════════════════════╗
║      MiniLang Pattern Generator      ║
╠══════════════════════════════════════╣
║ 1. Factorial Sequence                ║
║ 2. Prime Numbers                     ║
║ 3. Arithmetic Sequence               ║
║ 4. Geometric Sequence                ║
║ 5. Triangular Numbers                ║
║ 6. Run Custom File                   ║
║ 7. Exit                              ║
╚══════════════════════════════════════╝
)";
}

int main() {
    const string compilerPath = "./minilang";
    map<int, string> snippets = {
        {1, "factorial"},
        {2, "primes"},
        {3, "arithmetic"},
        {4, "geometric"},
        {5, "triangular"},
    };

    // First, compile the compiler if not exists
    system("g++ minilang.cpp -o minilang -std=c++17");

    int choice;
    string customFile;

    while (true) {
        showMenu();
        cout << "Enter your choice (1-7): ";
        cin >> choice;

        switch (choice) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                runSnippet(snippets[choice], compilerPath);
                break;
            
            case 6:
                cout << "Enter custom filename (without .minilang extension): ";
                cin >> customFile;
                runSnippet(customFile, compilerPath);
                break;
            
            case 7:
                cout << "Thank you for using MiniLang Pattern Generator!\n";
                return 0;
            
            default:
                cout << "Invalid choice! Please try again.\n";
        }

        cout << "\nPress Enter to continue...";
        cin.ignore();
        cin.get();
    }

    return 0;
}
