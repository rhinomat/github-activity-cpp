#include <algorithm>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void print_json(nlohmann::ordered_json info);
void print_key(nlohmann::ordered_json info);

int main(int argc, char **argv) {
  std::string gh_user;
  std::string prime_url = "https://api.github.com/users/";
  nlohmann::ordered_json GOOD_INFO;
  nlohmann::ordered_json SUBLINKS;
  nlohmann::json AVATAR;

  if (argc == 1) {
    std::cout << "Error: Please Enter GitHub Username for Procession!"
              << std::endl;
    std::cout << "Cannot Fulfill Request without Github Username :("
              << std::endl;
    return 1;
  } else if (argc > 2) {
    std::cout << "Please Only Input 1 Github Username" << std::endl;
    return 1;
  }

  gh_user = argv[1];
  std::string user_url = prime_url + gh_user;
  std::cout << std::format("Github Username: {}", gh_user) << std::endl;

  cpr::Response r = cpr::Get(cpr::Url{user_url},
                             cpr::Header{{"User-Agent", "github-activity"}});

  if (r.status_code == 200) {
    std::cout << std::format("GitHub Connection Successful") << std::endl;
    std::cout << std::format("Content Type: {}", r.header["content-type"])
              << std::endl;

    try {
      nlohmann::ordered_json data = nlohmann::ordered_json::parse(r.text);
      for (const auto &[key, value] : data.items()) {
        if (value != nullptr) {
          std::string value_str =
              value.is_string() ? value.get<std::string>() : value.dump();
          if (value_str.starts_with(user_url) && value_str != user_url)
            SUBLINKS[key] = value;
          else if (key == "avatar_url")
            AVATAR[key] = value;
          else if (!value_str.starts_with(user_url) && key != "avatar_url")
            GOOD_INFO[key] = value;
        }
      }
    } catch (nlohmann::json::parse_error &e) {
      std::cerr << std::format("Parsing Error: {}", e.what()) << std::endl;
    }

    std::string avatar_url;
    for (const auto &[key, value] : AVATAR.items())
      avatar_url = value.get<std::string>();

    if (!avatar_url.empty()) {
      cpr::Response avatar_response = cpr::Get(
          cpr::Url{avatar_url}, cpr::Header{{"User-Agent", "github-activity"}});

      if (avatar_response.status_code == 200) {
        try {
          int orig_w, orig_h, channels;
          unsigned char *orig_pixels = stbi_load_from_memory(
              reinterpret_cast<const unsigned char *>(
                  avatar_response.text.data()),
              static_cast<int>(avatar_response.text.size()), &orig_w, &orig_h,
              &channels, 3);

          if (!orig_pixels) {
            std::cerr << "Failed to decode avatar image data\n";
          } else {
            int target_w = 40;
            int target_h = (orig_h * target_w) / orig_w;
            if (target_h % 2 != 0)
              target_h++;

            for (int y = 0; y < target_h; y += 2) {
              for (int x = 0; x < target_w; ++x) {
                int orig_x = (x * orig_w) / target_w;
                int orig_y1 = (y * orig_h) / target_h;
                int orig_y2 =
                    std::min(((y + 1) * orig_h) / target_h, orig_h - 1);

                int top_idx = (orig_y1 * orig_w + orig_x) * 3;
                int bot_idx = (orig_y2 * orig_w + orig_x) * 3;

                int r1 = orig_pixels[top_idx];
                int g1 = orig_pixels[top_idx + 1];
                int b1 = orig_pixels[top_idx + 2];

                int r2 = orig_pixels[bot_idx];
                int g2 = orig_pixels[bot_idx + 1];
                int b2 = orig_pixels[bot_idx + 2];

                std::cout << std::format(
                    "\x1b[38;2;{};{};{}m\x1b[48;2;{};{};{}m▀", r1, g1, b1, r2,
                    g2, b2);
              }
              std::cout << "\x1b[0m\n";
            }
            stbi_image_free(orig_pixels);
          }
        } catch (const std::exception &e) {
          std::cerr << std::format(
              "An error occurred during image rendering: {}\n", e.what());
        }
      } else {
        std::cerr << std::format("API Avatar Request Failed with Code: {}",
                                 avatar_response.status_code)
                  << std::endl;
      }
    }

    print_json(GOOD_INFO);
  } else {
    std::cout << std::format("Connection Unsuccessful for User {}", gh_user)
              << std::endl;
  }
  if (r.status_code == 200) {
    std::cout << std::format("Printing Events Information") << std::endl;
    std::string event_link = SUBLINKS["events_url"];
    std::string postfix = "{/privacy}";
    size_t pos = event_link.find(postfix);
    event_link.erase(pos, postfix.length());
    std::cout << event_link << std::endl;
    cpr::Response e = cpr::Get(cpr::Url{event_link},
                               cpr::Header{{"User-Agent", "github-activity"}});
    if (e.status_code == 200) {
      std::cout << std::format("Content Type: {}", e.header["content-type"])
                << std::endl;
      try {
        nlohmann::ordered_json event_data =
            nlohmann::ordered_json::parse(e.text);
        std::cout << event_data.dump(2) << std::endl;
      } catch (nlohmann::json::parse_error &e) {
        std::cerr << std::format("Parsing Error: {}", e.what()) << std::endl;
      }
    }
  }
  return 0;
}

void print_json(nlohmann::ordered_json info) {
  for (const auto &[key, value] : info.items()) {
    std::cout << std::format("{}: {}", key, value.dump()) << std::endl;
  }
}
void print_key(nlohmann::ordered_json info) {
  for (const auto &[key, value] : info.items()) {
    std::cout << std::format("{}", key) << std::endl;
  }
}
