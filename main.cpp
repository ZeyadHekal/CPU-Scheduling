#include <iostream>
#include <iomanip>
#include <string>
using namespace std;
bool run = true;
enum states {RUNNING = 1, READY, NOT_BORN, FINISHED};

struct PCB {
    string name;
    int id, birthtime, instructionsTime, priority, state, totalWait, lastRunning;
    int static counter;
    bool started;
    PCB(string name, int birthtime, int instructionsTime, int priority) {
        id = counter++;
        this->name = name;
        this->birthtime = birthtime;
        this->lastRunning = birthtime;
        this->totalWait = 0;
        this->instructionsTime = instructionsTime;
        this->priority = priority;
        state = NOT_BORN;
        started = false;
    }
    PCB(const PCB &x) {
        name = x.name;
        id = x.id;
        birthtime = x.birthtime;
        instructionsTime = x.instructionsTime;
        priority = x.priority;
        state = x.state;
        started = x.started;
        lastRunning = x.birthtime;
        totalWait = x.totalWait;
    }
    PCB() {}
    friend bool operator ==(const PCB &x, const PCB &y) {
        return x.id == y.id;
    }
};
int PCB::counter = 1;

struct linkeditem {
    PCB* element;
    linkeditem* next;
    linkeditem(PCB* x) {
        element = x;
        next = NULL;
    }
    linkeditem() {
        element = NULL;
        next = NULL;
    }
};

struct LinkedList {
    int size;
    linkeditem* head;
    linkeditem* tail;
    LinkedList() {
        head = new linkeditem();
        tail = head;
        size = 0;
    }
    linkeditem* add(PCB* x) {
        linkeditem* elem = new linkeditem(x);
        tail->next = elem;
        tail = elem;
        size++;
        return elem;
    }
    void _delete(linkeditem* x) {
        linkeditem* del = x->next;
        if (del == NULL)
            return;
        if (del == tail)
            tail = x;
        x->next = del->next;
        delete del;
        size--;
    }
    void clear() {
        while (head->next != NULL) {
            _delete(head);
        }
    }
};

LinkedList myList = LinkedList();

void runProgram();
void showProcesses();
void addProcess();
void removeProcess();
void showScheduling();

int main() {
    cout << "#FreePalestine\n";
    cout << "Welcome to the CPU Scheduling emulator!\n";
    while (run) {
        runProgram();
    }
    return 0;
}

int chooseProgam() {
    cout << "\nYou currently have " << myList.size << " processes.\n";
    cout << "What would you like to do?\n";
    cout << "1- Show all processes\n";
    cout << "2- Add a process\n";
    cout << "3- Remove a process\n";
    cout << "4- Preemptive Priority Scheduling\n";
    cout << "5- Non-Preemptive Priority Scheduling\n";
    cout << "6- FCFS Scheduling\n";
    cout << "7- Shortest Job First (SJF) Preemptive Scheduling\n";
    cout << "8- Shortest Job First (SJF) Non-Preemptive Scheduling\n";
    cout << "9- Robin Round Scheduling\n";
    cout << "0- Exit program\n";
    cout << "Please enter the number that you want: ";
    int x;
    cin >> x;
    if (x < 0 || x > 9) {
        cout << "Invalid input, please choose a valid number.\n";
        return chooseProgam();
    }
    return x;
}

int type, quantumTime;

void runProgram() {
    type = chooseProgam();
    cout << '\n';
    if (type == 0) {
        run = false;
        cout << "Thank you for using CPU Scheduling emulator!\n";
    } else if (type == 1) {
        showProcesses();
    } else if (type == 2) {
        addProcess();
    } else if (type == 3) {
        removeProcess();
    } else {
        if (type == 9) {
            quantumTime = -1;
            while (quantumTime <= 0) {
                cout << "Please enter quantum time (greater than 0): ";
                cin >> quantumTime;
            }
        }
        showScheduling();
    }
}

void showProcesses() {
    cout << "Here are the processes that you currently have ( " << myList.size << " process" << ((myList.size != 1)? "es": "") <<" ):\n";
    linkeditem* cur = myList.head;
    while (cur->next != NULL) {
        PCB* p = cur->next->element;
        cout << "Pid: " << p->id << ", Name:" << p->name << ", Birth: " << p->birthtime << ", instructions: " << p->instructionsTime << ", Priority: " << p->priority << '\n';
        cur = cur->next;
    }
}

void addProcess() {
    string name;
    int birthtime = -1, instructionsTime = -1, priority = -1;
    cout << "Enter process name: ";
    cin >> name;
    while (birthtime < 0) {
        cout << "Enter arrival time (non-negative time when this process will be created): ";
        cin >> birthtime;
    }
    while (instructionsTime < 1) {
        cout << "Enter burst time (positive estimated run-time for this process): ";
        cin >> instructionsTime;
    }
    while (priority < 1 || priority > 10) {
        cout << "Enter priority for this process (1-10, 1 is highest, 10 is lowest): ";
        cin >> priority;
    }
    PCB* proc = new PCB(name, birthtime, instructionsTime, priority);
    myList.add(proc);
}

linkeditem* getProcess(int pid) {
    linkeditem* cur = myList.head;
    while (cur->next != NULL) {
        if (cur->next->element->id == pid)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

void removeProcess() {
    if (myList.size == 0) {
        cout << "You don't have any processes to remove!\n";
        return;
    }
    showProcesses();
    cout << "Choose the id of the process you want to remove: ";
    int pid;
    cin >> pid;
    linkeditem* process = getProcess(pid);
    if (process == NULL) {
        cout << "There is no process with this ID!\n";
    } else {
        myList._delete(process);
        cout << "Process with ID " << pid << " has been successfully deleted.\n";
    }
}

void swap(PCB*& x, PCB*& y) {
    PCB* temp = x;
    x = y;
    y = temp;
}

int finished, sec, lastUpdateSec, processWaitSize;
PCB** ar = NULL;
PCB** processWait = NULL;
void sortMyProcesses();
void showWaitingTime();

void showScheduling() {
    if (ar != NULL) {
        delete ar;
        delete processWait;
    }
    ar = new PCB*[myList.size];
    processWait = new PCB*[myList.size];
    int i = 0;
    linkeditem* cur = myList.head;
    while (cur->next != NULL) {
        ar[i] = new PCB(*(cur->next->element));
        cur = cur->next;
        i++;
    }
    finished = 0;
    sec = 0, lastUpdateSec = 0;
    PCB* last = NULL;
    //sortMyProcesses();
    cout << "------------ Simulation is starting from time 0.\n\n";
    while (finished < myList.size) {
        if (last != NULL) {
            last->instructionsTime--;
        }
        sortMyProcesses();
        PCB* now = NULL;
        if (ar[0]->state == READY || ar[0]->state == RUNNING)
            now = ar[0];
        if (last != now) {
            int lastBurst = sec-lastUpdateSec;
            if (last != NULL) {
                last->started = 1;
                last->lastRunning = sec;
                cout << "Process " << last->name << "(" << last->id << ") was in running state for " << lastBurst << " second" << (lastBurst == 1? "": "s") << ".\n";
                cout << "Process " << last->name;
                if (last->state == FINISHED)
                    cout << " has completed its burst time and is being terminated.\n";
                else
                    cout << " has been interrupted, remaining time for the process is " << last->instructionsTime << ".\n";
            } else if (lastBurst) {
                cout << "CPU is idle for " << lastBurst << " second" << (lastBurst == 1? "": "s") << ".\n";
            }
            if (now != NULL) {
                now->totalWait += sec-(now->lastRunning);
                cout << "Process " << now->name << "(" << now->id << ")" << " is ";
                if (now->started) {
                    cout << "continuing where it was and is in running state now.\n";
                } else
                    cout << "being executed for the first time and is in running state now.\n";
            } else {
                cout << "No ready processes, CPU will be idle\n";
            }
            lastUpdateSec = sec;
            last = now;
        }
        sec++;
    }
    cout << "\n------------ Simulation has been completed!\n\n";
    showWaitingTime();
}

void sortProcesses(PCB**& ar);

void showWaitingTime() {
    double totalWait = 0;
    PCB** processes;
    // implementation for robin round is different so if it's selected get its processes
    if (type == 9)
        processes = processWait;
    else
        processes = ar;
    sortProcesses(processes);
    cout << "Waiting time for processes:\n";
    for (int i = 0; i < myList.size; i++) {
        cout << "Process " << (processes[i]->name) << "(" << (processes[i]->id) << ") has waiting time: " << processes[i]->totalWait << '\n';
        totalWait += processes[i]->totalWait;
    }
    double avg = totalWait/myList.size;
    cout << "\nWith average waiting time " << fixed << setprecision(3) << avg << " seconds.\n\n";
    cout << "Press any key to continue using the program...";
    system("pause");
}

void sortProcesses(PCB**& ar) {
    for (int i = 0; i < myList.size-1; i++) {
        for (int j = i+1; j < myList.size; j++) {
            if (ar[i]->id > ar[j]->id) {
                swap(ar[i], ar[j]);
            }
        }
    }
}

void preemptivePriority();
void nonPreemptivePriority();
void FCFS();
void preemptiveSJF();
void nonPreemptiveSJF();
void robinRound();

void sortMyProcesses() {
    if (type == 4)
        preemptivePriority();
    else if (type == 5)
        nonPreemptivePriority();
    else if (type == 6)
        FCFS();
    else if (type == 7)
        preemptiveSJF();
    else if (type == 8)
        nonPreemptiveSJF();
    else
        robinRound();
}

int nextUpdate = 0;
LinkedList robinList = LinkedList();

void robinRound() {
    if (sec == 0) {
        nextUpdate = 0;
        robinList.clear();
        processWaitSize = 0;
    }
    for (int i = 0; i < myList.size; i++) {
        if (ar[i]->state == NOT_BORN && ar[i]->birthtime == sec) {
            robinList.add(ar[i]);
        }
    }
    if (ar[0]->instructionsTime != 0 && ar[0]->state == RUNNING && sec >= nextUpdate) {
        robinList.add(ar[0]);
        ar[0]->state = READY;
    }
    if (ar[0]->instructionsTime == 0 && ar[0]->state == RUNNING) {
        processWait[processWaitSize++] = ar[0];
    }
    if (sec < nextUpdate) {
        return;
    }
    if (ar[0]->instructionsTime == 0 && ar[0]->state != FINISHED) {
        ar[0]->state = FINISHED;
        finished++;
    }
    if (robinList.size != 0) {
        PCB* temp = robinList.head->next->element;
        robinList._delete(robinList.head);
        if (temp->instructionsTime > quantumTime) {
            nextUpdate = sec+quantumTime;
        } else {
            nextUpdate = sec+temp->instructionsTime;
        }
        ar[0] = temp;
        ar[0]->state = RUNNING;
    }
}

void SJF_Sort();
void preemptiveSJF() {
    for (int i = 0; i < myList.size; i++) {
        if (ar[i]->state == NOT_BORN && ar[i]->birthtime == sec) {
            ar[i]->state = READY;
        }
        if (ar[i]->instructionsTime == 0 && ar[i]->state == READY) {
            ar[i]->state = FINISHED;
            finished++;
        }
    }
    SJF_Sort();
}

void nonPreemptiveSJF() {
    for (int i = 0; i < myList.size; i++) {
        if (ar[i]->state == NOT_BORN && ar[i]->birthtime == sec) {
            ar[i]->state = READY;
        }
        if (ar[i]->instructionsTime == 0 && ar[i]->state == RUNNING) {
            ar[i]->state = FINISHED;
            finished++;
        }
    }
    SJF_Sort();
    if (ar[0]->state == READY)
        ar[0]->state = RUNNING;
}

void SJF_Sort() {
    for (int i = 0; i < myList.size-1; i++) {
        for (int j = i+1; j < myList.size; j++) {
            if (ar[i]->state != ar[j]->state) {
                if (ar[j]->state < ar[i]->state)
                    swap(ar[i], ar[j]);
            } else if (ar[i]->instructionsTime != ar[j]->instructionsTime) {
                if (ar[i]->instructionsTime > ar[j]->instructionsTime)
                    swap(ar[i], ar[j]);
            } else {
                if (ar[i]->birthtime > ar[j]->birthtime)
                    swap(ar[i], ar[j]);
            }
        }
    }
}

void FCFS() {
    for (int i = 0; i < myList.size; i++) {
        if (ar[i]->state == NOT_BORN && ar[i]->birthtime == sec) {
            ar[i]->state = READY;
        }
        if (ar[i]->instructionsTime == 0 && ar[i]->state == READY) {
            ar[i]->state = FINISHED;
            finished++;
        }
    }
    for (int i = 0; i < myList.size-1; i++) {
        for (int j = i+1; j < myList.size; j++) {
            if (ar[i]->state != ar[j]->state) {
                if (ar[j]->state < ar[i]->state)
                    swap(ar[i], ar[j]);
            } else {
                if (ar[i]->birthtime > ar[j]->birthtime)
                    swap(ar[i], ar[j]);
            }
        }
    }
}

void prioritySort();
void nonPreemptivePriority() {
    for (int i = 0; i < myList.size; i++) {
        if (ar[i]->state == NOT_BORN && ar[i]->birthtime == sec) {
            ar[i]->state = READY;
        }
        if (ar[i]->instructionsTime == 0 && ar[i]->state == RUNNING) {
            ar[i]->state = FINISHED;
            finished++;
        }
    }
    prioritySort();
    if (ar[0]->state == READY)
        ar[0]->state = RUNNING;
}

void preemptivePriority() {
    for (int i = 0; i < myList.size; i++) {
        if (ar[i]->state == NOT_BORN && ar[i]->birthtime == sec) {
            ar[i]->state = READY;
        }
        if (ar[i]->instructionsTime == 0 && ar[i]->state == READY) {
            ar[i]->state = FINISHED;
            finished++;
        }
    }
    prioritySort();
}

void prioritySort() {
    for (int i = 0; i < myList.size-1; i++) {
        for (int j = i+1; j < myList.size; j++) {
            if (ar[i]->state != ar[j]->state) {
                if (ar[j]->state < ar[i]->state)
                    swap(ar[i], ar[j]);
            } else if (ar[i]->priority != ar[j]->priority) {
                if (ar[i]->priority > ar[j]->priority)
                    swap(ar[i], ar[j]);
            } else {
                if (ar[i]->birthtime > ar[j]->birthtime)
                    swap(ar[i], ar[j]);
                else if (ar[i]->birthtime == ar[j]->birthtime && ar[i]->id > ar[j]->id)
                    swap(ar[i], ar[j]);
            }
        }
    }
}
