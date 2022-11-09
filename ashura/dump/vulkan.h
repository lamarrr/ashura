 //   {
  // std::ifstream stream{path, std::ios_base::ate | std::ios_base::binary};
  //
  // ASR_ENSURE(stream.is_open());
  //
  // size_t size = stream.tellg();
  //
  // ASR_ENSURE(size % 4 == 0);
  //
  // code.reserve(size / 4).unwrap();
  //
  // stream.seekg(0);
  //
  // stream.read(reinterpret_cast<char*>(code.data()), size);
  //   }