#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <streambuf>
#include <string>


#include <sdl/graehl/shared/split.hpp>
#include <sdl/graehl/shared/from_strings.hpp>
#include <sdl/Config/Config.hpp>
#include <sdl/Config/YAMLConfigProcessor.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Warn.hpp>
#include <sdl/Exception.hpp>
#if SDL_ENCRYPT
#include <sdl/Encrypt/Encrypt.hpp>
#endif
#include <sdl/Config/ConfigureYaml.hpp>

namespace YAML {

std::string to_string(const Node& in, bool longMap, bool newLine, const std::string& indent) {
  std::ostringstream os;
  sdl::Config::print(in, indent, os, longMap, newLine);
  return os.str();
}

void string_to_impl(std::string const& str, Node& node) {
  node = YAML::Load(str);
}
}

namespace sdl {

StringConsumer YamlConfigurable::log() const {
  return Util::logWarning(logname());
}

void YamlConfigurable::print(std::ostream& out) const {
  out << '[' << category_ << " " << name_ << ": type: " << type_ << ']';
}

ConfigNode YamlConfigurable::effective() const {
  SDL_THROW_LOG(Config, UnimplementedException,
                "TODO: effective YAML for YamlConfigurable not implemented yet");
  return ConfigNode();
}

std::string YamlConfigurable::getEffective(int verbosity) const {
  std::ostringstream o;
  showEffective(o, verbosity);
  return o.str();
}

void YamlConfigurable::apply(ConfigNode const& configNode, bool showEffective, int verbosity) {
  try {
    init();
    store(configNode);
    validateStored();
    if (showEffective) logEffective(verbosity);
  } catch (std::exception& e) {
    SDL_LOG_RETHROW(Config, *this << ": ", e);
  }
}

void YamlConfigurable::logEffective(int verbosity) const {
  LOG_INFO_NAMESTR(SDL_LOG_PREFIX_STR + logname(), "effective configuration for "
                                                   << *this << "\n "
                                                   << YamlConfigurableEffective(*this, verbosity));
  logEffectiveOnce_.finishNonAtomic();
}


void YamlConfigurableEffective::print(std::ostream& out) const {
  configurable.showEffective(out, verbosity);
}

namespace Config {

ConfigPath parsePath(std::string const& yamlPathDotSeparated) {
  ConfigPath r;
  graehl::split_into(yamlPathDotSeparated, r, ".");
  return r;
}

/**
   overrides already processed config in-place.
*/
ConfigNode overrideConfig(ConfigNode const& in, ConfigPath const& replaceAt,
                          ConfigNode const& substituteAtPath, unsigned replaceAtI) {
  if (replaceAtI >= replaceAt.size()) return substituteAtPath;

  using namespace std;
  string const& key = replaceAt[replaceAtI];
  ConfigNode out;
  if (!is_null(in)) {
    if (!in.IsMap())
      SDL_THROW_LOG(Config, ConfigException, "attempting to override config at path "
                                             << graehl::to_string_sep(replaceAt, ".")
                                             << " - parent was not a map node when descending to '" << key
                                             << "'");
    for (YAML::const_iterator i = in.begin(), e = in.end(); i != e; ++i) {
      std::string const& iKey = i->first.Scalar();
      if (iKey != key) out[iKey] = i->second;
    }
  }
  out[key] = overrideConfig(in[key], replaceAt, substituteAtPath, replaceAtI + 1);
  return out;
}

ConfigNode overrideConfig(ConfigNode const& root, ConfigPath const& replaceAt,
                          ConfigNode const& substituteAtPath) {
  return overrideConfig(root, replaceAt, substituteAtPath, 0);
}

/*
This method prints YAML nodes in a more readable format.
- Sequences are always printed within [] and on a single line.
- If called with longMap & newlines set to true, YAML Maps are scoped in { }.
  example:
   {
     foo: {
       bar: buz,
       hello: world!,
     },
     test-path: [../a.in, b.in, c.in]
   }

- If longMap is set to false & newlines to true the { are not printed and the output is more concise and
readable.
  example:
   foo:
     bar: buz
     hello: world!
  test-path: [../a.in, b.in, c.in]

- If called with longMap set to true & newlines to false, the entire YAML node is printed on a single line,
  e.g:
   { foo: { bar: buz, hello: world! }, test-path: [../a.in, b.in, c.in] }

- If called with longMap & newlines set to false, longMap gets reset to true as this is invalid config. and
the
  output is same as above.
 */
inline bool sequenceAllScalar(YAML::const_iterator i, YAML::const_iterator end) {
  for (; i != end; ++i)
    if (!i->IsScalar()) return false;
  return true;
}

void print(const ConfigNode& in, const std::string& indent, std::ostream& os, bool longMap, bool newlines,
           unsigned maxDepth) {
  if (maxDepth == 0) {
    os << "...";
    return;
  }
  if (in.IsScalar()) {
    os << indent << in.as<std::string>();
    return;
  }
  std::string deeperIndent;
  if (newlines) {
    deeperIndent = indent;
    deeperIndent.push_back(' ');
    deeperIndent.push_back(' ');
  }
  bool const sequence = in.IsSequence();
  bool const map = in.IsMap();
  if (!(sequence || map)) return;
  YAML::const_iterator i = in.begin(), end = in.end();
  if (sequence) {
    if (newlines && !sequenceAllScalar(in.begin(), in.end())) {
      for (; i != end; ++i) {
        os << indent << "- ";
        ConfigNode node = *i;
        if (node.IsScalar())
          os << node.as<std::string>();
        else
          print(*i, deeperIndent, os, longMap, newlines, --maxDepth);
        os << '\n';
      }
    } else {
      os << indent << "[ ";
      bool first = true;
      for (; i != end; ++i) {
        ConfigNode node = *i;
        if (!first) os << ", ";
        if (node.IsScalar())
          os << node.as<std::string>();
        else if (node.IsMap())
          print(node, "", os, longMap, newlines, --maxDepth);
        else if (node.IsSequence())
          print(node, "", os, longMap, newlines, --maxDepth);
        first = false;
      }
      os << " ]";
    }
  } else {
    assert(map);
    if (!longMap && !newlines) longMap = true;
    if (longMap) {
      os << " { ";
    }
    if (newlines) os << '\n';
    bool first = true;
    for (; i != end; ++i) {
      std::string const& key = i->first.as<std::string>();
      ConfigNode val = i->second;
      if (!val.IsNull()) {
        if (!first) {
          if (longMap) os << ", ";
          if (newlines) os << '\n';
        }
        os << (newlines && longMap ? deeperIndent : indent) << key << ": ";
        if (val.IsScalar())
          os << val.as<std::string>();
        else
          print(val, deeperIndent, os, longMap, newlines, --maxDepth);
      }
      first = false;
    }
    if (longMap) {
      if (newlines) {
        os << '\n';
        os << indent << "}";
      } else
        os << indent << " }";
    }
  }
}

#if SDL_ENCRYPT
ConfigNode loadRawEncryptedConfig(boost::filesystem::path const& path) {
  try {
    std::string decryptedtext;
    Encrypt::decryptFile(path.string(), decryptedtext);
    SDL_DEBUG(Configure.loadRawEncryptedConfig, "decrypted: " << decryptedtext);
    YAML::Node rawNode = YAML::Load(decryptedtext);

    if (is_null(rawNode))  // because it turns out that YAML::LoadFile doesn't throw if file can't be read!
      SDL_THROW_LOG(Config, ConfigException, "No YAML configuration could be read from file " << path.string());
    return rawNode;
  } catch (std::exception& e) {
    SDL_THROW_LOG(Configure.loadRawConfig, ConfigException, "Error loading encrypted YAML file: '"
                                                            << e.what() << "'. "
                                                            << " File path: [ " << path.string() << " ]");
  }
}
#endif

ConfigNode loadRawConfig(boost::filesystem::path const& path) {
  try {
    ConfigNode rawNode = YAML::LoadFile(path.string());
    if (is_null(rawNode))  // because it turns out that YAML::LoadFile doesn't throw if file can't be read!
      SDL_THROW_LOG(Config, ConfigException, "No YAML configuration could be read from file " << path.string());
    return rawNode;
  } catch (std::exception& e) {
    SDL_THROW_LOG(Configure.loadRawConfig, ConfigException, "Error loading YAML file: '"
                                                            << e.what() << "'. "
                                                            << " File path: [ " << path.string() << " ]");
  }
}

/// fully expanded starting from relative or absolute path 'basis',
/// and interpreted via conventions specified in ../docs/xmt-configuration-with-yaml.pdf
ConfigNode expandConfig(const ConfigNode& in, boost::filesystem::path const& filePath, OptPath const& forPath) {
  YAMLConfigProcessor configProc(forPath);
  configProc.setFilePath(filePath);
  return configProc.process(in, filePath.parent_path());
}

ConfigNode loadConfig(boost::filesystem::path const& path, OptPath const& forPath) {
  try {
    return expandConfig(
#if SDL_ENCRYPT
        Encrypt::isEncryptedFile(path.string()) ? loadRawEncryptedConfig(path) :
#endif
                                                loadRawConfig(path),
        path, forPath);
  } catch (std::exception& e) {
    SDL_THROW_LOG(Configure.loadConfig, ConfigException, "Error loading YAML file: '"
                                                         << e.what() << "'. "
                                                         << " File path: [ " << path.string() << " ] for "
                                                         << configure::join_opt_path(forPath));
  }
}


}}
