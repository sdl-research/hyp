




namespace Hypergraph {

void Token::append(Token const& other) {



  }

    setExtendableRight(other.isExtendableRight());
    setMustExtendRight(other.mustExtendRight());
  }

    setEndState(other.getEndState());
}

void Token::print(std::ostream& out, IVocabularyPtr pVoc) const {

    out << "?";
  else

  out << "-";
  if (getEndState() == kNoState)
    out << "?";
  else
    out << getEndState();
  out << ",";



    if (pVoc) {

    }
    else {
      out << symId;
    }
  }
  if (!this->empty()) {
    out << ",";
  }

}



