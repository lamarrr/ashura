// TODO(lamarrr): identifying widgets across frames

// TODO(lamarrr): we might need request detach so child widgets can request to
// be removed and remove all callbacks they may have attached or cancel tasks
// they have pending.
// consider: having tokens that de-register themselves once deleted

// TODO(lamarrr): we need re-calculable offsets so we can shift the parents
// around without shifting the children this is important for cursors, drag
// and drop? this might mean we need to totally remove the concept of area.
// storing transformed area might not be needed?
// TODO: how to circumvent using pointers for collecting children?
