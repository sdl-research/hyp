// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/**
   see spec: /docs/xmt-configure-with-yaml-userdoc.md

   with some additional xmt-shell specific stuff:

   'module x' 'pipeline x' 'resource x' categories become 'x: { category: module }' etc

   and you get 'pipeline x: steps: [list]' for 'pipeline x': [list]

   impl by skohli; amendment to immutable (return modified value) for
   yaml-& * ref safety by jgraehl

   XXX: Node const& might have mutable state somehow; Node (value copy) might be a shallow copy which could
   help
*/

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4146)
#endif
#include <yaml-cpp/yaml.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <sdl/Config/YAMLConfigProcessor.hpp>
#include <sdl/Path.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/String.hpp>
#include <sdl/Printer.hpp>

#include <sdl/Util/Input.hpp>
#include <sdl/Util/Map.hpp>
#include <sdl/Pool/object_pool.hpp>
#include <sdl/Util/Delete.hpp>
#include <sdl/Util/StableVector.hpp>

namespace sdl {
namespace Config {

/**
   not O(n) like YAML-we know our maps are keyed by string
*/

typedef std::map<std::string, ConfigNode> MapIndexBase;

struct MapIndex : MapIndexBase {
  MapIndex(ConfigNode const& in) {
    assert(in.IsMap());
    for (YAML::const_iterator i = in.begin(), e = in.end(); i != e; ++i)
      if (!insert(value_type(i->first.Scalar(), i->second)).second)
        SDL_THROW_LOG(Config.YAMLConfigProcessor, ProgrammerMistakeException,
                      "duplicate map key " << i->first << ": " << printer(i->second, oneline));
  }
};

template <class Key, class Val>
inline void addKeyVal(ConfigNode& node, Key const& key, Val const& val, YAMLConfigProcessor const& proc) {
  node.force_insert(key, val);
}

/**
   avoid yaml graph self-loops - if the api exposed addresses or node.id() we
   could keep a set of all the ancestors and detect indirect loops, too. but
   since the library only provides Node.is(Node) we don't want O(n^2) processing
   and only catch immediate child = parent loops
*/
inline ConfigNode const& childNotParent(ConfigNode const& child, ConfigNode const& parent) {
  if (child.is(parent))
    SDL_THROW_LOG(Configure.YAMLConfigProcessor, ConfigException,
                  "child is same as parent: '" << child << "' with parent '" << parent << "'.");
  return child;
}

/**
   \return a copy of a single YAML node, recursively if deep==true. does not copy tags.
*/
ConfigNode copy(ConfigNode const& in, bool deep) {
  ConfigNode copy;
  using namespace YAML;
  switch (in.Type()) {
    case NodeType::Undefined:
    case NodeType::Null:
      break;
    case NodeType::Scalar:
      copy = in.Scalar();
      break;
    case NodeType::Sequence:
      if (deep)
        for (const_iterator i = in.begin(), e = in.end(); i != e; ++i)
          copy.push_back(Config::copy(childNotParent(*i, in), deep));
      else
        for (const_iterator i = in.begin(), e = in.end(); i != e; ++i)
          copy.push_back(childNotParent(*i, in));  // O(1)
      break;
    case NodeType::Map:
      if (deep)
        for (const_iterator i = in.begin(), e = in.end(); i != e; ++i)
          addKeyVal(copy, i->first, Config::copy(childNotParent(i->second, in), deep));
      else
        for (const_iterator i = in.begin(), e = in.end(); i != e; ++i)
          addKeyVal(copy, i->first, childNotParent(i->second, in));
      break;
  }
  SDL_TRACE(Configure.YAMLConfigProcessor, "copy " << printer(in, oneline) << " => " << printer(copy, oneline));
  return copy;
}

/**
   \return a copy of a single YAML node, recursively if deep==true. does not copy tags.
*/
ConfigNode copyRemovingKey(ConfigNode const& in, std::string const& removeKey, bool deep) {
  using namespace YAML;
  ConfigNode copy;
  switch (in.Type()) {
    case NodeType::Undefined:
    case NodeType::Null:
      break;
    case NodeType::Scalar:
      copy = in.Scalar();
      break;
    case NodeType::Sequence:
      if (deep)
        for (const_iterator i = in.begin(), e = in.end(); i != e; ++i)
          copy.push_back(Config::copy(childNotParent(*i, in), deep));
      else
        for (const_iterator i = in.begin(), e = in.end(); i != e; ++i)
          copy.push_back(childNotParent(*i, in));  // O(1)
      break;
    case NodeType::Map:
      if (deep) {
        for (const_iterator i = in.begin(), e = in.end(); i != e; ++i)
          if (i->first.Scalar() != removeKey)
            addKeyVal(copy, i->first, Config::copy(childNotParent(i->second, in), deep));
      } else {
        for (const_iterator i = in.begin(), e = in.end(); i != e; ++i)
          if (i->first.Scalar() != removeKey) addKeyVal(copy, i->first, childNotParent(i->second, in));
      }
      break;
  }
  SDL_TRACE(Configure.YAMLConfigProcessor, "copyRemovingKey " << removeKey << " from " << printer(in, oneline)
                                                              << " => " << printer(copy, oneline));
  return copy;
}

/**
   exactly the same as above, but also logs the full path to this part of config.
*/
ConfigNode copy(ConfigNode const& in, bool deep, YAMLConfigProcessor const& proc) {
  using namespace YAML;
  ConfigNode copy;
  switch (in.Type()) {
    case NodeType::Undefined:
    case NodeType::Null:
      break;
    case NodeType::Scalar:
      copy = in.Scalar();
      break;
    case NodeType::Sequence:
      if (deep)
        for (const_iterator i = in.begin(), e = in.end(); i != e; ++i)
          copy.push_back(Config::copy(childNotParent(*i, in), deep));
      else
        for (const_iterator i = in.begin(), e = in.end(); i != e; ++i)
          copy.push_back(childNotParent(*i, in));  // O(1)
      break;
    case NodeType::Map:
      if (deep)
        for (const_iterator i = in.begin(), e = in.end(); i != e; ++i)
          addKeyVal(copy, i->first, Config::copy(childNotParent(i->second, in), deep), proc);
      else
        for (const_iterator i = in.begin(), e = in.end(); i != e; ++i) {
          ConfigNode newRefSecond((childNotParent(i->second, in)));
          // TODO: is the temporary named copy necessary due to some yaml-cpp lib deficiency?
          addKeyVal(copy, i->first, newRefSecond, proc);
        }
      break;
  }
  SDL_TRACE(Configure.YAMLConfigProcessor, "copy " << proc.path() << ": " << printer(in, oneline) << " => "
                                                   << printer(copy, oneline));
  return copy;
}

ConfigNode copyToTree(ConfigNode const& in) {
  return copy(in, true);
}

/// deep copy
ConfigNode clone(ConfigNode const& in) {
  return Clone(in);
}

// static linkage:
namespace {

/**
   for basis: filename - locate relative paths relative to directory of yaml file
*/
/**
   If the input path is a relative path, prepends the prefix. If the path is absolute, its a No-Op.
   returns the path as boost::filesystem::path instance.
 */
inline boost::filesystem::path addPrefixToPath(std::string const& path, boost::filesystem::path const& fsPrefix) {
  boost::filesystem::path fsPath(path);
  if (!fsPath.is_absolute()) fsPath = fsPrefix / path;
  return fsPath;
}

/**
   for x-path: config keys (and for change-working-directory) - locate relative paths relative to directory of
   yaml file
*/
std::string addPrefixToPathString(std::string const& path, boost::filesystem::path const& fsPrefix) {
  if (Util::isNonFilesystemPath(path)) return path;  // exclude e.g. '-' meaning STDOUT
  boost::filesystem::path fsPath(path);
  if (!fsPath.is_absolute()) {
    boost::filesystem::path fsPath = fsPrefix / path;
    if (!fsPath.is_absolute())
      return fsPath.relative_path().string();
    else
      return fsPath.string();
  } else
    return path;
}

std::string const cwdString(("change-working-directory"));

inline boost::filesystem::path maybeChangeWorkingDirectory(ConfigNode const& in,
                                                           boost::filesystem::path const& fsPrefixIn) {
  ConfigNode const& cwd = in[cwdString];
  if (is_null(cwd))
    return fsPrefixIn;
  else {
    if (!cwd.IsScalar())
      SDL_THROW_LOG(Configure.YAMLConfigProcessor, ConfigException,
                    "change-working-directory, which is used (like 'cd dir') to resolve relative paths for a "
                    "given subtree."
                    " must have a scalar (pathname) value, and may itself be relative (in which case it's "
                    "expanded against the previous working directory");
    SDL_INFO(Configure.YAMLConfigProcessor, "change-working-directory: " << cwd.Scalar());
    return addPrefixToPathString(cwd.Scalar(), fsPrefixIn);
  }
}

std::string const kSkipProcessing(("skip-processing"));
std::string const kLiterally(("literally "));
std::string const kReplace(("replace"));
std::string const kBasis(("basis"));
std::string const kCategory(("category"));
std::string const kSteps(("steps"));
}

ConfigNode YAMLConfigProcessor::process(ConfigNode const& in, boost::filesystem::path const& fsPrefixIn) {
  if (!in.IsMap())
    return in;
  else {
    ConfigNode out;

    if (in.size() == 1) {
      YAML::const_iterator i = in.begin();
      if (i->first.Scalar() == kSkipProcessing) {
        SDL_TRACE(Configure.YAMLConfigProcessor, "skip-processing: " << path());
        return i->second;
      }
    }

    boost::filesystem::path fsPrefix = maybeChangeWorkingDirectory(in, fsPrefixIn);
    for (YAML::const_iterator i = in.begin(), end = in.end(); i != end; ++i) {
      std::string const& key = i->first.Scalar();
      if (key != cwdString) {
        SDL_TRACE(Configure.YAMLConfigProcessor, "Processing: [ " << path() << ": " << key << " ]");
        push(key);
        addKeyVal(out, key, process(childNotParent(i->second, in), fsPrefix), *this);
        pop();
      }
    }
    ConfigNode const& toExpand = resolveCategories(resolveReplaceNodes(resolvePaths(out, fsPrefix)));

    ConfigNode const& expanded = expandBasis(toExpand, fsPrefix);

    SDL_TRACE(Configure.YAMLConfigProcessor.basis, "basis expanded path=["
                                                       << path() << "]: " << printer(toExpand, oneline)
                                                       << " ... to: " << printer(expanded, oneline));

    assert(optPath_.size() >= optPathInitialDepth_);
    bool rootLevel = optPath_.empty();  // TODO: or optPathInitialDepth_ == optPath_.size() ?
    if (rootLevel) {
      ConfigNode const& replaced = processReplaceNodes(expanded);  // TODO: rootLevel for file vs. at end?
      if (optPath_.empty()) {
        SDL_DEBUG(Configure.YAMLConfigProcessor,
                  "final root config file: " << printer(replaced, kMultilineBraces));
      } else {
        SDL_DEBUG(Configure.YAMLConfigProcessor,
                  "expanded config file for " << path() << ": " << printer(replaced, kMultilineBraces));
      }
      return replaced;
    } else
      return expanded;
  }
}

std::string const kPathSuffix("path");
unsigned const kLenPathSuffix = 4;

inline bool hasPathSuffix(std::string const& key) {
  if (Util::endsWith(key, kPathSuffix)) {
    unsigned nkey = key.size();
    return nkey == kLenPathSuffix || key[nkey - 5] == '-';
  } else
    return false;
}

ConfigNode YAMLConfigProcessor::resolvePaths(ConfigNode const& in, boost::filesystem::path const& fsPrefix) const {
  if (!in.IsMap())
    return in;
  else {
    ConfigNode out;
    for (YAML::const_iterator itMap = in.begin(), end = in.end(); itMap != end; ++itMap) {
      std::string const& key = itMap->first.Scalar();
      ConfigNode const& val = itMap->second;
      if (hasPathSuffix(key)) {
        if (val.IsScalar()) {
          std::string const& origPath = val.Scalar();
          std::string const& newPath = addPrefixToPathString(origPath, fsPrefix);
          addKeyVal(out, key, newPath, *this);
          SDL_TRACE(Configure.YAMLConfigProcessor, "Resolved path for [ "
                                                       << path() << ": " << itMap->first.Scalar()
                                                       << " ] from: " << origPath << " to: " << newPath);
        } else if (val.IsSequence()) {
          ConfigNode outSequence;
          SDL_TRACE(Configure.YAMLConfigProcessor, "Resolving path's for [ "
                                                       << path() << ": " << itMap->first.Scalar() << " ]");

          for (YAML::const_iterator iSeq = val.begin(), end = val.end(); iSeq != end; ++iSeq) {
            std::string const& origPath = iSeq->Scalar();
            std::string const& newPath = addPrefixToPathString(origPath, fsPrefix);
            SDL_DEBUG(Configure.YAMLConfigProcessor, "Changing original path: " << origPath << " to: " << newPath);
            outSequence.push_back(newPath);
          }
          addKeyVal(out, key, outSequence, *this);
        } else
          SDL_THROW_LOG(Configure.YAMLConfigProcessor, ConfigException,
                        "Syntax error! Value for a key 'path' or ending in '-"
                            << kPathSuffix << "' can only be a path or a list of paths." << '\n' << "path: [ "
                            << path() << " ]\n"
                            << "Key: " << key << '\n' << "File: " << getFilePath());
      } else
        addKeyVal(out, key, val, *this);
    }
    return out;
  }
}

ConfigNode YAMLConfigProcessor::mergeMaps(ConfigNode const& base, ConfigNode const& over,
                                          bool skipBasisOverwrite) const {
  MapIndex overs(over);
  if (skipBasisOverwrite) overs.erase(kBasis);
  ConfigNode merged;
  for (YAML::const_iterator i = base.begin(), end = base.end(); i != end; ++i) {
    std::string const& key = i->first.Scalar();
    ConfigNode const& val = childNotParent(i->second, base);
    MapIndex::iterator overi = overs.find(key);
    if (overi == overs.end())
      addKeyVal(merged, i->first, val);  // should be same as building node afresh using merged[key] = val
    else {
      push(key);  // for log msgs
      addKeyVal(merged, i->first, mergeNodes(val, overi->second, kCopyBasisKey));
      pop();
      overs.erase(overi);
    }
  }
  for (MapIndex::const_iterator i = overs.begin(), e = overs.end(); i != e; ++i)
    addKeyVal(merged, i->first, i->second);
  return merged;
}

ConfigNode YAMLConfigProcessor::mergeSeqs(ConfigNode const& base, ConfigNode const& append) const {
  ConfigNode merged;
  for (YAML::const_iterator i = base.begin(), end = base.end(); i != end; ++i)
    merged.push_back(childNotParent(*i, base));
  for (YAML::const_iterator i = append.begin(), end = append.end(); i != end; ++i)
    merged.push_back(childNotParent(*i, append));
  return merged;
}

/**
   \return 'over' on top of 'base' (nondestructively)

   recursive.

   if both scalar: return over (without recursing)

   if both sequence: return base ++ over (without recursing)

   if both map: overlay recursively mergeNodes(...)
   returns whichever is a map, or if both are maps overlay both maps, favoring 2nd (in) on conflict

   if either null: return other
*/
ConfigNode YAMLConfigProcessor::mergeNodes(ConfigNode const& base, ConfigNode const& over,
                                           bool skipBasisOverwrite) const {
  using namespace YAML;
  NodeType::value baseType = base.Type(), overType = over.Type();
  if (baseType == NodeType::Null || baseType == NodeType::Undefined) return over;
  if (overType == NodeType::Null || overType == NodeType::Undefined) return base;
  if (overType != baseType)
    SDL_THROW_LOG(sdl.Config.YAMLConfigProcessor, ConfigException,
                  "Two basis node types do not match. File: " << getFilePath() << ", path: " << path()
                                                              << ", '" << base << "' != '" << over << "'");
  if (baseType == NodeType::Scalar)
    return over;
  else {
    if (baseType == NodeType::Map)
      return mergeMaps(base, over, skipBasisOverwrite);
    else if (baseType == NodeType::Sequence)
      return mergeSeqs(base, over);
    else
      SDL_THROW_LOG(sdl.Config.YAMLConfigProcessor, ProgrammerMistakeException,
                    "Unknown YAML node type: " << (int)baseType);
    return ConfigNode();  // never reached
  }
}

typedef std::vector<ConfigNode> BasisNodes;
/// we would just directly iterate over children but yaml-cpp hides a bug for us in gcc 4.9 c++11 -O3

/// odd: ConfigNode const& seq => undefined node. copy => child same as parent
inline void childNodes(ConfigNode seq, BasisNodes& children) {
  assert(seq.IsSequence());
  assert(!seq.IsMap());
  children.resize(seq.size());
  unsigned o = 0;
  for (YAML::const_iterator i = seq.begin(), e = seq.end(); i != e; ++i)
    children[o++] = static_cast<YAML::Node const&>(*i);
  for (BasisNodes::const_iterator i = children.begin(), e = children.end(); i != e; ++i) {
    if (i->is(seq))
      SDL_THROW_LOG(Configure.YAMLConfigProcessor, ConfigException,
                    "sequence child is same as parent: '" << *i << "' with parent '" << seq << "'.");
  }
}

inline void addBasisElement(BasisNodes& basisNodes, ConfigNode const& inBasis,
                            boost::filesystem::path const& fsPrefix, YAMLConfigProcessor const& proc) {
  OptPath const& optPath = proc.optPath();
  assert(!inBasis.IsSequence());
  if (inBasis.IsScalar()) {
    std::string const& basis = inBasis.Scalar();
    boost::filesystem::path fsBasis = addPrefixToPath(basis, fsPrefix);
    SDL_INFO(Configure.YAMLConfigProcessor, "File: " << proc.getFilePath()
                                                     << "- Loading basis file: " << fsBasis.string());
    try {
      ConfigNode loaded = loadConfig(fsBasis, proc.optPath());
      // TODO: memoize e.g. multi-regextokenizer-in-one-file
      basisNodes.push_back(loaded);
    } catch (ConfigException& e) {
      SDL_THROW_LOG(Configure.loadConfig, ConfigException, "Error loading YAML file: '"
                                                               << e.what() << "'. "
                                                               << " File: '" << proc.getFilePath()
                                                               << "' path: [ " << proc.path() << " ]");
    }
    // TODO: cache (only load/parse basis file once)
  } else if (inBasis.IsMap()) {
    basisNodes.push_back(inBasis);
  } else
    SDL_THROW_LOG(Configure.YAMLConfigProcessor, ConfigException,
                  "Syntax error! Value for 'basis' can not be Null!!" << '\n' << "path: [ " << proc.path() << " ]\n"
                                                                      << "File: " << proc.getFilePath());
}

/**
   semi-recursive (via mergeNodes)

      Processes basis nodes such as:
      basis: [a.yml, b.yml, c.yml, ...
      as follows:
      temp = a.yml <- b.yml [b is overlaid on a]
      temp = temp <- c.yml [c is overlaid on temp] and so on.
      This method calls mergeNodes to do the overlay.
      During this operation sequences in the overlaid node get append to the base node.
      Scalar values are over-written.
      Maps are traversed and one of the above two operations are perfromed at each leaf node.

      //TODO: should not be a YAMLConfigProcessor member but rather a free fn
   */
ConfigNode YAMLConfigProcessor::expandBasis(ConfigNode const& in, boost::filesystem::path const& fsPrefix) const {
  if (!in.IsMap())
    return in;
  else {
    ConfigNode const& inBasis = childNotParent(in[kBasis], in);
    using namespace YAML;
    // if (!inBasis.IsDefined()) return in;
    NodeType::value btype = inBasis.Type();
    if (btype == NodeType::Null || btype == NodeType::Undefined) {
      return in;
    } else {
      BasisNodes basisNodes;
      std::string const& filePath = filePath_.string();
      if (btype == NodeType::Sequence) {
        assert(inBasis.IsDefined());
        assert(inBasis.IsSequence());
        BasisNodes children;
        childNodes(inBasis, children);
        //        for (YAML::const_iterator i = inBasis.begin(), e = inBasis.end(); i != e; ++i)
        /// we would just directly iterate over children but yaml-cpp hides a bug for us in gcc 4.9 c++11 -O3
        for (BasisNodes::const_iterator i = children.begin(), e = children.end(); i != e; ++i)
          addBasisElement(basisNodes, *i, fsPrefix, *this);
      } else {
        assert(btype == NodeType::Map || btype == NodeType::Scalar);
        addBasisElement(basisNodes, inBasis, fsPrefix, *this);
      }

      SDL_TRACE(Configure.YAMLConfigProcessor.expandBasis,
                "Removing node:  < [ " << path() << ": basis ] -> " << printer(inBasis, oneline) << " >");

      // BasisAccumulator accum(*this, fsPrefix);
      Util::AutoDelete<ConfigNode> result;
      for (BasisNodes::const_iterator i = basisNodes.begin(), e = basisNodes.end(); i != e; ++i) {
        if (!result) {
          result.reset(new ConfigNode(*i));
        } else {
          assert(result);
          ConfigNode prev = *result;
          ConfigNode merged
              = mergeNodes(prev, *i, (bool)kCopyBasisKey);  // hoping to avoid overwrite of shared basis nodes
          // here (otherwise would skip 'merged' var)
          // TODO: simplify after debugging
          result.reset(new ConfigNode(merged));
        }

        // accum(**i, kCopyBasisKey);
        // SDL_TRACE(Configure.YAMLConfigProcessor.expandBasis, "merged basis; result = " <<
        // printer(accum, oneline));
      }

      // return accum(in, kSkipBasisKey);
      return result ? mergeNodes(*result, in, (bool)kSkipBasisKey) : in;
    }
  }
}

inline std::ostream& operator<<(std::ostream& out, YAMLConfigProcessor::Instance const& instance) {
  return out << instance.first << " " << printer(instance.second, oneline);
}

void YAMLConfigProcessor::registerInstance(std::string const& name, std::string const& category,
                                           ConfigNode const& conf) {
  Instance instance(category, conf);
  Instance* existing;
  if (!Util::update(instances, name, instance, existing))
    SDL_THROW_LOG(Config.YAMLConfigProcessor, ConfigException,
                  "Existing " << name << " " << *existing << " blocks creation of new " << name << " "
                              << instance << " - rename one of them");
}

/**
   non-recursive.
*/
ConfigNode YAMLConfigProcessor::resolveCategories(ConfigNode const& in) {
  if (!in.IsMap() || !optPath_.empty())  // only at root level (module/pipeline/resource)
    return in;
  else {
    assert(in.IsMap());
    ConfigNode out;
    using namespace YAML;

    for (const_iterator it = in.begin(), end = in.end(); it != end; ++it) {
      Node const& keyNode = it->first;
      Node const& valNode = it->second;
      bool isSteps = false;

      std::string category;
      std::string key = keyNode.Scalar();
      boost::trim(key);
      if (Util::stripPrefix(key, "module "))
        registerInstance(key, category = "module", valNode);
      else if (Util::stripPrefix(key, "pipeline ")) {
        registerInstance(key, category = "pipeline", valNode);
        if (valNode.IsSequence())
          isSteps = true;
        else if (!(valNode.IsMap() && valNode["steps"]))
          SDL_THROW_LOG(Configure.YAMLConfigProcessor, ConfigException,
                        "Pipeline should be a sequence of steps or a map with a steps: [sequence of steps]!"
                            << " File: " << getFilePath() << ", path: [ " << path() << ": "
                            << it->first.Scalar() << " ]");
      } else if (Util::stripPrefix(key, "resource ")) {
        registerInstance(key, category = "resource", valNode);
      } else {
        addKeyVal(out, key, valNode, *this);  // might have trimmed whitespace from original keyNode
        continue;
      }
      // category
      assert(!category.empty());
      NodeType::value valType = valNode.Type();
      bool isSeq = valType == NodeType::Sequence;
      if (!(isSteps && isSeq || valType == NodeType::Map))
        SDL_THROW_LOG(Configure.YAMLConfigProcessor, ConfigException,
                      "Category nodes cannot be scalar - should be map or, for category pipeline, sequence "
                      "(which expands to { steps: sequence }) "
                          << " File: " << getFilePath() << ", path: [ " << path() << ": "
                          << it->first.Scalar() << " ]");
      ConfigNode finalVal;
      if (isSeq)
        addKeyVal(finalVal, kSteps, valNode, *this);
      else {
        for (const_iterator i = valNode.begin(), e = valNode.end(); i != e; ++i)
          addKeyVal(finalVal, i->first, childNotParent(i->second, valNode));
      }
      addKeyVal(finalVal, kCategory, category, *this);
      addKeyVal(out, key, finalVal, *this);
      SDL_TRACE(Configure.YAMLConfigProcessor, "After resolveCategories node: < [ "
                                                   << path() << ": " << key << " ] "
                                                   << "of category: " << category << ": "
                                                   << printer(finalVal, oneline));
    }
    return out;
  }
}

/**
   called once from root level, recurses. top-down to avoid wasted effort (bottom-up would be fine, though)

   also looks for map with lone key 'skip-processing': and replaces that node by value

   if lone map key is 'literally skip-processing:, replace that key by just 'skip-processing'. if lone map key
   starts with 'literally literally', replace that prefix with just 'literally'.
*/
ConfigNode YAMLConfigProcessor::processReplaceNodes(ConfigNode const& in) {
  if (!in.IsMap()) return in;
  ConfigNode out;
  if (in.size() == 1) {
    YAML::const_iterator i = in.begin();
    if (i->first.Scalar() == kSkipProcessing) return i->second;
  }

  ConfigNode const& replaces = in[kReplace];

  for (YAML::const_iterator i = in.begin(), end = in.end(); i != end; ++i) {
    std::string const& key = i->first.Scalar();
    SDL_TRACE(Configure.YAMLConfigProcessor, "Processing replace for: [ " << path() << "." << key << " ]");
    if (key != kReplace) {
      ConfigNode const& replaceBy = replaces[key];
      // O(# of 'replace ' things) - not too bad. could transform into map<std::string,...> first
      if (replaceBy.IsNull())
        SDL_TRACE(Configure.YAMLConfigProcessor,
                  "Removing [ " << path() << "." << key
                                << " ] because it was redefined as NULL by replace operator");
      else {
        if (replaceBy)
          SDL_TRACE(Configure.YAMLConfigProcessor,
                    "Replacing [ " << path() << "." << key << " ] by: " << printer(replaceBy, oneline));
        std::string keyUnescaped(key);
        Util::stripPrefix(keyUnescaped, kLiterally);
        push(keyUnescaped);
        addKeyVal(out, keyUnescaped,
                  processReplaceNodes(replaceBy ? replaceBy : childNotParent(i->second, in)), *this);
        pop();
      }
    }
  }
  SDL_TRACE(Configure.YAMLConfigProcessor, "Done replacing: [ " << path() << " ] => " << printer(out, oneline));
  return out;
}

/**
   groups multiple 'replace key: val' instructions into a single replace: {key: val, ...}. nonrecursive.
   idempotent.
   actual replace happens once with processReplaceNodes
*/
ConfigNode YAMLConfigProcessor::resolveReplaceNodes(ConfigNode const& in) const {
  if (!in.IsMap())
    return in;
  else {
    ConfigNode out;
    std::vector<std::string> nodesToRemove;
    ConfigNode replacesGrouped = out["replace"];  // only gets created if we add to replacesGrouped. //TODO:
    // is this copy necessary or const& ?
    for (YAML::const_iterator i = in.begin(), end = in.end(); i != end; ++i) {
      std::string const& keyOrig = i->first.Scalar();
      ConfigNode const& valOrig = childNotParent(i->second, in);
      std::string key = keyOrig;
      boost::trim(key);
      if (Util::stripPrefix(key, "replace ")) {
        ConfigNode replaceVal = replacesGrouped[key];
        if (!replaceVal)
          replaceVal = valOrig;
        else
          SDL_THROW_LOG(Configure.YAMLConfigProcessor, ConfigException, "Two replace ops defined for: \""
                                                                            << key << "\"."
                                                                            << " File: " << getFilePath()
                                                                            << "path: [ " << path() << " ]");
      } else
        addKeyVal(out, keyOrig, valOrig, *this);
    }
    return out;
  }
}


}}
