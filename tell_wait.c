#include "csapp.h"
#include "tell_wait.h"

volatile sig_atomic_t sigflag_child;
volatile sig_atomic_t sigflag_parent;
sigset_t newmask, oldmask;

void sigusr1_handler(int signo)	
{
    sigflag_child = 1;
}

void sigusr2_handler(int signo)	
{
    sigflag_parent = 1;
}

void init_synchro()
{
	// Register signal handlers
	// They will be inherited by the child
    Signal(SIGUSR1, sigusr1_handler);
    Signal(SIGUSR2, sigusr2_handler);
		
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    sigaddset(&newmask, SIGUSR2);

    // Add SIGUSR1 and SIGUSR2 to the list of blocked signals
    // and save the previous signal mask
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
        unix_error("SIG_BLOCK error");
    }
}

void tell_parent()
{
    kill(getppid(), SIGUSR1);
}

void wait_parent()
{
    // Change the signal mask (SIGUSR1 and SIGUSR2 unblocked)
    // The while loop is required to make sure that
    // the process was awakened by the SIGUSR2 signal sent by its parent
    while (sigflag_parent == 0) {
        sigsuspend(&oldmask);
    }
    
    // When we reach this point, the signal mask is in the same state
    // as it was before calling sigsuspend
    // (oldmask + SIGUSR1 and SIGUSR2 blocked)
    
    // Reset signal flag
    sigflag_parent = 0;
}

void tell_child(pid_t p)
{
    kill(p, SIGUSR2);
}

void wait_child()
{
    // The while loop below is required to make sure that
    // the process was awakened by the SIGUSR1 signal sent by its child
    while (sigflag_child == 0) {
        sigsuspend(&oldmask);
    }
    
    // When we reach this point, the signal mask is in the same state
    // as it was before calling sigsuspend
    // (oldmask + SIGUSR1 and SIGUSR2 blocked)    
    
    // Reset signal flag
    sigflag_child = 0;
}

