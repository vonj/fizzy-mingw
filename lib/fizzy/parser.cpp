#include "parser.hpp"
#include "leb128.hpp"
#include <cassert>

namespace fizzy
{
template <>
struct parser<functype>
{
    parser_result<functype> operator()(const uint8_t* pos)
    {
        if (*pos != 0x60)
            throw parser_error{
                "unexpected byte value " + std::to_string(*pos) + ", expected 0x60 for functype"};
        ++pos;

        functype result;
        std::tie(result.inputs, pos) = parser<std::vector<valtype>>{}(pos);
        std::tie(result.outputs, pos) = parser<std::vector<valtype>>{}(pos);
        return {result, pos};
    }
};

module parse(bytes_view input)
{
    if (input.substr(0, wasm_prefix.size()) != wasm_prefix)
        throw parser_error{"invalid wasm module prefix"};

    input.remove_prefix(wasm_prefix.size());

    module mod;
    for (auto it = input.begin(); it != input.end();)
    {
        const auto id = static_cast<sectionid>(*it++);
        uint32_t size;
        std::tie(size, it) = leb128u_decode<uint32_t>(it);
        const auto expected_end_pos = it + size;
        switch (id)
        {
        case sectionid::type:
            std::tie(mod.typesec, it) = parser<std::vector<functype>>{}(it);
            break;
        default:
            it += size;
            break;
        }

        if (it != expected_end_pos)
            throw parser_error{"incorrect section " + std::to_string(static_cast<int>(id)) +
                               " size, difference: " + std::to_string(it - expected_end_pos)};
    }

    return mod;
}
}  // namespace fizzy
