#include <cstdio>
#include <ctime>
#include <vector>

#include <Windows.h>
#include <conio.h>

struct thread_t {
    HANDLE handle;
    bool isPrinting = false;
};

static std::vector<thread_t *> threads;
static CRITICAL_SECTION lock;

DWORD getThreadExitCode(const HANDLE& handle) { 
  DWORD exitCode;
  GetExitCodeThread(handle, &exitCode);
  return exitCode;
}

int getRandomInt(int min, int max) {
  return min + (rand() % static_cast<int>(max - min + 1));
}

char *getRandomString(size_t size) {
  char *string = new char[size + 1];
  for (size_t i = 0; i < size; i++) {
    if (getRandomInt(0, 3) > 1) {
      string[i] = static_cast<char>(getRandomInt('A', 'Z'));
    } else {
      string[i] = static_cast<char>(getRandomInt('a', 'z'));
    }
  }
  string[size] = 0;
  return string;
}

DWORD threadHandler(void *args) {
    auto* thread = (thread_t*)args;
    srand(GetCurrentThreadId());

    char *uniqueString = getRandomString(10);
    while (true) {
        EnterCriticalSection(&lock);
        thread->isPrinting = true;
        fprintf(stderr, "\r", GetCurrentThreadId());
        for (int i = 0; uniqueString[i] != 0; i++) {
            fprintf(stderr, "%c", uniqueString[i]);
            Sleep(100);
        }
        fprintf(stderr, "\n");
        LeaveCriticalSection(&lock);
        thread->isPrinting = false;
        Sleep(100);
    }
  return true;
}

void addThread() {
    auto* thread = new thread_t;
    HANDLE newThread = CreateThread(
        nullptr, 0, (LPTHREAD_START_ROUTINE)threadHandler, thread, 0, nullptr);
    thread->handle = newThread;
    if (newThread != nullptr) {
        threads.push_back(thread);
    } else {
        fprintf(stderr, "\rFailed to create thread. %d\n", GetLastError());
        delete thread;
    }
}

void terminateThread() {
  if (!threads.empty()) {
    auto* thread = threads.back();
    threads.pop_back();
    TerminateThread(thread->handle, 0);
    WaitForSingleObject(thread->handle, INFINITE);
    if (thread->isPrinting)
        LeaveCriticalSection(&lock);
    delete thread;
  }
}

void terminateAllThreads() {
  for (const auto &thread : threads) {
    TerminateThread(thread->handle, 0);
    WaitForSingleObject(thread->handle, INFINITE);
  }
}

void checkSelfTerminated() {
  std::vector<thread_t*> allocator;
  for (const auto &thread : threads) {
    if (getThreadExitCode(thread->handle) == STILL_ACTIVE) {
      allocator.push_back(thread);
    }
  }
  threads = allocator;
}
bool handleInput() {
  char c = _getch();
  switch (c) { 
  case '+': {
    addThread();
    break;
  }
  case '-': {
    terminateThread();
    break;
  }
  case 'q': {
    terminateAllThreads();
    return false;
  }
  }
  return true;
}

int main() {
  srand(time(nullptr));
  InitializeCriticalSection(&lock);
 
  while (handleInput()) {
    checkSelfTerminated();
  }

  DeleteCriticalSection(&lock);

  return 0;
}