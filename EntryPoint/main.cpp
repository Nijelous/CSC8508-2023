#include "../CSC8503/GameStart.cpp"

#ifdef USEPROSPERO
size_t sceUserMainThreadStackSize = 4 * 1024 * 1024;
extern const char sceUserMainThreadName[] = "TeamProjectGameMain";
int sceUserMainThreadPriority = SCE_KERNEL_PRIO_FIFO_DEFAULT;
size_t sceLibcHeapSize = 257 * 1024 * 1024;
#endif

int main() {
	RunGame();
}
