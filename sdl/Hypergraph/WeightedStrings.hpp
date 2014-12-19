























template<class W>
struct WeightedStrings {










    strings.push_back(s);
    weights.push_back(w);
  }
  void openString(W w = W::one()) {






  }
  bool operator()(std::string const& s) {

      addChar(charId(s));
    return true;
  }
  void addChar(std::string const& s) {








    }
    strings.back().push_back(s);
  }
  std::size_t addedLength() const
  {
    return strings.back().size();
  }






    if (unigramk) {





          counts[i] += unigramk;
          unigrams.erase(s);
        }
      }
      double k = unigramk;
      unigramk = 0; // prevent adding more counts as we add unseen as words
      for (typename Unigrams::const_iterator i = unigrams.begin(), e = unigrams.end(); i!=e; ++i) {



        assert(counts.size()==strings.size());
        assert(counts.size()==weights.size());
      }
    }
  }
  std::size_t size() const {
    assert(weights.size()==strings.size());
    return strings.size();
  }
  double unigramk;




  }

    assert(i<size());
    o << weights[i] << " ";
    print(o, strings[i]);
  }


  }


  }
  inline friend std::ostream &operator<<(std::ostream &o, WeightedStrings<W> const& ws) {
    ws.print(o);
    return o;
  }
};





