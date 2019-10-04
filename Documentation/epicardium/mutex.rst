Mutex
=====
Ontop of FreeRTOS, we have our own mutex implementation.  **Never use the
FreeRTOS mutexes directly!  Always use this abstraction layer instead**.  This
mutex implementation tries to make reasoning about program flow and locking
behavior easier.  And most importantly tries to help with debugging possible
dead-locks.

Design
------
There are a few guiding design principles:

- Mutexes can only be used from tasks, **never** from interrupts!
- Timers can use mutexes, but only with :c:func:`mutex_trylock`, **never** with
  :c:func:`mutex_lock` (Because they are not allowed to block).
- Locking can *never* fail (if it does, we consider this a fatal error ⇒ panic).
- No recursive locking.
- An unlock can only occur from the task which previously acquired the mutex.
- An unlock is only allowed if the mutex was previously acquired.

For a more elaborate explanation of the rationale behind these rules take a
look at the :ref:`mutex-design-reasons`.

Definitions
-----------
.. c:autodoc:: epicardium/modules/mutex.h

.. _mutex-design-reasons:

Reasons for this Design
-----------------------

Locking can *never* fail
^^^^^^^^^^^^^^^^^^^^^^^^
This might seem like a bold claim at first but in the end, it is just a matter
of definition and shifting responsibilities.  Instead of requiring all code to
be robust against a locking attempt failing, we require all code to properly
lock and unlock their mutexes and thus never producing a situation where
locking would fail.

Because all code using any of the mutexes is contained in the Epicardium
code-base, we can - *hopefully* - audit it properly behaving ahead of time and
thus don't need to add code to ensure correctness at runtime.  This makes
downstream code easier to read and easier to reason about.

History of this project has shown that most code does not properly deal with
locking failures anyway: There was code simply skipping the mutexed action on
failure, code blocking a module entirely until reboot, and worst of all: Code
exposing the locking failure to 'user-space' (Pycardium) instead of retrying.
This has lead to spurious errors where technically there would not need to be
any.

Only from tasks
^^^^^^^^^^^^^^^
Locking a mutex from an ISR, a FreeRTOS software timer or any other context
which does not allow blocking is complicated to do right.  The biggest
difficulty is that a task might be holding the mutex during execution of such a
context and there is no way to wait for it to release the mutex.  This requires
careful design of the program flow to choose an alternative option in such a
case.  A common approach is to 'outsource' the relevant parts of the code into
an 'IRQ worker' which is essentially just a task waiting for the IRQ to wake it
up and then attempts to lock the mutex.

If you absolutely do need it (and for legacy reasons), software timers *can*
lock a mutex using :c:func:`mutex_trylock` (which never blocks).  I strongly
recommend **not** doing that, though.  As shown above, you will have to deal
with the case of the mutex being held by another task and it is very well
possible that your timer will get starved of the mutex because the scheduler
has no knowledge of its intentions.  In most cases, it is a better idea to use
a task and attempt locking using :c:func:`mutex_lock`.

.. todo::

   We might introduce a generic IRQ worker queue system at some point.

No recursive locking
^^^^^^^^^^^^^^^^^^^^
Recursive locking refers to the ability to 'reacquire' a mutex already held by
the current task, deeper down in the call-chain.  Only the outermost unlock
will actually release the mutex.  This feature is sometimes implemented to
allow more elegant abstractions where downstream code does not need to know
about the mutexes upstream code uses and can still also create a larger region
where the same mutex is held.

But exactly by hiding the locking done by a function, these abstractions make
it hard to trace locking chains and in some cases even make it impossible to
create provably correct behavior.  As an alternative, I would suggest using
different mutexes for the different levels of abstraction.  This also helps
keeping each mutex separated and 'local' to its purpose.

Only unlock from the acquiring task
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Because of the above mentioned mutex locking semantics, there should never be a
need to force-unlock a forgein mutex.  Even in cases of failures, all code
should still properly release all mutexes it holds.  One notable exceptions is
``panic()``\s which will abort all ongoing operations anyway.

Only unlock once after acquisition
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Justified with an argument of robustness, sometimes the :c:func:`mutex_unlock`
call is written in a way that allows unlocking an already unlocked mutex.  But
robustness of downstream code will not really be improved by the upstream API
dealing with arguably invalid usage.  For example, this could encourage
practices like unlocking everything again at the end of a function "just to be
sure".

Instead, code should be written in a way where the lock/unlock pair is
immediately recognizable as belonging together and is thus easily auditable to
have correct locking behavior.  A common pattern to help with readability in
this regard is the *Single Function Exit* which looks like this:

.. code-block:: cpp

   int function()
   {
           int ret;
           mutex_lock(&some_mutex);

           ret = foo();
           if (ret) {
                   /* Return with an error code */
                   ret = -ENODEV;
                   goto out_unlock;
           }

           ret = bar();
           if (ret) {
                   /* Return the return value from foo */
                   goto out_unlock;
           }

           ret = 0;
   out_unlock:
           mutex_unlock(&some_mutex);
           return ret;
   }
