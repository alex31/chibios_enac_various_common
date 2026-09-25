template <size_t typename HEAD, typename... TAILs>
constexpr bool bufferize(Endianness endns, uint16_t regAddr, std::function<bool(T, U), std::ARGs... args) {
    auto toSend = bufferize_w(endns, args...);
    auto toReceive = bufferize_r(endns, args...);
    return toSend;
}






















template <typename T>
template <size_t M>
auto test(T t) {
  auto std::array<std::byte, sizeof(T)> ar{};
  return ar;
}
