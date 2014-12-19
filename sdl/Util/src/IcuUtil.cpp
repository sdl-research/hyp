#include <iostream>








#include <unicode/unistr.h>
#include <unicode/schriter.h>
#include <unicode/brkiter.h>
#include <unicode/uchar.h>
#include <unicode/chariter.h>



namespace Util {






















String16 toString16(std::string const& utf8Str) {
  String16 ws;

  return ws;
}

String32 toString32(std::string const& utf8Str) {

}

















bool isUtf8Whitespace(std::string const& utf8Str) {
  icu::CharacterIterator* charIter =
      new icu::StringCharacterIterator(
          icu::UnicodeString::fromUTF8(utf8Str.c_str()));
  bool result = true;
  for (; charIter->hasNext(); charIter->next32()) {
    if (!u_isspace(charIter->current32())) {
      result = false;
      break;


  delete charIter;
  return result;




