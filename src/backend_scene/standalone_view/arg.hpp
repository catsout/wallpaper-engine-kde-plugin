#include <argparse/argparse.hpp>
#include <string_view>

constexpr std::string_view ARG_ASSETS      = "<assets>";
constexpr std::string_view ARG_SCENE       = "<scene>";
constexpr std::string_view OPT_VALID_LAYER = "--valid-layer";
constexpr std::string_view OPT_GRAPHVIZ    = "--graphviz";
constexpr std::string_view OPT_FPS         = "--fps";

void setAndParseArg(argparse::ArgumentParser& arg, int argc, char** argv) {
    arg.add_argument(ARG_ASSETS).help("assets folder").nargs(1);
    arg.add_argument(ARG_SCENE).help("scene file").nargs(1);

    arg.add_argument("-f", OPT_FPS)
        .help("fps")
        .default_value<int32_t>(15)
        .nargs(1)
        .scan<'i', int32_t>();

    arg.add_argument("-V", OPT_VALID_LAYER)
        .help("enable vulkan valid layer")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .append();

    arg.add_argument("-G", OPT_GRAPHVIZ)
        .help("generate graphviz of render graph, output to 'graph.dot'")
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .append();

    try {
        arg.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << arg;
        std::exit(1);
    }
}