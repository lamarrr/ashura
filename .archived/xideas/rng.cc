
namespace impl {

static constexpr auto make_u8_power2_lookup_table() {
  std::array<bool, 256> table{};
  uint8_t next_power_of_2 = 1;
  for (uint16_t value = 1; value < 256; value++) {
    if (next_power_of_2 == value) {
      table[value] = true;
      next_power_of_2 = next_power_of_2 * 2;
    }
  }
  return table;
}

}  // namespace impl

  static constexpr auto u8_power2_lookup_table =
      impl::make_u8_power2_lookup_table();

  static constexpr bool is_power_of_2(uint64_t number) {
    return u8_power2_lookup_table[number & 0xFFUL] &&
           u8_power2_lookup_table[(number >> 8) & 0xFFUL] &&
           u8_power2_lookup_table[(number >> 16) & 0xFFUL] &&
           u8_power2_lookup_table[(number >> 24) & 0xFFUL];
  }