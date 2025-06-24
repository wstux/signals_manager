# Signal manager.

## Description

The signal manager allows you to install custom signal handlers. Signal
processing is performed in a separate thread.

The manager accepts as input the numbers of signals that will be intercepted and
processed, and their handlers. These handlers are registered inside the manager,
and for signals, a blocking is performed in the current thread and a default
handler is registered.

Signal processing occurs in a separate thread, which personally removes locks
for registered signals. Handlers cannot be added or removed while the signal
processing thread is running.

When the manager is destroyed, the installed signal blocks are unblocked, and
all handlers are deregistered.

Work order:
1. the main thread registers the necessary handlers and blocks the registered
   signals;
2. all subsequent threads inherit the handler block mask;
3. when the application is ready for work, the manager starts a separate thread
   to receive signals;
4. the signal processing thread removes blocks from registered signals and waits
   for them to be triggered;
5. when a signal is received, the signal is processed in the processing
   thread - information from the signal is placed in a queue and informs about
   the presence of a signal, after which signal processing is completed.
6. control returns to the processing thread, which locks all registered signals,
   reads signals from the queue and calls real registered handlers for them;
7. go to 4.

Signals can be triggered not only in the signal processing thread. If someone
removes a block from their thread or starts a new thread before all handlers are
registered, then signals will start to be triggered in that thread as well. To
protect against such situations, a lock-free signal queue is used, and the
signal processing thread will also see the presence of a new signal via a
semaphore.

## License

&copy; 2024 Chistyakov Alexander.

Open sourced under MIT license, the terms of which can be read here — [MIT License](http://opensource.org/licenses/MIT).

