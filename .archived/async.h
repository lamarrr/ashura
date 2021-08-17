  void executor___lock_result() {
    // this should only be locked for a very short period of time with ideally little to no contention.
    // this is enforced using the Future's .copy and .move methods
    while (!user___try_lock_result()) {
    }
  }

  void executor___unlock_result() {
user___unlock_result();
  }