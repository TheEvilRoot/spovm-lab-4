#include <cstdio>
#include <vector>
#include <csignal>
#include <unistd.h>
#include <ctime>

#include <pthread.h>

struct thread_t {
  pthread_t pthread;
  bool isPrintingNow;
};

static std::vector<thread_t*> threads;
static pthread_mutex_t printMutex;

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

void *threadHandler(void* args) {
  auto self = (thread_t*) args;
  srand(time(nullptr));
  char *string = getRandomString(10);
  while (true) {
    pthread_mutex_lock(&printMutex);
    self->isPrintingNow = true;
    fprintf(stderr, "\r");
    for (int i = 0; string[i] != 0; i++) {
      fprintf(stderr, "%c", string[i]);
      usleep(100000);
    }
    fprintf(stderr, "\n");
    pthread_mutex_unlock(&printMutex);
    self->isPrintingNow = false;
  }
}

void addThread() {
  auto *thread = new thread_t { { }, false };
  int errorNum = 0;
  if ((errorNum = pthread_create(&thread->pthread, nullptr, threadHandler, thread)) == 0) {
    threads.push_back(thread);
  } else {
    fprintf(stderr, "Failed to create thread. Error %d\n", errorNum);
    delete thread;
  }
}

void terminateThread() {
  if (!threads.empty()) {
    auto thread = threads.back();
    threads.pop_back();
    pthread_cancel(thread->pthread);
    if (thread->isPrintingNow)
      pthread_mutex_unlock(&printMutex);
    delete thread;
  }
}

void terminateAllThreads() {
  while (!threads.empty())
    terminateThread();
}

bool handleInput() {
  int c = getchar();
  switch(c) {
    case '+': addThread(); break;
    case '-': terminateThread(); break;
    case 'q': terminateAllThreads(); return false;
    default: break;
  }
  return true;
}

int main() {
  int mutexError = 0;
  if ((mutexError = pthread_mutex_init(&printMutex, nullptr)) != 0) {
      fprintf(stderr, "Failed to create mutex. Error: %d\n", mutexError);
      return 0;
  }
  system("/bin/stty raw");
  while (handleInput());
  system("/bin/stty cooked");

  pthread_mutex_destroy(&printMutex);
  return 0;
}

