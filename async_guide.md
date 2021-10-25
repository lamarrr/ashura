_guide



TASKS vs Functions


Tasks must be re-entrant
Tasks must be stateless
Tasks must be re-tryable
Tasks must not mutate global or local state
Tasks must not hold or use locks. (i.e. if it panics whilst doing so, there'll be problems)
Tasks must not block with useless work (i.e. waiting on an event)
Tasks must do useful work for a reasonable amount of time ( ideally microseconds and above )
Tasks must not share state with other tasks
For inter-thread communication use Streams, Generators, Future, and Promises
Tasks must not throw exceptions (clarify that it should be handled)
Tasks must not panic unless absolutely necessary