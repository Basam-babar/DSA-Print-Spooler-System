#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <limits>
#include <algorithm>
#include <cstring>  // for memset

#ifdef _WIN32
#include <windows.h>
#define CLEAR_SCREEN "cls"
#else
#define CLEAR_SCREEN "clear"
#endif

// Fix Windows.h macro pollution
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

using namespace std;

// ==================== COLOR HANDLING ====================
namespace Colors {
    const int HEADER = 11;  // Light Cyan
    const int SUCCESS = 10;  // Light Green
    const int error1 = 12;  // Light Red
    const int WARNING = 14;  // Light Yellow
    const int INFO = 9;   // Light Blue
    const int MENU = 15;  // Bright White
    const int SUBTITLE = 6;   // Yellow
    const int BORDER = 13;  // Light Magenta

    void set(int color) {
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(color));
#else
        const char* ansi[] = {
            "\033[0m", "\033[0;34m", "\033[0;32m", "\033[0;36m", "\033[0;31m",
            "\033[0;35m", "\033[0;33m", "\033[0;37m", "\033[1;30m",
            "\033[1;34m", "\033[1;32m", "\033[1;36m", "\033[1;31m",
            "\033[1;35m", "\033[1;33m", "\033[1;37m"
        };
        if (color >= 0 && color < 16) cout << ansi[color];
#endif
    }
    void reset() { set(7); }
}

void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    this_thread::sleep_for(chrono::milliseconds(ms));
#endif
}

// ==================== UI CLASS ====================
struct UI {
    static void clear() { system(CLEAR_SCREEN); }

    static void line(char c = '=', int n = 80, int col = Colors::BORDER) {
        Colors::set(col);
        cout << string(n, c) << '\n';
        Colors::reset();
    }

    static void header(const string& title) {
        clear();
        line('=', 80, Colors::BORDER);
        Colors::set(Colors::HEADER);
        int pad = (78 - static_cast<int>(title.size())) / 2;
        cout << '|' << string(pad, ' ') << title << string(78 - pad - title.size(), ' ') << "|\n";
        Colors::reset();
        line('=', 80, Colors::BORDER);
        cout << '\n';
    }

    static void success(const string& msg) {
        Colors::set(Colors::SUCCESS);
        cout << "\n[SUCCESS] " << msg << '\n';
        Colors::reset();
    }

    static void error(const string& msg) {
        Colors::set(Colors::error1);
        cout << "\n[ERROR] " << msg << '\n';
        Colors::reset();
    }

    static void warning(const string& msg) {
        Colors::set(Colors::WARNING);
        cout << "\n[WARNING] " << msg << '\n';
        Colors::reset();
    }

    static void loading(const string& msg) {
        Colors::set(Colors::INFO);
        cout << "\n" << msg;
        for (int i = 0; i < 3; ++i) { cout << "."; cout.flush(); sleep_ms(400); }
        cout << '\n';
        Colors::reset();
    }

    static void enter() {
        Colors::set(Colors::SUBTITLE);
        cout << "\nPress Enter to continue...";
        Colors::reset();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }

    static void menu(int n, const string& text) {
        Colors::set(Colors::INFO);
        cout << "  [" << n << "] ";
        Colors::set(Colors::MENU);
        cout << text << '\n';
        Colors::reset();
    }

    static void banner() {
        clear();
        Colors::set(Colors::HEADER);
        cout <<
            "\n"
            "    =====================================================================\n"
            "      ____       _       _         ____                  _           \n"
            "     |  _ \\ _ __(_)_   _| |_ ___  / ___| _ __   ___   ___| | ___ _ __ \n"
            "     | |_) | '__| \\ \\ / / __/ _ \\ \\___ \\| '_ \\ / _ \\ / __| |/ _ \\ '__|\n"
            "     |  __/| |  | |\\ V /| || (_) | ___) | |_) | (_) | (__| |  __/ |   \n"
            "     |_|   |_|  |_| \\_/  \\__\\___/ |____/| .__/ \\___/ \\___|_|\\___|_|   \n"
            "                                         |_|                         \n"
            "                  PRINT SPOOLER MANAGEMENT SYSTEM\n"
            "    =====================================================================\n"
            "\n";
        Colors::reset();
        Colors::set(Colors::SUBTITLE);
        cout << "                        Version 1.0 | Advanced Print Queue Manager\n\n";
        Colors::reset();
    }
};

// ==================== DATA STRUCTURES ====================
enum JobStatus { QUEUED, DELAYED, COMPLETED, CANCELLED };

struct PrintJob {
    int jobID = 0;
    string documentName;
    int priority = 5;
    int pageCount = 0;
    JobStatus status = QUEUED;
    string delayReason;
    time_t submissionTime = 0;
};

struct HashNode {
    PrintJob* job;
    HashNode* next;
    HashNode(PrintJob* j) : job(j), next(nullptr) {}
};

class HashMap {
    static const int SIZE = 100;
    HashNode* table[SIZE] = {};
    int hash(int key) const { return key % SIZE; }
public:
    ~HashMap() {
        for (auto& bucket : table) {
            auto temp = bucket;
            while (temp) {
                auto del = temp;
                temp = temp->next;
                delete del->job;
                delete del;
            }
        }
    }
    void insert(PrintJob* job) {
        int idx = hash(job->jobID);
        auto node = new HashNode(job);
        if (!table[idx]) table[idx] = node;
        else {
            auto temp = table[idx];
            while (temp->next) temp = temp->next;
            temp->next = node;
        }
    }
    PrintJob* find(int id) {
        int idx = hash(id);
        auto temp = table[idx];
        while (temp) {
            if (temp->job->jobID == id) return temp->job;
            temp = temp->next;
        }
        return nullptr;
    }
    void getAll(PrintJob**& jobs, int& count) {
        count = 0;
        for (auto bucket : table) {
            auto temp = bucket;
            while (temp) { count++; temp = temp->next; }
        }
        if (count == 0) { jobs = nullptr; return; }
        jobs = new PrintJob * [count];
        int i = 0;
        for (auto bucket : table) {
            auto temp = bucket;
            while (temp) { jobs[i++] = temp->job; temp = temp->next; }
        }
    }
};

class MinHeap {
    int* arr;
    int cap, sz;
    HashMap* map = nullptr;
    void up(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            auto c = map->find(arr[i]);
            auto par = map->find(arr[p]);
            if (c && par && c->priority < par->priority) {
                swap(arr[i], arr[p]);
                i = p;
            }
            else break;
        }
    }
    void down(int i) {
        while (true) {
            int l = 2 * i + 1, r = 2 * i + 2, s = i;
            if (l < sz) {
                auto sc = map->find(arr[s]);
                auto lc = map->find(arr[l]);
                if (lc && sc && lc->priority < sc->priority) s = l;
            }
            if (r < sz) {
                auto sc = map->find(arr[s]);
                auto rc = map->find(arr[r]);
                if (rc && sc && rc->priority < sc->priority) s = r;
            }
            if (s != i) { swap(arr[i], arr[s]); i = s; }
            else break;
        }
    }
public:
    MinHeap(int c = 1000) : cap(c), sz(0) { arr = new int[cap]; }
    ~MinHeap() { delete[] arr; }
    void setMap(HashMap* m) { map = m; }
    void insert(int id) { arr[sz++] = id; up(sz - 1); }
    int extract() {
        if (sz == 0) return -1;
        int root = arr[0];
        arr[0] = arr[--sz];
        if (sz > 0) down(0);
        return root;
    }
    bool empty() const { return sz == 0; }
    int size() const { return sz; }
    void rebuild() { for (int i = sz / 2 - 1; i >= 0; --i) down(i); }
};

// ==================== USER AUTH ====================
struct User { string name, pass; User* next; User(string n, string p) : name(n), pass(p), next(nullptr) {} };

class UserAuth {
    User* head = nullptr;
    string user;
    bool in = false;
    void save() {
        ofstream f("users.dat");
        for (auto t = head; t; t = t->next) f << t->name << " " << t->pass << "\n";
    }
public:
    UserAuth() { load(); }
    void load() {
        ifstream f("users.dat");
        if (!f) return;
        string n, p;
        while (f >> n >> p) head = new User(n, p);
        { head; };
    }
    void signup() {
        UI::header("SIGN UP");
        string n, p;
        cout << "Username: "; cin >> n;
        for (auto t = head; t; t = t->next)
            if (t->name == n) { UI::error("Username taken!"); UI::enter(); return; }
        cout << "Password: "; cin >> p;
        head = new User(n, p);
        { head; };
        save();
        UI::success("Account created!");
        UI::enter();
    }
    bool login() {
        UI::header("LOGIN");
        string n, p;
        cout << "Username: "; cin >> n;
        cout << "Password: "; cin >> p;
        for (auto t = head; t; t = t->next)
            if (t->name == n && t->pass == p) {
                user = n; in = true;
                UI::success("Welcome, " + n + "!");
                sleep_ms(800);
                return true;
            }
        UI::error("Wrong credentials!");
        UI::enter();
        return false;
    }
    void logout() { in = false; user.clear(); UI::success("Logged out."); sleep_ms(800); }
    bool loggedIn() const { return in; }
    string username() const { return user; }
    ~UserAuth() { while (head) { auto t = head; head = head->next; delete t; } }
};

// ==================== PRINT SPOOLER ====================
class PrintSpooler {
    MinHeap* heap;
    HashMap* map;
    int nextID = 1;
    static const int PRINTERS = 5;

    void saveState() {
        ofstream f("state.dat");
        if (!f) return;
        f << nextID << "\n";
        PrintJob** jobs = nullptr; int cnt = 0;
        map->getAll(jobs, cnt);
        f << cnt << "\n";
        for (int i = 0; i < cnt; ++i) {
            auto j = jobs[i];
            f << j->jobID << "\n" << j->documentName << "\n"
                << j->priority << "\n" << j->pageCount << "\n"
                << j->status << "\n" << j->delayReason << "\n"
                << j->submissionTime << "\n";
        }
        if (jobs) delete[] jobs;
    }

    void loadState() {
        ifstream f("state.dat");
        if (!f) return;
        f >> nextID; int cnt; f >> cnt; f.ignore();
        for (int i = 0; i < cnt; ++i) {
            auto j = new PrintJob();
            f >> j->jobID; f.ignore();
            getline(f, j->documentName);
            f >> j->priority >> j->pageCount;
            int s; f >> s; j->status = (JobStatus)s; f.ignore();
            getline(f, j->delayReason);
            f >> j->submissionTime; f.ignore();
            map->insert(j);
            if (j->status == QUEUED || j->status == DELAYED)
                heap->insert(j->jobID);
        }
    }

public:
    PrintSpooler() {
        heap = new MinHeap();
        map = new HashMap();
        heap->setMap(map);
        loadState();
    }

    void addJob() {
        UI::header("ADD PRINT JOB");
        auto j = new PrintJob();
        j->jobID = nextID++;
        Colors::set(Colors::SUCCESS);
        cout << "\nJob ID: " << j->jobID << "\n\n";
        Colors::reset();

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Document name: "; getline(cin, j->documentName);
        cout << "Priority (1-5): "; cin >> j->priority;
        while (j->priority < 1 || j->priority > 5) {
            UI::error("Invalid priority!");
            cout << "Priority (1-5): "; cin >> j->priority;
        }
        cout << "Pages: "; cin >> j->pageCount;

        j->status = QUEUED;
        j->submissionTime = time(nullptr);
        map->insert(j);
        heap->insert(j->jobID);

        UI::success("Job added!");
        UI::enter();
    }

    void cancelJob() {
        UI::header("CANCEL JOB");
        int id; cout << "Job ID: "; cin >> id;
        auto j = map->find(id);
        if (!j) { UI::error("Job not found!"); UI::enter(); return; }
        if (j->status == CANCELLED) { UI::warning("Already cancelled!"); UI::enter(); return; }
        j->status = CANCELLED;
        UI::success("Job cancelled!");
        UI::enter();
    }

    void listJobs() {
        UI::header("ALL JOBS");
        PrintJob** jobs = nullptr; int cnt = 0;
        map->getAll(jobs, cnt);
        if (cnt == 0) { UI::warning("No jobs in queue."); UI::enter(); return; }

        cout << left
            << setw(8) << "ID"
            << setw(30) << "Document"
            << setw(10) << "Priority"
            << setw(8) << "Pages"
            << setw(12) << "Status" << "\n";
        UI::line('-', 80);

        for (int i = 0; i < cnt; ++i) {
            auto j = jobs[i];
            string stat;
            int col = Colors::MENU;
            switch (j->status) {
            case QUEUED:    stat = "Queued";    col = Colors::INFO;    break;
            case DELAYED:   stat = "Delayed";   col = Colors::WARNING; break;
            case COMPLETED: stat = "Done";      col = Colors::SUCCESS; break;
            case CANCELLED: stat = "Cancelled"; col = Colors::error1;   break;
            }
            Colors::set(Colors::MENU);
            cout << setw(8) << j->jobID
                << setw(30) << (j->documentName.size() > 28 ? j->documentName.substr(0, 25) + "..." : j->documentName);
            Colors::set(j->priority <= 2 ? Colors::error1 : j->priority <= 3 ? Colors::WARNING : Colors::INFO);
            cout << setw(10) << j->priority;
            Colors::set(Colors::MENU);
            cout << setw(8) << j->pageCount;
            Colors::set(col);
            cout << setw(12) << stat << '\n';
            Colors::reset();
        }
        UI::line('-', 80);
        cout << "Total jobs: " << cnt << "\n\n";
        delete[] jobs;
        UI::enter();
    }

    void simulatePrinting() {
        UI::header("SIMULATE PRINTING");
        cout << "Starting " << PRINTERS << " printers...\n\n";
        int done = 0, skipped = 0;
        while (!heap->empty() && done < PRINTERS) {
            int id = heap->extract();
            auto j = map->find(id);
            if (!j || j->status == CANCELLED || j->status == COMPLETED) { skipped++; continue; }
            if (j->status == DELAYED) {
                Colors::set(Colors::WARNING);
                cout << "[Printer " << (done + 1) << "] Skipping Job " << id << " (delayed)\n";
                Colors::reset();
                skipped++; continue;
            }
            Colors::set(Colors::SUCCESS);
            cout << "[Printer " << (done + 1) << "] Printing Job " << id << " - \"" << j->documentName << "\"\n";
            Colors::reset();
            j->status = COMPLETED;
            done++;
            sleep_ms(600);
        }
        cout << "\nFinished! Processed: " << done << " | Skipped: " << skipped << "\n";
        UI::enter();
    }

    ~PrintSpooler() {
        saveState();
        delete heap;
        delete map;
    }
};

// ==================== MENUS ====================
void mainMenu(UserAuth& auth, PrintSpooler*& spooler) {
    UI::header("MAIN MENU");
    Colors::set(Colors::SUCCESS);
    cout << "Logged in as: " << auth.username() << "\n\n";
    Colors::reset();
    UI::menu(1, "Add Print Job");
    UI::menu(2, "Cancel Job");
    UI::menu(3, "List All Jobs");
    UI::menu(4, "Simulate Printing");
    UI::menu(5, "Save & Exit");
    UI::menu(6, "Logout");

    int ch; cout << "\nChoice: "; cin >> ch;
    if (!spooler) return;

    switch (ch) {
    case 1: spooler->addJob(); break;
    case 2: spooler->cancelJob(); break;
    case 3: spooler->listJobs(); break;
    case 4: spooler->simulatePrinting(); break;
    case 5: delete spooler; spooler = nullptr; exit(0);
    case 6: delete spooler; spooler = nullptr; auth.logout(); break;
    }
}

int main() {
    UserAuth auth;
    PrintSpooler* spooler = nullptr;

    while (true) {
        if (!auth.loggedIn()) {
            UI::banner();
            UI::menu(1, "Login");
            UI::menu(2, "Sign Up");
            UI::menu(3, "Exit");
            int ch; cout << "\nChoice: "; cin >> ch;
            if (ch == 1) { if (auth.login()) spooler = new PrintSpooler(); }
            else if (ch == 2) auth.signup();
            else return 0;
        }
        else {
            mainMenu(auth, spooler);
        }
    }
}