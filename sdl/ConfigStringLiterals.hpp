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
/** \file

    TODO: remove this or at least quit using preprocessor for it - mostly for
    legacy xml configuration formats.
*/

#ifndef SDL_CONFIGSTRINGLITERALS_HPP_
#define SDL_CONFIGSTRINGLITERALS_HPP_
#pragma once

#include <string>

namespace sdl {
namespace Config {

// TODO: Add other modules here

namespace RegexTokenizer {
static std::string const kModuleName = "RegexTokenizer";
namespace ParamNames {
static std::string const kDefaultTokenSeparator = "DefaultTokenSeparator";
static std::string const kTokenDefinitionsFile = "TokenDefinitionsFile";
}
}
}

// Generic
char const* const SDL_CSTR_NAME = "Name";
char const* const SDL_CSTR_CONFIG = "Configuration";
char const* const SDL_CSTR_TYPE = "Type";
char const* const SDL_CSTR_CACHESIZE = "CacheSize";
char const* const SDL_CSTR_DB_TYPE = "DBType";
char const* const SDL_CSTR_PATH = "Path";
char const* const SDL_CSTR_FLAGS = "Flags";
char const* const SDL_CSTR_SOURCE = "Source";
char const* const SDL_CSTR_TARGET = "Target";
char const* const SDL_CSTR_FILEPATH = "File";
char const* const SDL_CSTR_CONFIGVERSION = "Version";
char const* const SDL_CSTR_VALUE = "Value";

// Apparatus.
char const* const SDL_CSTR_MODULEDEFS = "ModuleDeclarations";
char const* const SDL_CSTR_RESOURCES = "ResourceDeclarations";
char const* const SDL_CSTR_APPARATUSCONFIG = "ApparatusConfig";

// Resource
char const* const SDL_CSTR_RESOURCECONFIG = "ResourceDeclarations";
char const* const SDL_CSTR_RESOURCE = "Resource";

// Module
char const* const SDL_CSTR_MODULE = "Module";

// Protocol
char const* const SDL_CSTR_PROTOCOL = "Protocol";
char const* const SDL_CSTR_STEP = "Step";
char const* const SDL_CSTR_confINPUT = "Input";
char const* const SDL_CSTR_OUTPUT = "Output";
char const* const SDL_CSTR_BINARY = "Binary";

// VocabularyConfig
char const* const SDL_CSTR_VOCABULARY_CONFIG = "VocabularyConfig";
char const* const SDL_CSTR_VOCABULARY = "Vocabulary";
char const* const SDL_CSTR_SYMBOL_TYPE = "SymbolType";
char const* const SDL_CSTR_MAX_SYMBOL_ID = "MaxSymbolID";
char const* const SDL_CSTR_MIN_SYMBOL_ID = "MinSymbolID";
char const* const SDL_CSTR_NEXT_SYMBOL_ID = "NextSymbolID";

char const* const SDL_CSTR_NONRESIDENT = "NonResident";
char const* const SDL_CSTR_RESIDENT = "Resident";
char const* const SDL_CSTR_READONLY = "ReadOnly";
char const* const SDL_CSTR_ATSVOCAB = "ATSVocab";

// Grammar
char const* const SDL_CSTR_GRAMMAR = "Grammar";
char const* const SDL_CSTR_FEATURETYPES = "Features";
char const* const SDL_CSTR_FEATURETYPE = "Feature";
char const* const SDL_CSTR_ATSPHRASE = "ATS-Phrase-Table";
char const* const SDL_CSTR_XMTPHRASE = "SDL-Phrase-Table";
char const* const SDL_CSTR_NUMRULES = "Num-Rules";

char const* const SDL_CSTR_FEATUREWEIGHTS = "FeatureWeights";
char const* const SDL_CSTR_FEATUREWEIGHT = "FeatureWeight";

// Decoder
char const* const SDL_CSTR_DECODERCONFIG = "Decoder";
char const* const SDL_CSTR_PBMT = "PBMT";
char const* const SDL_CSTR_STACKLIMIT = "StackLimit";
char const* const SDL_CSTR_DISTORTIONLIMIT = "DistortionLimit";
char const* const SDL_CSTR_UNKNOWNWORDPENALTY = "unk-weight";
char const* const SDL_CSTR_LMWEIGHT = "lm-weight";
char const* const SDL_CSTR_DISTORTIONPENALTY = "distortion-weight";
char const* const SDL_CSTR_LENGTHPENALTY = "length-weight";
char const* const SDL_CSTR_INPUTARCWEIGHT = "InputArcWeight";
char const* const SDL_CSTR_MAXSOURCELEN = "MaxSourceLength";
char const* const SDL_CSTR_NUMBEST = "NumBest";
char const* const SDL_CSTR_TARGET_CONSTRAINTS = "TargetConstraints";
char const* const SDL_CSTR_ALLOW_PREFIX_CONSTRAINTS = "AllowPrefixConstraints";
char const* const SDL_CSTR_DECODEROUPUT = "DecoderOutput";

// DB Config
char const* const SDL_CSTR_DBCONFIG = "DBConfig";

// LanguageModel
char const* const SDL_CSTR_LANGUAGEMODEL = "LanguageModel";
char const* const SDL_CSTR_ORDER = "Order";

// BDB
char const* const SDL_CSTR_BDB = "BDB";
char const* const SDL_CSTR_BDBENVIRONMENT = "BDBEnvironment";
char const* const SDL_CSTR_ENCRYPT = "Encrypt";
char const* const SDL_CSTR_BDBCONFIG = "BDBConfig";
char const* const SDL_CSTR_BUFFERSIZE = "BufferSize";
char const* const SDL_CSTR_PAGESIZE = "PageSize";
char const* const SDL_CSTR_ADDITIONALFLAGS("AdditionalFlags");
char const* const SDL_CSTR_MAXATTEMPTS("MaxAttempts");
char const* const SDL_CSTR_MAXBUFFERSIZE("MaxBufferSize");

// MDB
char const* const SDL_CSTR_MDB = "MDB";
char const* const SDL_CSTR_MDBENVIRONMENT = "MDBEnvironment";
char const* const SDL_CSTR_MDBCONFIG = "MDBConfig";
char const* const SDL_CSTR_MAPSIZE = "MapSize";

// LowerCaser
char const* const SDL_CSTR_LOWERCASER = "LowerCaser";

// Tokenizer
// Basic Tokenizer
char const* const SDL_CSTR_BASICTOKENIZER = "BasicTokenizer";
char const* const SDL_CSTR_DELIMITER = "Delimiter";
char const* const SDL_CSTR_FIXUTF8 = "FixUTF8";

// FSTokenizer
char const* const SDL_CSTR_FSTOKENIZER = "FSTokenizer";
char const* const SDL_CSTR_TRAINEDFST = "TrainedFST";
char const* const SDL_CSTR_CHARVOCABULARY = "CharVocabulary";
char const* const SDL_CSTR_TOKENVOCABULARY = "TokenVocabulary";


}

#endif
