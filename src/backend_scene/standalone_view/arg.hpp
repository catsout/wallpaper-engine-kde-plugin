#include <regex>
#include <tuple>
#include <utility>
#include <argparse/argparse.hpp>
#include <string_view>

constexpr std::string_view ARG_ASSETS      = "<assets>";
constexpr std::string_view ARG_SCENE       = "<scene>";
constexpr std::string_view OPT_VALID_LAYER = "--valid-layer";
constexpr std::string_view OPT_GRAPHVIZ    = "--graphviz";
constexpr std::string_view OPT_FPS         = "--fps";
constexpr std::string_view OPT_RESOLUTION  = "--resolution";
constexpr std::string_view OPT_CACHE_PATH  = "--cache-path";

struct Resolution {
	uint w;
	uint h;
};
std::ostream& operator<<(std::ostream& os, const Resolution& res) {
    return os << res.w << 'x' << res.h;
}

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

    arg.add_argument("-C", OPT_CACHE_PATH)
        .help("generate graphviz of render graph, output to 'graph.dot'")
        .default_value(std::string())
        .nargs(1)
        .append();

    arg.add_argument("-R", OPT_RESOLUTION)
        .help("Set the resolution, eg. 1920x1080")
        .default_value(Resolution{1280, 720})
        .implicit_value(true)
        .nargs(1)
        .append()
		.action([](const std::string& value) {
			const std::regex re_res(R"(([0-9]+)x([0-9]+))");
			std::smatch match;
			uint width = 1280, height = 720;
			if(std::regex_match(value, match, re_res)) {
				const std::string w_str = match[1].str(), h_str = match[2].str();
				std::from_chars(w_str.c_str(), w_str.c_str() + w_str.length(), width);
				std::from_chars(h_str.c_str(), h_str.c_str() + h_str.length(), height);
			}
			return Resolution{width, height};
		});

    try {
        arg.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << arg;
        std::exit(1);
    }
}
