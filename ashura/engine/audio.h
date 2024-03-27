

f32 db_to_volume(f32 db)
{
  return powf(10, 0.05f * db);
}

// can generate u8 log table
f32 volume_to_db(f32 volume)
{
  return 20 * logf(volume);
}