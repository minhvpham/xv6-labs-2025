#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

// user/sysinfotest.c

void
testloadavg(void) {
  struct sysinfo info;
  int n_stress = 2;
  int pids[2];

  printf("\n--- Starting Load Average Test ---\n");

  // 1. Create artificial load
  for(int i = 0; i < n_stress; i++){
    int pid = fork();
    if(pid == 0){
      // Child spins forever to ensure state is RUNNING
      while(1);
    }
    pids[i] = pid;
  }

  // 2. Wait long enough for the kernel to trigger updates (ticks % 100)
  // We sleep 300 ticks to get ~3 updates
  sleep(300);

  // 3. Measure
  if (sysinfo(&info) < 0) {
    printf("FAIL: sysinfo failed\n");
    exit(1);
  }

  // 4. Print
  int whole = (int)(info.loadavg >> 16);
  int frac  = (int)(((info.loadavg & 0xFFFF) * 100) / 65536);
  printf("Load Average: %d.%d\n", whole, frac);

  // 5. Cleanup
  for(int i = 0; i < n_stress; i++){
    kill(pids[i]);
    wait(0);
  }
  printf("--- End Load Average Test ---\n\n");
}
void
sinfo(struct sysinfo *info) {
  if (sysinfo(info) < 0) {
    printf("FAIL: sysinfo failed\n");
    exit(1);
  }
}
//
// use sbrk() to count how many free physical memory pages there are.
//
int
countfree()
{
  uint64 sz0 = (uint64)sbrk(0);
  struct sysinfo info;
  int n = 0;

  while(1){
    if((uint64)sbrk(PGSIZE) == 0xffffffffffffffff){
      break;
    }
    n += PGSIZE;
  }
  sinfo(&info);
  if (info.freemem != 0) {
    printf("FAIL: there is no free mem, but sysinfo.freemem=%ld\n",
      info.freemem);
    exit(1);
  }
  sbrk(-((uint64)sbrk(0) - sz0));
  return n;
}

void
testmem() {
  struct sysinfo info;
  uint64 n = countfree();
  
  sinfo(&info);

  if (info.freemem!= n) {
    printf("FAIL: free mem %ld (bytes) instead of %ld\n", info.freemem, n);
    exit(1);
  }
  
  if((uint64)sbrk(PGSIZE) == 0xffffffffffffffff){
    printf("sbrk failed");
    exit(1);
  }

  sinfo(&info);
    
  if (info.freemem != n-PGSIZE) {
    printf("FAIL: free mem %ld (bytes) instead of %ld\n", n-PGSIZE, info.freemem);
    exit(1);
  }
  
  if((uint64)sbrk(-PGSIZE) == 0xffffffffffffffff){
    printf("sbrk failed");
    exit(1);
  }

  sinfo(&info);
    
  if (info.freemem != n) {
    printf("FAIL: free mem %ld (bytes) instead of %ld\n", n, info.freemem);
    exit(1);
  }
}

void
testcall() {
  struct sysinfo info;
  
  if (sysinfo(&info) < 0) {
    printf("FAIL: sysinfo failed\n");
    exit(1);
  }

  if (sysinfo((struct sysinfo *) 0xeaeb0b5b00002f5e) !=  0xffffffffffffffff) {
    printf("FAIL: sysinfo succeeded with bad argument\n");
    exit(1);
  }
}

void testproc() {
  struct sysinfo info;
  uint64 nproc;
  int status;
  int pid;
  
  sinfo(&info);
  nproc = info.nproc;

  pid = fork();
  if(pid < 0){
    printf("sysinfotest: fork failed\n");
    exit(1);
  }
  if(pid == 0){
    sinfo(&info);
    if(info.nproc != nproc+1) {
      printf("sysinfotest: FAIL nproc is %ld instead of %ld\n", info.nproc, nproc+1);
      exit(1);
    }
    exit(0);
  }
  wait(&status);
  sinfo(&info);
  if(info.nproc != nproc) {
      printf("sysinfotest: FAIL nproc is %ld instead of %ld\n", info.nproc, nproc);
      exit(1);
  }
}

void testbad() {
  int pid = fork();
  int xstatus;
  
  if(pid < 0){
    printf("sysinfotest: fork failed\n");
    exit(1);
  }
  if(pid == 0){
      sinfo(0x0);
      exit(0);
  }
  wait(&xstatus);
  if(xstatus == -1)  // kernel killed child?
    exit(0);
  else {
    printf("sysinfotest: testbad succeeded %d\n", xstatus);
    exit(xstatus);
  }
}

int
main(int argc, char *argv[])
{
  printf("sysinfotest: start\n");
  testcall();
  testmem();
  testproc();
  testloadavg();
  printf("sysinfotest: OK\n");
  exit(0);
}
